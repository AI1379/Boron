#ifndef BORON_SRC_BYTEARRAYALGORITHMS_HPP_
#define BORON_SRC_BYTEARRAYALGORITHMS_HPP_

#include "Boron/ByteArray.hpp"

namespace Boron::Detail {

size_t findByte(ByteArrayView haystack, size_t from, uint8_t chr);
size_t findByteArray(ByteArrayView haystack, size_t from, ByteArrayView needle);

}

#endif