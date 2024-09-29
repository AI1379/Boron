#ifndef BORON_INCLUDE_BORON_NUMERIC_HPP_
#define BORON_INCLUDE_BORON_NUMERIC_HPP_

#include "Boron/Global.hpp"

#include <concepts>
#include <limits>
#include <numeric>

namespace Boron {

template <typename T>
concept UnsignedAndSigned = std::is_unsigned_v<T> || std::is_signed_v<T>;

template <typename T>
concept Unsigned = std::is_unsigned_v<T>;

template <typename T>
concept Signed = std::is_signed_v<T>;

// IntegerForSize
template <size_t N> struct IntegerForSize {};

template <> struct IntegerForSize<1> {
  using Signed = int8_t;
  using Unsigned = uint8_t;
};

template <> struct IntegerForSize<2> {
  using Signed = int16_t;
  using Unsigned = uint16_t;
};

template <> struct IntegerForSize<4> {
  using Signed = int32_t;
  using Unsigned = uint32_t;
};

template <> struct IntegerForSize<8> {
  using Signed = int64_t;
  using Unsigned = uint64_t;
};

#if defined(__GNUC__) || defined(__clang__)
template <> struct IntegerForSize<16> {
  using Signed = __int128;
  using Unsigned = __int128;
};
#endif

// code below is from Qt 6.2.0
// https://code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/global/qnumeric.h

// Overflow math.
// This provides efficient implementations for int, unsigned, qsizetype and
// size_t. Implementations for 8- and 16-bit types will work but may not be as
// efficient. Implementations for 64-bit may be missing on 32-bit platforms.

// TODO: actually the check of compiler version is not necessary since C++20
// standard is required
#if ((__GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 0)) ||               \
     __has_builtin(__builtin_add_overflow)) &&                                 \
    !(BORON_POINTER_SIZE == 4 && defined(__clang__))
// GCC 5 and Clang 3.8 have builtins to detect overflows
// 32 bit Clang has the builtins but tries to link a library which hasn't
#define INTRINSIC_MUL_OVERFLOW64

template <UnsignedAndSigned T> inline bool addOverflow(T v1, T v2, T *r) {
  return __builtin_add_overflow(v1, v2, r);
}

template <UnsignedAndSigned T> inline bool subOverflow(T v1, T v2, T *r) {
  return __builtin_sub_overflow(v1, v2, r);
}

template <UnsignedAndSigned T> inline bool mulOverflow(T v1, T v2, T *r) {
  return __builtin_mul_overflow(v1, v2, r);
}

#else
// TODO: Check if this is capable of MSVC and other C++ compilers

// Generic implementations

template <Unsigned T> inline bool addOverflow(T v1, T v2, T *r) {
  // unsigned additions are well-defined
  *r = v1 + v2;
  return v1 > T(v1 + v2);
}

template <Signed T> inline bool addOverflow(T v1, T v2, T *r) {
  // Here's how we calculate the overflow:
  // 1) unsigned addition is well-defined, so we can always execute it
  // 2) conversion from unsigned back to signed is implementation-
  //    defined and in the implementations we use, it's a no-op.
  // 3) signed integer overflow happens if the sign of the two input operands
  //    is the same but the sign of the result is different. In other words,
  //    the sign of the result must be the same as the sign of either
  //    operand.

  using U = typename std::make_unsigned_t<T>;
  *r = T(U(v1) + U(v2));

  // If int is two's complement, assume all integer types are too.
  if (std::is_same_v<int32_t, int>) {
    // Two's complement equivalent (generates slightly shorter code):
    //  x ^ y             is negative if x and y have different signs
    //  x & y             is negative if x and y are negative
    // (x ^ z) & (y ^ z)  is negative if x and z have different signs
    //                    AND y and z have different signs
    return ((v1 ^ *r) & (v2 ^ *r)) < 0;
  }

  bool s1 = (v1 < 0);
  bool s2 = (v2 < 0);
  bool sr = (*r < 0);
  return s1 != sr && s2 != sr;
  // also: return s1 == s2 && s1 != sr;
}

