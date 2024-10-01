#ifndef BORON_INCLUDE_BORON_BYTEARRAY_HPP_
#define BORON_INCLUDE_BORON_BYTEARRAY_HPP_

#include "Boron/ArrayData.hpp"
#include "Boron/Common.hpp"
#include "Boron/Global.hpp"

#include <algorithm>
#include <cassert>
#include <compare>
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
  // BORON_NODISCARD static ByteArray
  // toByteArray(const ByteArrayView &view); // TODO: Implement
  [[nodiscard]] ByteArray toByteArray() const;

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

  BORON_NODISCARD constexpr bool isNull() const { return data_ == nullptr; }

  BORON_NODISCARD friend inline constexpr auto
  operator<=>(const ByteArrayView &lhs, const ByteArrayView &rhs) {
    if (!lhs.isNull() && !rhs.isNull()) {
      int ret =
          memcmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));
      if (ret != 0)
        return ret <=> 0;
    }

    return lhs.size() == rhs.size() ? std::strong_ordering::equal
                                    : lhs.size() <=> rhs.size();
  }
  friend inline constexpr bool operator==(const ByteArrayView &lhs,
                                          const ByteArrayView &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::equal;
  }

  friend inline constexpr bool operator!=(const ByteArrayView &lhs,
                                          const ByteArrayView &rhs) {
    return (lhs <=> rhs) != std::strong_ordering::equal;
  }

  friend inline constexpr bool operator<(const ByteArrayView &lhs,
                                         const ByteArrayView &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::less;
  }

  friend inline constexpr bool operator<=(const ByteArrayView &lhs,
                                          const ByteArrayView &rhs) {
    return (lhs <=> rhs) != std::strong_ordering::greater;
  }

  friend inline constexpr bool operator>(const ByteArrayView &lhs,
                                         const ByteArrayView &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::greater;
  }

  friend inline constexpr bool operator>=(const ByteArrayView &lhs,
                                          const ByteArrayView &rhs) {
    return (lhs <=> rhs) != std::strong_ordering::less;
  }

private:
  size_type size_;
  const storage_type *data_;
};

using ByteArrayData = ArrayDataPointer<uint8_t>;

class BORON_EXPORT ByteArray {
  // public:
  // using DataPointer = ByteArrayData;

private:
  using Container = std::vector<byte>;
  Container data_;
  // typedef TypedArrayData<uint8_t> Data;

  // DataPointer d;
  static const uint8_t kEmpty = 0;

public:
  // enum Base64Option {
  //   Base64Encoding = 0,
  //   Base64UrlEncoding = 1,

  //   KeepTrailingEquals = 0,
  //   OmitTrailingEquals = 2,

  //   IgnoreBase64DecodingErrors = 0,
  //   AbortOnBase64DecodingErrors = 4,
  // };
  // using Base64Options = uint8_t;

  // enum class Base64DecodingStatus {
  //   Ok,
  //   IllegalInputLength,
  //   IllegalCharacter,
  //   IllegalPadding,
  // };

  inline ByteArray() noexcept;
  ByteArray(const uint8_t *, size_t size = -1);
  ByteArray(size_t size, uint8_t c);
  inline ByteArray(const ByteArray &) noexcept = default;
  inline ~ByteArray();

  ByteArray &operator=(const ByteArray &) noexcept;
  // TODO: implement operator= for uint8_t *
  ByteArray &operator=(const uint8_t *str);
  inline ByteArray(ByteArray &&other) noexcept = default;
  // TODO: check why Qt use pure swap
  ByteArray &operator=(ByteArray &&other) noexcept = default;
  inline void swap(ByteArray &other) noexcept {
    std::swap(this->data_, other.data_);
  }

  bool isEmpty() const noexcept { return size() == 0; }
  void resize(size_t size);
  void resize(size_t size, uint8_t c);

  ByteArray &fill(uint8_t c, size_t size = -1);

  inline size_t capacity() const { return this->data_.capacity(); }
  inline void reserve(size_t size) { return this->data_.reserve(size); }
  inline void squeeze() { this->data_.shrink_to_fit(); }

