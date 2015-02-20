/* MoveGenChecker.cpp
 * 
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "MoveGenChecker.h"
#include "../move/MoveGenerator.h"
#include "../record/Record.h"
#include "../util/Random.h"
#include "logger/Logger.h"
#include <algorithm>

namespace sunfish {

	template <bool black>
	void MoveGenChecker::generate(const Board& board, Moves& moves) const {
		POSITION_EACH(to) {
			POSITION_EACH(from) {
				Piece piece = board.getBoardPiece(from);
				if (piece.isEmpty()) {
					continue;
				}
				Move move1(piece.unpromote(), from, to, true);
				Move move2(piece, from, to, false);
				if (board.isValidMoveStrict(move1)) {
					moves.add(move1);
					if ((piece.kindOnly() == Piece::Silver ||
							 piece.kindOnly() == Piece::Knight ||
							 (piece.kindOnly() == Piece::Lance && to.isLanceSignficant<black>())) &&
							board.isValidMoveStrict(move2)) {
						moves.add(move2);
					}
				} else if (board.isValidMoveStrict(move2)) {
					moves.add(move2);
				}
			}

			HAND_EACH(from) {
				Move move(from, to);
				if (board.isValidMoveStrict(move)) {
					moves.add(move);
				}
			}
		}
	}

	bool MoveGenChecker::compare(Moves& moves1, Moves& moves2) {
		std::sort(moves1.begin(), moves1.end());
		std::sort(moves2.begin(), moves2.end());

		if (moves1.size() != moves2.size()) {
			return false;
		}
		
		for (int i = 0; i < moves1.size(); i++) {
			if (moves1[i] != moves2[i]) {
				return false;
			}
		}

		return true;
	}

	void MoveGenChecker::showMoves(const Moves& moves) const {
		for (int i = 0; i < moves.size(); i++) {
			std::cout << moves[i].toString() << ' ';
		}
		std::cout << '\n';
	}

	bool MoveGenChecker::check() {

		const int maxCount = 1000;
		Board board;
		board.init(Board::Handicap::Even);
		Record record(std::move(board));
		Random random;

		for (int cnt = 1; cnt <= maxCount; cnt++) {
			Moves moves1;
			Moves moves2;

			generate(record.getBoard(), moves1);
			MoveGenerator::generate(record.getBoard(), moves2);
			for (auto ite = moves2.begin(); ite != moves2.end();) {
				if (!record.getBoard().isValidMoveStrict(*ite)) {
					ite = moves2.remove(ite);
				} else {
					++ite;
				}
			}

			bool ok = compare(moves1, moves2);

			auto move = record.getMove();
			std::cout << cnt << ": " << (move.isEmpty() ? "********" : record.getMove().toString());
			if (ok) {
				std::cout << ": mached. (size=" << moves1.size() << ")\n";
			} else {
				std::cout << ": not mached!!!\n";
			}

			if (!ok) {
				std::cout << "*************** Error !!! ***************\n";
				std::cout << record.getBoard().toStringCsa();
				std::cout << "moves1\n";
				showMoves(moves1);
				std::cout << "moves2\n";
				showMoves(moves2);
				std::cout << "*****************************************\n";
				return false;
			}

			if (moves1.size() == 0) {
				std::cout << record.getBoard().toStringCsa();
				break;
			}

			unsigned index = random.getInt32(moves1.size());
			if (!record.makeMove(moves1[index])) {
				std::cout << "*************** Error !!! ***************\n";
				std::cout << "fatal error!!!\n";
				std::cout << "*****************************************\n";
				return false;
			}

		}

		return true;
	}

}

#endif // !defined(NDEBUG)
