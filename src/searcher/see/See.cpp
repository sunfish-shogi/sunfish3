/* See.cpp
 * 
 * Kubo Ryosuke
 */

#include "See.h"
#include "core/move/MoveGenerator.h"
#include "core/move/MoveTable.h"
#include "core/util/Data.h"
#include <algorithm>
#include <iostream>

namespace sunfish {

template <bool shallow, Direction dir, bool isFirst>
void See::generateAttacker(const Board& board, const Square& to, const Bitboard& occ, Attacker* dependOn, bool shortOnly) {

  // 斜め
  if (dir == Direction::LeftUp || dir == Direction::LeftDown ||
      dir == Direction::RightUp || dir == Direction::RightDown) {
    // 距離1
    if (isFirst) {
      auto from = (dir == Direction::LeftUp ? to.leftUp() :
                   dir == Direction::LeftDown ? to.leftDown() :
                   dir == Direction::RightUp ? to.rightUp() :
                   to.rightDown());
      auto piece = board.getBoardPiece(from);
      if (!piece.isEmpty()) {
        if (dir == Direction::LeftUp   ? (MovableTable[piece.index()] & MovableFlag::RIGHT_DOWN) :
            dir == Direction::LeftDown ? (MovableTable[piece.index()] & MovableFlag::RIGHT_UP) :
            dir == Direction::RightUp  ? (MovableTable[piece.index()] & MovableFlag::LEFT_DOWN) :
                                         (MovableTable[piece.index()] & MovableFlag::LEFT_UP)) {
          if (piece.isBlack()) {
            b_[bnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &b_[bnum_-1]); }
          } else {
            assert(piece.isWhite());
            w_[wnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &w_[wnum_-1]); }
          }
        }
        return;
      }
    }

    // 長い距離
    if (!shortOnly) {
      auto bb = (dir == Direction::LeftUp ? MoveTables::leftUp(to, occ) :
                 dir == Direction::LeftDown ? MoveTables::leftDown(to, occ) :
                 dir == Direction::RightUp ? MoveTables::rightUp(to, occ) :
                 MoveTables::rightDown(to, occ));
      bb &= occ;
      auto from = bb.pickFirst();
      if (from != Square::Invalid) {
        auto piece = board.getBoardPiece(from);
        if (dir == Direction::LeftUp   ? (LongMovableTable[piece.index()] & MovableFlag::RIGHT_DOWN) :
            dir == Direction::LeftDown ? (LongMovableTable[piece.index()] & MovableFlag::RIGHT_UP) :
            dir == Direction::RightUp  ? (LongMovableTable[piece.index()] & MovableFlag::LEFT_DOWN) :
                                         (LongMovableTable[piece.index()] & MovableFlag::LEFT_UP)) {
          if (piece.isBlack()) {
            b_[bnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &b_[bnum_-1]); }
          } else {
            assert(piece.isWhite());
            w_[wnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &w_[wnum_-1]); }
          }
        }
      }
    }
  }

  // 縦と横
  else if (dir == Direction::Up || dir == Direction::Down ||
           dir == Direction::Left || dir == Direction::Right) {
    // 距離1
    if (isFirst) {
      auto from = (dir == Direction::Up ? to.up() :
                   dir == Direction::Down ? to.down() :
                   dir == Direction::Left ? to.left() :
                   to.right());
      auto piece = board.getBoardPiece(from);
      if (!piece.isEmpty()) {
        if (dir == Direction::Up   ? (MovableTable[piece.index()] & MovableFlag::DOWN) :
            dir == Direction::Down ? (MovableTable[piece.index()] & MovableFlag::UP) :
            dir == Direction::Left ? (MovableTable[piece.index()] & MovableFlag::RIGHT) :
                                     (MovableTable[piece.index()] & MovableFlag::LEFT)) {
          if (piece.isBlack()) {
            b_[bnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &b_[bnum_-1]); }
          } else {
            assert(piece.isWhite());
            w_[wnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &w_[wnum_-1]); }
          }
        }
        return;
      }
    }

    // 長い距離
    if (!shortOnly) {
      auto bb = (dir == Direction::Up ? MoveTables::blance(to, occ) :
                 dir == Direction::Down ? MoveTables::wlance(to, occ) :
                 dir == Direction::Left ? MoveTables::left(to, occ) :
                 MoveTables::right(to, occ));
      bb &= occ;
      auto from = bb.pickFirst();
      if (from != Square::Invalid) {
        auto piece = board.getBoardPiece(from);
        if (dir == Direction::Up   ? (LongMovableTable[piece.index()] & MovableFlag::DOWN) :
            dir == Direction::Down ? (LongMovableTable[piece.index()] & MovableFlag::UP) :
            dir == Direction::Left ? (LongMovableTable[piece.index()] & MovableFlag::RIGHT) :
                                     (LongMovableTable[piece.index()] & MovableFlag::LEFT)) {
          if (piece.isBlack()) {
            b_[bnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &b_[bnum_-1]); }
          } else {
            assert(piece.isWhite());
            w_[wnum_++] = { dependOn, material::pieceExchange(piece), false };
            if (!shallow && !shortOnly) { generateAttackerR<false, dir>(board, from, occ, &w_[wnum_-1]); }
          }
        }
      }
    }
  }

}

