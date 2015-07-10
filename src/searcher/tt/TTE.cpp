/* TTE.cpp
 *
 * Kubo Ryosuke
 */

#include "TTE.h"
#include "core/move/Move.h"

namespace sunfish {

bool TTE::update(uint64_t newHash,
    Value newValue,
    ValueType newValueType,
    int newDepth, int ply,
    uint16_t move,
    uint32_t newAge,
    const NodeStat& stat) {

  assert(newAge < AgeMax);
  assert(newValueType < (ValueType)(1<<TT_VTYPE_WIDTH));

  if (newDepth < 0) {
    newDepth = 0;
  } else if (newDepth > (1<<TT_DEPTH_WIDTH)-1) {
    newDepth = (1<<TT_DEPTH_WIDTH)-1;
  }

  if (checkHash(newHash)) {
    // 深さが劣るものは登録させない。
    if (newDepth < getDepth() && getAge() == newAge &&
        newValue < Value::Mate && newValue > -Value::Mate) {
      return false;
    }
    // 更新
    _2 &= TT_MOVE_MASK;
  } else {
    // 新規登録
    _2 = Move::S16_EMPTY << TT_MOVE_SHIFT;
  }

  if (newValue >= Value::Mate) {
    if (newValueType == Lower) {
      if (newValue < Value::Inf - ply) {
        newValue += ply;
      } else {
        newValue = Value::Inf;
      }
    }
  } else if (newValue <= -Value::Mate) {
    if (newValueType == Upper) {
      if (newValue > -Value::Inf + ply) {
        newValue -= ply;
      } else {
        newValue = -Value::Inf;
      }
    }
  }

  assert(newValue >= -Value::Inf);
  assert(newValue <= Value::Inf);
  int32_t value = TT_ENC_VALUE(newValue);
  assert(value >= 0);
  assert(value < (1<<TT_VALUE_WIDTH));

  _1 = newHash & TT_HASH_MASK;
  _1 |= ((uint64_t)newAge) << TT_AGE_SHIFT;
  _1 |= ((uint64_t)stat.isMateThreat()) << TT_MATE_SHIFT;
         
  _2 |= ((uint64_t)value) << TT_VALUE_SHIFT;
  _2 |= ((uint64_t)newValueType) << TT_VTYPE_SHIFT;
  _2 |= ((uint64_t)newDepth) << TT_DEPTH_SHIFT;
  if (move != Move::S16_EMPTY) {
    _2 &= ~TT_MOVE_MASK;
    _2 |= ((uint64_t)move) << TT_MOVE_SHIFT;
  }
  _2 |= calcCheckSum();

  return true;

}

void TTE::updatePV(uint64_t newHash, int newDepth, uint16_t move, uint32_t newAge) {
  if (newDepth < 0) {
    newDepth = 0;
  }

  if (checkHash(newHash)) {
    if (newDepth >= getDepth() || getAge() != newAge) {
      _2 &= ~(TT_VTYPE_MASK | TT_DEPTH_MASK | TT_CSUM_MASK);
      _2 |= ((uint64_t)None) << TT_VTYPE_SHIFT;
      _2 |= ((uint64_t)newDepth) << TT_DEPTH_SHIFT;
    } else {
      _2 &= ~(TT_CSUM_MASK);
    }
  } else {
    _2 = Move::S16_EMPTY << TT_MOVE_SHIFT;
    _2 |= ((uint64_t)None) << TT_VTYPE_SHIFT;
    _2 |= ((uint64_t)newDepth) << TT_DEPTH_SHIFT;
  }

  _1 = newHash & TT_HASH_MASK;
  _1 |= ((uint64_t)newAge) << TT_AGE_SHIFT;

  if (move != Move::S16_EMPTY) {
    _2 &= ~TT_MOVE_MASK;
    _2 |= ((uint64_t)move) << TT_MOVE_SHIFT;
  }
  _2 |= calcCheckSum();
}

TTStatus TTEs::set(const TTE& entity) {
  // ハッシュ値が一致するスロットを探す
  uint32_t l = lastAccess_ % Size;
  for (uint32_t i = 0; i < Size; i++) {
    const uint32_t index = (l + i) % Size;
    if (slots_[index].getHash() == entity.getHash()) {
      slots_[index] = entity;
      lastAccess_ = index;
      return TTStatus::Update;
    }
  }

  // 空きスロットを探す
  l++;
  for (uint32_t i = 0; i < Size; i++) {
    const uint32_t index = (l + i) % Size;
    if (slots_[index].getAge() != entity.getAge()) {
      slots_[index] = entity;
      lastAccess_ = index;
      return TTStatus::New;
    }
  }

  // 上書きする
  const uint32_t index = l % Size;
  slots_[index] = entity;
  lastAccess_ = index;
  return TTStatus::Collide;

}

bool TTEs::get(uint64_t hash, TTE& entity) {

  uint32_t l = lastAccess_ % Size;
  for (uint32_t i = 0; i < Size; i++) {
    const uint32_t index = (l + i) % Size;
    if (slots_[index].checkHash(hash)) {
      entity = slots_[index];
      lastAccess_ = index;
      return true;
    }
  }
  return false;

}

} // namespace sunfish
