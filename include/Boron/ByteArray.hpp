#ifndef BORON_INCLUDE_BORON_BYTEARRAY_HPP_
#define BORON_INCLUDE_BORON_BYTEARRAY_HPP_

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
#include <bit>

namespace Boron
{
  class String;
  class ByteArray;
  class ByteArrayView;

  template <typename T>
  concept VectorOfByteLike = requires(T t)
  {
    typename T::value_type;
    requires ByteLike<typename T::value_type>;
    requires std::same_as<T, std::vector<typename T::value_type>>;
  };

  template <typename T>
  concept ByteContainer = std::is_same_v<T, ByteArray> ||
    std::is_same_v<T, ByteArrayView> || VectorOfByteLike<T>;

  class BORON_EXPORT ByteArrayView
  {
  public:
    using storage_type = byte;
    using value_type = const storage_type;
    using difference_type = std::ptrdiff_t;
    using size_type = size_t;
    using reference = storage_type&;
    using const_reference = const storage_type&;
    using pointer = storage_type*;
    using const_pointer = const storage_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_interator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  private:
    template <ByteLike Byte>
    static const_pointer castHelper(const Byte* data)
    {
      return reinterpret_cast<const_pointer>(data);
    }

    static const_pointer castHelper(const storage_type* data) { return data; }

    template <ByteLike Byte>
    static constexpr size_type arrayLengthHelper(const Byte* data,
                                                 size_type size)
    {
      const auto it = std::find(data, data + size, 0);
      const auto end = it != data + size ? it : data + size;
      return end - data;
    }

    constexpr void verify(size_type pos = 0, size_type n = 1) const
    {
      assert(pos <= size_);
      assert(n <= size_ - pos);
    }

  public:
    constexpr ByteArrayView() : size_(0), data_(nullptr)
    {
    };

    template <ByteLike Byte>
    constexpr ByteArrayView(const Byte* data, size_t size)
      : size_(size), data_(castHelper(data))
    {
    }

    // range [begin, end)
    template <ByteLike Byte>
    constexpr ByteArrayView(const Byte* begin, const Byte* end)
      : size_(end - begin), data_(castHelper(begin))
    {
    }

    template <ByteLike Byte>
    constexpr explicit ByteArrayView(const Byte* data)
      : size_(ByteTraits<Byte>::length(data)), data_(castHelper(data))
    {
    }

    template <ByteContainer Container>
    constexpr ByteArrayView(const Container& container)
      : size_(container.size()), data_(container.data())
    {
    }

    // TODO: check how Qt implements this
    // template <ByteLike Byte, size_t Size>
    // constexpr ByteArrayView(const Byte (&data)[Size])
    // : size_(arrayLengthHelper(data, Size)), data_(castHelper(data)) {}

    template <ByteLike Byte, size_t Size>
    BORON_NODISCARD constexpr static ByteArrayView fromArray(
      const Byte (&data)[Size])
    {
      return ByteArrayView(data, arrayLengthHelper(data, Size));
    }

    // BORON_NODISCARD static ByteArray
    // toByteArray(const ByteArrayView &view); // TODO: Implement
    BORON_NODISCARD ByteArray toByteArray() const;

    ByteArrayView(const ByteArrayView& other) = default;
    ByteArrayView(ByteArrayView&& other) noexcept = default;
    ~ByteArrayView() = default;

    ByteArrayView& operator=(const ByteArrayView& other) = default;
    ByteArrayView& operator=(ByteArrayView&& other) noexcept = default;

    BORON_NODISCARD constexpr size_t size() const { return size_; }
    BORON_NODISCARD constexpr const uint8_t* data() const { return data_; }

    BORON_NODISCARD constexpr bool empty() const { return size_ == 0; }

    BORON_NODISCARD constexpr const_reference operator[](size_t index) const
    {
      return data_[index];
    }

    BORON_NODISCARD constexpr const_reference at(size_t index) const
    {
      if (index >= size_)
      {
        throw std::out_of_range("Index out of range");
      }
      return data_[index];
    }