template <bool black>
void See::generateKnightAttacker(const Board& board, const Square& from) {
  auto& num = black ? bnum_ : wnum_;
  auto list = black ? b_ : w_;

  auto piece = board.getBoardPiece(from);
  if ((black && piece == Piece::BKnight) || (!black && piece == Piece::WKnight)) {
    list[num++] = { nullptr, material::pieceExchange(piece), false };
    return;
  }
}

template <bool shallow>
void See::generateAttackers(const Board& board, const Move& move) {
  Square to = move.to();
  HSideType sideTypeH = to.sideTypeH();
  VSideType sideTypeV = to.sideTypeV();
  Bitboard occ = board.getBOccupy() | board.getWOccupy();
  Square exceptSq;
  Direction exceptDir;

  if (move.isHand()) {
    exceptSq = Square::Invalid;
    exceptDir = Direction::None;
  } else {
    auto from = move.from();
    exceptSq = from;
    exceptDir = to.dir(from);
    occ &= ~Bitboard(from);
  }

  bnum_ = 0;
  wnum_ = 0;

#define GEN(dirname, except, shortOnly) \
  if (!(except)) { \
    if (exceptDir != Direction::dirname) { \
      generateAttacker<shallow, Direction::dirname, true>(board, to, occ, nullptr, (shortOnly)); \
    } else { \
      generateAttackerR<shallow, Direction::dirname>(board, exceptSq, occ, nullptr); \
    } \
  }
#define GEN_HV(sideTypeH, sideTypeV) \
  GEN(Up, \
      sideTypeH == HSideType::Top, \
      sideTypeH == HSideType::Top2); \
  GEN(Down, \
      sideTypeH == HSideType::Bottom, \
      sideTypeH == HSideType::Bottom2); \
  GEN(Left, \
      sideTypeV == VSideType::Left, \
      sideTypeV == VSideType::Left2); \
  GEN(Right, \
      sideTypeV == VSideType::Right, \
      sideTypeV == VSideType::Right2); \
  GEN(LeftUp, \
      sideTypeH == HSideType::Top || sideTypeV == VSideType::Left, \
      sideTypeH == HSideType::Top2 || sideTypeV == VSideType::Left2); \
  GEN(RightUp, \
      sideTypeH == HSideType::Top || sideTypeV == VSideType::Right, \
      sideTypeH == HSideType::Top2 || sideTypeV == VSideType::Right2); \
  GEN(LeftDown, \
      sideTypeH == HSideType::Bottom || sideTypeV == VSideType::Left, \
      sideTypeH == HSideType::Bottom2 || sideTypeV == VSideType::Left2); \
  GEN(RightDown, \
      sideTypeH == HSideType::Bottom || sideTypeV == VSideType::Right, \
      sideTypeH == HSideType::Bottom2 || sideTypeV == VSideType::Right2); \
  if (sideTypeH != HSideType::Bottom && sideTypeH != HSideType::Bottom2) { \
    if (exceptDir != Direction::LeftDownKnight && sideTypeV != VSideType::Left) { \
      generateKnightAttacker<true>(board, to.left().down(2)); \
    } \
    if (exceptDir != Direction::RightDownKnight && sideTypeV != VSideType::Right) { \
      generateKnightAttacker<true>(board, to.right().down(2)); \
    } \
  } \
  if (sideTypeH != HSideType::Top && sideTypeH != HSideType::Top2) { \
    if (exceptDir != Direction::LeftUpKnight && sideTypeV != VSideType::Left) { \
      generateKnightAttacker<false>(board, to.left().up(2)); \
    } \
    if (exceptDir != Direction::RightUpKnight && sideTypeV != VSideType::Right) { \
      generateKnightAttacker<false>(board, to.right().up(2)); \
    } \
  }
#define GEN_H(sideTypeH) \
  switch (sideTypeV) { \
  case VSideType::None: \
    GEN_HV(sideTypeH, VSideType::None); \
    break; \
  case VSideType::Left: \
    GEN_HV(sideTypeH, VSideType::Left); \
    break; \
  case VSideType::Right: \
    GEN_HV(sideTypeH, VSideType::Right); \
    break; \
  case VSideType::Left2: \
    GEN_HV(sideTypeH, VSideType::Left2); \
    break; \
  case VSideType::Right2: \
    GEN_HV(sideTypeH, VSideType::Right2); \
    break; \
  }

  switch (sideTypeH) {
  case HSideType::None:
    GEN_H(HSideType::None);
    break;
  case HSideType::Top:
    GEN_H(HSideType::Top);
    break;
  case HSideType::Bottom:
    GEN_H(HSideType::Bottom);
    break;
  case HSideType::Top2:
    GEN_H(HSideType::Top2);
    break;
  case HSideType::Bottom2:
    GEN_H(HSideType::Bottom2);
    break;
  }

#undef GEN_H
#undef GEN_HV
#undef GEN

  assert(bnum_ < (int)(sizeof(b_) / sizeof(b_[0])) - 1);
  assert(wnum_ < (int)(sizeof(w_) / sizeof(w_[0])) - 1);
  Attacker dummyAttacker;
  dummyAttacker.value = Value::Inf;

  bref_[bnum_] = &dummyAttacker;
  for (int i = bnum_ - 1; i >= 0; i--) {
    AttackerRef tmp = bref_[i] = &b_[i];
    Value value = b_[i].value;
    int j = i + 1;
    for (; bref_[j]->value < value; j++) {
      bref_[j-1] = bref_[j];
    }
    bref_[j-1] = tmp;
  }

  wref_[wnum_] = &dummyAttacker;
  for (int i = wnum_ - 1; i >= 0; i--) {
    AttackerRef tmp = wref_[i] = &w_[i];
    Value value = w_[i].value;
    int j = i + 1;
    for (; wref_[j]->value < value; j++) {
      wref_[j-1] = wref_[j];
    }
    wref_[j-1] = tmp;
  }

}

