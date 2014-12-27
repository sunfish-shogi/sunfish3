/* Record.cpp
 *
 * Kubo Ryosuke
 */

#include "Record.h"

namespace sunfish {

	/**
	 * 指定した手で1手進めます。
	 */
	bool Record::makeMove(const Move& move) {

		Move mtemp = move;
		if (!_board.makeMoveStrict(mtemp)) {
			return false;
		}

		if (_moveStack.size() <= _count || _moveStack[_count] != mtemp) {
			while (_moveStack.size() > _count) {
				_moveStack.pop_back();
			}
			_moveStack.push_back(mtemp);
		}
		_count++;

		return true;

	}

	/**
	 * 1手進めます。
	 */
	bool Record::makeMove() {

		if (_count >= _moveStack.size()) {
			return false;
		}

		Move mtemp = _moveStack[_count];
		if (!_board.makeMoveStrict(mtemp)) {
			return false;
		}
		_count++;

		return true;

	}

	/**
	 * 1手戻します。
	 */
	bool Record::unmakeMove() {

		if (_count == 0) {
			return false;
		}

		const Move& move = _moveStack[_count-1];
		if (_board.unmakeMove(move)) {
			_count--;
		} else {
			assert(false);
		}

		return true;

	}

	/**
	 * 初期局面を取得します。
	 */
	Board Record::getInitialBoard() const {
		Board board = _board;
		int count = _count;

		while(count != 0) {
			const Move& move = _moveStack[count-1];
			board.unmakeMove(move);
			count--;
		}

		return std::move(board);
	}

}
