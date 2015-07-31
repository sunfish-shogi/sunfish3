/* Piece.cpp
 *
 * Kubo Ryosuke
 */

#include "Piece.h"
#include <cstring>
#include <cctype>

namespace sunfish {

Piece Piece::parse(const char* str) {
  PIECE_EACH(piece) {
    if (strncmp(str, piece.toString(), 2) == 0) {
      return piece;
    }
  }
  return Piece::Empty;
}

Piece Piece::parseCsa(const char* str) {
  if (str[0] == '+' || str[0] == '-') {
    PIECE_EACH(piece) {
      if (strncmp(str, piece.toStringCsa(false), 3) == 0) {
        return piece;
      }
    }
  } else if (isalpha(str[0])) {
    PIECE_KIND_EACH(piece) {
      if (strncmp(str, piece.toStringCsa(true), 2) == 0) {
        return piece;
      }
    }
  }
  return Piece::Empty;
}

} // namespace sunfish
