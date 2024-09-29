#ifndef BORON_INCLUDE_BORON_ARRAYDATA_HPP_
#define BORON_INCLUDE_BORON_ARRAYDATA_HPP_

#include "Boron/Global.hpp"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace Boron {
template <typename T> struct TypedArrayData;

struct ArrayData {
  enum AllocationOption { Grow, KeepSize };
  enum GrowthPosition { Front, Back };
  enum class ArrayOption : uint8_t {
    ArrayOptionDefault = 0x00,
    CapacityReserved = 0x01
  };
  using ArrayOptions = uint8_t;
  std::atomic_int ref_count;
  ArrayOptions options;
  size_t alloc_size;
  constexpr size_t allocatedCapacity() const noexcept { return alloc_size; }
  bool ref() noexcept {
    ref_count.fetch_add(1, std::memory_order_relaxed);
    return true;
  }
  bool deref() noexcept {
    return ref_count.fetch_sub(1, std::memory_order_acq_rel) != 1;
  }
  bool isShared() noexcept {
    return ref_count.load(std::memory_order_relaxed) > 1;
  }
  size_t detachCapacity(size_t newSize) const noexcept {
    if (options & static_cast<uint8_t>(ArrayOption::CapacityReserved) &&
        newSize < alloc_size) {
      return alloc_size;
    }
    return newSize;
  }

  // allocate functions
  BORON_MALLOCLIKE static BORON_EXPORT void *
  allocate(ArrayData **pdata, size_t objectSize, size_t alignment,
           size_t capacity,
           AllocationOption option = AllocationOption::KeepSize) noexcept;
  BORON_MALLOCLIKE static BORON_EXPORT void *
  allocate1(ArrayData **pdata, size_t capacity,
            AllocationOption option = AllocationOption::KeepSize) noexcept;
  BORON_MALLOCLIKE static BORON_EXPORT void *
  allocate2(ArrayData **pdata, size_t capacity,
            AllocationOption option = AllocationOption::KeepSize) noexcept;

  [[nodiscard]] static BORON_EXPORT std::pair<ArrayData *, void *>
  reallocateUnaligned(ArrayData *data, void *dataPointer, size_t objectSize,
                      size_t newCapacity, AllocationOption option) noexcept;
  static BORON_EXPORT void deallocate(ArrayData *data, size_t objectSize,
                                      size_t alignment) noexcept;
};

namespace Detail {
#if GCC
constexpr size_t MaxPrimitiveAlignment = 2 * sizeof(void *);
#else
constexpr size_t MaxPrimitiveAlignment = alignof(std::max_align_t);
#endif

struct alignas(MaxPrimitiveAlignment) AlignedArrayData : ArrayData {};

} // namespace Detail

