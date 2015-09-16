/* MoveTable.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MOVETABLE__
#define SUNFISH_MOVETABLE__

#include "../base/Piece.h"
#include "../board/Bitboard.h"
#include "../bmi.h"

namespace sunfish {

/**
 * DirectionMaskTable
 */
class DirectionMaskTable {
private:
  static const Bitboard right_[Square::N];
  static const Bitboard left_[Square::N];
  static const Bitboard up_[Square::N];
  static const Bitboard down_[Square::N];
  static const Bitboard leftUp_[Square::N];
  static const Bitboard rightDown_[Square::N];
  static const Bitboard rightUp_[Square::N];
  static const Bitboard leftDown_[Square::N];

public:
  DirectionMaskTable() {}
  DirectionMaskTable(const DirectionMaskTable&) = delete;
  DirectionMaskTable(DirectionMaskTable&) = delete;
  static const Bitboard& left(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return left_[sq.index()];
  }
  static const Bitboard& right(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return right_[sq.index()];
  }
  static const Bitboard& up(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return up_[sq.index()];
  }
  static const Bitboard& down(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return down_[sq.index()];
  }
  static const Bitboard& leftUp(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return leftUp_[sq.index()];
  }
  static const Bitboard& rightDown(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rightDown_[sq.index()];
  }
  static const Bitboard& rightUp(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rightUp_[sq.index()];
  }
  static const Bitboard& leftDown(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return leftDown_[sq.index()];
  }
};

/**
 * DirectionMaskTable7x7
 */
class DirectionMaskTable7x7 {
private:
  static const Bitboard rank_[Square::N];
  static const Bitboard leftUpX_[Square::N];
  static const Bitboard rightUpX_[Square::N];

public:
  DirectionMaskTable7x7() {}
  DirectionMaskTable7x7(const DirectionMaskTable7x7&) = delete;
  DirectionMaskTable7x7(DirectionMaskTable7x7&) = delete;
  static const Bitboard& rank(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rank_[sq.index()];
  }
  static const Bitboard& leftUpX(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return leftUpX_[sq.index()];
  }
  static const Bitboard& rightUpX(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rightUpX_[sq.index()];
  }
};

#if !USE_BMI2
/**
 * MagicNumberTable
 */
class MagicNumberTable {
private:
  static const Bitboard rank_[Square::N];
  static const Bitboard leftUp_[Square::N];
  static const Bitboard rightUp_[Square::N];

public:
  MagicNumberTable() {}
  MagicNumberTable(const MagicNumberTable&) = delete;
  MagicNumberTable(MagicNumberTable&&) = delete;
  static const Bitboard& rank(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rank_[sq.index()];
  }
  static const Bitboard& leftUp(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return leftUp_[sq.index()];
  }
  static const Bitboard& rightUp(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rightUp_[sq.index()];
  }
};
#endif

/**
 * MovePatternTable
 */
