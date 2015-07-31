/* Square.cpp
 *
 * Kubo Ryosuke
 */

#include "Square.h"
#include <sstream>

namespace sunfish {

std::string Square::toString() const {
  std::ostringstream oss;

  oss << (int)getFile() << (int)getRank();

  return oss.str();
}

Square Square::parse(const char* str) {
  if (str[0] >= '1' && str[0] <= '9' && str[1] >= '1' && str[1] <= '9') {
    return Square(str[0] - '0', str[1] - '0');
  }
  return Square::Invalid;
}

} // namespace sunfish
