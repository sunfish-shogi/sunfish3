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
			Hash,
			Capture,
			History1,
			History2,
			Misc,
			CaptureOnly,
			End,
		};
	}
	typedef _GenPhase::Type GenPhase;

	class Tree {
	public:

		static const int StackSize = 64;

	private:

		struct Node {
			Move move;
			Moves moves;
			GenPhase genPhase;
			bool isThroughPhase;
			Moves::iterator ite;
			bool checking;
			Pv pv;
			ValuePair valuePair;
			Move hash1;
			Move hash2;
			Move killer1;
			Move killer2;
			Value kvalue1;
			Value kvalue2;
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

		~Tree() {
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
			_stack[0].killer1 = Move::empty();
			_stack[0].killer2 = Move::empty();
			_stack[1].killer1 = Move::empty();
			_stack[1].killer2 = Move::empty();
		}

		int getPly() const {
			return _ply;
		}

		bool isStackFull() const {
			// killer の初期化で 1 つ先のインデクスにアクセスするため 1 つ余裕を設ける。
			assert(_ply <= StackSize - 2);
			return _ply >= StackSize - 2;
		}

		Moves& getMoves() {
			return _stack[_ply].moves;
		}

		GenPhase& getGenPhase() {
			return _stack[_ply].genPhase;
		}

		bool isThroughPhase() const {
			return _stack[_ply].isThroughPhase;
		}

		void setThroughPhase(bool b) {
			_stack[_ply].isThroughPhase = b;
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

		Moves::iterator addMove(const Move& move) {
			_stack[_ply].moves.add(move);
			return _stack[_ply].moves.end() - 1;
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

		bool isChecking() {
			return _stack[_ply].checking;
		}

		bool isCheckingOnFrontier() {
			return _ply >= 1 ? _stack[_ply-1].checking : false;
		}

		const Board& getBoard() const {
			return _board;
		}

		void initGenPhase(GenPhase phase = GenPhase::Hash) {
			auto& node = _stack[_ply];
			node.moves.clear();
			node.genPhase = phase;
			node.isThroughPhase = false;
			node.ite = node.moves.begin();
		}

		void resetGenPhase() {
			auto& node = _stack[_ply];
			node.genPhase = GenPhase::End;
			node.isThroughPhase = false;
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
			// try make move
			if (_board.makeMove(move)) {
				_ply++;
				// current node
				auto& curr = _stack[_ply];
				curr.move = move;
				curr.checking = checking;
				assert(checking == _board.isChecking());
				curr.pv.init();
				// frontier node
				auto& front = _stack[_ply-1];
				curr.valuePair = eval.evaluateDiff(_board, front.valuePair, move);
				// child node
				auto& child = _stack[_ply+1];
				child.killer1 = Move::empty();
				child.killer2 = Move::empty();
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
			// current node
			auto& curr = _stack[_ply];
			curr.move.setEmpty();
			curr.checking = false;
			assert(!_board.isChecking());
			curr.pv.init();
			// frontier node
			auto& front = _stack[_ply-1];
			curr.valuePair = front.valuePair;
			// child node
			auto& child = _stack[_ply+1];
			child.killer1 = Move::empty();
			child.killer2 = Move::empty();
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

		bool isPriorMove(const Move& move) const {
			auto& curr = _stack[_ply];
			return curr.hash1 == move || curr.hash2 == move ||
				curr.killer1 == move || curr.killer2 == move;
		}

		void setHash1(const Move& move) {
			auto& curr = _stack[_ply];
			curr.hash1 = move;
		}

		void setHash2(const Move& move) {
			auto& curr = _stack[_ply];
			curr.hash2 = move;
		}

		const Move& getHash1() const {
			auto& curr = _stack[_ply];
			return curr.hash1;
		}

		const Move& getHash2() const {
			auto& curr = _stack[_ply];
			return curr.hash2;
		}

		void addKiller(const Move& killer, const Value& value) {
			auto& curr = _stack[_ply];
			if (curr.killer1 != killer) {
				curr.killer2 = curr.killer1;
				curr.kvalue2 = curr.kvalue1;
				curr.killer1 = killer;
				curr.kvalue1 = value;
			}
		}

		const Move& getKiller1() const {
			auto& curr = _stack[_ply];
			return curr.killer1;
		}

		const Move& getKiller2() const {
			auto& curr = _stack[_ply];
			return curr.killer2;
		}

		Value getKiller1Value() const {
			auto& curr = _stack[_ply];
			return curr.kvalue1;
		}

		Value getKiller2Value() const {
			auto& curr = _stack[_ply];
			return curr.kvalue2;
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
