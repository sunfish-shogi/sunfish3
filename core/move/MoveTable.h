/* MoveTable.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MOVETABLE__
#define __SUNFISH_MOVETABLE__

#include "../base/Piece.h"
#include "../board/Bitboard.h"

namespace sunfish {

/**
 * DirectionMaskTable
 */
template <bool full>
class DirectionMaskTable {
private:
  Bitboard _file[Position::N];
  Bitboard _rank[Position::N];
  Bitboard _leftUpX[Position::N];
  Bitboard _rightUpX[Position::N];

  Bitboard _right[Position::N];
  Bitboard _left[Position::N];
  Bitboard _up[Position::N];
  Bitboard _down[Position::N];
  Bitboard _leftUp[Position::N];
  Bitboard _rightDown[Position::N];
  Bitboard _rightUp[Position::N];
  Bitboard _leftDown[Position::N];

public:
  DirectionMaskTable();
  DirectionMaskTable(const DirectionMaskTable&) = delete;
  DirectionMaskTable(DirectionMaskTable&) = delete;
  const Bitboard& file(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _file[pos];
  }
  const Bitboard& rank(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _rank[pos];
  }
  const Bitboard& leftUpX(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _leftUpX[pos];
  }
  const Bitboard& rightUpX(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _rightUpX[pos];
  }
  const Bitboard& left(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _left[pos];
  }
  const Bitboard& right(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _right[pos];
  }
  const Bitboard& up(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _up[pos];
  }
  const Bitboard& down(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _down[pos];
  }
  const Bitboard& leftUp(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _leftUp[pos];
  }
  const Bitboard& rightDown(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _rightDown[pos];
  }
  const Bitboard& rightUp(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _rightUp[pos];
  }
  const Bitboard& leftDown(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _leftDown[pos];
  }
};
extern const DirectionMaskTable<true> dirMask;
extern const DirectionMaskTable<false> dirMask7x7;

/**
 * MagicNumberTable
 */
class MagicNumberTable {
private:
  Bitboard _rank[Position::N];
  Bitboard _leftMagic[Position::N];
  Bitboard _leftUp[Position::N];
  Bitboard _rightUp[Position::N];

public:
  MagicNumberTable();
  MagicNumberTable(const MagicNumberTable&) = delete;
  MagicNumberTable(MagicNumberTable&&) = delete;
  const Bitboard& rank(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _rank[pos];
  }
  const Bitboard& leftUp(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _leftUp[pos];
  }
  const Bitboard& rightUp(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _rightUp[pos];
  }
};
extern const sunfish::MagicNumberTable magic;

/**
 * MovePatternTable
 */
class MovePatternTable {
private:
  Bitboard _up[Position::N][0x80];
  Bitboard _down[Position::N][0x80];
  Bitboard _file[Position::N][0x80];
  Bitboard _rank[Position::N][0x80];
  Bitboard _leftUpX[Position::N][0x80];
  Bitboard _rightUpX[Position::N][0x80];
  Bitboard _leftUp[Position::N][0x80];
  Bitboard _leftDown[Position::N][0x80];
  Bitboard _rightUp[Position::N][0x80];
  Bitboard _rightDown[Position::N][0x80];
  Bitboard _left[Position::N][0x80];
  Bitboard _right[Position::N][0x80];

public:
  MovePatternTable();
  MovePatternTable(const MovePatternTable&) = delete;
  MovePatternTable(MovePatternTable&&) = delete;
  const Bitboard& up(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _up[pos][pattern];
  }
  const Bitboard& down(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _down[pos][pattern];
  }
  const Bitboard& file(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _file[pos][pattern];
  }
  const Bitboard& rank(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _rank[pos][pattern];
  }
  const Bitboard& leftUpX(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _leftUpX[pos][pattern];
  }
  const Bitboard& rightUpX(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _rightUpX[pos][pattern];
  }
  const Bitboard& leftUp(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _leftUp[pos][pattern];
  }
  const Bitboard& leftDown(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _leftDown[pos][pattern];
  }
  const Bitboard& rightUp(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _rightUp[pos][pattern];
  }
  const Bitboard& rightDown(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _rightDown[pos][pattern];
  }
  const Bitboard& left(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _left[pos][pattern];
  }
  const Bitboard& right(const Position& pos, unsigned pattern) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    assert(pattern < 0x80);
    return _right[pos][pattern];
  }
};
extern const sunfish::MovePatternTable movePattern;

