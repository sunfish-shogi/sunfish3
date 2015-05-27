/* SeeEntity.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SEEENTITY__
#define __SUNFISH_SEEENTITY__

#include "../eval/Value.h"
#include "core/def.h"
#include <cassert>

namespace sunfish {

template <int KeyLength>
class SeeEntity {
private:
  static CONSTEXPR uint64_t ValueTypeShift = KeyLength - 2;
  static CONSTEXPR uint64_t ValueMask = (1ULL << ValueTypeShift) - 1ULL;
  static CONSTEXPR uint64_t ValueTypeMask = 3ULL << ValueTypeShift;
  static CONSTEXPR uint64_t HashMask = ~(ValueMask | ValueTypeMask);

  static CONSTEXPR int ValueInf = 1U << (ValueTypeShift - 1);

  static CONSTEXPR uint64_t Exact = 0ULL << ValueTypeShift;
  static CONSTEXPR uint64_t Upper = 1ULL << ValueTypeShift;
  static CONSTEXPR uint64_t Lower = 2ULL << ValueTypeShift;

  static_assert((ValueTypeMask & ValueMask) == 0, "invalid");
  static_assert((ValueTypeMask & HashMask) == 0, "invalid");
  static_assert((ValueMask & HashMask) == 0, "invalid");
  static_assert((ValueTypeMask | ValueMask | HashMask) == ~0ULL, "invalid");
  static_assert(ValueInf >= 30000, "invalid");

  uint64_t _data;

  static int32_t convertValue(int32_t value) {
    return ValueInf - value;
  }

public:

  SeeEntity() {
    static_assert(sizeof(_data) == sizeof(uint64_t), "invalid data size");
    init();
  }

  void init() {
    _data = 0ull;
  }

  void init(unsigned) {
    init();
  }

  bool get(uint64_t hash, Value& value, const Value& alpha, const Value& beta) const {
    uint64_t temp = _data;
    if ((temp & HashMask) == (hash & HashMask)) {
      uint64_t valueType = temp & ValueTypeMask;
      value = convertValue((int32_t)(temp & ValueMask));
      assert(value > -ValueInf);
      assert(value < ValueInf);
      if (valueType == Exact) {
        return true;
      } else if (valueType == Lower && value >= beta) {
        return true;
      } else if (valueType == Upper && value <= alpha) {
        return true;
      }
    }
    return false;
  }

  void set(uint64_t hash, const Value& value, const Value& alpha, const Value& beta) {
    uint64_t valueType;
    if (value >= beta) {
      valueType = Lower;
    } else if (value > alpha) {
      valueType = Exact;
    } else {
      valueType = Upper;
    }

    assert(value > -ValueInf);
    assert(value < ValueInf);
    uint32_t v = (uint32_t)convertValue(value.int32());
    assert((v & ~ValueMask) == 0U);
    uint64_t temp = (hash & HashMask) | valueType | v;
    _data = temp;
  }
};

} // namespace sunfish

#endif // __SUNFISH_SEEENTITY__
