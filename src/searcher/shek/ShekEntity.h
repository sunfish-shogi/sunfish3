/* ShekEntity.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_SHEKENTITY__
#define SUNFISH_SHEKENTITY__

#include "core/def.h"
#include "HandSet.h"
#include <cstdint>

#define SHEK_INDEX_WIDTH 16

#define SHEK_COUNT_WIDTH 5
#define SHEK_TURN_WIDTH  1
#define SHEK_HASH_WIDTH  58

#define SHEK_COUNT_SHIFT 0
#define SHEK_TURN_SHIFT  (SHEK_COUNT_SHIFT + SHEK_COUNT_WIDTH)

#define SHEK_COUNT_MASK  ((1llu << SHEK_COUNT_WIDTH) - 1llu)
#define SHEK_TURN_MASK   (1llu << SHEK_COUNT_WIDTH)
#define SHEK_HASH_MASK   (~((1llu << (64 - SHEK_HASH_WIDTH)) - 1llu))

static_assert(SHEK_COUNT_WIDTH + SHEK_TURN_WIDTH + SHEK_HASH_WIDTH <= 64, "invalid data size");
static_assert((SHEK_COUNT_MASK & SHEK_TURN_MASK) == 0llu, "invalid mask");
static_assert((SHEK_COUNT_MASK & SHEK_HASH_MASK) == 0llu, "invalid mask");
static_assert((SHEK_TURN_MASK & SHEK_HASH_MASK) == 0llu, "invalid mask");
static_assert((SHEK_INDEX_WIDTH > 64 - SHEK_HASH_WIDTH), "invalid hash length");

namespace sunfish {

class ShekEntity {
private:

  HandSet handSet_;
  uint64_t _;

public:

  void init(uint64_t invalidKey) {
    _ = invalidKey;
  }

  ShekStat check(const HandSet& handSet, bool blackTurn) const {
    // 持ち駒をチェックする
    ShekStat stat = handSet.compareTo(handSet_, blackTurn);

    if (((_ & SHEK_TURN_MASK) != 0llu) != blackTurn) {
      if (stat == ShekStat::Equal) {
        // 手番が逆で持ち駒が等しい => 優越
        stat = ShekStat::Superior;
      } else if (stat == ShekStat::Inferior) {
        // 手番が逆で持ち駒が劣っている => 該当なし
        stat = ShekStat::None;
      }
    }

    return stat;
  }

  void set(uint64_t hash, const HandSet& handSet, bool blackTurn) {
    handSet_ = handSet;
    _ = hash & SHEK_HASH_MASK;
    _ |= (uint64_t)(blackTurn) << SHEK_TURN_SHIFT;
  }

  void retain() {
    assert((_ & SHEK_COUNT_MASK) != SHEK_COUNT_MASK);
    _++;
  }

  void release(uint64_t invalidKey) {
    assert((_ & SHEK_COUNT_MASK) != 0llu);
    _--;
    if ((_ & SHEK_COUNT_MASK) == 0llu) {
      _ = invalidKey;
    }
  }

  bool checkHash(uint64_t hash) const {
    return ((_ ^ hash) & SHEK_HASH_MASK) == 0llu;
  }

};

class ShekEntities {
private:

  static CONSTEXPR uint32_t Size = 4;

  uint32_t invalidKey_;
  ShekEntity entities_[Size];

public:

  void init(uint32_t key) {
    invalidKey_ = ~key;
    for (uint32_t i = 0; i < Size; i++) {
      entities_[i].init(invalidKey_);
    }
  }

  ShekStat check(uint64_t hash, const HandSet& handSet, bool blackTurn) const {
    for (uint32_t i = 0; i < Size; i++) {
      if (entities_[i].checkHash(hash)) {
        return entities_[i].check(handSet, blackTurn);
      }
    }
    return ShekStat::None;
  }

  void set(uint64_t hash, const HandSet& handSet, bool blackTurn) {
    for (uint32_t i = 0; i < Size; i++) {
      if (entities_[i].checkHash(hash)) {
        entities_[i].retain();
        return;
      }
    }
    for (uint32_t i = 0; i < Size; i++) {
      if (entities_[i].checkHash(invalidKey_)) {
        entities_[i].set(hash, handSet, blackTurn);
        entities_[i].retain();
        return;
      }
    }
  }

  void unset(uint64_t hash) {
    for (uint32_t i = 0; i < Size; i++) {
      if (entities_[i].checkHash(hash)) {
        entities_[i].release(invalidKey_);
        return;
      }
    }
    assert(false);
  }

  bool isCleared() const {
    for (uint32_t i = 0; i < Size; i++) {
      if (!entities_[i].checkHash(invalidKey_)) {
        return false;
      }
    }
    return true;
  }

};

} // namespace sunfish

#endif // SUNFISH_SHEKENTITY__
