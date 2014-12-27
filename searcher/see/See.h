/* See.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SEE__
#define __SUNFISH_SEE__

#include "core/board/Board.h"
#include "../eval/Evaluator.h"

namespace sunfish {

	class See {
	private:

		Value _b[16];
		int _bnum;
		Value _w[16];
		int _wnum;

		template <bool black>
		void generateAttackers(const Evaluator& eval, const Board& board, const Position& to, const Bitboard& occ, const Bitboard& mask);

		template <bool black>
		Value search(int b, int w, Value value);

	public:

		Value search(const Evaluator& eval, const Board& board, const Move& move);

		void generateAttackers(const Evaluator& eval, const Board& board, const Move& move) {
			auto from = move.from();
			auto to = move.to();
			auto occ = board.getBOccupy() | board.getWOccupy();
			auto mask = ~Bitboard::mask(from);
			generateAttackers<true>(eval, board, to, occ, mask);
			generateAttackers<false>(eval, board, to, occ, mask);
		}

		const Value* getBlackList() const {
			return _b;
		}

		int getBlackNum() const {
			return _bnum;
		}

		const Value* getWhiteList() const {
			return _w;
		}

		int getWhiteNum() const {
			return _wnum;
		}

	};

}

#endif // __SUNFISH_SEE__
