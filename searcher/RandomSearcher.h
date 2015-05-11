/* RandomSearcher.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_RANDOMSEARCHER__
#define __SUNFISH_RANDOMSEARCHER__

#include "core/util/Random.h"
#include "core/board/Board.h"

namespace sunfish {

class RandomSearcher {
private:

  Random random;

public:

  /**
   * 指定した局面に対して探索を実行します。
   * @return {負けたいか中断された場合にfalseを返します。}
   */
  bool search(const Board& board, Move& move);

};

} // namespace sunfish

#endif //__SUNFISH_RANDOMSEARCHER__
