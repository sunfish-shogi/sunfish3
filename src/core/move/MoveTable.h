/* MoveTable.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MOVETABLE__
#define SUNFISH_MOVETABLE__

#include "../base/Piece.h"
#include "../board/Bitboard.h"

namespace sunfish {

/**
 * DirectionMaskTable
 */
template <bool full>
class DirectionMaskTable {
private:
  Bitboard file_[Square::N];
  Bitboard rank_[Square::N];
  Bitboard leftUpX_[Square::N];
  Bitboard rightUpX_[Square::N];

  Bitboard right_[Square::N];
  Bitboard left_[Square::N];
  Bitboard up_[Square::N];
  Bitboard down_[Square::N];
  Bitboard leftUp_[Square::N];
  Bitboard rightDown_[Square::N];
  Bitboard rightUp_[Square::N];
  Bitboard leftDown_[Square::N];

public:
  DirectionMaskTable();
  DirectionMaskTable(const DirectionMaskTable&) = delete;
  DirectionMaskTable(DirectionMaskTable&) = delete;
  const Bitboard& file(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return file_[sq];
  }
  const Bitboard& rank(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return rank_[sq];
  }
  const Bitboard& leftUpX(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return leftUpX_[sq];
  }
  const Bitboard& rightUpX(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return rightUpX_[sq];
  }
  const Bitboard& left(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return left_[sq];
  }
  const Bitboard& right(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return right_[sq];
  }
  const Bitboard& up(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return up_[sq];
  }
  const Bitboard& down(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return down_[sq];
  }
  const Bitboard& leftUp(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return leftUp_[sq];
  }
  const Bitboard& rightDown(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return rightDown_[sq];
  }
  const Bitboard& rightUp(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return rightUp_[sq];
  }
  const Bitboard& leftDown(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return leftDown_[sq];
  }
};
extern const DirectionMaskTable<true> dirMask;
extern const DirectionMaskTable<false> dirMask7x7;

/**
 * MagicNumberTable
 */
class MagicNumberTable {
private:
  Bitboard rank_[Square::N];
  Bitboard leftUp_[Square::N];
  Bitboard rightUp_[Square::N];

public:
  MagicNumberTable();
  MagicNumberTable(const MagicNumberTable&) = delete;
  MagicNumberTable(MagicNumberTable&&) = delete;
  const Bitboard& rank(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return rank_[sq];
  }
  const Bitboard& leftUp(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return leftUp_[sq];
  }
  const Bitboard& rightUp(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return rightUp_[sq];
  }
};
extern const sunfish::MagicNumberTable magic;

/**
 * MovePatternTable
 */
class MovePatternTable {
private:
  Bitboard up_[Square::N][0x80];
  Bitboard down_[Square::N][0x80];
  Bitboard file_[Square::N][0x80];
  Bitboard rank_[Square::N][0x80];
  Bitboard leftUpX_[Square::N][0x80];
  Bitboard rightUpX_[Square::N][0x80];
  Bitboard leftUp_[Square::N][0x80];
  Bitboard leftDown_[Square::N][0x80];
  Bitboard rightUp_[Square::N][0x80];
  Bitboard rightDown_[Square::N][0x80];
  Bitboard left_[Square::N][0x80];
  Bitboard right_[Square::N][0x80];

public:
  MovePatternTable();
  MovePatternTable(const MovePatternTable&) = delete;
  MovePatternTable(MovePatternTable&&) = delete;
  const Bitboard& up(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return up_[sq][pattern];
  }
  const Bitboard& down(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return down_[sq][pattern];
  }
  const Bitboard& file(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return file_[sq][pattern];
  }
  const Bitboard& rank(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return rank_[sq][pattern];
  }
  const Bitboard& leftUpX(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return leftUpX_[sq][pattern];
  }
  const Bitboard& rightUpX(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return rightUpX_[sq][pattern];
  }
  const Bitboard& leftUp(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return leftUp_[sq][pattern];
  }
  const Bitboard& leftDown(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return leftDown_[sq][pattern];
  }
  const Bitboard& rightUp(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return rightUp_[sq][pattern];
  }
  const Bitboard& rightDown(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return rightDown_[sq][pattern];
  }
  const Bitboard& left(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return left_[sq][pattern];
  }
  const Bitboard& right(const Square& sq, unsigned pattern) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    assert(pattern < 0x80);
    return right_[sq][pattern];
  }
};
extern const sunfish::MovePatternTable movePattern;

enum MoveTableType : int {
  BPawn, BLance, BKnight, BSilver, BGold,
  WPawn, WLance, WKnight, WSilver, WGold,
  Bishop, Rook, Bishop2, Rook2, King, Horse, Dragon,
  Vertical, Horizontal, RightUpX, RightDownX,
  RightUp, RightDown, LeftUp, LeftDown,
  Left, Right
};

/**
 * OneStepMoveTable
 * 跳び駒以外の移動
 */
template <MoveTableType type>
class OneStepMoveTable {
private:
  Bitboard table_[Square::N];

public:
  OneStepMoveTable();
  OneStepMoveTable(const OneStepMoveTable&) = delete;
  OneStepMoveTable(OneStepMoveTable&&) = delete;

