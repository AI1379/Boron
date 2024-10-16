#include "Boron/ByteArray.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "ByteArrayAlgorithms.hpp"

namespace Boron
{

  ByteArray::ByteArray(const uint8_t* data, size_t size)
  {
    if (!data)
    {
      this->data_ = Container();
    }
    else
    {
      if (size == kDetectLength)
        size = strlen(reinterpret_cast<const char*>(data));
      if (!size)
      {
        this->data_ = Container();
      }
      else
      {
        this->data_ = Container(data, data + size);
      }
    }
  }

  // TODO: zero-terminated string
  ByteArray::ByteArray(size_t size, uint8_t ch)
  {
    if (size <= 0)
    {
      this->data_ = Container();
    }
    else
    {
      this->data_ = Container(size, ch);
    }
  }

  void ByteArray::resize(size_t size)
  {
    this->data_.resize(size);
  }

  void ByteArray::resize(size_t size, uint8_t ch)
  {
    this->data_.resize(size, ch);
  }

  ByteArray& ByteArray::fill(uint8_t ch, size_t size)
  {
    this->resize(size == kDetectLength ? this->size() : size);
    if (this->size())
      memset(this->data(), ch, this->size());
    return *this;
  }

  size_t ByteArray::count(uint8_t c) const
  {
    return std::count(this->begin(), this->end(), c);
  }

  size_t ByteArray::count(ByteArrayView needle) const
  {
    return Detail::countByteArray(*this, needle);
  }

  size_t ByteArray::indexOf(uint8_t chr, size_t from) const
  {
    return Detail::findByte(*this, from, chr);
  }

  size_t ByteArray::indexOf(ByteArrayView needle, size_t from) const
  {
    return Detail::findByteArray(*this, from, needle);
  }

  // TODO: check if unnecessary copy is made
  ByteArray ByteArray::sliced_helper(ByteArray& ba, size_t pos, size_t n)
  {
    return ba.sliced(pos, n);
  }

  ByteArray ByteArray::trimmed_helper(const ByteArray& a)
  {
    auto l = a.begin(), r = a.end();
    while (l < r && (std::isspace(*l) || *l == 0))
      ++l;
    while (l < r && (std::isspace(*(r - 1)) || *(r - 1) == 0))
      --r;
    return {l, r};
  }

  ByteArray ByteArray::trimmed_helper(ByteArray& a)
  {
    return trimmed_helper(static_cast<const ByteArray&>(a));
  }


  bool ByteArray::startsWith(ByteArrayView bv) const
  {
    // TODO: fix some security issues
    if (bv.size() > this->size())
      return false;
    for (auto i = 0_sz; i < bv.size(); i++)
      if (bv[i] != this->data_[i])
        return false;
    return true;
  }

  bool ByteArray::endsWith(ByteArrayView bv) const
  {
    if (bv.size() > this->size())
      return false;
    for (auto i = 0_sz; i < bv.size(); i++)
      if (bv[i] != this->data_[this->size() - bv.size() + i])
        return false;
    return true;
  }

  // TODO: try to optimize this
  std::vector<ByteArray> ByteArray::split(uint8_t sep) const
  {
    std::vector<ByteArray> result;
    for (auto it = this->begin(), last = this->begin(); it != this->end(); ++it)
    {
      if (*it == sep && it != last)
      {
        result.emplace_back(last, it);
        last = it + 1;
      }
    }
    return result;
  }

  ByteArray ByteArray::repeated(size_t times) const
  {
    ByteArray result;
    result.data_.resize(this->size() * times);
    for (auto i = 0_sz; i < times; i++)
      memcpy(result.data_.data() + i * this->size(), this->data_.data(), this->size());
    return result;
  }

  ByteArray& ByteArray::setRawData(const uint8_t* a, size_t n)
  {
    this->data_.reserve(n);
    memcpy(this->data_.data(), a, n);
    return *this;
  }

  ByteArray ByteArray::fromStdString(const std::string& s)
  {
    return fromRawData(reinterpret_cast<const uint8_t*>(s.data()), s.size());
  }

  std::string ByteArray::toStdString() const
  {
    return {reinterpret_cast<const char*>(this->data_.data()), this->data_.size()};
  }

  std::string ByteArray::toHex(char separator) const
  {
    static constexpr const char kHexChars[] = "0123456789ABCDEF";
    std::string result;
    auto res_size = this->size() * (separator == '\0' ? 2 : 3);
    result.reserve(res_size);
    for (auto i = 0_sz; i < this->size(); i++)
    {
      result.push_back(kHexChars[this->data_[i] >> 4]);
      result.push_back(kHexChars[this->data_[i] & 0x0F]);
      if (separator && i + 1 < this->size())
        result.push_back(separator);
    }
    return result;
  }

  ByteArray ByteArray::fromHex(const std::string& hexEncoded)
  {
    static constexpr const byte kHexChars[256] = {
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 0-15
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 16-31
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 32-47
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, // '0'-'9' and others
      255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 'A'-'F' and others
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 80-95
      255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 'a'-'f' and others
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 112-127
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 128-143
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 144-159
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 160-175
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 176-191
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 192-207
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 208-223
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 224-239
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 // 240-255

    };
    ByteArray result;
    assert(hexEncoded.size() % 2 == 0);
    result.data_.reserve(hexEncoded.size() / 2);
    for (auto i = 0_sz; i < hexEncoded.size(); i += 2)
    {
      auto hi = kHexChars[static_cast<unsigned char>(hexEncoded[i])];
      auto lo = kHexChars[static_cast<unsigned char>(hexEncoded[i + 1])];
      result.data_.push_back(hi << 4 | lo);
    }
    return result;
  }


} // namespace Boron