    BORON_NODISCARD constexpr const_reference front() const { return data_[0]; }
    BORON_NODISCARD constexpr const_reference back() const
    {
      return data_[size_ - 1];
    }

    BORON_NODISCARD constexpr const_pointer begin() const { return data_; }
    BORON_NODISCARD constexpr const_pointer end() const { return data_ + size_; }

    BORON_NODISCARD constexpr const_reverse_iterator rbegin() const
    {
      return const_reverse_iterator(end());
    }

    BORON_NODISCARD constexpr const_reverse_iterator rend() const
    {
      return const_reverse_iterator(begin());
    }

    BORON_NODISCARD constexpr ByteArrayView sliced(size_t pos, size_t n) const
    {
      verify(pos, n);
      return {data_ + pos, n};
    }

    BORON_NODISCARD String toString() const;

    BORON_NODISCARD constexpr bool isNull() const { return data_ == nullptr; }

    BORON_NODISCARD friend inline constexpr auto operator<=>
    (const ByteArrayView& lhs, const ByteArrayView& rhs)
    {
      if (!lhs.isNull() && !rhs.isNull())
      {
        int ret =
          memcmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));
        if (ret != 0)
          return ret <=> 0;
      }

      return lhs.size() <=> rhs.size();
    }

    BORON_NODISCARD friend inline constexpr auto operator==(const ByteArrayView& lhs, const ByteArrayView& rhs)
    {
      if (lhs.size() != rhs.size()) return false;
      // TODO: check if memcmp is the best way to compare
      return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

  private:
    size_type size_;
    const storage_type* data_;
  };

  // TODO: add range-based api
  class BORON_EXPORT ByteArray
  {
  private:
    using Container = std::vector<byte>;
    Container data_;

    static constexpr uint8_t kEmpty = 0;

  public:
    using storage_type = byte;
    using value_type = const storage_type;
    using difference_type = std::ptrdiff_t;
    using size_type = size_t;
    using reference = storage_type&;
    using const_reference = const storage_type&;
    using pointer = storage_type*;
    using const_pointer = const storage_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr const size_t kNpos = -1;
    static constexpr const size_t kDetectLength = -1;

    inline ByteArray() noexcept;

    inline ByteArray(const_iterator begin, const_iterator end): data_(begin, end)
    {
    }

    ByteArray(const uint8_t*, size_t size = -1);
    ByteArray(size_t size, uint8_t c);
    inline ByteArray(const ByteArray&) noexcept = default;
    inline ~ByteArray();

    ByteArray& operator=(const ByteArray&) noexcept;
    // TODO: implement operator= for uint8_t *
    ByteArray& operator=(const uint8_t* str);
    // FIXME: move constructor seems to be wrong
    inline ByteArray(ByteArray&& other) noexcept = default;
    // TODO: check why Qt use pure swap
    ByteArray& operator=(ByteArray&& other) noexcept = default;

    inline void swap(ByteArray& other) noexcept
    {
      std::swap(this->data_, other.data_);
    }

    BORON_NODISCARD bool isEmpty() const noexcept { return size() == 0; }
    void resize(size_t size);
    void resize(size_t size, uint8_t c);

    ByteArray& fill(uint8_t c, size_t size = -1);

    BORON_NODISCARD inline size_t capacity() const { return this->data_.capacity(); }
    inline void reserve(size_t size) { return this->data_.reserve(size); }
    inline void squeeze() { this->data_.shrink_to_fit(); }

    inline uint8_t* data();
    BORON_NODISCARD inline const uint8_t* data() const noexcept;
    BORON_NODISCARD const uint8_t* constData() const noexcept { return data(); }
    void clear() { this->data_.clear(); }

    BORON_NODISCARD inline uint8_t at(size_t i) const;
    inline uint8_t operator[](size_t i) const;
    BORON_NODISCARD inline uint8_t& operator[](size_t i);
    BORON_NODISCARD uint8_t front() const { return at(0); }
    BORON_NODISCARD inline uint8_t& front();
    BORON_NODISCARD uint8_t back() const { return at(size() - 1); }
    BORON_NODISCARD inline uint8_t& back();

    BORON_NODISCARD size_t indexOf(uint8_t c, size_t from = 0) const;
    BORON_NODISCARD size_t indexOf(ByteArrayView bv, size_t from = 0) const;

    // TODO: implement Detail::findLastByte and Detail::findLastByteArray
    BORON_NODISCARD size_t lastIndexOf(uint8_t c, size_t from = -1) const;
    BORON_NODISCARD size_t lastIndexOf(ByteArrayView bv) const;
    BORON_NODISCARD size_t lastIndexOf(ByteArrayView bv, size_t from) const;

    BORON_NODISCARD inline bool contains(uint8_t c) const;
    BORON_NODISCARD inline bool contains(ByteArrayView bv) const;

    BORON_NODISCARD size_t count(uint8_t c) const;
    BORON_NODISCARD size_t count(ByteArrayView bv) const;

    BORON_NODISCARD inline int compare(ByteArrayView a) const noexcept;

    BORON_NODISCARD ByteArray left(size_t n) const &
    {
      if (n >= size())
        return *this;
      return first(std::max(n, 0_sz));
    }

    BORON_NODISCARD ByteArray left(size_t n) &&
    {
      if (n >= size())
        return std::move(*this);
      return std::move(*this).first(std::max(n, 0_sz));
    }

    BORON_NODISCARD ByteArray right(size_t n) const &
    {
      if (n >= size())
        return *this;
      return last(std::max(n, 0_sz));
    }

    BORON_NODISCARD ByteArray right(size_t n) &&
    {
      if (n >= size())
        return std::move(*this);
      return std::move(*this).last(std::max(n, 0_sz));
    }

    // TODO: mid
    BORON_NODISCARD ByteArray mid(size_t index, size_t len = -1) const &;
    BORON_NODISCARD ByteArray mid(size_t index, size_t len = -1) &&;

    BORON_NODISCARD ByteArray first(size_t n) const &
    {
      verify(0, n);
      return sliced(0, n);
    }

    BORON_NODISCARD ByteArray last(size_t n) const &
    {
      verify(0, n);
      return sliced(size() - n, n);
    }

    BORON_NODISCARD ByteArray sliced(size_t pos) const &
    {
      verify(pos, 0);
      return sliced(pos, size() - pos);
    }

    BORON_NODISCARD ByteArray sliced(size_t pos, size_t n) const &
    {
      verify(pos, n);
      return {data_.data() + pos, n};
    }

    BORON_NODISCARD ByteArray chopped(size_t len) const &
    {
      verify(0, len);
      return sliced(0, size() - len);
    }

    BORON_NODISCARD ByteArray first(size_t n) &&
    {
      verify(0, n);
      resize(n); // may detach and allocate memory
      return std::move(*this);
    }

    BORON_NODISCARD ByteArray last(size_t n) &&
    {
      verify(0, n);
      return sliced_helper(*this, size() - n, n);
    }

    BORON_NODISCARD ByteArray sliced(size_t pos) &&
    {
      verify(pos, 0);
      return sliced_helper(*this, pos, size() - pos);
    }

    BORON_NODISCARD ByteArray sliced(size_t pos, size_t n) &&
    {
      verify(pos, n);
      return sliced_helper(*this, pos, n);
    }

    BORON_NODISCARD ByteArray chopped(size_t len) &&
    {
      verify(0, len);
      return std::move(*this).first(size() - len);
    }

    // TODO: implement Detail::startsWith and Detail::endsWith
    BORON_NODISCARD bool startsWith(ByteArrayView bv) const;
    BORON_NODISCARD bool startsWith(uint8_t c) const { return size() > 0 && front() == c; }

    BORON_NODISCARD bool endsWith(uint8_t c) const { return size() > 0 && back() == c; }
    BORON_NODISCARD bool endsWith(ByteArrayView bv) const;

    // TODO: implement isUpper and isLower
    // bool isUpper() const;
    // bool isLower() const;

    // BORON_NODISCARD bool isValidUtf8() const noexcept {
    //   return QtPrivate::isValidUtf8(qToByteArrayViewIgnoringNull(*this));
    // }

    // TODO: implement truncate
    void truncate(size_t pos)
    {
      assert(pos <= size());
      this->data_.resize(pos);
    }

    void chop(size_t n)
    {
      assert(n <= size());
      this->data_.resize(size() - n);
    }

    // BORON_NODISCARD ByteArray toLower() const & { return toLower_helper(*this); }
    // BORON_NODISCARD ByteArray toLower() && { return toLower_helper(*this); }
    // BORON_NODISCARD ByteArray toUpper() const & { return toUpper_helper(*this); }
    // BORON_NODISCARD ByteArray toUpper() && { return toUpper_helper(*this); }
    BORON_NODISCARD ByteArray trimmed() const & { return trimmed_helper(*this); }
    BORON_NODISCARD ByteArray trimmed() && { return trimmed_helper(*this); }

    // TODO: check if these simplify functions are necessary
    // BORON_NODISCARD ByteArray simplified() const & {
    //   return simplified_helper(*this);
    // }
    // BORON_NODISCARD ByteArray simplified() && { return simplified_helper(*this);
    // }
    // BORON_NODISCARD ByteArray leftJustified(size_t width, uint8_t fill = ' ',
    // bool truncate = false) const;
    // BORON_NODISCARD ByteArray rightJustified(size_t width, uint8_t fill = ' ',
    // bool truncate = false) const;

    ByteArray& prepend(uint8_t c) { return insert(0, ByteArrayView(&c, 1)); }
    inline ByteArray& prepend(size_t n, uint8_t c);
    // TODO: WARNING: This function is not safe to use with null-terminated
    ByteArray& prepend(const uint8_t* s)
    {
      return insert(
        0, ByteArrayView(s, static_cast<size_t>(strlen(reinterpret_cast<const char*>(s)))));
    }

    ByteArray& prepend(const uint8_t* s, size_t len)
    {
      return insert(0, ByteArrayView(s, len));
    }

    ByteArray& prepend(const ByteArray& a) { return insert(0, ByteArrayView(a)); }
    ByteArray& prepend(const ByteArrayView a) { return insert(0, a); }

    // TODO: check if using std::vector<byte>::push_back would be more efficient
    ByteArray& append(const uint8_t c)
    {
      return insert(size(), ByteArrayView(&c, 1));
    }

    inline ByteArray& append(size_t n, uint8_t ch);
    ByteArray& append(const uint8_t* s) { return append(s, -1); }

    ByteArray& append(const uint8_t* s, const size_t len)
    {
      return append(
        ByteArrayView(s, len == kDetectLength
                           ? static_cast<size_t>(strlen(reinterpret_cast<const char*>(s)))
                           : len));
    }

    ByteArray& append(const ByteArray& a)
    {
      return insert(size(), ByteArrayView(a));
    }

    ByteArray& append(const ByteArrayView a) { return insert(size(), a); }

    ByteArray& assign(ByteArrayView v)
    {
      this->data_.assign(v.begin(), v.end());
      return *this;
    }

    ByteArray& assign(const size_t n, const uint8_t c)
    {
      // assert(n >= 0);
      return fill(c, n);
    }

    template <InputIterator Iter>
    ByteArray& assign(Iter first, Iter last)
    {
      this->data_.assign(first, last);
      return *this;
    }

    ByteArray& insert(size_t i, ByteArrayView data)
    {
      const auto ib = this->data_.begin();
      this->data_.insert(ib + i, data.begin(), data.end());
      return *this;
    }

    inline ByteArray& insert(size_t i, const uint8_t* s)
    {
      return insert(i, ByteArrayView(s));
    }

    inline ByteArray& insert(size_t i, const ByteArray& data)
    {
      return insert(i, ByteArrayView(data));
    }

    ByteArray& insert(size_t i, size_t count, uint8_t c)
    {
      auto ib = this->data_.begin();
      this->data_.insert(ib + i, count, c);
      return *this;
    }

    ByteArray& insert(size_t i, uint8_t c)
    {
      return insert(i, ByteArrayView(&c, 1));
    }

    ByteArray& insert(size_t i, const uint8_t* s, size_t len)
    {
      return insert(i, ByteArrayView(s, len));
    }

    ByteArray& remove(size_t index, size_t len)
    {
      if (len == 0)
        return *this;
      verify(index, len);
      this->data_.erase(this->data_.begin() + index,
                        this->data_.begin() + index + len);
      return *this;
    }

    ByteArray& removeAt(size_t pos)
    {
      return pos < size() ? remove(pos, 1) : *this;
    }

    ByteArray& removeFirst() { return !isEmpty() ? remove(0, 1) : *this; }
    ByteArray& removeLast() { return !isEmpty() ? remove(size() - 1, 1) : *this; }

    template <typename Predicate>
    ByteArray& removeIf(Predicate pred)
    {
      // TODO: removeIf_helper
      removeIf_helper(pred);
      return *this;
    }

    ByteArray& replace(size_t index, size_t len, const uint8_t* s, size_t alen)
    {
      return replace(index, len, ByteArrayView(s, alen));
    }

    ByteArray& replace(size_t index, size_t len, ByteArrayView s)
    {
      verify(index, len);
      if (len == 0 && s.empty())
        return *this;
      auto it = this->data_.begin() + index;
      if (s.size() == len)
      {
        // TODO: memcpy or range::copy or std::copy
        std::copy(s.begin(), s.end(), it);
      }
      else if (s.size() < len)
      {
        std::copy(s.begin(), s.end(), it);
        this->data_.erase(it + s.size(), it + len);
      }
      else
      {
        std::copy(s.begin(), s.begin() + len, it);
        this->data_.insert(it + len, s.begin() + len, s.end());
      }
      return *this;
    }

    ByteArray& replace(uint8_t before, ByteArrayView after)
    {
      return replace(ByteArrayView(&before, 1), after);
    }

    ByteArray& replace(const uint8_t* before, size_t bsize, const uint8_t* after,
                       size_t asize)
    {
      return replace(ByteArrayView(before, bsize), ByteArrayView(after, asize));
    }

    ByteArray& replace(ByteArrayView before, ByteArrayView after);
    ByteArray& replace(uint8_t before, uint8_t after);

    ByteArray& operator+=(uint8_t c) { return append(c); }
    ByteArray& operator+=(const uint8_t* s) { return append(s); }
    ByteArray& operator+=(const ByteArray& a) { return append(a); }
    ByteArray& operator+=(ByteArrayView a) { return append(a); }

    BORON_NODISCARD std::vector<ByteArray> split(uint8_t sep) const;

    BORON_NODISCARD ByteArray repeated(size_t times) const;

    // TODO: comparison between ByteArray and const uint8_t *
    BORON_NODISCARD friend inline constexpr auto operator<=>
    (const ByteArray& lhs, const ByteArray& rhs)
    {
      return ByteArrayView(lhs) <=> ByteArrayView(rhs);
    }

    BORON_NODISCARD friend inline constexpr auto operator==(const ByteArray& lhs, const ByteArray& rhs)
    {
      return ByteArrayView(lhs) == ByteArrayView(rhs);
    }

    // Check isEmpty() instead of isNull() for backwards compatibility.
    friend inline bool operator==(const ByteArray& a1, std::nullptr_t) noexcept
    {
      return a1.isEmpty();
    }

    friend inline bool operator!=(const ByteArray& a1, std::nullptr_t) noexcept
    {
      return !a1.isEmpty();
    }


    // short toShort(bool* ok = nullptr, int base = 10) const;
    // unsigned short toUShort(bool* ok = nullptr, int base = 10) const;
    // int toInt(bool* ok = nullptr, int base = 10) const;
    // unsigned int toUInt(bool* ok = nullptr, int base = 10) const;
    // long toLong(bool* ok = nullptr, int base = 10) const;
    // unsigned long toULong(bool* ok = nullptr, int base = 10) const;
    // long long toLongLong(bool* ok = nullptr, int base = 10) const;
    // unsigned long long toULongLong(bool* ok = nullptr, int base = 10) const;
    // float toFloat(bool* ok = nullptr) const;
    // double toDouble(bool* ok = nullptr) const;
    // ByteArray toBase64(Base64Options options = Base64Encoding) const;
    // TODO: implement Boron::String
    BORON_NODISCARD std::string toHex(char separator = '\0') const;
    std::string toPercentEncoding(const ByteArray& exclude = ByteArray(),
                                  const ByteArray& include = ByteArray(),
                                  uint8_t percent = '%') const;
    BORON_NODISCARD ByteArray percentDecoded(uint8_t percent = '%') const;

    // inline ByteArray& setNum(short, std::endian endian);
    // inline ByteArray& setNum(unsigned short, std::endian endian);
    // inline ByteArray& setNum(int, std::endian endian);
    // inline ByteArray& setNum(unsigned int, std::endian endian);
    // inline ByteArray& setNum(long, std::endian endian);
    // inline ByteArray& setNum(unsigned long, std::endian endian);
    // TODO: the design of setNum in Qt makes no sense. It should return a ByteArray with the number in it in proper endianness
    template <std::integral T>
    inline ByteArray& setNum(T number, std::endian endian)
    {
      if constexpr (std::is_signed_v<T>)
      {
        setNum_helper(static_cast<unsigned long long>(number), endian, sizeof(T));
      }
      else
      {
        setNum_helper(static_cast<long long>(number), endian, sizeof(T));
      }
      return *this;
    }

    // ByteArray& setNum(long long, std::endian endian);
    // ByteArray& setNum(unsigned long long, std::endian endian);
#if BORON_ENABLE_GMP
    // TODO: implement setNum for mpz_class
#endif
    // TODO: setNum for float and double
    ByteArray& setNum(float, uint8_t format = 'g', int precision = 6);
    ByteArray& setNum(double, uint8_t format = 'g', int precision = 6);
    // To re-use existed ByteArray to save memory re-allocations.
    ByteArray& setRawData(const uint8_t* a, size_t n);

    // BORON_NODISCARD static ByteArray number(int, int base = 10);
    // BORON_NODISCARD static ByteArray number(unsigned int, int base = 10);
    // BORON_NODISCARD static ByteArray number(long, int base = 10);
    // BORON_NODISCARD static ByteArray number(unsigned long, int base = 10);
    // BORON_NODISCARD static ByteArray number(long long, int base = 10);
    // BORON_NODISCARD static ByteArray number(unsigned long long, int base = 10);
    // BORON_NODISCARD static ByteArray number(double, uint8_t format = 'g',
    //                                       int precision = 6);

    BORON_NODISCARD static ByteArray fromRawData(const uint8_t* data, size_t size)
    {
      return {const_cast<uint8_t*>(data), size};
    }

    // TODO: redesign Base64
    // TODO: implement fromHex
    // TODO: String
    BORON_NODISCARD static ByteArray fromHex(const std::string& hexEncoded);
    // TODO: implement fromPercentEncoding
    BORON_NODISCARD static ByteArray
    fromPercentEncoding(const ByteArray& pctEncoded, uint8_t percent = '%');

    // TODO: typedef iterator
    typedef uint8_t* iterator;
    typedef const uint8_t* const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    iterator begin() { return data(); }
    BORON_NODISCARD const_iterator begin() const noexcept { return data(); }
    BORON_NODISCARD const_iterator cbegin() const noexcept { return begin(); }
    BORON_NODISCARD const_iterator constBegin() const noexcept { return begin(); }
    iterator end() { return begin() + size(); }
    BORON_NODISCARD const_iterator end() const noexcept { return begin() + size(); }
    BORON_NODISCARD const_iterator cend() const noexcept { return end(); }
    BORON_NODISCARD const_iterator constEnd() const noexcept { return end(); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    BORON_NODISCARD const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }

    BORON_NODISCARD const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }

    BORON_NODISCARD const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    BORON_NODISCARD const_reverse_iterator crend() const noexcept { return rend(); }

    // stl compatibility
    void push_back(uint8_t c) { append(c); }
    void push_back(const uint8_t* s) { append(s); }
    void push_back(const ByteArray& a) { append(a); }
    void push_back(ByteArrayView a) { append(a); }
    void push_front(uint8_t c) { prepend(c); }
    void push_front(const uint8_t* c) { prepend(c); }
    void push_front(const ByteArray& a) { prepend(a); }
    void push_front(ByteArrayView a) { prepend(a); }
    void shrink_to_fit() { squeeze(); }
    iterator erase(const_iterator first, const_iterator last);
    inline iterator erase(const_iterator it) { return erase(it, it + 1); }
    BORON_NODISCARD inline bool empty() const { return data_.empty(); }

    static ByteArray fromStdString(const std::string& s);
    BORON_NODISCARD std::string toStdString() const;

    BORON_NODISCARD inline size_t size() const noexcept { return data_.size(); }
    BORON_NODISCARD inline size_t length() const noexcept { return size(); }
    BORON_NODISCARD inline bool isNull() const noexcept { return data_.empty(); }

  private:
    static ByteArray setNum_helper(unsigned long long, std::endian, size_t);
    static ByteArray setNum_helper(long long, std::endian, size_t);

    void expand(size_t i);

    inline void verify([[maybe_unused]] size_t pos = 0,
                       [[maybe_unused]] size_t n = 1) const
    {
      // assert(pos >= 0);
      assert(pos <= data_.size());
      // assert(n >= 0);
      assert(n <= data_.size() - pos);
    }

    static ByteArray sliced_helper(ByteArray& a, size_t pos, size_t n);
    static ByteArray trimmed_helper(const ByteArray& a);
    static ByteArray trimmed_helper(ByteArray& a);

    friend class String;
  };


  // TODO: constexpr ByteArray::ByteArray() noexcept {}
  inline ByteArray::ByteArray() noexcept = default;

  inline ByteArray::~ByteArray() = default;

  inline uint8_t ByteArray::at(size_t i) const
  {
    verify(i, 1);
    return data_[i];
  }

  inline uint8_t ByteArray::operator[](size_t i) const
  {
    verify(i, 1);
    return data_[i];
  }

  inline uint8_t* ByteArray::data()
  {
    return data_.data();
  }

  inline const uint8_t* ByteArray::data() const noexcept
  {
    return data_.data();
  }

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

  inline uint8_t& ByteArray::operator[](size_t i)
  {
    verify(i, 1);
    return data()[i];
  }

  inline uint8_t& ByteArray::front()
  {
    return operator[](0);
  }

  inline uint8_t& ByteArray::back()
  {
    return operator[](size() - 1);
  }

  inline ByteArray& ByteArray::append(size_t n, uint8_t ch)
  {
    return insert(size(), n, ch);
  }

  inline ByteArray& ByteArray::prepend(size_t n, uint8_t ch)
  {
    return insert(0, n, ch);
  }

  inline bool ByteArray::contains(uint8_t c) const
  {
    return indexOf(c) != static_cast<size_t>(-1);
  }

  inline bool ByteArray::contains(ByteArrayView bv) const
  {
    return indexOf(bv) != static_cast<size_t>(-1);
  }

  inline int ByteArray::compare(ByteArrayView a) const noexcept
  {
    return orderToInt(ByteArrayView(*this) <=> a);
  }

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
  inline ByteArray ByteArrayView::toByteArray() const
  {
    return {data(), size()};
  }
} // namespace Boron

#endif
