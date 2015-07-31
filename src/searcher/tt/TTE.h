/* TTE.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_TTE__
#define SUNFISH_TTE__

#include "../eval/Value.h"
#include "../tree/NodeStat.h"
#include "core/def.h"
#include <cassert>
#include <iostream>

#define TT_INDEX_WIDTH 19

// 1st word
#define TT_AGE_WIDTH   3  // 2^3 = 8 [0, 7]
#define TT_MATE_WIDTH  1
#define TT_HASH_WIDTH  54

#define TT_AGE_SHIFT   0
#define TT_MATE_SHIFT  (TT_AGE_SHIFT + TT_AGE_WIDTH)

#define TT_AGE_MASK    (((1llu << TT_AGE_WIDTH) - 1llu) << TT_AGE_SHIFT)
#define TT_MATE_MASK   (((1llu << TT_MATE_WIDTH) - 1llu) << TT_MATE_SHIFT)
#define TT_HASH_MASK   (~((1llu << (64 - TT_HASH_WIDTH)) - 1))

// 2nd word
#define TT_MOVE_WIDTH  16
#define TT_VALUE_WIDTH 16 // 2^16
#define TT_VTYPE_WIDTH 2  // 2^2 = 4 [0, 3]
#define TT_DEPTH_WIDTH 10 // 2^10 = 1024
#define TT_CSUM_WIDTH  16

#define TT_MOVE_SHIFT  0
#define TT_VALUE_SHIFT (TT_MOVE_SHIFT + TT_MOVE_WIDTH)
#define TT_VTYPE_SHIFT (TT_VALUE_SHIFT + TT_VALUE_WIDTH)
#define TT_DEPTH_SHIFT (TT_VTYPE_SHIFT + TT_VTYPE_WIDTH)

#define TT_MOVE_MASK   (((1llu << TT_MOVE_WIDTH) - 1) << TT_MOVE_SHIFT)
#define TT_VALUE_MASK  (((1llu << TT_VALUE_WIDTH) - 1) << TT_VALUE_SHIFT)
#define TT_VTYPE_MASK  (((1llu << TT_VTYPE_WIDTH) - 1) << TT_VTYPE_SHIFT)
#define TT_DEPTH_MASK  (((1llu << TT_DEPTH_WIDTH) - 1) << TT_DEPTH_SHIFT)
#define TT_CSUM_MASK   (~((1llu << (64 - TT_CSUM_WIDTH)) - 1))

#define TT_VALUE_OFFSET (((int32_t)1<<TT_VALUE_WIDTH)/2)
#define TT_ENC_VALUE(value) ((value.int32()) + TT_VALUE_OFFSET)
#define TT_DEC_VALUE(value) ((int32_t)(value) - TT_VALUE_OFFSET)

static_assert(TT_AGE_WIDTH + TT_MATE_WIDTH + TT_HASH_WIDTH <= 64, "invalid data size");
static_assert(TT_MOVE_WIDTH + TT_VALUE_WIDTH + TT_VTYPE_WIDTH + TT_DEPTH_WIDTH + TT_CSUM_WIDTH <= 64, "invalid data size");
static_assert((TT_INDEX_WIDTH > 64 - TT_HASH_WIDTH), "invalid hash length");

namespace sunfish {

enum class TTStatus : int {
  None,
  Reject,
  New,
  Update,
  Collide,
};

class TTE {
public:
  static CONSTEXPR_CONST uint32_t InvalidAge = 0x00;
  static CONSTEXPR_CONST uint32_t AgeMax = 0x01 << TT_AGE_WIDTH;

  enum ValueType : int {
    Exact = 0,
    Upper, /* = 1 */
    Lower, /* = 2 */
    None, /* = 3 */
  };

private:

  uint64_t _1;
  uint64_t _2;

  bool update(uint64_t newHash,
      Value newValue,
      ValueType newValueType,
      int newDepth, int ply,
      uint16_t move,
      uint32_t newAge,
      const NodeStat& stat);

  uint64_t calcCheckSum() const {
    static_assert(TT_CSUM_WIDTH == 16, "invalid data size");
    static_assert(TT_CSUM_MASK == 0xffff000000000000llu, "invalid data size");
    return TT_CSUM_MASK & ((_1)
      ^ (_1 << 16)
      ^ (_1 << 32)
      ^ (_1 << 48)
      ^ (_2 << 16)
      ^ (_2 << 32)
      ^ (_2 << 48));
  }

public:
  TTE() {
    init();
  }

  void init() {
    assert((InvalidAge << TT_AGE_SHIFT) < TT_AGE_MASK);
    _1 = InvalidAge << TT_AGE_SHIFT;
  }

  bool update(uint64_t newHash,
      Value alpha,
      Value beta,
      Value newValue,
      int newDepth, int ply,
      uint16_t move,
      uint32_t newAge,
      const NodeStat& stat) {

    ValueType newValueType;
    if (newValue >= beta) {
      newValueType = Lower;
    } else if (newValue <= alpha) {
      newValueType = Upper;
    } else {
      newValueType = Exact;
    }

    return update(newHash, newValue, newValueType,
        newDepth, ply, move, newAge, stat);

  }

  void updatePV(uint64_t newHash, int newDepth, uint16_t move, uint32_t newAge);

  bool checkHash(uint64_t hash) const {
    return ((_1 ^ hash) & TT_HASH_MASK) == 0llu && ((_2 ^ calcCheckSum()) & TT_CSUM_MASK) == 0llu;
  }

  uint64_t getHash() const {
    return _1 & TT_HASH_MASK;
  }

  bool isMateThreat() const {
    return _1 & TT_MATE_MASK;
  }

  Value getValue(int ply) const {
    Value value = TT_DEC_VALUE((_2 & TT_VALUE_MASK) >> TT_VALUE_SHIFT);
    ValueType valueType = getValueType();
    assert(valueType == ValueType::None || value >= -Value::Inf);
    assert(valueType == ValueType::None || value <= Value::Inf);
    if (value >= Value::Mate) {
      if (valueType == Lower) { return value - ply; }
    } else if (value <= -Value::Mate) {
      if (valueType == Upper) { return value + ply; }
    }
    return value;
  }

  ValueType getValueType() const {
    return static_cast<ValueType>((_2 & TT_VTYPE_MASK) >> TT_VTYPE_SHIFT);
  }

  int getDepth() const {
    return (int)((_2 & TT_DEPTH_MASK) >> TT_DEPTH_SHIFT);
  }

  uint16_t getMove() const {
    return (_2 & TT_MOVE_MASK) >> TT_MOVE_SHIFT;
  }

  uint32_t getAge() const {
    return (_1 & TT_AGE_MASK) >> TT_AGE_SHIFT;
  }

};

class TTEs {
private:

  static CONSTEXPR_CONST uint32_t Size = 4;
  TTE slots_[Size];
  volatile uint32_t lastAccess_;

public:

  TTEs() : lastAccess_(0) {
  }

  void init(uint32_t) {
    for (uint32_t i = 0; i < Size; i++) {
      slots_[i].init();
    }
  }

  TTStatus set(const TTE& entity);
  bool get(uint64_t hash, TTE& entity);

};

} // namespace sunfish

#endif // SUNFISH_TTE__
