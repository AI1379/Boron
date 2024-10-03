#include "ByteArrayAlgorithms.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>

namespace Boron::Detail
{

  // TODO: Implement countByteArray using Boyer-Moore search
  size_t countByteArray(ByteArrayView haystack, ByteArrayView needle)
  {
    size_t count = 0;
    size_t pos = 0;
    while ((pos = findByteArray(haystack, pos, needle)) != kNotFound)
    {
      ++count;
      pos += needle.size();
    }
    return count;
  }

  // TODO: Implement Boyer-Moore search

  size_t findByte(ByteArrayView haystack, size_t from, uint8_t chr)
  {
    const auto l = haystack.size();
    if (l == 0)
      return -1;
    // if (from < 0)
    // from += l;
    // if (from < 0)
    // from = 0;
    if (from >= l)
      return -1;
    // TODO: use memchr
    const auto it = std::find(haystack.begin() + from, haystack.end(), chr);
    return it == haystack.end() ? -1 : it - haystack.begin();
  }

  size_t findByteArray(ByteArrayView haystack, size_t from,
                       ByteArrayView needle)
  {
    const auto ol = needle.size();
    const auto l = haystack.size();
    if (ol == 0)
    {
      // if (from < 0)
      // return std::max(from + l, 0ull);
      // else
      return from > l ? -1 : from;
    }
    if (ol == 1)
    {
      return findByte(haystack, from, needle[0]);
    }
    if (from > l || ol + from > l)
      return -1;

    // use Boyer-Moore search for large haystacks
    if (l > 500 && ol > 5)
    {
      auto it =
        std::search(haystack.begin() + from, haystack.end(),
                    std::boyer_moore_searcher(needle.begin(), needle.end()));
      return std::distance(haystack.begin(), it);
    }

    // else use hash search
    static constexpr const size_t kBase = 31;
    // TODO: check if this kMod is reasonable
    // static constexpr const size_t kMod = 75903750772792949;
    auto haystackptr = haystack.data() + from;
    auto end = haystack.data() + l - ol;
    auto needleptr = needle.data();
    size_t pow_base = 1;
    size_t needle_hash = 0, haystack_hash = 0;
    size_t idx = 0;
    for (idx = 0; idx < ol - 1; ++idx)
    {
      needle_hash = needle_hash * kBase + needleptr[idx];
      haystack_hash = haystack_hash * kBase + haystackptr[idx];
      pow_base *= kBase;
    }
    needle_hash = needle_hash * kBase + needleptr[idx];
    while (haystackptr <= end)
    {
      haystack_hash = haystack_hash * kBase + haystackptr[idx];
      if (haystack_hash == needle_hash && *haystackptr == *needleptr)
      {
        // TODO: check the implementation of std::equal and memcmp
        if (std::equal(needleptr, needleptr + ol, haystackptr))
          return haystackptr - haystack.data();
      }
      haystack_hash -= *haystackptr * pow_base;
      ++haystackptr;
    }
    return -1;
  }
} // namespace Boron::Detail
