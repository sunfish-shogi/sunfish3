/* HandSet.h
 *
 * Kubo Ryosuke
 */

#ifndef sunfish_HandSet__
#define sunfish_HandSet__

#include "ShekStat.h"
#include "core/board/Hand.h"
#include "core/util/StringUtil.h"
#include <cstdint>
#include <string>

namespace sunfish {

class HandSet {
private:

  uint64_t data_;

  void setPiece(const Hand& hand, Piece piece, int shift) {
    data_ <<= shift;
    data_ |= ((uint64_t)1U << hand.getUnsafe(piece)) - 1;
  }

public:

  HandSet() = default;

  explicit HandSet(const Hand& hand) {
    set(hand);
  }

  void set(const Hand& hand) {
    data_ = (uint64_t)0LLU;
    setPiece(hand, Piece::Pawn, 0);
    setPiece(hand, Piece::Lance, 4);
    setPiece(hand, Piece::Knight, 4);
    setPiece(hand, Piece::Silver, 4);
    setPiece(hand, Piece::Gold, 4);
    setPiece(hand, Piece::Bishop, 2);
    setPiece(hand, Piece::Rook, 2);
  }

  ShekStat compareTo(const HandSet& h, bool selfTurn) const {
    if (data_ == h.data_) {
      return ShekStat::Equal;
    }
    bool sup = data_ & (~h.data_);
    bool inf = (~data_) & h.data_;
    if (sup && !inf) {
      return selfTurn ? ShekStat::Superior : ShekStat::Inferior;
    } else if (!sup && inf) {
      return selfTurn ? ShekStat::Inferior : ShekStat::Superior;
    } else {
      return ShekStat::None;
    }
  }

  std::string toString() const {
    return StringUtil::stringify(data_);
  }
};

} // namespace sunfish

#endif // sunfish_HandSet__
