/* Mate.cpp
 *
 * Kubo Ryosuke
 */

#include "Mate.h"
#include "core/move/MoveTable.h"
#include "core/move/MoveGenerator.h"
#include "core/util/Data.h"

#include "logger/Logger.h"
#include <iostream>

namespace sunfish {

template<bool black, bool recursive>
bool Mate::_isProtected(const Board& board, const Position& to, const Bitboard& occ, const Bitboard& occNoAttacker, const Position& king) {
  // pawn
  Bitboard bb = (black ? board.getBPawn() : board.getWPawn()) & occNoAttacker;
  if (bb.check(black ? to.safetyDown() : to.safetyUp())) {
    return true;
  }

  // lance
  bb = (black ? board.getBLance() : board.getWLance()) & occNoAttacker;
  bb &= black ? MoveTables::wlance(to, occ) : MoveTables::blance(to, occ);
  if (bb) { return true; }

  // knight
  bb = (black ? board.getBKnight() : board.getWKnight()) & occNoAttacker;
  bb &= black ? MoveTables::wknight(to) : MoveTables::bknight(to);
  if (bb) { return true; }

  // silver
  bb = (black ? board.getBSilver() : board.getWSilver()) & occNoAttacker;
  bb &= black ? MoveTables::wsilver(to) : MoveTables::bsilver(to);
  if (bb) { return true; }

  // gold
  bb = (black ? board.getBGold() : board.getWGold()) & occNoAttacker;
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  if (bb) { return true; }

  // bishop
  bb = (black ? board.getBBishop() : board.getWBishop()) & occNoAttacker;
  bb &= MoveTables::bishop(to, occ);
  if (bb) { return true; }

  // rook
  bb = (black ? board.getBRook() : board.getWRook()) & occNoAttacker;
  bb &= MoveTables::rook(to, occ);
  if (bb) { return true; }

  // tokin
  bb = (black ? board.getBTokin() : board.getWTokin()) & occNoAttacker;
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  if (bb) { return true; }

  // promoted lance
  bb = (black ? board.getBProLance() : board.getWProLance()) & occNoAttacker;
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  if (bb) { return true; }

  // promoted knight
  bb = (black ? board.getBProKnight() : board.getWProKnight()) & occNoAttacker;
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  if (bb) { return true; }

  // promoted silver
  bb = (black ? board.getBProSilver() : board.getWProSilver()) & occNoAttacker;
  bb &= black ? MoveTables::wgold(to) : MoveTables::bgold(to);
  if (bb) { return true; }

  // horse
  bb = (black ? board.getBHorse() : board.getWHorse()) & occNoAttacker;
  bb &= MoveTables::horse(to, occ);
  if (bb) { return true; }

  // dragon
  bb = (black ? board.getBDragon() : board.getWDragon()) & occNoAttacker;
  bb &= MoveTables::dragon(to, occ);
  if (bb) { return true; }

  // king
  if (king.isValid()) {
    if (MoveTables::king(king).check(to) &&
        (!recursive || !_isProtected<!black>(board, to, occ, occNoAttacker, Position::Invalid))) {
      return true;
    }
  }

  return false;
}
template bool Mate::_isProtected<true>(const Board&, const Position&, const Bitboard&, const Bitboard&, const Position&);
template bool Mate::_isProtected<false>(const Board&, const Position&, const Bitboard&, const Bitboard&, const Position&);

template<bool black>
bool Mate::_isProtected(const Board& board, Bitboard& bb, const Bitboard& occ, const Bitboard& occNoAttacker) {
  const auto& king = black ? board.getBKingPosition() : board.getWKingPosition();
  bool hasHand = black
    ? (board.getBlackHand(Piece::Pawn) != 0 ||
       board.getBlackHand(Piece::Lance) != 0 ||
       board.getBlackHand(Piece::Knight) != 0 ||
       board.getBlackHand(Piece::Silver) != 0 ||
       board.getBlackHand(Piece::Gold) != 0 ||
       board.getBlackHand(Piece::Bishop) != 0 ||
       board.getBlackHand(Piece::Rook) != 0)
    : (board.getWhiteHand(Piece::Pawn) != 0 ||
       board.getWhiteHand(Piece::Lance) != 0 ||
       board.getWhiteHand(Piece::Knight) != 0 ||
       board.getWhiteHand(Piece::Silver) != 0 ||
       board.getWhiteHand(Piece::Gold) != 0 ||
       board.getWhiteHand(Piece::Bishop) != 0 ||
       board.getWhiteHand(Piece::Rook) != 0);

  if (hasHand) {
    BB_EACH_OPE(to, bb, {
        if (_isProtected<black>(board, to, occ, occNoAttacker, king)) { return true; }
    });
  } else {
    BB_EACH_OPE(to, bb, {
        if (_isProtected<black>(board, to, occ, occNoAttacker, Position::Invalid)) { return true; }
    });
  }

  return false;
}
template bool Mate::_isProtected<true>(const Board&, Bitboard&, const Bitboard&, const Bitboard&);
template bool Mate::_isProtected<false>(const Board&, Bitboard&, const Bitboard&, const Bitboard&);

template<bool black>
bool Mate::_isMate(const Board& board, const Move& move) {

  bool isHand = move.isHand();
  // 王手放置を除外
  if (!isHand && !board.isValidMove(move)) {
    return false;
  }

  const auto& king = black ? board.getWKingPosition() : board.getBKingPosition();
  Bitboard occ = board.getBOccupy() | board.getWOccupy();
  if (!isHand) {
    occ &= ~Bitboard::mask(move.from());
  }
  Position to = move.to();
  occ |= Bitboard::mask(to);
  Bitboard occNoKing = occ & ~Bitboard::mask(king);
  Bitboard occNoAttacker = occ & ~Bitboard::mask(to);

  // 王手している駒を取れるか調べる
  if (_isProtected<!black>(board, to, occ, occNoAttacker, king)) {
    return false;
  }

  // 玉が移動可能な箇所
  Bitboard movable = MoveTables::king(king);
  movable &= black ? ~board.getWOccupy() : ~board.getBOccupy();

  Piece piece = move.piece();
  if (move.promote()) {
    piece = piece.promote();
  }

  switch (piece) {
  case Piece::Pawn: // fall-through
  case Piece::Knight: {
    // do nothing
    break;
  }
  case Piece::Lance: {
    Bitboard route = black
      ? (MoveTables::blance(to, occ) & MoveTables::wlance(king, occ))
      : (MoveTables::wlance(to, occ) & MoveTables::blance(king, occ));
    if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
    Bitboard mask = black ? MoveTables::blance(to, occNoKing) : MoveTables::wlance(to, occNoKing);
    movable &= ~mask;
    occ |= mask;
    break;
  }
  case Piece::Silver: {
    Bitboard mask = black ? MoveTables::bsilver(to) : MoveTables::wsilver(to);
    movable &= ~mask;
    break;
  }
  case Piece::Gold: // fall-through
  case Piece::Tokin: // fall-through
  case Piece::ProLance: // fall-through
  case Piece::ProKnight: // fall-through
  case Piece::ProSilver: {
    Bitboard mask = black ? MoveTables::bgold(to) : MoveTables::wgold(to);
    movable &= ~mask;
    break;
  }
  case Piece::Bishop: {
    Bitboard route = MoveTables::bishop(to, occ) & MoveTables::bishop(king, occ);
    if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
    Bitboard mask = MoveTables::bishop(to, occNoKing);
    movable &= ~mask;
    occ |= mask;
    break;
  }
  case Piece::Rook: {
    Bitboard route = MoveTables::rook(to, occ) & MoveTables::rook(king, occ);
    if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
    Bitboard mask = MoveTables::rook(to, occNoKing);
    movable &= ~mask;
    occ |= mask;
    break;
  }
  case Piece::Horse: {
    Bitboard route = MoveTables::bishop(to, occ) & MoveTables::bishop(king, occ);
    if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
    Bitboard mask = MoveTables::horse(to, occNoKing);
    movable &= ~mask;
    occ |= mask;
    break;
  }
  case Piece::Dragon: {
    Bitboard route = MoveTables::rook(to, occ) & MoveTables::rook(king, occ);
    if (_isProtected<!black>(board, route, occ, occNoAttacker)) { return false; }
    Bitboard mask = MoveTables::dragon(to, occNoKing);
    movable &= ~mask;
    occ |= mask;
    break;
  }
  default:
    assert(false);
  }

  BB_EACH_OPE(pos, movable, {
      if (!_isProtected<black>(board, pos, occ, occNoAttacker, Position::Invalid)) {
        return false;
      }
  });

  return true;
}
template bool Mate::_isMate<true>(const Board&, const Move&);
template bool Mate::_isMate<false>(const Board&, const Move&);

template<bool black>
bool Mate::_mate1Ply(const Board& board) {
  // TODO: 開き王手の生成
  const auto& occ = board.getBOccupy() | board.getWOccupy();
  Bitboard movable = ~(black ? board.getBOccupy() : board.getWOccupy());
  const auto& king = black ? board.getWKingPosition() : board.getBKingPosition();

  // 成金が王手できる位置
  Bitboard bbtGold = movable & (black ? MoveTables::wgold(king) : MoveTables::bgold(king));

  // gold
  {
    // drop
    int handCount = black ? board.getBlackHand(Piece::Gold) : board.getWhiteHand(Piece::Gold);
    if (handCount) {
      Bitboard bb = bbtGold & ~occ;
      BB_EACH_OPE(to, bb,
        if (_isMate<black>(board, Move(Piece::Gold, to, false))) { return true; }
      );
    }

    // board
    Bitboard bb = (black ? board.getBGold() : board.getWGold())
      | (black ? board.getBTokin() : board.getWTokin())
      | (black ? board.getBProLance() : board.getWProLance())
      | (black ? board.getBProKnight() : board.getWProKnight())
      | (black ? board.getBProSilver() : board.getWProSilver());
    bb &= black ? AttackableTables::bgold(king) : AttackableTables::wgold(king);
    BB_EACH_OPE(from, bb,
      Bitboard bb2 = bbtGold & (black ? MoveTables::bgold(from) : MoveTables::wgold(from));
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Gold, from, to, false, false))) { return true; }
      });
    );
  }

  bbtGold &= (black ? Bitboard::BPromotable : Bitboard::WPromotable);

  // silver
  {
    Bitboard bbt = black ? MoveTables::wsilver(king) : MoveTables::bsilver(king);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Silver) : board.getWhiteHand(Piece::Silver);
    if (handCount) {
      Bitboard bb = bbt & ~occ;
      BB_EACH_OPE(to, bb,
        if (_isMate<black>(board, Move(Piece::Silver, to, false))) { return true; }
      );
    }

    // board
    Bitboard bb = black ? board.getBSilver() : board.getWSilver();
    bb &= black ? AttackableTables::bsilver(king) : AttackableTables::wsilver(king);
    bbt &= ~bbtGold;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = black ? MoveTables::bsilver(from) : MoveTables::wsilver(from);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Silver, from, to, false, false))) { return true; }
      });
      bb2 = bbtGold & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Silver, from, to, true, false))) { return true; }
      });
    );
  }

  // knight
  {
    // drop
    Position to1 = black ? king.safetyLeftDownKnight() : king.safetyLeftUpKnight();
    Position to2 = black ? king.safetyRightDownKnight() : king.safetyRightUpKnight();
    int handCount = black ? board.getBlackHand(Piece::Knight) : board.getWhiteHand(Piece::Knight);
    if (handCount) {
      if (to1.isValid() && !occ.check(to1)) {
        if (_isMate<black>(board, Move(Piece::Knight, to1, false))) { return true; }
      }
      if (to2.isValid() && !occ.check(to2)) {
        if (_isMate<black>(board, Move(Piece::Knight, to2, false))) { return true; }
      }
    }

    // board
    Bitboard bb = black ? board.getBKnight() : board.getWKnight();
    bb &= black ? AttackableTables::bknight(king) : AttackableTables::wknight(king);
    Bitboard bbt = Bitboard::mask(to1) | Bitboard::mask(to2);
    bbt &= movable;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = black ? MoveTables::bknight(from) : MoveTables::wknight(from);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Knight, from, to, false, false))) { return true; }
      });
      bb2 = bbtGold & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Knight, from, to, true, false))) { return true; }
      });
    );
  }

  // pawn
  {
    Position to = black ? king.safetyDown() : king.safetyUp();
    if (to.isValid()) {
      Bitboard bb = black ? board.getBPawn() : board.getWPawn();
      bb &= black ? AttackableTables::bpawn(king) : AttackableTables::wpawn(king);
      if (black) {
        bb.cheapRightShift(1);
      } else {
        bb.cheapLeftShift(1);
      }
      bb &= bbtGold | Bitboard::mask(to);
      bb &= movable;
      BB_EACH_OPE(to, bb,
        if (to.isPromotable<black>()) {
          if (_isMate<black>(board, Move(Piece::Pawn, black ? to.down() : to.up(), to, true, false))) { return true; }
        } else {
          if (_isMate<black>(board, Move(Piece::Pawn, black ? to.down() : to.up(), to, false, false))) { return true; }
        }
      );
    }
  }

  // lance
  {
    Bitboard bbt = black ? MoveTables::wlance(king, occ) : MoveTables::blance(king, occ);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Lance) : board.getWhiteHand(Piece::Lance);
    if (handCount) {
      Position to = black ? king.safetyDown() : king.safetyUp();
      if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_lance_drop_end; }
      if (_isMate<black>(board, Move(Piece::Lance, to, false))) { return true; }

      to = black ? to.safetyDown() : to.safetyUp();
      if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_lance_drop_end; }
      if (_isMate<black>(board, Move(Piece::Lance, to, false))) { return true; }

      to = black ? to.safetyDown() : to.safetyUp();
      if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_lance_drop_end; }
      if (_isMate<black>(board, Move(Piece::Lance, to, false))) { return true; }
    }
