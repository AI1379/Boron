#include "Boron/ByteArray.hpp"
#include "ByteArrayAlgorithms.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Boron {

ByteArray::ByteArray(const uint8_t *data, size_t size) {
  if (!data) {
    this->d = DataPointer();
  } else {
    if (size < 0)
      size = strlen(reinterpret_cast<const char *>(data));
    if (!size) {
      d = DataPointer::fromRawData(data, size);
    } else {
      d = DataPointer(size, size);
      assert(d.data() != nullptr);
      memcpy(d.data(), data, size);
      d.data()[size] = 0;
    }
  }
}

size_t ByteArray::indexOf(uint8_t chr, size_t from) const {
  return Detail::findByte(*this, from, chr);
}

size_t ByteArray::indexOf(ByteArrayView needle, size_t from) const {
  return Detail::findByteArray(*this, from, needle);
}

} // namespace Boron
