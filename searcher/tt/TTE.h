/* TTE.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TTE__
#define __SUNFISH_TTE__

#include "../eval/Value.h"
#include "../tree/NodeStat.h"
#include "core/def.h"
#include <cassert>
#include <iostream>

#define TT_INDEX_WIDTH 19

// 1st word
#define TT_HASH_WIDTH  54
#define TT_AGE_WIDTH   3  // 2^3 = 8 [0, 7]
#define TT_MATE_WIDTH  1

// 2nd word
#define TT_MOVE_WIDTH  16
#define TT_VALUE_WIDTH 16 // 2^16
#define TT_VTYPE_WIDTH 2  // 2^2 = 4 [0, 3]
#define TT_DEPTH_WIDTH 10 // 2^10 = 1024
#define TT_CSUM_WIDTH  20

#define TT_VALUE_OFFSET ((1<<TT_VALUE_WIDTH)/2)
#define TT_ENC_VALUE(value) ((value.int32()) + TT_VALUE_OFFSET)
#define TT_DEC_VALUE(value) ((int32_t)(value) - TT_VALUE_OFFSET)
#define TT_ENC_HASH(hash) ((hash) >> (64 - TT_HASH_WIDTH))
#define TT_CSUM_MASK ((1<<TT_CSUM_WIDTH)-1)

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
  static CONSTEXPR uint32_t InvalidAge = 0x00;
  static CONSTEXPR uint32_t AgeMax = 0x01 << TT_AGE_WIDTH;

  enum ValueType : int {
    Exact = 0,
    Upper, /* = 1 */
    Lower, /* = 2 */
    None, /* = 3 */
  };

private:

  struct alignas(8) {
    uint64_t hash : TT_HASH_WIDTH;
    uint32_t age : TT_AGE_WIDTH;
    bool mateThreat : TT_MATE_WIDTH;
  } _1;

  struct alignas(8) {
    uint16_t move : TT_MOVE_WIDTH;
    uint32_t value : TT_VALUE_WIDTH;
    uint32_t valueType : TT_VTYPE_WIDTH;
    uint32_t depth : TT_DEPTH_WIDTH;
    uint32_t checkSum : TT_CSUM_WIDTH;
  } _2;

  static_assert(sizeof(_1) == 8, "invalid struct size");
  static_assert(sizeof(_2) == 8, "invalid struct size");

  bool update(uint64_t newHash,
      Value newValue,
      ValueType newValueType,
      int newDepth, int ply,
      uint16_t move,
      uint32_t newAge,
      const NodeStat& stat);

  uint32_t calcCheckSum() const {
    return TT_CSUM_MASK & (
      _1.hash ^ _1.age ^ _1.mateThreat ^
      _2.move ^ _2.value ^ _2.valueType ^ _2.depth);
  }

public:
  TTE() {
    init();
  }

  void init() {
    _1.age = InvalidAge;
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

  void updatePv(uint64_t newHash, int newDepth, uint16_t move, uint32_t newAge);

  bool checkHash(uint64_t hash) const {
    return _1.hash == TT_ENC_HASH(hash) && _2.checkSum == calcCheckSum();
  }

  uint64_t getHash() const {
    return _1.hash;
  }

  bool isMateThreat() const {
    return _1.mateThreat;
  }

  Value getValue(int ply) const {
    Value value = TT_DEC_VALUE(_2.value);
    assert(_2.valueType == ValueType::None || value >= -Value::Inf);
    assert(_2.valueType == ValueType::None || value <= Value::Inf);
    if (value >= Value::Mate) {
      if (_2.valueType == Lower) { return value - ply; }
    } else if (value <= -Value::Mate) {
      if (_2.valueType == Upper) { return value + ply; }
    }
    return value;
  }

  uint32_t getValueType() const {
    return _2.valueType;
  }

  int getDepth() const {
    return (int)_2.depth;
  }

  uint16_t getMove() const {
    return _2.move;
  }

  uint32_t getAge() const {
    return _1.age;
  }

};

class TTEs {
private:

  static CONSTEXPR uint32_t Size = 4;
  TTE _slots[Size];
  volatile uint32_t _lastAccess;

public:

  TTEs() : _lastAccess(0) {
  }

  void init(uint32_t) {
    for (uint32_t i = 0; i < Size; i++) {
      _slots[i].init();
    }
  }

  TTStatus set(const TTE& entity);
  bool get(uint64_t hash, TTE& entity);

};

} // namespace sunfish

#endif // __SUNFISH_TTE__