// reuse qt code
template <class T> struct TypedArrayData : ArrayData {
  struct AlignmentDummy {
    Detail::AlignedArrayData header;
    T data;
  };

  [[nodiscard]] static std::pair<TypedArrayData *, T *>
  allocate(size_t capacity, AllocationOption option = ArrayData::KeepSize) {
    static_assert(sizeof(TypedArrayData) == sizeof(ArrayData));
    ArrayData *d;
    void *result;
    if constexpr (sizeof(T) == 1) {
      // necessarily, alignof(T) == 1
      result = allocate1(&d, capacity, option);
    } else if constexpr (sizeof(T) == 2) {
      // alignof(T) may be 1, but that makes no difference
      result = allocate2(&d, capacity, option);
    } else {
      result = ArrayData::allocate(&d, sizeof(T), alignof(AlignmentDummy),
                                   capacity, option);
    }
#if __has_builtin(__builtin_assume_aligned)
    // and yet we do offer results that have stricter alignment
    result = __builtin_assume_aligned(result, alignof(AlignmentDummy));
#endif
    return {static_cast<TypedArrayData *>(d), static_cast<T *>(result)};
  }

  static std::pair<TypedArrayData *, T *>
  reallocateUnaligned(TypedArrayData *data, T *dataPointer, size_t capacity,
                      AllocationOption option) {
    static_assert(sizeof(TypedArrayData) == sizeof(ArrayData));
    std::pair<ArrayData *, void *> pair = ArrayData::reallocateUnaligned(
        data, dataPointer, sizeof(T), capacity, option);
    return {static_cast<TypedArrayData *>(pair.first),
            static_cast<T *>(pair.second)};
  }

  // FIXME: MSVCRT complains about the deallocate function
  static void deallocate(ArrayData *data) noexcept {
    static_assert(sizeof(TypedArrayData) == sizeof(ArrayData));
    ArrayData::deallocate(data, sizeof(T), alignof(AlignmentDummy));
  }

  static T *dataStart(ArrayData *data, size_t alignment) noexcept {
    // Alignment is a power of two
    assert(alignment >= size_t(alignof(ArrayData)) &&
           !(alignment & (alignment - 1)));
    void *start = reinterpret_cast<void *>(
        (reinterpret_cast<size_t>(data) + sizeof(ArrayData) + alignment - 1) &
        ~(alignment - 1));
    return static_cast<T *>(start);
  }
};

template <typename T> class ArrayDataPointer {
public:
  using Data = TypedArrayData<T>;

  // Copy constructor
  ArrayDataPointer(const ArrayDataPointer &other)
      : ptr_(other.ptr_), data_(other.data_), size_(other.size_) {
    if (data_) {
      data_->ref();
    }
  }

  // Move constructor
  ArrayDataPointer(ArrayDataPointer &&other) noexcept
      : ptr_(other.ptr_), data_(other.data_), size_(other.size_) {
    other.ptr_ = nullptr;
    other.data_ = nullptr;
    other.size_ = 0;
  }

  // Copy assignment operator
  ArrayDataPointer &operator=(const ArrayDataPointer &other) {
    if (this != &other) {
      if (data_ && !data_->deref()) {
        TypedArrayData<T>::deallocate(data_);
      }
      ptr_ = other.ptr_;
      data_ = other.data_;
      size_ = other.size_;
      if (data_) {
        data_->ref();
      }
    }
    return *this;
  }

  // Move assignment operator
  ArrayDataPointer &operator=(ArrayDataPointer &&other) noexcept {
    if (this != &other) {
      if (data_ && !data_->deref()) {
        TypedArrayData<T>::deallocate(data_);
      }
      ptr_ = other.ptr_;
      data_ = other.data_;
      size_ = other.size_;
      other.ptr_ = nullptr;
      other.data_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }
  ArrayDataPointer() : ptr_(nullptr), data_(nullptr), size_(0) {}

  ArrayDataPointer(T *ptr, ArrayData *d, size_t size)
      : ptr_(ptr), data_(d), size_(size) {}

  explicit ArrayDataPointer(std::pair<TypedArrayData<T> *, T *> adata,
                            size_t n = 0) noexcept
      : ptr_(adata.second), data_(adata.first), size_(n) {}

  explicit ArrayDataPointer(
      size_t alloc, size_t n = 0,
      ArrayData::AllocationOption op = ArrayData::KeepSize)
      : ArrayDataPointer(Data::allocate(alloc, op), n) {}

  ~ArrayDataPointer() {
    if (data_ && !data_->deref()) {
      // FIXME: MEMORY LEAK!!!!!
      // TypedArrayData<T>::deallocate(data_);
    }
  }
  auto data() const { return ptr_; }
  auto size() const { return size_; }

  static ArrayDataPointer fromRawData(const T *raw_data,
                                      size_t length) noexcept {
    assert(raw_data || !length);
    return {const_cast<T *>(raw_data), nullptr, length};
  }

private:
  T *ptr_;
  ArrayData *data_;
  size_t size_;
};
} // namespace Boron

#endif