  inline uint8_t *data();
  inline const uint8_t *data() const noexcept;
  const uint8_t *constData() const noexcept { return data(); }
  // TODO: implement Detail::detach
  // inline void detach();
  // inline bool isDetached() const;
  // inline bool isSharedWith(const ByteArray &other) const noexcept {
  //   return data() == other.data() && size() == other.size();
  // }
  inline void clear() { this->data_.clear(); }

  inline uint8_t at(size_t i) const;
  inline uint8_t operator[](size_t i) const;
  [[nodiscard]] inline uint8_t &operator[](size_t i);
  [[nodiscard]] uint8_t front() const { return at(0); }
  [[nodiscard]] inline uint8_t &front();
  [[nodiscard]] uint8_t back() const { return at(size() - 1); }
  [[nodiscard]] inline uint8_t &back();

  size_t indexOf(uint8_t c, size_t from = 0) const;
  size_t indexOf(ByteArrayView bv, size_t from = 0) const;

  // TODO: implement Detail::findLastByte and Detail::findLastByteArray
  size_t lastIndexOf(uint8_t c, size_t from = -1) const;
  size_t lastIndexOf(ByteArrayView bv) const;
  size_t lastIndexOf(ByteArrayView bv, size_t from) const;

  inline bool contains(uint8_t c) const;
  inline bool contains(ByteArrayView bv) const;

  size_t count(uint8_t c) const;
  size_t count(ByteArrayView bv) const;

  // TODO: implement compare
  inline int compare(ByteArrayView a) const noexcept;

  [[nodiscard]] ByteArray left(size_t n) const & {
    if (n >= size())
      return *this;
    return first(std::max(n, 0ull));
  }
  [[nodiscard]] ByteArray left(size_t n) && {
    if (n >= size())
      return std::move(*this);
    return std::move(*this).first(std::max(n, 0ull));
  }
  [[nodiscard]] ByteArray right(size_t n) const & {
    if (n >= size())
      return *this;
    return last(std::max(n, 0ull));
  }
  [[nodiscard]] ByteArray right(size_t n) && {
    if (n >= size())
      return std::move(*this);
    return std::move(*this).last(std::max(n, 0ull));
  }
  // TODO: mid
  [[nodiscard]] ByteArray mid(size_t index, size_t len = -1) const &;
  [[nodiscard]] ByteArray mid(size_t index, size_t len = -1) &&;

  [[nodiscard]] ByteArray first(size_t n) const & {
    verify(0, n);
    return sliced(0, n);
  }
  [[nodiscard]] ByteArray last(size_t n) const & {
    verify(0, n);
    return sliced(size() - n, n);
  }
  [[nodiscard]] ByteArray sliced(size_t pos) const & {
    verify(pos, 0);
    return sliced(pos, size() - pos);
  }
  [[nodiscard]] ByteArray sliced(size_t pos, size_t n) const & {
    verify(pos, n);
    return ByteArray(data_.data() + pos, n);
  }
  [[nodiscard]] ByteArray chopped(size_t len) const & {
    verify(0, len);
    return sliced(0, size() - len);
  }

  [[nodiscard]] ByteArray first(size_t n) && {
    verify(0, n);
    resize(n); // may detach and allocate memory
    return std::move(*this);
  }
  [[nodiscard]] ByteArray last(size_t n) && {
    verify(0, n);
    return sliced_helper(*this, size() - n, n);
  }
  [[nodiscard]] ByteArray sliced(size_t pos) && {
    verify(pos, 0);
    return sliced_helper(*this, pos, size() - pos);
  }
  [[nodiscard]] ByteArray sliced(size_t pos, size_t n) && {
    verify(pos, n);
    return sliced_helper(*this, pos, n);
  }
  [[nodiscard]] ByteArray chopped(size_t len) && {
    verify(0, len);
    return std::move(*this).first(size() - len);
  }

  // TODO: implement Detail::startsWith and Detail::ends With
  bool startsWith(ByteArrayView bv) const;
  bool startsWith(uint8_t c) const { return size() > 0 && front() == c; }

  bool endsWith(uint8_t c) const { return size() > 0 && back() == c; }
  bool endsWith(ByteArrayView bv) const;

