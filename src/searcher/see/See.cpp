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

template <bool black>
void See::generateKnightAttacker(const Board& board, const Square& from) {
  auto& num = black ? bnum_ : wnum_;
  auto list = black ? b_ : w_;

  auto piece = board.getBoardPiece(from);
  if ((black && piece == Piece::BKnight) || (!black && piece == Piece::WKnight)) {
    list[num++] = { material::pieceExchange(piece) };
    return;
  }
}

void See::generateAttackers(const Board& board, const Move& move) {
  Square to = move.to();
  HSideType sideTypeH = to.sideTypeH();
  VSideType sideTypeV = to.sideTypeV();
  Bitboard occ = board.getBOccupy() | board.getWOccupy();
  Direction exceptDir;

  if (move.isHand()) {
    exceptDir = Direction::None;
  } else {
    auto from = move.from();
    exceptDir = to.dir(from);
    occ &= ~Bitboard(from);
  }

  bnum_ = 0;
  wnum_ = 0;

  {
    Bitboard bb = MoveTables::rook(to, occ);
  }

  {
    Bitboard bb = MoveTables::bishop(to, occ);
  }

  if (sideTypeH != HSideType::Bottom && sideTypeH != HSideType::Bottom2) {
    if (exceptDir != Direction::LeftDownKnight && sideTypeV != VSideType::Left) {
      generateKnightAttacker<true>(board, to.left().down(2));
    }
    if (exceptDir != Direction::RightDownKnight && sideTypeV != VSideType::Right) {
      generateKnightAttacker<true>(board, to.right().down(2));
    }
  }

  if (sideTypeH != HSideType::Top && sideTypeH != HSideType::Top2) {
    if (exceptDir != Direction::LeftUpKnight && sideTypeV != VSideType::Left) {
      generateKnightAttacker<false>(board, to.left().up(2));
    }
    if (exceptDir != Direction::RightUpKnight && sideTypeV != VSideType::Right) {
      generateKnightAttacker<false>(board, to.right().up(2));
    }
  }

  assert(bnum_ < (int)(sizeof(b_) / sizeof(b_[0])) - 1);
  assert(wnum_ < (int)(sizeof(w_) / sizeof(w_[0])) - 1);

  b_[bnum_].value = Value::Inf;
  for (int i = bnum_ - 1; i >= 0; i--) {
    Attacker tmp = b_[i];
    int j = i + 1;
    for (; b_[j].value < tmp.value; j++) {
      b_[j-1] = b_[j];
    }
    b_[j-1] = tmp;
  }

  w_[wnum_].value = Value::Inf;
  for (int i = wnum_ - 1; i >= 0; i--) {
    Attacker tmp = w_[i];
    int j = i + 1;
    for (; w_[j].value < tmp.value; j++) {
      w_[j-1] = w_[j];
    }
    w_[j-1] = tmp;
  }

}

Value See::search(Value value, Value alpha, Value beta, Attacker* b, Attacker* w) {

  if (b->value != Value::Inf) {
    if (value - b->value >= beta) { return beta; }
    auto result = Value::max(0, value - search(b->value, -beta+value, -alpha+value, w, b+1));
    return result;
  }

  return 0;

}

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
  generateAttackers(board, move);

  Value value;
  if (!board.isBlack()) {
    value = search(attacker, -beta+captured, -alpha+captured, b_, w_);
  } else {
    value = search(attacker, -beta+captured, -alpha+captured, w_, b_);
  }

  return captured - value;

}

} // namespace sunfish
