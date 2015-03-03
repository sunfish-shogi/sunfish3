/* Tree.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TREE__
#define __SUNFISH_TREE__

#include "Pv.h"
#include "../eval/Evaluator.h"
#include "../shek/ShekTable.h"
#include "core/move/Moves.h"
#include <cassert>

namespace sunfish {

	namespace _GenPhase {
		enum Type {
			Prior,
			Capture,
			NoCapture,
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

		/** SHEK table */
		ShekTable _shekTable;

		/** 局面 */
		Board _board;

		/** ルート局面からの手数 */
		int _ply;

		/** ソートキー */
		int32_t _sortValues[1024];

	public:

		Tree() : _ply(0) {
		}

		virtual ~Tree() {
		}

		void init(const Board& board, Evaluator& eval) {
			_ply = 0;
			_board = board;
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

		void rejectPreviousMove() {
			_stack[_ply].ite--;
			_stack[_ply].moves.removeStable(_stack[_ply].ite);
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

		bool isCheckingOnFrontier() {
			return _ply >= 1 ? _stack[_ply-1].checking : false;
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

		Value estimate(const Move& move, Evaluator& eval) {
			return eval.estimate(_board, move);
		}

		bool makeMove(Move move, Evaluator& eval) {
			_shekTable.set(_board);
			bool checking = _board.isCheck(move);
			if (_board.makeMove(move)) {
				_ply++;
				auto& curr = _stack[_ply];
				auto& front = _stack[_ply-1];
				curr.move = move;
				curr.valuePair = eval.evaluateDiff(_board, front.valuePair, move);
				curr.checking = checking;
				assert(checking == _board.isChecking());
				curr.pv.init();
				return true;
			}
			_shekTable.unset(_board);
			return false;
		}

		void unmakeMove() {
			auto& curr = _stack[_ply];
			_ply--;
			_board.unmakeMove(curr.move);
			_shekTable.unset(_board);
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

		bool makeMoveFast(Move move) {
			if (_board.makeMove(move)) {
				_ply++;
				auto& curr = _stack[_ply];
				curr.move = move;
				return true;
			}
			return false;
		}

		void unmakeMoveFast() {
			auto& curr = _stack[_ply];
			_ply--;
			_board.unmakeMove(curr.move);
		}

		void updatePv(const Move& move, int depth) {
			auto& curr = _stack[_ply];
			auto& next = _stack[_ply+1];
			curr.pv.set(move, depth, next.pv);
		}

		const Pv& getPv() {
			auto& node = _stack[_ply];
			return node.pv;
		}

		ShekTable& getShekTable() {
			return _shekTable;
		}

		ShekStat checkShek() const {
			return _shekTable.check(_board);
		}

		const Pv& __debug__getNextPv() const {
			auto& next = _stack[_ply+1];
			return next.pv;
		}

		std::string __debug__getPath() const;

		bool __debug__matchPath(const char* path ) const;

		const Move& __debug__getFrontMove() const {
			assert(_ply >= 1);
			auto& front = _stack[_ply];
			return front.move;
		}

	};

}

#endif // __SUNFISH_TREE__