  // TODO: implement isUpper and isLower
  bool isUpper() const;
  bool isLower() const;

  // [[nodiscard]] bool isValidUtf8() const noexcept {
  //   return QtPrivate::isValidUtf8(qToByteArrayViewIgnoringNull(*this));
  // }

  // TODO: implement truncate and chop
  void truncate(size_t pos);
  void chop(size_t n);

  [[nodiscard]] ByteArray toLower() const & { return toLower_helper(*this); }
  [[nodiscard]] ByteArray toLower() && { return toLower_helper(*this); }
  [[nodiscard]] ByteArray toUpper() const & { return toUpper_helper(*this); }
  [[nodiscard]] ByteArray toUpper() && { return toUpper_helper(*this); }
  [[nodiscard]] ByteArray trimmed() const & { return trimmed_helper(*this); }
  [[nodiscard]] ByteArray trimmed() && { return trimmed_helper(*this); }
  // [[nodiscard]] ByteArray simplified() const & {
  //   return simplified_helper(*this);
  // }
  // [[nodiscard]] ByteArray simplified() && { return simplified_helper(*this);
  // }

  [[nodiscard]] ByteArray leftJustified(size_t width, uint8_t fill = ' ',
                                        bool truncate = false) const;
  [[nodiscard]] ByteArray rightJustified(size_t width, uint8_t fill = ' ',
                                         bool truncate = false) const;

  ByteArray &prepend(uint8_t c) { return insert(0, ByteArrayView(&c, 1)); }
  inline ByteArray &prepend(size_t count, uint8_t c);
  // TODO: WARNING: This function is not safe to use with null-terminated
  ByteArray &prepend(const uint8_t *s) {
    return insert(
        0, ByteArrayView(s, size_t(strlen(reinterpret_cast<const char *>(s)))));
  }
  ByteArray &prepend(const uint8_t *s, size_t len) {
    return insert(0, ByteArrayView(s, len));
  }
  ByteArray &prepend(const ByteArray &a);
  ByteArray &prepend(ByteArrayView a) { return insert(0, a); }

  ByteArray &append(uint8_t c);
  inline ByteArray &append(size_t count, uint8_t c);
  ByteArray &append(const uint8_t *s) { return append(s, -1); }
  ByteArray &append(const uint8_t *s, size_t len) {
    return append(ByteArrayView(
        s, len < 0 ? size_t(strlen(reinterpret_cast<const char *>(s))) : len));
  }
  ByteArray &append(const ByteArray &a);
  ByteArray &append(ByteArrayView a) { return insert(size(), a); }

  ByteArray &assign(ByteArrayView v);
  ByteArray &assign(size_t n, uint8_t c) {
    assert(n >= 0);
    return fill(c, n);
  }
  // TODO: implement assign method of ArrayDataPointer
  // template <InputIterator Iter> ByteArray &assign(Iter first, Iter last) {
  //   d.assign(first, last);
  //   if (d.data())
  //     d.data()[d.size] = '\0';
  //   return *this;
  // }

  ByteArray &insert(size_t i, ByteArrayView data);
  inline ByteArray &insert(size_t i, const uint8_t *s) {
    return insert(i, ByteArrayView(s));
  }
  inline ByteArray &insert(size_t i, const ByteArray &data) {
    return insert(i, ByteArrayView(data));
  }
  ByteArray &insert(size_t i, size_t count, uint8_t c);
  ByteArray &insert(size_t i, uint8_t c) {
    return insert(i, ByteArrayView(&c, 1));
  }
  ByteArray &insert(size_t i, const uint8_t *s, size_t len) {
    return insert(i, ByteArrayView(s, len));
  }

  ByteArray &remove(size_t index, size_t len);
  ByteArray &removeAt(size_t pos) {
    return size_t(pos) < size_t(size()) ? remove(pos, 1) : *this;
  }
  ByteArray &removeFirst() { return !isEmpty() ? remove(0, 1) : *this; }
  ByteArray &removeLast() { return !isEmpty() ? remove(size() - 1, 1) : *this; }

