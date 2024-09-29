#include "Boron/ArrayData.hpp"
#include "Boron/Numeric.hpp"

#include <atomic>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <malloc.h>

namespace Boron {

namespace {
struct AllocationResult {
  void *data;
  ArrayData *header;
};
using Detail::AlignedArrayData;

// {elem_count, alloc_size}
using CalcBlockSizeRes = std::pair<size_t, size_t>;

size_t calcBlockSizeImpl(size_t elem_count, size_t elem_size,
                         size_t header_size) noexcept {
  assert(elem_size > 0);
  size_t bytes;
  if (mulOverflow(elem_size, elem_count, &bytes) ||
      addOverflow(bytes, header_size, &bytes)) {
    return -1;
  }
  if (bytes < 0)
    return -1;
  return bytes;
}

CalcBlockSizeRes calcGrowBlockSize(size_t elem_count, size_t elem_size,
                                   size_t header_size) noexcept {
  CalcBlockSizeRes res = {-1, -1};
  size_t bytes = calcBlockSizeImpl(elem_count, elem_size, header_size);
  if (bytes < 0)
    return res;
  auto morebytes =
      static_cast<size_t>(std::bit_ceil(static_cast<uint64_t>(bytes) + 1));
  if (morebytes < 0) {
    // TODO: check if this branch is necessary
    bytes += (morebytes - bytes) / 2;
  } else {
    bytes = morebytes;
  }
  res.first = (bytes - header_size) / elem_size;
  res.second = res.first * elem_size + header_size;
  return res;
}

CalcBlockSizeRes calculateBlockSize(size_t capacity, size_t obj_size,
                                    size_t header_size,
                                    ArrayData::AllocationOption option) {
  // constexpr qsizetype FooterSize = qMax(sizeof(QString::value_type),
  // sizeof(QByteArray::value_type));
  constexpr size_t kFooterSize = sizeof(uint8_t);
  if (obj_size <= kFooterSize)
    header_size += kFooterSize;
  if (option == ArrayData::Grow) {
    return calcGrowBlockSize(capacity, obj_size, header_size);
  } else {
    return {calcBlockSizeImpl(capacity, obj_size, header_size), capacity};
  }
}

static ArrayData *allocateData(size_t alloc_size) {
  auto header = static_cast<ArrayData *>(::malloc(alloc_size));
  if (header) {
    header->ref_count.store(1, std::memory_order_relaxed);
    header->options = 0;
    header->alloc_size = 0;
  }
  return header;
}

static inline AllocationResult
allocateHelper(size_t object_size, size_t alignment, size_t capacity,
               ArrayData::AllocationOption option) noexcept {
  if (capacity == 0)
    return {nullptr, nullptr};
  size_t header_size = sizeof(AlignedArrayData);
  const size_t kHeaderAlignment = alignof(AlignedArrayData);
  if (alignment > kHeaderAlignment) {
    header_size += alignment - kHeaderAlignment;
  }
  assert(header_size > 0);

  auto [elem_count, alloc_size] =
      calculateBlockSize(capacity, object_size, header_size, option);
  if (alloc_size < 0)
    return {nullptr, nullptr};
  auto header = allocateData(alloc_size);
  void *data = nullptr;
  if (header) {
    data = TypedArrayData<void>::dataStart(header, alignment);
    header->alloc_size = capacity;
  }
  return {data, header};
}
} // namespace

void *ArrayData::allocate(ArrayData **pdata, size_t object_size,
                          size_t alignment, size_t capacity,
                          AllocationOption option) noexcept {
  assert(pdata != nullptr);
  assert(alignment >= size_t(alignof(ArrayData)) &&
         !(alignment & (alignment - 1)));
  auto r = allocateHelper(object_size, alignment, capacity, option);
  *pdata = r.header;
  return r.data;
}

void *ArrayData::allocate1(ArrayData **pdata, size_t capacity,
                           AllocationOption option) noexcept {
  assert(pdata != nullptr);
  auto r = allocateHelper(1, alignof(AlignedArrayData), capacity, option);
  *pdata = r.header;
  return r.data;
}

void *ArrayData::allocate2(ArrayData **pdata, size_t capacity,
                           AllocationOption option) noexcept {
  assert(pdata != nullptr);
  auto r = allocateHelper(2, alignof(AlignedArrayData), capacity, option);
  *pdata = r.header;
  return r.data;
}

std::pair<ArrayData *, void *>
ArrayData::reallocateUnaligned(ArrayData *data, void *data_pointer,
                               size_t object_size, size_t new_capacity,
                               AllocationOption option) noexcept {
  assert(!data || !data->isShared());
  const size_t kHeaderSize = sizeof(AlignedArrayData);
  auto r = calculateBlockSize(new_capacity, object_size, kHeaderSize, option);
  size_t alloc_size = r.second;
  new_capacity = r.first;
  if (alloc_size < 0)
    return {nullptr, nullptr};
  const ptrdiff_t kOffset = data_pointer
                                ? reinterpret_cast<uint8_t *>(data_pointer) -
                                      reinterpret_cast<uint8_t *>(data)
                                : kHeaderSize;
  assert(0 <= kOffset && static_cast<size_t>(kOffset) <= alloc_size);
  auto header = static_cast<ArrayData *>(::realloc(data, alloc_size));
  if (header) {
    header->alloc_size = new_capacity;
    data_pointer = reinterpret_cast<uint8_t *>(header) + kOffset;
  } else {
    data_pointer = nullptr;
  }
  return {header, data_pointer};
}

void ArrayData::deallocate([[maybe_unused]] ArrayData *data,
                           [[maybe_unused]] size_t object_size,
                           size_t alignment) noexcept {
  assert(alignment >= size_t(alignof(ArrayData)) &&
         !(alignment & (alignment - 1)));
  ::free(data);
}

} // namespace Boron