/* Tree.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TREE__
#define __SUNFISH_TREE__

#include "Pv.h"
#include "../eval/Evaluator.h"
#include "core/move/Moves.h"
#include <cassert>

namespace sunfish {

	namespace _GenPhase {
		enum Type {
			Prior,
			Capture,
			NoCapture,
			Drop,
			TacticalOnly,
			CaptureOnly,
			End,
		};
	}
	typedef _GenPhase::Type GenPhase;

	typedef TempMoves<4> PriorMoves;

	class Tree {
	public:

		static const int StackSize = 64;

	private:

		struct Node {
			Move move;
			Moves moves;
			PriorMoves priorMoves;
			GenPhase genPhase;
			Moves::iterator ite;
			bool checking;
			Pv pv;
			ValuePair valuePair;
		};

		/** stack */
		Node _stack[StackSize];

		/** ルート局面からの手数 */
		int _ply;

		/** 局面 */
		Board _board;

		/** ソートキー */
		int32_t _sortValues[1024];

	public:

		void init(const Board& board, const Evaluator& eval) {
			_ply = 0;
			_board.init(board);
#ifndef NDEBUG
			_board.validate();
#endif
			_stack[0].valuePair = eval.evaluate(board);
			_stack[0].checking = _board.isChecking();
			_stack[0].pv.init();
		}

		int getPly() const {
			return _ply;
		}

		bool isStackFull() const {
			return _ply == StackSize - 1;
		}

		Moves& getMoves() {
			return _stack[_ply].moves;
		}

		GenPhase& getGenPhase() {
			return _stack[_ply].genPhase;
		}

		bool isRecapture() const {
			if (_ply == 0) {
				return false;
			}
			const auto& m0 = *(_stack[_ply-1].ite - 1);
			const auto& m1 = *(_stack[_ply].ite - 1);
			return m0.to() == m1.to() && m0.isCapturing();
		}

		Moves::iterator getNext() {
			return _stack[_ply].ite;
		}

		Moves::iterator getPrevious() {
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
			return _stack[_ply].ite - 1;
		}

		Moves::iterator getBegin() {
			return _stack[_ply].moves.begin();
		}

		Moves::iterator getEnd() {
			return _stack[_ply].moves.end();
		}

		Moves::iterator selectNextMove() {
			assert(_stack[_ply].ite != _stack[_ply].moves.end());
			return _stack[_ply].ite++;
		}

		Moves::iterator remove(Moves::iterator ite) {
			if (ite < _stack[_ply].ite) {
				_stack[_ply].ite--; // 前方を削除する場合はイテレータを繰り下げる。
			}
			return _stack[_ply].moves.remove(ite);
		}

		void removeAfter(const Moves::iterator ite) {
			return _stack[_ply].moves.removeAfter(ite);
		}

		int getIndexByIterator(const Moves::iterator ite) const {
			return (int)(ite - _stack[_ply].moves.begin());
		}

		int getIndexByMove(const Move& move) const {
			const auto& moves = _stack[_ply].moves;
			for (int i = 0; i < moves.size(); i++) {
				if (moves[i].equals(move)) {
					return i;
				}
			}
			return -1;
		}

		void setSortValue(const Moves::iterator ite, int32_t value) {
			auto index = getIndexByIterator(ite);
			_sortValues[index] = value;
		}

		int32_t getSortValue(const Moves::iterator ite) {
			auto index = getIndexByIterator(ite);
			return _sortValues[index];
		}

		void setSortValues(const int32_t* sortValues) {
			unsigned size = _stack[_ply].moves.size();
			memcpy(_sortValues, sortValues, sizeof(int32_t) * size);
		}

		void sort(const Moves::iterator begin);

		void sortAll() {
			sort(_stack[_ply].moves.begin());
		}

		void sortAfterCurrent() {
			sort(_stack[_ply].ite);
		}

		PriorMoves& getPriorMoves() {
			return _stack[_ply].priorMoves;
		}

		bool isChecking() {
			return _stack[_ply].checking;
		}

		const Board& getBoard() const {
			return _board;
		}

		void initGenPhase(GenPhase phase = GenPhase::Prior) {
			auto& node = _stack[_ply];
			node.moves.clear();
			node.genPhase = phase;
			node.ite = node.moves.begin();
			node.priorMoves.clear();
		}

		void resetGenPhase() {
			auto& node = _stack[_ply];
			node.genPhase = GenPhase::End;
			node.ite = node.moves.begin();
		}

		Value getValue() {
			auto& node = _stack[_ply];
			return node.valuePair.value();
		}

		bool makeMove(Move& move, const Evaluator& eval) {
			if (_board.makeMove(move)) {
				_ply++;
				auto& curr = _stack[_ply];
				auto& prev = _stack[_ply-1];
				curr.move = move;
				curr.valuePair = eval.evaluateDiff(_board, prev.valuePair, move);
				curr.checking = _board.isChecking();
				curr.pv.init();
				return true;
			}
			return false;
		}

		void unmakeMove(const Move& move) {
			_ply--;
			_board.unmakeMove(move);
		}

		void makeNullMove() {
			_board.makeNullMove();
			_ply++;
			_stack[_ply].move.setEmpty();
			_stack[_ply].valuePair = _stack[_ply-1].valuePair;
			_stack[_ply].checking = false;
			assert(!_board.isChecking());
			_stack[_ply].pv.init();
		}

		void unmakeNullMove() {
			_ply--;
			_board.unmakeNullMove();
		}

		void updatePv() {
			auto& curr = _stack[_ply];
			auto& next = _stack[_ply+1];
			curr.pv.set(Move::empty(), next.pv);
		}

		void updatePv(const Move& move) {
			auto& curr = _stack[_ply];
			auto& next = _stack[_ply+1];
			curr.pv.set(move, next.pv);
		}

		const Pv& getPv() {
			auto& node = _stack[_ply];
			return node.pv;
		}

		const Pv& __debug__getNextPv() const {
			auto& next = _stack[_ply+1];
			return next.pv;
		}

		std::string __debug__getPath() const;

		bool __debug__matchPath(const char* path ) const;

	};

}

#endif // __SUNFISH_TREE__
