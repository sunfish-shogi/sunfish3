/* TTMoves.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_HASHMOVE__
#define __SUNFISH_HASHMOVE__

#include "core/move/Move.h"

namespace sunfish {

	class TTMoves {
	private:

		Move _move1;
		Move _move2;

	public:

		TTMoves() {
			init();
		}

		TTMoves(const TTMoves& moveMove) {
			_move1 = moveMove._move1;
			_move2 = moveMove._move2;
		}

		void init() {
			_move1.setEmpty();
			_move2.setEmpty();
		}

		void update(const Move& move) {
			if (_move1 != move) {
				_move2 = _move1;
				_move1 = move;
			}
		}

		const Move& getMove1() const {
			return _move1;
		}

		void setMove1(const Move& move) {
			this->_move1 = move;
		}

		const Move& getMove2() const {
			return _move2;
		}

		void setMove2(const Move& move) {
			this->_move2 = move;
		}

		operator uint64_t() const {
			return ((uint64_t)(unsigned)_move1 << 32) + (uint64_t)(unsigned)_move2;
		}

	};

}

#endif // __SUNFISH_HASHMOVE__
