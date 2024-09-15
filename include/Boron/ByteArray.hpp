#ifndef BORON_INCLUDE_BORON_BYTEARRAY_HPP_
#define BORON_INCLUDE_BORON_BYTEARRAY_HPP_

#include "Boron/ArrayData.hpp"
#include "Boron/Global.hpp"


#include <cstdint>

namespace Boron {
class String;
class BORON_EXPORT ByteArray {
public:
  ByteArray();
  ByteArray(const uint8_t *data, size_t size);
  ByteArray(const ByteArray &other);
  ByteArray(ByteArray &&other) noexcept;
  ~ByteArray();

  ByteArray &operator=(const ByteArray &other);
  ByteArray &operator=(ByteArray &&other) noexcept;

  [[nodiscard]] size_t size() const;
  [[nodiscard]] const uint8_t *data() const;

  [[nodiscard]] String toString() const;
};
} // namespace Boron

#endif