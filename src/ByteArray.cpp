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

  std::vector<ByteArray> ByteArray::split(uint8_t sep) const
  {
    std::vector<ByteArray> result;
    for(auto it = this->begin(), last = this->begin(); it != this->end(); ++it)
    {
      if (*it == sep)
      {
        result.emplace_back(last, it);
        last = it + 1;
      }
    }
    return result;
  }



} // namespace Boron