mate1ply_lance_drop_end:

    // board
    Bitboard bb = black ? board.getBLance() : board.getWLance();
    bb &= black ? AttackableTables::blance(king) : AttackableTables::wlance(king);
    bbt &= ~bbtGold;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = black ? MoveTables::blance(from, occ) : MoveTables::wlance(from, occ);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Lance, from, to, false, false))) { return true; }
      });
      bb2 = bbtGold & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Lance, from, to, true, false))) { return true; }
      });
    );
  }

  // 馬、竜が王手できる位置
  Bitboard bbtKing = movable & MoveTables::king(king);
  bbtKing &= (black ? Bitboard::BPromotable : Bitboard::WPromotable);

  // bishop
  {
    Bitboard bbt = MoveTables::bishop(king, occ);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Bishop) : board.getWhiteHand(Piece::Bishop);
    if (handCount) {
#define GEN_CHECK_DROP_BISHOP(dir) { \
Position to = black ? king.safety ## dir() : king.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_bishop_drop_end ## dir; } \
if (_isMate<black>(board, Move(Piece::Bishop, to, false))) { return true; } \
to = black ? to.safety ## dir() : to.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_bishop_drop_end ## dir; } \
if (_isMate<black>(board, Move(Piece::Bishop, to, false))) { return true; } \
mate1ply_bishop_drop_end ## dir: ; \
}
      GEN_CHECK_DROP_BISHOP(LeftUp);
      GEN_CHECK_DROP_BISHOP(LeftDown);
      GEN_CHECK_DROP_BISHOP(RightUp);
      GEN_CHECK_DROP_BISHOP(RightDown);
    }

    // board
    Bitboard bb = black ? board.getBBishop() : board.getWBishop();
    bb &= black ? AttackableTables::bbishop(king) : AttackableTables::wbishop(king);
    bbt &= ~bbtKing;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = MoveTables::bishop(from, occ);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        if (to.isPromotable<black>()) {
          if (_isMate<black>(board, Move(Piece::Bishop, from, to, true, false))) { return true; }
        } else {
          if (_isMate<black>(board, Move(Piece::Bishop, from, to, false, false))) { return true; }
        }
      });
      bb2 = bbtKing & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Bishop, from, to, true, false))) { return true; }
      });
    );
  }

  // rook
  {
    Bitboard bbt = MoveTables::rook(king, occ);
    bbt &= movable;

    // drop
    int handCount = black ? board.getBlackHand(Piece::Rook) : board.getWhiteHand(Piece::Rook);
    if (handCount) {
#define GEN_CHECK_DROP_ROOK(dir) { \
Position to = black ? king.safety ## dir() : king.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_rook_drop_end ## dir; } \
if (_isMate<black>(board, Move(Piece::Rook, to, false))) { return true; } \
to = black ? to.safety ## dir() : to.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_rook_drop_end ## dir; } \
if (_isMate<black>(board, Move(Piece::Rook, to, false))) { return true; } \
to = black ? to.safety ## dir() : to.safety ## dir(); \
if (!to.isValid() || !board.getBoardPiece(to).isEmpty()) { goto mate1ply_rook_drop_end ## dir; } \
if (_isMate<black>(board, Move(Piece::Rook, to, false))) { return true; } \
mate1ply_rook_drop_end ## dir: ; \
}
      GEN_CHECK_DROP_ROOK(Left);
      GEN_CHECK_DROP_ROOK(Right);
      GEN_CHECK_DROP_ROOK(Up);
      GEN_CHECK_DROP_ROOK(Down);
    }

    // board
    Bitboard bb = black ? board.getBRook() : board.getWRook();
    bbt &= ~bbtKing;
    BB_EACH_OPE(from, bb,
      Bitboard bbe = MoveTables::rook(from, occ);
      Bitboard bb2 = bbt & bbe;
      BB_EACH_OPE(to, bb2, {
        if (to.isPromotable<black>()) {
          if (_isMate<black>(board, Move(Piece::Rook, from, to, true, false))) { return true; }
        } else {
          if (_isMate<black>(board, Move(Piece::Rook, from, to, false, false))) { return true; }
        }
      });
      bb2 = bbtKing & bbe;
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Rook, from, to, true, false))) { return true; }
      });
    );
  }

  // horse
  {
    Bitboard bbt = MoveTables::horse(king, occ);
    bbt &= movable;
    Bitboard bb = black ? board.getBHorse() : board.getWHorse();
    bb &= AttackableTables::horse(king);
    BB_EACH_OPE(from, bb,
      Bitboard bb2 = bbt & MoveTables::horse(from, occ);
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Horse, from, to, false, false))) { return true; }
      });
    );
  }

  // dragon
  {
    Bitboard bbt = MoveTables::dragon(king, occ);
    bbt &= movable;
    Bitboard bb = black ? board.getBDragon() : board.getWDragon();
    BB_EACH_OPE(from, bb,
      Bitboard bb2 = bbt & MoveTables::dragon(from, occ);
      BB_EACH_OPE(to, bb2, {
        if (_isMate<black>(board, Move(Piece::Dragon, from, to, false, false))) { return true; }
      });
    );
  }

  return false;
}
template bool Mate::_mate1Ply<true>(const Board&);
template bool Mate::_mate1Ply<false>(const Board&);

