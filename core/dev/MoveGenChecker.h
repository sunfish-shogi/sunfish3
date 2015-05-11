/* MoveGenChecker.h
 * 
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "../move/Moves.h"
#include "../board/Board.h"

namespace sunfish {

class MoveGenChecker {
private:

  template <bool black>
  void generate(const Board& board, Moves& moves) const;
  void generate(const Board& board, Moves& moves) const {
    if (board.isBlack()) {
      generate<true>(board, moves);
    } else {
      generate<false>(board, moves);
    }
  }
  bool compare(Moves& moves1, Moves& moves2);
  void showMoves(const Moves& moves) const;

public:

  bool check();

};

} // namespace sunfish

#endif // !defined(NDEBUG)