class MovePatternTable {
private:
  static const Bitboard up_[Square::N][0x80];
  static const Bitboard down_[Square::N][0x80];
  static const Bitboard file_[Square::N][0x80];
  static const Bitboard rank_[Square::N][0x80];
  static const Bitboard leftUpX_[Square::N][0x80];
  static const Bitboard rightUpX_[Square::N][0x80];
  static const Bitboard leftUp_[Square::N][0x80];
  static const Bitboard leftDown_[Square::N][0x80];
  static const Bitboard rightUp_[Square::N][0x80];
  static const Bitboard rightDown_[Square::N][0x80];
  static const Bitboard left_[Square::N][0x80];
  static const Bitboard right_[Square::N][0x80];

public:
  MovePatternTable() {}
  MovePatternTable(const MovePatternTable&) = delete;
  MovePatternTable(MovePatternTable&&) = delete;
  template <class T>
  static const Bitboard& up(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return up_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& down(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return down_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& file(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return file_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& rank(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return rank_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& leftUpX(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return leftUpX_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& rightUpX(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return rightUpX_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& leftUp(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return leftUp_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& leftDown(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return leftDown_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& rightUp(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return rightUp_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& rightDown(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return rightDown_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& left(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return left_[sq.index()][pattern];
  }
  template <class T>
  static const Bitboard& right(const Square& sq, T pattern) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    assert(pattern < 0x80);
    return right_[sq.index()][pattern];
  }
};

/**
 * 利き算出テーブル
 */
class MoveTables {
private:
  MoveTables();

  static const Bitboard bpawn_[Square::N];
  static const Bitboard bknight_[Square::N];
  static const Bitboard bsilver_[Square::N];
  static const Bitboard bgold_[Square::N];
  static const Bitboard wpawn_[Square::N];
  static const Bitboard wknight_[Square::N];
  static const Bitboard wsilver_[Square::N];
  static const Bitboard wgold_[Square::N];
  static const Bitboard bishop1_[Square::N];
  static const Bitboard rook1_[Square::N];
  static const Bitboard king_[Square::N];
  static const Bitboard horseOneStepMove_[Square::N];
  static const Bitboard dragonOneStepMove_[Square::N];

public:
  static const Bitboard& bpawn(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return bpawn_[sq.index()];
  }
  static const Bitboard& bknight(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return bknight_[sq.index()];
  }
  static const Bitboard& bsilver(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return bsilver_[sq.index()];
  }
  static const Bitboard& bgold(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return bgold_[sq.index()];
  }
  static const Bitboard& wpawn(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return wpawn_[sq.index()];
  }
  static const Bitboard& wknight(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return wknight_[sq.index()];
  }
  static const Bitboard& wsilver(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return wsilver_[sq.index()];
  }
  static const Bitboard& wgold(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return wgold_[sq.index()];
  }
  static const Bitboard& bishop1(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return bishop1_[sq.index()];
  }
  static const Bitboard& rook1(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rook1_[sq.index()];
  }
  static const Bitboard& king(const Square& sq) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return king_[sq.index()];
  }

  static const Bitboard& vertical(const Square& sq, const Bitboard& bb) {
    // 縦方向
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    auto b = Bitboard::isHigh(sq) ? (bb.high() >> ((Bitboard::HighFiles * 9 + 1) - sq.getFile() * 9))
                                  : (bb.low() >> ((81 + 1) - sq.getFile() * 9));
    return MovePatternTable::file(sq, b & 0x7f);
  }

  static const Bitboard& horizontal(const Square& sq, const Bitboard& bb) {
    // 横方向
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::rank(sq);
    const auto& m = MagicNumberTable::rank(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::rank(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::rank(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::rank(sq, b);
#endif
  }

  static const Bitboard& rightUpX(const Square& sq, const Bitboard& bb) {
    // 双方向右上がり
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::rightUpX(sq);
    const auto& m = MagicNumberTable::rightUp(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::rightUpX(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::rightUpX(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::rightUpX(sq, b);
#endif
  }

  static const Bitboard& rightDownX(const Square& sq, const Bitboard& bb) {
    // 双方向右下がり
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::leftUpX(sq);
    const auto& m = MagicNumberTable::leftUp(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::leftUpX(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::leftUpX(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::leftUpX(sq, b);
#endif
  }

  static const Bitboard& rightUp(const Square& sq, const Bitboard& bb) {
    // 右上がり
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::rightUpX(sq);
    const auto& m = MagicNumberTable::rightUp(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::rightUp(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::rightUpX(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::rightUp(sq, b);
#endif
  }

  static const Bitboard& rightDown(const Square& sq, const Bitboard& bb) {
    // 右下がり
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::leftUpX(sq);
    const auto& m = MagicNumberTable::leftUp(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::rightDown(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::leftUpX(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::rightDown(sq, b);
#endif
  }

  static const Bitboard& leftUp(const Square& sq, const Bitboard& bb) {
    // 左上がり
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::leftUpX(sq);
    const auto& m = MagicNumberTable::leftUp(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::leftUp(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::leftUpX(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::leftUp(sq, b);
#endif
  }

  static const Bitboard& leftDown(const Square& sq, const Bitboard& bb) {
    // 左下がり
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::rightUpX(sq);
    const auto& m = MagicNumberTable::rightUp(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::leftDown(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::rightUpX(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::leftDown(sq, b);
#endif
  }

  static const Bitboard& right(const Square& sq, const Bitboard& bb) {
    // 右
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::rank(sq);
    const auto& m = MagicNumberTable::rank(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::right(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::rank(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::right(sq, b);
#endif
  }

  static const Bitboard& left(const Square& sq, const Bitboard& bb) {
    // 左
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
#if !USE_BMI2
    Bitboard attack = bb & DirectionMaskTable7x7::rank(sq);
    const auto& m = MagicNumberTable::rank(sq);
    auto b = ((attack.high() * m.high()) ^ (attack.low() * m.low())) >> (64-7);
    return MovePatternTable::left(sq, b & 0x7f);
#else
    Bitboard mask = DirectionMaskTable7x7::rank(sq);
    Bitboard attack = bb & mask;
    auto b = _pext_u64(attack.high() | attack.low(), mask.high() | mask.low());
    return MovePatternTable::left(sq, b);
#endif
  }

  static const Bitboard& blance(const Square& sq, const Bitboard& bb) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    auto b = Bitboard::isHigh(sq) ? (bb.high() >> ((Bitboard::HighFiles * 9 + 1) - sq.getFile() * 9))
                                  : (bb.low() >> ((81 + 1) - sq.getFile() * 9));
    return MovePatternTable::up(sq, b & 0x7f);
  }

  static const Bitboard& wlance(const Square& sq, const Bitboard& bb) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    auto b = Bitboard::isHigh(sq) ? (bb.high() >> ((Bitboard::HighFiles * 9 + 1) - sq.getFile() * 9))
                                  : (bb.low() >> ((81 + 1) - sq.getFile() * 9));
    return MovePatternTable::down(sq, b & 0x7f);
  }

  /**
   * 角の利き
   */
  static Bitboard bishop(const Square& sq, const Bitboard& bb) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rightUpX(sq, bb) | rightDownX(sq, bb);
  }

  /**
   * 飛車の利き
   */
  static Bitboard rook(const Square& sq, const Bitboard& bb) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return vertical(sq, bb) | horizontal(sq, bb);
  }

  /**
   * 角の利き(距離2以上)
   */
  static Bitboard bishop2(const Square& sq, const Bitboard& bb) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return (rightUpX(sq, bb) | rightDownX(sq, bb)) & ~king(sq);
  }

  /**
   * 飛車の利き(距離2以上)
   */
  static Bitboard rook2(const Square& sq, const Bitboard& bb) {
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return (vertical(sq, bb) | horizontal(sq, bb)) & ~king(sq);
  }

  /**
   * 馬の利き
   */
  static Bitboard horse(const Square& sq, const Bitboard& bb) {
    // 角の利きに縦横1マスの移動を加える。
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return rightUpX(sq, bb) | rightDownX(sq, bb) | horseOneStepMove_[sq.index()];
  }

  /**
   * 竜の利き
   */
  static Bitboard dragon(const Square& sq, const Bitboard& bb) {
    // 飛車の利きに斜め1マスの移動を加える。
    assert(sq.index() >= 0);
    assert(sq.index() < Square::N);
    return vertical(sq, bb) | horizontal(sq, bb) | dragonOneStepMove_[sq.index()];
  }
};

} // namespace sunfish

#endif //SUNFISH_MOVETABLE__
