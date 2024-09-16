#ifndef BORON_INCLUDE_BORON_COMMON_HPP_
#define BORON_INCLUDE_BORON_COMMON_HPP_

#include <cstdint>
#include <ios>
#include <string>
#include <type_traits>

namespace Boron {
using byte = std::uint8_t;
template <typename T>
concept ByteLike = std::is_same_v<T, byte> || std::is_same_v<T, std::int8_t> ||
                   std::is_same_v<T, std::uint8_t> || std::is_same_v<T, char> ||
                   std::is_same_v<T, unsigned char>;

template <typename T>
concept CharLike = std::is_same_v<T, char> || std::is_same_v<T, wchar_t> ||
                   std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

template <typename T> struct ByteTraits;
template <CharLike T> struct ByteTraits<T> : std::char_traits<T> {};
template <ByteLike T> struct ByteTraits<T> {
  using char_type = T;
  using int_type = T;
  using off_type = std::streamoff;
  using pos_type = std::streampos;
  using state_type = std::mbstate_t;
  static void assign(char_type &c1, const char_type &c2) noexcept { c1 = c2; }
  static bool eq(const char_type &c1, const char_type &c2) noexcept {
    return c1 == c2;
  }
  static bool lt(const char_type &c1, const char_type &c2) noexcept {
    return c1 < c2;
  }
  static int compare(const char_type *s1, const char_type *s2, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
      if (s1[i] < s2[i])
        return -1;
      if (s1[i] > s2[i])
        return 1;
    }
    return 0;
  }
  static std::size_t length(const char_type *s) {
    std::size_t len = 0;
    while (s[len] != 0)
      ++len;
    return len;
  }
  static const char_type *find(const char_type *s, std::size_t n,
                               const char_type &a) {
    for (std::size_t i = 0; i < n; ++i) {
      if (s[i] == a)
        return s + i;
    }
    return nullptr;
  }
  static char_type *move(char_type *s1, const char_type *s2, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i)
      s1[i] = s2[i];
    return s1;
  }
  static char_type *copy(char_type *s1, const char_type *s2, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i)
      s1[i] = s2[i];
    return s1;
  }
  static char_type *assign(char_type *s, std::size_t n, char_type a) {
    for (std::size_t i = 0; i < n; ++i)
      s[i] = a;
    return s;
  }
  static int_type eof() noexcept { return static_cast<int_type>(-1); }
  static int_type not_eof(const int_type &c) noexcept {
    return c == eof() ? 0 : c;
  }
  static char_type to_char_type(const int_type &c) noexcept {
    return static_cast<char_type>(c);
  }
  static int_type to_int_type(const char_type &c) noexcept {
    return static_cast<int_type>(c);
  }
  static bool eq_int_type(const int_type &c1, const int_type &c2) noexcept {
    return c1 == c2;
  }
};

} // namespace Boron

#endif