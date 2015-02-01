/* Record.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_RECORD__
#define __SUNFISH_RECORD__

#include "../board/Board.h"
#include "../move/Move.h"
#include <vector>

namespace sunfish {

	class Record {
	private:

		Board _board;
		std::vector<Move> _moveStack;
		unsigned _count;

	public:

		Record() : _count(0) {
		}
		Record(const Board& board) : _board(board), _count(0) {
		}

		void init(const Board& board) {
			_board = board;
			_moveStack.clear();
			_count = 0;
		}
		void init(Board::Handicap handicap) {
			_board.init(handicap);
			_moveStack.clear();
			_count = 0;
		}

		/**
		 * 指定した手で1手進めます。
		 */
		bool makeMove(const Move& move);

		/**
		 * 1手進めます。
		 */
		bool makeMove();

		/**
		 * 1手戻します。
		 */
		bool unmakeMove();

		/**
		 * 現在の局面を取得します。
		 */
		const Board& getBoard() const {
			return _board;
		}

		/**
		 * 初期局面を取得します。
		 */
		Board getInitialBoard() const;

		/**
		 * 総手数を返します。
		 */
		int getSize() const {
			return (int)_moveStack.size();
		}

		/**
		 * 先手番かチェックします。
		 */
		bool isBlack() const {
			return _board.isBlack();
		}

		/**
		 * 先手番かチェックします。
		 */
		bool isBlackAt(int i) const {
			return isBlack() ^ (((int)_count - i) % 2 != 0);
		}

		/**
		 * 後手番かチェックします。
		 */
		bool isWhite() const {
			return !isBlack();
		}

		/**
		 * 後手番かチェックします。
		 */
		bool isWhiteAt(int i) const {
			return !isBlackAt(i);
		}

		/**
		 * 指し手を返します。
		 */
		Move getMoveAt(int i) const {
			return _moveStack[i];
		}

		/**
		 * 指し手を返します。
		 */
		Move getMove() const {
			return _count >= 1 ? getMoveAt(_count-1) : Move();
		}

	};

}

#endif //__SUNFISH_RECORD__