  const Bitboard& get(const Square& sq) const {
    assert(sq >= 0);
    assert(sq < Square::N);
    return table_[sq];
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
  static const Bitboard& bpawn(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return BPawn.get(sq);
  }
  static const Bitboard& bknight(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return BKnight.get(sq);
  }
  static const Bitboard& bsilver(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return BSilver.get(sq);
  }
  static const Bitboard& bgold(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return BGold.get(sq);
  }
  static const Bitboard& wpawn(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return WPawn.get(sq);
  }
  static const Bitboard& wknight(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return WKnight.get(sq);
  }
  static const Bitboard& wsilver(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return WSilver.get(sq);
  }
  static const Bitboard& wgold(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return WGold.get(sq);
  }
  static const Bitboard& bishop1(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return Bishop1.get(sq);
  }
  static const Bitboard& rook1(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return Rook1.get(sq);
  }
  static const Bitboard& king(const Square& sq) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return King.get(sq);
  }

  static const Bitboard& vertical(const Square& sq, const Bitboard& bb) {
    // 縦方向
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.file(sq);
    unsigned b = Bitboard::isHigh(sq) ? (unsigned)(attack.high() >> ((Bitboard::HighFiles - sq.getFile()) * 9 + 1))
                                       : (unsigned)(attack.low() >> ((9 - sq.getFile()) * 9 + 1));
    return movePattern.file(sq, b & 0x7f);
  }

  static const Bitboard& horizontal(const Square& sq, const Bitboard& bb) {
    // 横方向
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.rank(sq);
    const auto& m = magic.rank(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rank(sq, b & 0x7f);
  }

  static const Bitboard& rightUpX(const Square& sq, const Bitboard& bb) {
    // 双方向右上がり
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.rightUpX(sq);
    const auto& m = magic.rightUp(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rightUpX(sq, b & 0x7f);
  }

  static const Bitboard& rightDownX(const Square& sq, const Bitboard& bb) {
    // 双方向右下がり
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.leftUpX(sq);
    const auto& m = magic.leftUp(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.leftUpX(sq, b & 0x7f);
  }

  static const Bitboard& rightUp(const Square& sq, const Bitboard& bb) {
    // 右上がり
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.rightUp(sq);
    const auto& m = magic.rightUp(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rightUp(sq, b & 0x7f);
  }

  static const Bitboard& rightDown(const Square& sq, const Bitboard& bb) {
    // 右下がり
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.rightDown(sq);
    const auto& m = magic.leftUp(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.rightDown(sq, b & 0x7f);
  }

  static const Bitboard& leftUp(const Square& sq, const Bitboard& bb) {
    // 左上がり
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.leftUp(sq);
    const auto& m = magic.leftUp(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.leftUp(sq, b & 0x7f);
  }

  static const Bitboard& leftDown(const Square& sq, const Bitboard& bb) {
    // 左下がり
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.leftDown(sq);
    const auto& m = magic.rightUp(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.leftDown(sq, b & 0x7f);
  }

  static const Bitboard& right(const Square& sq, const Bitboard& bb) {
    // 右
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.right(sq);
    const auto& m = magic.rank(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.right(sq, b & 0x7f);
  }

  static const Bitboard& left(const Square& sq, const Bitboard& bb) {
    // 左
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.left(sq);
    const auto& m = magic.rank(sq);
    unsigned b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return movePattern.left(sq, b & 0x7f);
  }

  static const Bitboard& blance(const Square& sq, const Bitboard& bb) {
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.file(sq);
    unsigned b = Bitboard::isHigh(sq) ? (unsigned)(attack.high() >> ((Bitboard::HighFiles - sq.getFile()) * 9 + 1))
                                       : (unsigned)(attack.low() >> ((9 - sq.getFile()) * 9 + 1));
    return movePattern.up(sq, b & 0x7f);
  }

  static const Bitboard& wlance(const Square& sq, const Bitboard& bb) {
    assert(sq >= 0);
    assert(sq < Square::N);
    Bitboard attack = bb & dirMask7x7.file(sq);
    unsigned b = Bitboard::isHigh(sq) ? (unsigned)(attack.high() >> ((Bitboard::HighFiles - sq.getFile()) * 9 + 1))
                                       : (unsigned)(attack.low() >> ((9 - sq.getFile()) * 9 + 1));
    return movePattern.down(sq, b & 0x7f);
  }

  /**
   * 角の利き
   */
  static Bitboard bishop(const Square& sq, const Bitboard& bb) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return rightUpX(sq, bb) | rightDownX(sq, bb);
  }

  /**
   * 飛車の利き
   */
  static Bitboard rook(const Square& sq, const Bitboard& bb) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return vertical(sq, bb) | horizontal(sq, bb);
  }

  /**
   * 角の利き(距離2以上)
   */
  static Bitboard bishop2(const Square& sq, const Bitboard& bb) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return (rightUpX(sq, bb) | rightDownX(sq, bb)) & ~MoveTables::King.get(sq);
  }

  /**
   * 飛車の利き(距離2以上)
   */
  static Bitboard rook2(const Square& sq, const Bitboard& bb) {
    assert(sq >= 0);
    assert(sq < Square::N);
    return (vertical(sq, bb) | horizontal(sq, bb)) & ~MoveTables::King.get(sq);
  }

  /**
   * 馬の利き
   */
  static Bitboard horse(const Square& sq, const Bitboard& bb) {
    // 角の利きに縦横1マスの移動を加える。
    assert(sq >= 0);
    assert(sq < Square::N);
    return rightUpX(sq, bb) | rightDownX(sq, bb) | horseOneStepMove.get(sq);
  }

  /**
   * 竜の利き
   */
  static Bitboard dragon(const Square& sq, const Bitboard& bb) {
    // 飛車の利きに斜め1マスの移動を加える。
    assert(sq >= 0);
    assert(sq < Square::N);
    return vertical(sq, bb) | horizontal(sq, bb) | dragonOneStepMove.get(sq);
  }
};

} // namespace sunfish

#endif //SUNFISH_MOVETABLE__