Value See::search(bool black, Value value, Value alpha, Value beta) {

  auto ref = black ? bref_ : wref_;
  int num = black ? bnum_ : wnum_;

  for (int i = 0; i < num; i++) {
    auto att = ref[i];
    if (!att->used && (att->dependOn == nullptr || att->dependOn->used)) {
      if (value - att->value >= beta) { return beta; }
      att->used = true;
      auto result = Value::max(0, value - search(!black, att->value, -beta+value, -alpha+value));
      att->used = false;
      return result;
    }
  }

  return 0;

}

template <bool shallow>
Value See::search(const Board& board, const Move& move, Value alpha, Value beta) {

  assert(beta <= Value::PieceInf);
  assert(alpha >= -Value::PieceInf);

  // 取った駒の価値
  Value captured = material::pieceExchange(board.getBoardPiece(move.to()));

  // 動かした駒の価値
  Piece piece = move.piece();
  if (move.promote()) {
    piece = piece.unpromote();
  }
  Value attacker = material::pieceExchange(piece);

  if (captured <= alpha) {
    return alpha;
  }

  if (captured - attacker >= beta) {
    return beta;
  }

  // 移動可能な駒を列挙する。
  generateAttackers<shallow>(board, move);

  return captured - search(!board.isBlack(), attacker, -beta+captured, -alpha+captured);

}
template Value See::search<true>(const Board&, const Move&, Value, Value);
template Value See::search<false>(const Board&, const Move&, Value, Value);

} // namespace sunfish