  template <typename Predicate> ByteArray &removeIf(Predicate pred) {
    removeIf_helper(pred);
    return *this;
  }

  ByteArray &replace(size_t index, size_t len, const uint8_t *s, size_t alen) {
    return replace(index, len, ByteArrayView(s, alen));
  }
  ByteArray &replace(size_t index, size_t len, ByteArrayView s);
  ByteArray &replace(uint8_t before, ByteArrayView after) {
    return replace(ByteArrayView(&before, 1), after);
  }
  ByteArray &replace(const uint8_t *before, size_t bsize, const uint8_t *after,
                     size_t asize) {
    return replace(ByteArrayView(before, bsize), ByteArrayView(after, asize));
  }
  ByteArray &replace(ByteArrayView before, ByteArrayView after);
  ByteArray &replace(uint8_t before, uint8_t after);

  ByteArray &operator+=(uint8_t c) { return append(c); }
  ByteArray &operator+=(const uint8_t *s) { return append(s); }
  ByteArray &operator+=(const ByteArray &a) { return append(a); }
  ByteArray &operator+=(ByteArrayView a) { return append(a); }

  std::vector<ByteArray> split(uint8_t sep) const;

  [[nodiscard]] ByteArray repeated(size_t times) const;

  // TODO: comparison between ByteArray and const uint8_t *
  [[nodiscard]] friend inline constexpr auto operator<=>(const ByteArray &lhs,
                                                         const ByteArray &rhs) {
    return ByteArrayView(lhs) <=> ByteArrayView(rhs);
  }

  // TODO: check why clang cannot find operator== and operator!= based on
  // operator<=> definition
  [[nodiscard]] friend inline constexpr bool operator==(const ByteArray &lhs,
                                                        const ByteArray &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::equal;
  }

  [[nodiscard]] friend inline constexpr bool operator!=(const ByteArray &lhs,
                                                        const ByteArray &rhs) {
    return (lhs <=> rhs) != std::strong_ordering::equal;
  }

  [[nodiscard]] friend inline constexpr bool operator<(const ByteArray &lhs,
                                                       const ByteArray &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::less;
  }

  [[nodiscard]] friend inline constexpr bool operator<=(const ByteArray &lhs,
                                                        const ByteArray &rhs) {
    return (lhs <=> rhs) != std::strong_ordering::greater;
  }

  [[nodiscard]] friend inline constexpr bool operator>(const ByteArray &lhs,
                                                       const ByteArray &rhs) {
    return (lhs <=> rhs) == std::strong_ordering::greater;
  }

  [[nodiscard]] friend inline constexpr bool operator>=(const ByteArray &lhs,
                                                        const ByteArray &rhs) {
    return (lhs <=> rhs) != std::strong_ordering::less;
  }

  // Check isEmpty() instead of isNull() for backwards compatibility.
  friend inline bool operator==(const ByteArray &a1, std::nullptr_t) noexcept {
    return a1.isEmpty();
  }
  friend inline bool operator!=(const ByteArray &a1, std::nullptr_t) noexcept {
    return !a1.isEmpty();
  }
  friend inline bool operator<(const ByteArray &, std::nullptr_t) noexcept {
    return false;
  }
  friend inline bool operator>(const ByteArray &a1, std::nullptr_t) noexcept {
    return !a1.isEmpty();
  }
  friend inline bool operator<=(const ByteArray &a1, std::nullptr_t) noexcept {
    return a1.isEmpty();
  }
  friend inline bool operator>=(const ByteArray &, std::nullptr_t) noexcept {
    return true;
  }

  friend inline bool operator==(std::nullptr_t, const ByteArray &a2) noexcept {
    return a2 == nullptr;
  }
  friend inline bool operator!=(std::nullptr_t, const ByteArray &a2) noexcept {
    return a2 != nullptr;
  }
  friend inline bool operator<(std::nullptr_t, const ByteArray &a2) noexcept {
    return a2 > nullptr;
  }
  friend inline bool operator>(std::nullptr_t, const ByteArray &a2) noexcept {
    return a2 < nullptr;
  }
  friend inline bool operator<=(std::nullptr_t, const ByteArray &a2) noexcept {
    return a2 >= nullptr;
  }
  friend inline bool operator>=(std::nullptr_t, const ByteArray &a2) noexcept {
    return a2 <= nullptr;
  }

