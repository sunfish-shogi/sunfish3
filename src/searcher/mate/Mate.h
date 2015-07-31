/* Mate.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MATE__
#define SUNFISH_MATE__

#include "../tree/Tree.h"
#include "../table/HashTable.h"
#include "core/def.h"
#include "core/board/Board.h"

namespace sunfish {

class MateEntity {
private:
  static CONSTEXPR_CONST uint64_t KeyMask = 0xfffffffffffffffellu;
  static CONSTEXPR_CONST uint64_t MateMask = 0x0000000000000001llu;

  uint64_t data_;

public:

  MateEntity() {
    data_ = 0x00ull;
  }

  void init(unsigned index) {
    data_ = ~(uint64_t)index;
  }

  bool is(uint64_t key) const {
    return !((data_ ^ key) & KeyMask);
  }

  bool isMate() const {
    return data_ & MateMask;
  }

  void set(uint64_t key, bool mate) {
    data_ = (key & KeyMask) | (mate ? MateMask : 0x0ull);
  }
};

template <int KeyLength>
class MateTable : public HashTable<MateEntity> {
public:
  MateTable() : HashTable<MateEntity>(KeyLength) {
  }
  MateTable(const MateTable&) = delete;
  MateTable(MateTable&&) = delete;

  bool get(uint64_t hash, bool& mate) const {
    const auto& entity = getEntity(hash);
    if (entity.is(hash)) {
      mate = entity.isMate();
      return true;
    }
    return false;
  }

  void set(uint64_t hash, bool mate) {
    getEntity(hash).set(hash, mate);
  }
};

class Mate {
private:

  Mate();

  template<bool black, bool recursive = true>
  static bool isProtected_(const Board& board, const Square& to, const Bitboard& occ, const Bitboard& occNoAttacker, const Square& king);

  template<bool black>
  static bool isProtected_(const Board& board, Bitboard& bb, const Bitboard& occ, const Bitboard& occNoAttacker);

  template<bool black>
  static bool isMate_(const Board& board, const Move& move);

  template<bool black>
  static bool mate1Ply_(const Board& board);

  static bool isIneffectiveEvasion(const Board& board, const Move& move, const Move& check, const Bitboard& occ);

  static bool evade(Tree& tree, const Move& check);

public:

  /**
   * 1手詰めを探します。
   * 王手の局面では使用できません。
   * TODO: 開き王手の生成
   */
  static bool mate1Ply(const Board& board) {
    if (board.isBlack()) {
      return mate1Ply_<true>(board);
    } else {
      return mate1Ply_<false>(board);
    }
  }

  /**
   * 3手詰めを探します。
   * 王手の局面では使用できません。
   * TODO: 開き王手の生成
   */
  static bool mate3Ply(Tree& tree);

};

} // namespace sunfish

#endif // SUNFISH_MATE__
