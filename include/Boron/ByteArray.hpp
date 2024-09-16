#ifndef BORON_INCLUDE_BORON_BYTEARRAY_HPP_
#define BORON_INCLUDE_BORON_BYTEARRAY_HPP_

#include "Boron/ArrayData.hpp"
#include "Boron/Common.hpp"
#include "Boron/Global.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace Boron {

class String;
class ByteArray;
class ByteArrayView;

template <typename T>
concept VectorOfByteLike = requires(T t) {
  typename T::value_type;
  requires ByteLike<typename T::value_type>;
  requires std::same_as<T, std::vector<typename T::value_type>>;
};

template <typename T>
concept ByteContainer = std::is_same_v<T, ByteArray> ||
                        std::is_same_v<T, ByteArrayView> || VectorOfByteLike<T>;

class BORON_EXPORT ByteArrayView {
public:
  using storage_type = byte;
  using value_type = const storage_type;
  using difference_type = std::ptrdiff_t;
  using size_type = size_t;
  using reference = storage_type &;
  using const_reference = const storage_type &;
  using pointer = storage_type *;
  using const_pointer = const storage_type *;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_interator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  template <ByteLike Byte> static const_pointer castHelper(const Byte *data) {
    return reinterpret_cast<const_pointer>(data);
  }
  static const_pointer castHelper(const storage_type *data) { return data; }

  template <ByteLike Byte>
  static constexpr size_type arrayLengthHelper(const Byte *data,
                                               size_type size) {
    const auto it = std::find(data, data + size, 0);
    const auto end = it != data + size ? it : data + size;
    return end - data;
  }

  constexpr void verify(size_type pos = 0, size_type n = 1) const {
    assert(0 <= pos && pos <= size_);
    assert(0 <= n && n <= size_ - pos);
  }

public:
  constexpr ByteArrayView() : size_(0), data_(nullptr) {};
  template <ByteLike Byte>
  constexpr ByteArrayView(const Byte *data, size_t size)
      : size_(size), data_(castHelper(data)) {}
  // range [begin, end)
  template <ByteLike Byte>
  constexpr ByteArrayView(const Byte *begin, const Byte *end)
      : size_(end - begin), data_(castHelper(begin)) {}
  template <ByteLike Byte>
  constexpr ByteArrayView(const Byte *data)
      : size_(ByteTraits<Byte>::length(data)), data_(castHelper(data)) {}
  template <ByteContainer Container>
  constexpr ByteArrayView(const Container &container)
      : size_(container.size()), data_(container.data()) {}
  // TODO: check how Qt implements this
  // template <ByteLike Byte, size_t Size>
  // constexpr ByteArrayView(const Byte (&data)[Size])
  // : size_(arrayLengthHelper(data, Size)), data_(castHelper(data)) {}

  template <ByteLike Byte, size_t Size>
  BORON_NODISCARD constexpr static ByteArrayView
  fromArray(const Byte (&data)[Size]) {
    return ByteArrayView(data, arrayLengthHelper(data, Size));
  }
  BORON_NODISCARD static ByteArray
  toByteArray(const ByteArrayView &view); // TODO: Implement

  ByteArrayView(const ByteArrayView &other) = default;
  ByteArrayView(ByteArrayView &&other) noexcept = default;
  ~ByteArrayView() = default;

  ByteArrayView &operator=(const ByteArrayView &other) = default;
  ByteArrayView &operator=(ByteArrayView &&other) noexcept = default;

  BORON_NODISCARD constexpr size_t size() const { return size_; }
  BORON_NODISCARD constexpr const uint8_t *data() const { return data_; }

  BORON_NODISCARD constexpr bool empty() const { return size_ == 0; }

  BORON_NODISCARD constexpr const_reference operator[](size_t index) const {
    return data_[index];
  }

  BORON_NODISCARD constexpr const_reference at(size_t index) const {
    if (index >= size_) {
      throw std::out_of_range("Index out of range");
    }
    return data_[index];
  }

  BORON_NODISCARD constexpr const_reference front() const { return data_[0]; }
  BORON_NODISCARD constexpr const_reference back() const {
    return data_[size_ - 1];
  }

  BORON_NODISCARD constexpr const_pointer begin() const { return data_; }
  BORON_NODISCARD constexpr const_pointer end() const { return data_ + size_; }

  BORON_NODISCARD constexpr const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  BORON_NODISCARD constexpr const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  BORON_NODISCARD constexpr ByteArrayView sliced(size_t pos, size_t n) const {
    verify(pos, n);
    return ByteArrayView(data_ + pos, n);
  }

  BORON_NODISCARD String toString() const;

private:
  size_type size_;
  const storage_type *data_;
};

class BORON_EXPORT ByteArray {
public:
  ByteArray();
  ByteArray(const uint8_t *data, size_t size);
  ByteArray(const ByteArray &other);
  ByteArray(ByteArray &&other) noexcept;
  ~ByteArray();

  ByteArray &operator=(const ByteArray &other);
  ByteArray &operator=(ByteArray &&other) noexcept;

  BORON_NODISCARD size_t size() const;
  BORON_NODISCARD const uint8_t *data() const;

  BORON_NODISCARD String toString() const;
};
} // namespace Boron

#endif