  short toShort(bool *ok = nullptr, int base = 10) const;
  unsigned short toUShort(bool *ok = nullptr, int base = 10) const;
  int toInt(bool *ok = nullptr, int base = 10) const;
  unsigned int toUInt(bool *ok = nullptr, int base = 10) const;
  long toLong(bool *ok = nullptr, int base = 10) const;
  unsigned long toULong(bool *ok = nullptr, int base = 10) const;
  long long toLongLong(bool *ok = nullptr, int base = 10) const;
  unsigned long long toULongLong(bool *ok = nullptr, int base = 10) const;
  float toFloat(bool *ok = nullptr) const;
  double toDouble(bool *ok = nullptr) const;
  // ByteArray toBase64(Base64Options options = Base64Encoding) const;
  ByteArray toHex(uint8_t separator = '\0') const;
  ByteArray toPercentEncoding(const ByteArray &exclude = ByteArray(),
                              const ByteArray &include = ByteArray(),
                              uint8_t percent = '%') const;
  [[nodiscard]] ByteArray percentDecoded(uint8_t percent = '%') const;

  inline ByteArray &setNum(short, int base = 10);
  inline ByteArray &setNum(unsigned short, int base = 10);
  inline ByteArray &setNum(int, int base = 10);
  inline ByteArray &setNum(unsigned int, int base = 10);
  inline ByteArray &setNum(long, int base = 10);
  inline ByteArray &setNum(unsigned long, int base = 10);
  ByteArray &setNum(long long, int base = 10);
  ByteArray &setNum(unsigned long long, int base = 10);
  inline ByteArray &setNum(float, uint8_t format = 'g', int precision = 6);
  ByteArray &setNum(double, uint8_t format = 'g', int precision = 6);
  ByteArray &setRawData(const uint8_t *a, size_t n);

  [[nodiscard]] static ByteArray number(int, int base = 10);
  [[nodiscard]] static ByteArray number(unsigned int, int base = 10);
  [[nodiscard]] static ByteArray number(long, int base = 10);
  [[nodiscard]] static ByteArray number(unsigned long, int base = 10);
  [[nodiscard]] static ByteArray number(long long, int base = 10);
  [[nodiscard]] static ByteArray number(unsigned long long, int base = 10);
  [[nodiscard]] static ByteArray number(double, uint8_t format = 'g',
                                        int precision = 6);
  [[nodiscard]] static ByteArray fromRawData(const uint8_t *data, size_t size) {
    return ByteArray(const_cast<uint8_t *>(data), size);
    // return ByteArray(DataPointer(nullptr, const_cast<uint8_t *>(data),
    // size));
  }

  // TODO: redesign Base64
  // class FromBase64Result;
  // [[nodiscard]] static FromBase64Result
  // fromBase64Encoding(ByteArray &&base64,
  //                    Base64Options options = Base64Encoding);
  // [[nodiscard]] static FromBase64Result
  // fromBase64Encoding(const ByteArray &base64,
  //                    Base64Options options = Base64Encoding);
  // [[nodiscard]] static ByteArray
  // fromBase64(const ByteArray &base64, Base64Options options =
  // Base64Encoding);
  [[nodiscard]] static ByteArray fromHex(const ByteArray &hexEncoded);
  // TODO: implement fromPercentEncoding
  // [[nodiscard]] static ByteArray
  // fromPercentEncoding(const ByteArray &pctEncoded, uint8_t percent = '%');

