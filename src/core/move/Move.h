/* Move.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_MOVE__
#define SUNFISH_MOVE__

#include "../def.h"
#include "../base/Piece.h"
#include "../base/Square.h"
#include <cassert>
#include <climits>
#include <algorithm>

namespace sunfish {

class Board;

class Move {
public:

  static CONSTEXPR_CONST uint16_t S16_HAND   = 0x8000;
  static CONSTEXPR_CONST uint16_t S16_EMPTY  = 0xffff;
  static CONSTEXPR_CONST uint32_t S16_HAND_SHIFT = 7;

  static CONSTEXPR_CONST uint32_t FROM       = 0x0000007f;
  static CONSTEXPR_CONST uint32_t TO         = 0x00003f80;
  static CONSTEXPR_CONST uint32_t PROMOTE    = 0x00004000;
  static CONSTEXPR_CONST uint32_t PIECE      = 0x00078000;
  static CONSTEXPR_CONST uint32_t CAP        = 0x00f80000;
  static CONSTEXPR_CONST uint32_t UNUSED     = 0xff000000;
  static CONSTEXPR_CONST uint32_t EMPTY      = 0xffffffff;

  static CONSTEXPR_CONST uint32_t TO_SHIFT = 7;
  static CONSTEXPR_CONST uint32_t PIECE_SHIFT = 15;
  static CONSTEXPR_CONST uint32_t CAP_SHIFT = 19;

private:

  uint32_t move_;

public:

  static Move empty() {
    Move empty;
    empty.setEmpty();
    return empty;
  }

  Move() {
  }
  Move(const Piece& piece, const Square& from, const Square& to, bool promote, bool safe = true) {
    if (safe) {
      set(piece, from, to, promote);
    } else {
      setUnsafe(piece, from, to, promote);
    }
  }
  Move(const Piece& piece, const Square& to, bool safe = true) {
    if (safe) {
      set(piece, to);
    } else {
      setUnsafe(piece, to);
    }
  }

  // serialization
  static uint32_t serialize(const Move& obj) {
    return obj.move_ & (FROM | TO | PROMOTE | PIECE);
  }
  static Move deserialize(uint32_t value) {
    Move move;
    move.move_ = value;
    return move;
  }
  static uint16_t serialize16(const Move& obj);
  static Move deserialize16(uint16_t value, const Board& board);

  static void swap(Move& move1, Move& move2) {
    Move temp = move1;
    move1 = move2;
    move2 = temp;
  }

private:
  template<bool safe>
  void set_(const Piece& piece, const Square& from, const Square& to, bool promote) {
    assert(!piece.isEmpty());
    assert(piece.isUnpromoted() || !promote);
    move_ = (static_cast<uint32_t>(from.index()) + 1)
          | (static_cast<uint32_t>(to.index()) << TO_SHIFT)
          | (static_cast<uint32_t>(safe ? piece.kindOnly() : piece) << PIECE_SHIFT);
    if (promote) {
      move_ |= PROMOTE;
    }
  }
public:
  void set(const Piece& piece, const Square& from, const Square& to, bool promote) {
    set_<true>(piece, from, to, promote);
  }
  void setUnsafe(const Piece& piece, const Square& from, const Square& to, bool promote) {
    set_<false>(piece, from, to, promote);
  }

  // move from hand
private:
  template<bool safe>
  void set_(const Piece& piece, const Square& to) {
    assert(!piece.isEmpty());
    assert(!piece.isWhite());
    assert(piece.isUnpromoted());
    const Piece& hand = (safe ? piece.hand() : piece);
    move_ = (static_cast<uint32_t>(to.index()) << TO_SHIFT)
          | (static_cast<uint32_t>(hand) << PIECE_SHIFT);
  }
public:
  void set(const Piece& piece, const Square& to) {
    set_<true>(piece, to);
  }
  void setUnsafe(const Piece& piece, const Square& to) {
    set_<false>(piece, to);
  }

  void setEmpty() {
    move_ = EMPTY;
  }
  bool isEmpty() const {
    return move_ == EMPTY;
  }

  // setters
  void setFrom(const Square& from) {
    move_ = (move_ & ~FROM) | (static_cast<uint32_t>(from.index()) + 1);
  }
  void setTo(const Square& to) {
    move_ = (move_ & ~TO) | (static_cast<uint32_t>(to.index()) << TO_SHIFT);
  }
  void setPromote(bool enable = true) {
    if (enable) {
      move_ |= PROMOTE;
    } else {
      move_ &= ~PROMOTE;
    }
  }
  void setPiece(const Piece& piece) {
    move_ = (move_ & (~PIECE)) | (static_cast<uint32_t>(piece.kindOnly()) << PIECE_SHIFT);
  }
  void setPieceUnsafe(const Piece& piece) {
    move_ = (move_ & (~PIECE)) | (static_cast<uint32_t>(piece) << PIECE_SHIFT);
  }
  void setCaptured(const Piece& captured) {
    move_ = (move_ & (~CAP)) | (static_cast<uint32_t>(captured.kindOnly() + 1U) << CAP_SHIFT);
  }
  void setCapturedUnsafe(const Piece& captured) {
    move_ = (move_ & (~CAP)) | (static_cast<uint32_t>(captured + 1U) << CAP_SHIFT);
  }
  void unsetCaptured() {
    move_ = move_ & (~CAP);
  }

  // getters
  Square from() const {
    return (move_ & FROM) - 1;
  }
  Square to() const {
    return (move_ & TO) >> TO_SHIFT;
  }
  bool promote() const {
    return move_ & PROMOTE;
  }
  Piece piece() const {
    return (move_ & PIECE) >> PIECE_SHIFT;
  }
  Piece captured() const {
    uint32_t cap = move_ & CAP;
    return cap ? ((cap >> CAP_SHIFT) - 1L) : Piece::Empty;
  }
  bool isCapturing() const {
    return move_ & CAP;
  }
  bool isHand() const {
    return !(move_ & FROM);
  }

  // comparator
  bool equals(const Move& obj) const {
    return (move_ & ~(UNUSED | CAP)) == (obj.move_ & ~(UNUSED | CAP));
  }
  bool operator==(const Move& obj) const {
    return equals(obj);
  }
  bool operator!=(const Move& obj) const {
    return !equals(obj);
  }

  // cast operator
  operator uint32_t() const {
    return move_;
  }

  std::string toString() const;

  std::string toStringCsa(bool black) const;

};

} // namespace sunfish

#endif //SUNFISH_MOVE__