inline
bool Mate::isIneffectiveEvasion(const Board& board, const Move& move, const Move& check, const Bitboard& occ) {
  assert(board.getBoardPiece(move.to()).isEmpty());

  if (move.piece() == Piece::King) {
    return false;
  }

  bool black = board.isBlack();
  Position king = black ? board.getBKingPosition() : board.getWKingPosition();
  if (check.piece() == Piece::Lance &&
      move.to() == black ? king.safetyUp(2) : king.safetyDown(2)) {
    return false;
  }

  if (black ? _isProtected<true>(board, move.to(), occ, occ, king)
      : _isProtected<false>(board, move.to(), occ, occ, king)) {
    return true;
  }

  return true;
}

bool Mate::evade(Tree& tree, const Move& check) {
  const Board& board = tree.getBoard();
  Moves& moves = tree.getMoves();
  Bitboard occ = board.getBOccupy() | board.getWOccupy();

  moves.clear();
  MoveGenerator::generateEvasion(board, moves);

  for (auto ite = tree.getBegin(); ite != tree.getEnd(); ) {
    const Move& move = *ite;
    bool cap = !board.getBoardPiece(move.to()).isEmpty();

    if (!cap && isIneffectiveEvasion(board, move, check, occ)) {
      ite = tree.getMoves().remove(ite);
      continue;
    }

    // 逆王手
    if (board.isCheck(move)) {
      return true;
    }

    Piece piece = move.piece();
    if (piece == Piece::Pawn) {
      tree.setSortValue(ite, 6);
    } else if (piece == Piece::Lance) {
      tree.setSortValue(ite, 5);
    } else if (piece == Piece::Knight) {
      tree.setSortValue(ite, 4);
    } else if (piece == Piece::Bishop) {
      tree.setSortValue(ite, 3);
    } else if (piece.hand()) {
      tree.setSortValue(ite, 2);
    } else if (piece != Piece::King) {
      tree.setSortValue(ite, 1);
    } else {
      tree.setSortValue(ite, 0);
    }

    ite++;
  }

  tree.sortAll();

  for (const auto& move : moves) {
    if (!tree.makeMoveFast(move)) {
      continue;
    }

    if (!mate1Ply(board)) {
      tree.unmakeMoveFast();
      return true;
    }

    tree.unmakeMoveFast();
  }

  return false;
}