  // TODO: typedef iterator
  typedef uint8_t *iterator;
  typedef const uint8_t *const_iterator;
  typedef iterator Iterator;
  typedef const_iterator ConstIterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  iterator begin() { return data(); }
  const_iterator begin() const noexcept { return data(); }
  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator constBegin() const noexcept { return begin(); }
  iterator end() { return begin() + size(); }
  const_iterator end() const noexcept { return begin() + size(); }
  const_iterator cend() const noexcept { return end(); }
  const_iterator constEnd() const noexcept { return end(); }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }

  // stl compatibility
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef const uint8_t &const_reference;
  typedef uint8_t &reference;
  typedef uint8_t *pointer;
  typedef const uint8_t *const_pointer;
  typedef uint8_t value_type;
  void push_back(uint8_t c) { append(c); }
  void push_back(const uint8_t *s) { append(s); }
  void push_back(const ByteArray &a) { append(a); }
  void push_back(ByteArrayView a) { append(a); }
  void push_front(uint8_t c) { prepend(c); }
  void push_front(const uint8_t *c) { prepend(c); }
  void push_front(const ByteArray &a) { prepend(a); }
  void push_front(ByteArrayView a) { prepend(a); }
  void shrink_to_fit() { squeeze(); }
  iterator erase(const_iterator first, const_iterator last);
  inline iterator erase(const_iterator it) { return erase(it, it + 1); }

  static ByteArray fromStdString(const std::string &s);
  std::string toStdString() const;

  inline size_t size() const noexcept { return data_.size(); }
  inline size_t length() const noexcept { return size(); }
  inline bool isNull() const noexcept;

  // inline const DataPointer &data_ptr() const { return d; }
  // inline DataPointer &data_ptr() { return d; }
  // explicit inline ByteArray(DataPointer &&dd) : d(std::move(dd)) {}

private:
  void reallocData(size_t alloc, ArrayData::AllocationOption option);
  void reallocGrowData(size_t n);
  void expand(size_t i);

  inline constexpr void verify([[maybe_unused]] size_t pos = 0,
                               [[maybe_unused]] size_t n = 1) const {
    assert(pos >= 0);
    assert(pos <= data_.size());
    assert(n >= 0);
    assert(n <= data_.size() - pos);
  }

  static ByteArray sliced_helper(ByteArray &a, size_t pos, size_t n);
  static ByteArray toLower_helper(const ByteArray &a);
  static ByteArray toLower_helper(ByteArray &a);
  static ByteArray toUpper_helper(const ByteArray &a);
  static ByteArray toUpper_helper(ByteArray &a);
  static ByteArray trimmed_helper(const ByteArray &a);
  static ByteArray trimmed_helper(ByteArray &a);
  // static ByteArray simplified_helper(const ByteArray &a);
  // static ByteArray simplified_helper(ByteArray &a);
  // template <typename Predicate> size_t removeIf_helper(Predicate pred) {
  //   const size_t result = d->eraseIf(pred);
  //   if (result > 0)
  //     d.data()[d.size()] = '\0';
  //   return result;
  // }

  friend class String;

  // template <typename T> friend size_t erase(ByteArray &ba, const T &t);
  // template <typename Predicate>
  // friend size_t erase_if(ByteArray &ba, Predicate pred);
};

// Q_DECLARE_OPERATORS_FOR_FLAGS(ByteArray::Base64Options)

// TODO: constexpr ByteArray::ByteArray() noexcept {}
inline ByteArray::ByteArray() noexcept {}
inline ByteArray::~ByteArray() {}

inline uint8_t ByteArray::at(size_t i) const {
  verify(i, 1);
  return data_.data()[i];
}
inline uint8_t ByteArray::operator[](size_t i) const {
  verify(i, 1);
  return data_.data()[i];
}

inline uint8_t *ByteArray::data() {
  // detach();
  assert(data_.data());
  return data_.data();
}
inline const uint8_t *ByteArray::data() const noexcept { return data_.data(); }
// inline void ByteArray::detach() {
//   if (d->needsDetach())
//     reallocData(size(), QArrayData::KeepSize);
// }
// inline bool ByteArray::isDetached() const { return !d->isShared(); }
// inline ByteArray::ByteArray(const ByteArray &a) noexcept : d(a.d) {}

// inline size_t ByteArray::capacity() const {
//   return size_t(d->constAllocatedCapacity());
// }

// inline void ByteArray::reserve(size_t asize) {
//   if (d->needsDetach() || asize > capacity() - d->freeSpaceAtBegin())
//     reallocData(std::max(size(), asize), QArrayData::KeepSize);
//   if (d->constAllocatedCapacity())
//     d->setFlag(Data::CapacityReserved);
// }

