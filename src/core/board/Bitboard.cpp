/* Bitboard.cpp
 *
 * Kubo Ryosuke
 */

#include "Bitboard.h"

namespace sunfish {

std::string Bitboard::toString2D() const {
  std::ostringstream oss;
  SQUARE_EACH_RD(sq) {
    oss << (check(sq) ? '1' : '0');
    if (sq.getFile() == 1) {
      oss << '\n';
    }
  }
  return oss.str();
}

} // namespace sunfish
