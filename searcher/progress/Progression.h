/* Progression.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_PROGRESSION__
#define SUNFISH_PROGRESSION__

#include "core/def.h"
#include "core/board/Board.h"

namespace sunfish {

class Progression {
private:

  Progression();

public:

  static CONSTEXPR int Scale = 100;

  static int evaluate(const Board& board);

};

} // namespace sunfish

#endif // SUNFISH_PROGRESSION__