template <Unsigned T> inline bool subOverflow(T v1, T v2, T *r) {
  // unsigned subtractions are well-defined
  *r = v1 - v2;
  return v1 < v2;
}

template <Signed T> inline bool subOverflow(T v1, T v2, T *r) {
  // See above for explanation. This is the same with some signs reversed.
  // We can't use addOverflow(v1, -v2, r) because it would be UB if
  // v2 == std::numeric_limits<T>::min().

  using U = typename std::make_unsigned_t<T>;
  *r = T(U(v1) - U(v2));

  if (std::is_same_v<int32_t, int>)
    return ((v1 ^ *r) & (~v2 ^ *r)) < 0;

  bool s1 = (v1 < 0);
  bool s2 = !(v2 < 0);
  bool sr = (*r < 0);
  return s1 != sr && s2 != sr;
  // also: return s1 == s2 && s1 != sr;
}

template <UnsignedAndSigned T> inline bool mulOverflow(T v1, T v2, T *r) {
  // use the next biggest type
  // Note: for 64-bit systems where __int128 isn't supported, this will cause an
  // error.
  using LargerInt = IntegerForSize<sizeof(T) * 2>;
  using Larger = typename std::conditional_t<std::is_signed_v<T>,
                                             typename LargerInt::Signed,
                                             typename LargerInt::Unsigned>;
  Larger lr = Larger(v1) * Larger(v2);
  *r = T(lr);
  return lr > (std::numeric_limits<T>::max)() ||
         lr < (std::numeric_limits<T>::min)();
}

#if defined(INTRINSIC_MUL_OVERFLOW64)
template <> inline bool mulOverflow(uint64_t v1, uint64_t v2, uint64_t *r) {
  *r = v1 * v2;
  return __umulh(v1, v2);
}
template <> inline bool mulOverflow(int64_t v1, int64_t v2, int64_t *r) {
  // This is slightly more complex than the unsigned case above: the sign bit
  // of 'low' must be replicated as the entire 'high', so the only valid
  // values for 'high' are 0 and -1. Use unsigned multiply since it's the same
  // as signed for the low bits and use a signed right shift to verify that
  // 'high' is nothing but sign bits that match the sign of 'low'.

  int64_t high = __mulh(v1, v2);
  *r = int64_t(uint64_t(v1) * uint64_t(v2));
  return (*r >> 63) != high;
}

#if defined(Q_OS_INTEGRITY) && defined(Q_PROCESSOR_ARM_64)
template <> inline bool mulOverflow(uint64_t v1, uint64_t v2, uint64_t *r) {
  return mulOverflow<uint64_t>(v1, v2, reinterpret_cast<uint64_t *>(r));
}

template <> inline bool mulOverflow(int64_t v1, int64_t v2, int64_t *r) {
  return mulOverflow<int64_t>(v1, v2, reinterpret_cast<int64_t *>(r));
}
#endif // OS_INTEGRITY ARM64
#endif // INTRINSIC_MUL_OVERFLOW64

#if defined(_MSC_VER)
// We can use intrinsics for the unsigned operations with MSVC
template <> inline bool addOverflow(unsigned v1, unsigned v2, unsigned *r) {
  return _addcarry_u32(0, v1, v2, r);
}

// 32-bit mulOverflow is fine with the generic code above

template <> inline bool addOverflow(uint64_t v1, uint64_t v2, uint64_t *r) {
#if defined(Q_PROCESSOR_X86_64)
  return _addcarry_u64(0, v1, v2, reinterpret_cast<unsigned __int64 *>(r));
#else
  unsigned int low, high;
  unsigned char carry = _addcarry_u32(0, unsigned(v1), unsigned(v2), &low);
  carry = _addcarry_u32(carry, v1 >> 32, v2 >> 32, &high);
  *r = (uint64_t(high) << 32) | low;
  return carry;
#endif // !x86-64
}
#endif // MSVC X86
#endif // !GCC

} // namespace Boron

#endif // BORON_INCLUDE_BORON_NUMERIC_HPP_