bool Mate::mate3Ply(Tree& tree) {
  const Board& board = tree.getBoard();
  Moves& moves = tree.getMoves();
  bool black = board.isBlack();
  Bitboard occ = board.getBOccupy() | board.getWOccupy();

  moves.clear();
  MoveGenerator::generateCheckLight(board, moves);

  for (const auto& move : moves) {
#if 1 // ただで取られる手を除外
    Position to = move.to();
    Bitboard attacker = Bitboard::mask(to);
    Bitboard occWithAttacker = occ | attacker;
    Bitboard occNoAttacker = occ & ~attacker;
    if (black) {
      if (!_isProtected<true, false>(board, to, occWithAttacker, occNoAttacker, board.getBKingPosition()) &&
          _isProtected<false, false>(board, to, occWithAttacker, occNoAttacker, board.getWKingPosition())) {
        continue;
      }
    } else {
      if (!_isProtected<false, false>(board, to, occWithAttacker, occNoAttacker, board.getWKingPosition()) &&
          _isProtected<true, false>(board, to, occWithAttacker, occNoAttacker, board.getBKingPosition())) {
        continue;
      }
    }
#endif

    if (!tree.makeMoveFast(move)) {
      continue;
    }

    if (!evade(tree, move)) {
      tree.unmakeMoveFast();
      return true;
    }

    tree.unmakeMoveFast();
  }

  return false;
}

} // namespace sunfish
