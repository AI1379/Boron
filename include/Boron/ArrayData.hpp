#ifndef BORON_INCLUDE_BORON_ARRAYDATA_HPP_
#define BORON_INCLUDE_BORON_ARRAYDATA_HPP_

#include "Boron/Global.hpp"
#include <atomic>

namespace Boron {
template <typename T> class TypedArrayData;

struct ArrayData {
  enum AllocationOption { Grow, KeepSize };
  enum GrowthPosition { Front, Back };
  enum ArrayOption { ArrayOptionDefault = 0, CapacityReserved = 0x1 };
  std::atomic_int refCount;
};

class ArrayDataPointer {};
} // namespace Boron

#endif