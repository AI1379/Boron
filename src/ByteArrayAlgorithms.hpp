#ifndef BORON_SRC_BYTEARRAYALGORITHMS_HPP_
#define BORON_SRC_BYTEARRAYALGORITHMS_HPP_

#include "Boron/ByteArray.hpp"

namespace Boron::Detail {

constexpr const size_t kNotFound = -1;

size_t countByteArray(ByteArrayView haystack, ByteArrayView needle);
size_t findByte(ByteArrayView haystack, size_t from, uint8_t chr);
size_t findByteArray(ByteArrayView haystack, size_t from, ByteArrayView needle);

}

#endif