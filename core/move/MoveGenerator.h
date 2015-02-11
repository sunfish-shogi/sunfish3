/* MoveGenerator.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MOVEGENERATOR__
#define __SUNFISH_MOVEGENERATOR__

#include "../board/Board.h"
#include "Moves.h"
#include <cassert>

namespace sunfish {

	/**
	 * MoveGenerator
	 * 指し手生成
	 */
	class MoveGenerator {
	private:

		template <bool black, bool exceptNonEffectiveProm, bool exceptNonPromAll, bool exceptKingMasking>
		static void _generateOnBoard(const Board& board, Moves& moves, const Bitboard& mask);
		template <bool black>
		static void _generateDrop(const Board& board, Moves& moves, const Bitboard& mask);
		template <bool black>
		static void _generateEvasion(const Board& board, Moves& moves);
		template <bool black>
		static void _generateEvasionShort(const Board& board, Moves& moves, const Bitboard& attacker);
		template <bool black>
		static void _generateKing(const Board& board, Moves& moves);

	public:

		/**
		 * 全ての合法手を生成します。
		 * 打ち歩詰めや王手放置の手を含む可能性があります。
		 */
		static void generate(const Board& board, Moves& moves) {
			if (!board.isChecking()) {
				generateCap(board, moves);
				generateNoCap(board, moves);
				generateDrop(board, moves);
			} else {
				generateEvasion(board, moves);
			}
		}

		/**
		 * 駒を取る手を生成します。
		 * 王手がかかっていない場合のみに使用します。
		 * 王手放置の手を含む可能性があります。
		 */
		static void generateCap(const Board& board, Moves& moves) {
			assert(!board.isChecking());
			if (board.isBlack()) {
				_generateOnBoard<true, true, false, false>(board, moves, board.getWOccupy());
			} else {
				_generateOnBoard<false, true, false, false>(board, moves, board.getBOccupy());
			}
		}

		/**
		 * 駒を取らずに成る手のみを生成します。
		 * 王手がかかっていない場合のみに使用します。
		 * 王手放置の手を含む可能性があります。
		 */
		static void generateProm(const Board& board, Moves& moves) {
			assert(!board.isChecking());
			// TODO: 移動元が成れない位置の場合に移動先をマスクして高速化
			Bitboard occ = board.getBOccupy() | board.getWOccupy();
			if (board.isBlack()) {
				_generateOnBoard<true, true, true, false>(board, moves, ~occ);
			} else {
				_generateOnBoard<false, true, true, false>(board, moves, ~occ);
			}
		}

		/**
		 * 駒を取らない移動手を生成します。
		 * 王手がかかっていない場合のみに使用します。
		 * 王手放置の手を含む可能性があります。
		 */
		static void generateNoCap(const Board& board, Moves& moves) {
			assert(!board.isChecking());
			Bitboard occ = board.getBOccupy() | board.getWOccupy();
			if (board.isBlack()) {
				_generateOnBoard<true, true, false, false>(board, moves, ~occ);
			} else {
				_generateOnBoard<false, true, false, false>(board, moves, ~occ);
			}
		}

		/**
		 * 持ち駒を打つ手を生成します。
		 * 王手がかかっていない場合のみに使用します。
		 * 打ち歩詰めや王手放置の手を含む可能性があります。
		 */
		static void generateDrop(const Board& board, Moves& moves) {
			assert(!board.isChecking());
			Bitboard occ = board.getBOccupy() | board.getWOccupy();
			if (board.isBlack()) {
				_generateDrop<true>(board, moves, ~occ);
			} else {
				_generateDrop<false>(board, moves, ~occ);
			}
		}

		/**
		 * 王手を防ぐ手を生成します。
		 * 王手がかかっている場合のみに使用します。
		 * 打ち歩詰めや王手放置の手を含む可能性があります。
		 */
		static void generateEvasion(const Board& board, Moves& moves) {
			assert(board.isChecking());
			if (board.isBlack()) {
				_generateEvasion<true>(board, moves);
			} else {
				_generateEvasion<false>(board, moves);
			}
		}

	};

}

#endif //__SUNFISH_MOVEGENERATOR__
