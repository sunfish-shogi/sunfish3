/* Memory.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MEMORY__
#define SUNFISH_MEMORY__

#include "../def.h"
#include <cstddef>

namespace sunfish {

namespace memory {

template <size_t size, int rw = 0, int locality = 1>
inline void prefetch(const char* addr) {
  CONSTEXPR_CONST size_t CacheLineSize = 64;
#if defined(UNIX)
  for (size_t i = 0; i < size; i += CacheLineSize) {
    __builtin_prefetch(addr + i, rw, locality);
  }
#endif
}

} // memory

} // sunfish

#endif // SUNFISH_MEMORY__
