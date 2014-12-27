/* RandomSearcher.cpp
 * 
 * Kubo Ryosuke
 */

#include "RandomSearcher.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"

namespace sunfish {

	bool RandomSearcher::search(const Board& board, Move& move) {
		Moves moves;

		// generate all moves
		MoveGenerator::generate(board, moves);

		for (auto ite = moves.begin(); ite != moves.end(); ite++) {
			if (!board.isValidMove(*ite)) {
				ite = moves.remove(ite) - 1;
			}
		}

		if (moves.size() == 0) {
			return false;
		}

		// random select
		int r = random.getInt32() % moves.size();
		move = moves[r];
		return true;
	}

}