namespace _MoveTableType {
  enum Type {
    BPawn, BLance, BKnight, BSilver, BGold,
    WPawn, WLance, WKnight, WSilver, WGold,
    Bishop, Rook, Bishop2, Rook2, King, Horse, Dragon,
    Vertical, Horizontal, RightUpX, RightDownX,
    RightUp, RightDown, LeftUp, LeftDown,
    Left, Right
  };
}
typedef _MoveTableType::Type MoveTableType;

/**
 * OneStepMoveTable
 * 跳び駒以外の移動
 */
template <MoveTableType type>
class OneStepMoveTable {
private:
  Bitboard _table[Position::N];

public:
  OneStepMoveTable();
  OneStepMoveTable(const OneStepMoveTable&) = delete;
  OneStepMoveTable(OneStepMoveTable&&) = delete;

  const Bitboard& get(const Position& pos) const {
    assert(pos >= 0);
    assert(pos < Position::N);
    return _table[pos];
  }
};
extern const OneStepMoveTable<MoveTableType::Horse> horseOneStepMove;
extern const OneStepMoveTable<MoveTableType::Dragon> dragonOneStepMove;

/**
 * 利き算出テーブル
 */
class MoveTables {
private:
  MoveTables();

  static const OneStepMoveTable<MoveTableType::BPawn> BPawn;
  static const OneStepMoveTable<MoveTableType::BKnight> BKnight;
  static const OneStepMoveTable<MoveTableType::BSilver> BSilver;
  static const OneStepMoveTable<MoveTableType::BGold> BGold;
  static const OneStepMoveTable<MoveTableType::WPawn> WPawn;
  static const OneStepMoveTable<MoveTableType::WKnight> WKnight;
  static const OneStepMoveTable<MoveTableType::WSilver> WSilver;
  static const OneStepMoveTable<MoveTableType::WGold> WGold;
  static const OneStepMoveTable<MoveTableType::Bishop> Bishop1;
  static const OneStepMoveTable<MoveTableType::Rook> Rook1;
  static const OneStepMoveTable<MoveTableType::King> King;

public:
  static const Bitboard& bpawn(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return BPawn.get(pos);
  }
  static const Bitboard& bknight(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return BKnight.get(pos);
  }
  static const Bitboard& bsilver(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return BSilver.get(pos);
  }
  static const Bitboard& bgold(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return BGold.get(pos);
  }
  static const Bitboard& wpawn(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return WPawn.get(pos);
  }
  static const Bitboard& wknight(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return WKnight.get(pos);
  }
  static const Bitboard& wsilver(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return WSilver.get(pos);
  }
  static const Bitboard& wgold(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return WGold.get(pos);
  }
  static const Bitboard& bishop1(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return Bishop1.get(pos);
  }
  static const Bitboard& rook1(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return Rook1.get(pos);
  }
  static const Bitboard& king(const Position& pos) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return King.get(pos);
  }

  static const Bitboard& vertical(const Position& pos, const Bitboard& bb) {
    // 縦方向
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.file(pos);
    unsigned b = Bitboard::isHigh(pos) ? (unsigned)(attack.high() >> ((Bitboard::HighFiles - pos.getFile()) * 9 + 1))
                                       : (unsigned)(attack.low() >> ((9 - pos.getFile()) * 9 + 1));
    return movePattern.file(pos, b & 0x7f);
  }

  static const Bitboard& horizontal(const Position& pos, const Bitboard& bb) {
    // 横方向
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.rank(pos);
    const auto& m = magic.rank(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rank(pos, b & 0x7f);
  }

  static const Bitboard& rightUpX(const Position& pos, const Bitboard& bb) {
    // 双方向右上がり
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.rightUpX(pos);
    const auto& m = magic.rightUp(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rightUpX(pos, b & 0x7f);
  }

  static const Bitboard& rightDownX(const Position& pos, const Bitboard& bb) {
    // 双方向右下がり
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.leftUpX(pos);
    const auto& m = magic.leftUp(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.leftUpX(pos, b & 0x7f);
  }

  static const Bitboard& rightUp(const Position& pos, const Bitboard& bb) {
    // 右上がり
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.rightUp(pos);
    const auto& m = magic.rightUp(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rightUp(pos, b & 0x7f);
  }

  static const Bitboard& rightDown(const Position& pos, const Bitboard& bb) {
    // 右下がり
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.rightDown(pos);
    const auto& m = magic.leftUp(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rightDown(pos, b & 0x7f);
  }

  static const Bitboard& leftUp(const Position& pos, const Bitboard& bb) {
    // 左上がり
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.leftUp(pos);
    const auto& m = magic.leftUp(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.leftUp(pos, b & 0x7f);
  }

  static const Bitboard& leftDown(const Position& pos, const Bitboard& bb) {
    // 左下がり
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.leftDown(pos);
    const auto& m = magic.rightUp(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.leftDown(pos, b & 0x7f);
  }

  static const Bitboard& right(const Position& pos, const Bitboard& bb) {
    // 右
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.right(pos);
    const auto& m = magic.rank(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.right(pos, b & 0x7f);
  }

  static const Bitboard& left(const Position& pos, const Bitboard& bb) {
    // 左
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.left(pos);
    const auto& m = magic.rank(pos);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.left(pos, b & 0x7f);
  }

  static const Bitboard& blance(const Position& pos, const Bitboard& bb) {
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.file(pos);
    unsigned b = Bitboard::isHigh(pos) ? (unsigned)(attack.high() >> ((Bitboard::HighFiles - pos.getFile()) * 9 + 1))
                                       : (unsigned)(attack.low() >> ((9 - pos.getFile()) * 9 + 1));
    return movePattern.up(pos, b & 0x7f);
  }

  static const Bitboard& wlance(const Position& pos, const Bitboard& bb) {
    assert(pos >= 0);
    assert(pos < Position::N);
    Bitboard attack = bb & dirMask7x7.file(pos);
    unsigned b = Bitboard::isHigh(pos) ? (unsigned)(attack.high() >> ((Bitboard::HighFiles - pos.getFile()) * 9 + 1))
                                       : (unsigned)(attack.low() >> ((9 - pos.getFile()) * 9 + 1));
    return movePattern.down(pos, b & 0x7f);
  }

  /**
   * 角の利き
   */
  static Bitboard bishop(const Position& pos, const Bitboard& bb) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return rightUpX(pos, bb) | rightDownX(pos, bb);
  }

  /**
   * 飛車の利き
   */
  static Bitboard rook(const Position& pos, const Bitboard& bb) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return vertical(pos, bb) | horizontal(pos, bb);
  }

  /**
   * 角の利き(距離2以上)
   */
  static Bitboard bishop2(const Position& pos, const Bitboard& bb) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return (rightUpX(pos, bb) | rightDownX(pos, bb)) & ~MoveTables::King.get(pos);
  }

  /**
   * 飛車の利き(距離2以上)
   */
  static Bitboard rook2(const Position& pos, const Bitboard& bb) {
    assert(pos >= 0);
    assert(pos < Position::N);
    return (vertical(pos, bb) | horizontal(pos, bb)) & ~MoveTables::King.get(pos);
  }

  /**
   * 馬の利き
   */
  static Bitboard horse(const Position& pos, const Bitboard& bb) {
    // 角の利きに縦横1マスの移動を加える。
    assert(pos >= 0);
    assert(pos < Position::N);
    return rightUpX(pos, bb) | rightDownX(pos, bb) | horseOneStepMove.get(pos);
  }

  /**
   * 竜の利き
   */
  static Bitboard dragon(const Position& pos, const Bitboard& bb) {
    // 飛車の利きに斜め1マスの移動を加える。
    assert(pos >= 0);
    assert(pos < Position::N);
    return vertical(pos, bb) | horizontal(pos, bb) | dragonOneStepMove.get(pos);
  }
};

} // namespace sunfish

#endif //__SUNFISH_MOVETABLE__