inline uint8_t &ByteArray::operator[](size_t i) {
  verify(i, 1);
  return data()[i];
}
inline uint8_t &ByteArray::front() { return operator[](0); }
inline uint8_t &ByteArray::back() { return operator[](size() - 1); }
inline ByteArray &ByteArray::append(size_t n, uint8_t ch) {
  return insert(size(), n, ch);
}
inline ByteArray &ByteArray::prepend(size_t n, uint8_t ch) {
  return insert(0, n, ch);
}
inline bool ByteArray::contains(uint8_t c) const {
  return indexOf(c) != static_cast<size_t>(-1);
}
inline bool ByteArray::contains(ByteArrayView bv) const {
  return indexOf(bv) != static_cast<size_t>(-1);
}
inline int ByteArray::compare(ByteArrayView a) const noexcept {
  return orderToInt(ByteArrayView(*this) <=> a);
}

inline ByteArray &ByteArray::setNum(short n, int base) {
  return setNum((long long)(n), base);
}
inline ByteArray &ByteArray::setNum(unsigned short n, int base) {
  return setNum((unsigned long long)(n), base);
}
inline ByteArray &ByteArray::setNum(int n, int base) {
  return setNum((long long)(n), base);
}
inline ByteArray &ByteArray::setNum(unsigned int n, int base) {
  return setNum((unsigned long long)(n), base);
}
inline ByteArray &ByteArray::setNum(long n, int base) {
  return setNum((long long)(n), base);
}
inline ByteArray &ByteArray::setNum(unsigned long n, int base) {
  return setNum((unsigned long long)(n), base);
}
inline ByteArray &ByteArray::setNum(float n, uint8_t format, int precision) {
  return setNum(double(n), format, precision);
}

// TODO: check if d.isNull is equvalent to d->isNull
bool ByteArray::isNull() const noexcept { return data_.data() == nullptr; }

// TODO: qCompress

// TODO: swap
// Q_DECLARE_SHARED(ByteArray)

// class ByteArray::FromBase64Result {
// public:
//   ByteArray decoded;
//   ByteArray::Base64DecodingStatus decodingStatus;

//   void swap(ByteArray::FromBase64Result &other) noexcept {
//     decoded.swap(other.decoded);
//     std::swap(decodingStatus, other.decodingStatus);
//   }

//   explicit operator bool() const noexcept {
//     return decodingStatus == ByteArray::Base64DecodingStatus::Ok;
//   }

//   ByteArray &operator*() noexcept { return decoded; }
//   const ByteArray &operator*() const noexcept { return decoded; }

//   friend inline bool
//   operator==(const ByteArray::FromBase64Result &lhs,
//              const ByteArray::FromBase64Result &rhs) noexcept {
//     if (lhs.decodingStatus != rhs.decodingStatus)
//       return false;

//     if (lhs.decodingStatus == ByteArray::Base64DecodingStatus::Ok &&
//         lhs.decoded != rhs.decoded)
//       return false;

//     return true;
//   }

//   friend inline bool
//   operator!=(const ByteArray::FromBase64Result &lhs,
//              const ByteArray::FromBase64Result &rhs) noexcept {
//     return !(lhs == rhs);
//   }
// };

// TODO: swap
// Q_DECLARE_SHARED(ByteArray::FromBase64Result)

// Q_CORE_EXPORT Q_DECL_PURE_FUNCTION size_t
// qHash(const ByteArray::FromBase64Result &key, size_t seed = 0) noexcept;

// template <typename T> size_t erase(ByteArray &ba, const T &t) {
//   return ba.removeIf_helper([&t](const auto &e) { return t == e; });
// }

// template <typename Predicate> size_t erase_if(ByteArray &ba, Predicate pred)
// {
//   return ba.removeIf_helper(pred);
// }

//
// ByteArrayView members that require ByteArray:
//
inline ByteArray ByteArrayView::toByteArray() const {
  return ByteArray(data(), size());
}
} // namespace Boron

#endif