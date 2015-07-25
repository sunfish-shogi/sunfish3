/* Random.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_RANDOM__
#define SUNFISH_RANDOM__

#include "../def.h"
#include <random>
#include <algorithm>
#include <ctime>
#include <cstdint>

namespace sunfish {

template <class GenType>
class BaseRandom {
private:
  GenType rgen;

public:
  BaseRandom() : rgen(static_cast<unsigned>(time(NULL))) {
  }
  BaseRandom(const BaseRandom&) = delete;
  BaseRandom(BaseRandom&&) = delete;

  uint16_t getInt16() {
    std::uniform_int_distribution<uint16_t> dst16;
    return dst16(rgen);
  }

  uint16_t getInt16(uint16_t num) {
    std::uniform_int_distribution<uint16_t> dst16(0, num-1);
    return getInt16() % num;
  }

  uint32_t getInt32() {
    std::uniform_int_distribution<uint32_t> dst32;
    return dst32(rgen);
  }

  uint32_t getInt32(uint32_t num) {
    std::uniform_int_distribution<uint32_t> dst32(0, num-1);
    return getInt32() % num;
  }

  uint64_t getInt64() {
    std::uniform_int_distribution<uint64_t> dst64;
    return dst64(rgen);
  }

  uint64_t getInt64(uint64_t num) {
    std::uniform_int_distribution<uint64_t> dst64(0, num-1);
    return getInt64() % num;
  }

  unsigned getBit() {
    std::uniform_int_distribution<unsigned> dstBit(0, 1);
    return dstBit(rgen);
  }

  template <class Iterator>
  void shuffle(Iterator begin, Iterator end) {
    std::shuffle(begin, end, rgen);
  }
};

using Random = BaseRandom<std::mt19937>;

} // namespace sunfish

#endif //SUNFISH_RANDOM__
