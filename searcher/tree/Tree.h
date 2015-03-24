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
#include <cstdint>
#include <cassert>

namespace sunfish {

	enum class GenPhase : int32_t {
		Hash,
		Capture,
		History1,
		History2,
		Misc,
		CaptureOnly,
		End,
	};

	using ExpStat = uint32_t;
	static CONSTEXPR uint32_t HashDone    = 0x01;
	static CONSTEXPR uint32_t Killer1Added = 0x02;
	static CONSTEXPR uint32_t Killer2Added = 0x04;
	static CONSTEXPR uint32_t Killer1Done = 0x08;
	static CONSTEXPR uint32_t Killer2Done = 0x10;
	static CONSTEXPR uint32_t Capture1Done = 0x20;
	static CONSTEXPR uint32_t Capture2Done = 0x40;

	class Tree {
	public:

		static const int StackSize = 64;

	private:

		struct Node {
			Move move;
			Moves moves;
			GenPhase genPhase;
			ExpStat expStat;
			bool isThroughPhase;
			Moves::iterator ite;
			bool checking;
			Pv pv;
			ValuePair valuePair;
			Move hash;
			Move killer1;
			Move killer2;
			Value kvalue1;
			Value kvalue2;
			Move capture1;
			Move capture2;
			Value cvalue1;
			Value cvalue2;
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
			// killer や mate で判定を省略するため余裕を設ける。
			assert(_ply <= StackSize - 8);
			return _ply >= StackSize - 8;
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

		const Move& getFrontMove() const {
			assert(_ply >= 1);
			return _stack[_ply].move;
		}

		bool isRecapture() const {
			assert(_ply >= 1);
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
			const auto& m0 = _stack[_ply].move;
			const auto& m1 = *(_stack[_ply].ite - 1);
			return !m0.isEmpty() && m0.to() == m1.to() &&
				(m0.isCapturing() || (m0.promote() && m0.piece() != Piece::Silver));
		}

		Moves::iterator getNextMove() {
			return _stack[_ply].ite;
		}

		Moves::iterator getCurrentMove() {
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
			return _stack[_ply].ite - 1;
		}

		Moves::iterator getBegin() {
			return _stack[_ply].moves.begin();
		}

		Moves::iterator getEnd() {
			return _stack[_ply].moves.end();
		}

		Moves::iterator selectFirstMove() {
			return _stack[_ply].ite = _stack[_ply].moves.begin();
		}

		Moves::iterator selectNextMove() {
			assert(_stack[_ply].ite != _stack[_ply].moves.end());
			return _stack[_ply].ite++;
		}

		Moves::iterator selectPreviousMove() {
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
			return _stack[_ply].ite--;
		}

		Moves::iterator addMove(const Move& move) {
			_stack[_ply].moves.add(move);
			return _stack[_ply].moves.end() - 1;
		}

		void rejectPreviousMove() {
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
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
			node.expStat = 0x00;
			node.isThroughPhase = false;
			node.ite = node.moves.begin();
			node.capture1 = Move::empty();
			node.capture2 = Move::empty();
		}

		void resetGenPhase() {
			auto& node = _stack[_ply];
			node.genPhase = GenPhase::End;
			node.expStat = 0x00;
			node.isThroughPhase = false;
			node.ite = node.moves.begin();
			node.capture1 = Move::empty();
			node.capture2 = Move::empty();
		}

		Value getValue() const {
			auto& node = _stack[_ply];
			return node.valuePair.value();
		}

		const ValuePair& getValuePair() const {
			auto& node = _stack[_ply];
			return node.valuePair;
		}

		template <bool positionalOnly = false>
		Value estimate(const Move& move, Evaluator& eval) const {
			return eval.estimate<positionalOnly>(_board, move);
		}

		bool makeMove(Evaluator& eval) {
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
			Move& move = *(_stack[_ply].ite-1);
			move.unsetCaptured();
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
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
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

		bool makeMoveFast(const Move& move) {
			Move mtemp = move;
			mtemp.unsetCaptured();
			if (_board.makeMove(mtemp)) {
				_ply++;
				auto& curr = _stack[_ply];
				curr.move = mtemp;
				return true;
			}
			return false;
		}

		void unmakeMoveFast() {
			auto& curr = _stack[_ply];
			_ply--;
			_board.unmakeMove(curr.move);
		}

		void updatePv(int depth) {
			auto& curr = _stack[_ply];
			auto& next = _stack[_ply+1];
			assert(_stack[_ply].ite != _stack[_ply].moves.begin());
			auto& move = *(_stack[_ply].ite-1);
			curr.pv.set(move, depth, next.pv);
		}

		void updatePvNull(int depth) {
			auto& curr = _stack[_ply];
			auto& next = _stack[_ply+1];
			curr.pv.set(Move::empty(), depth, next.pv);
		}

		const Pv& getPv() const {
			auto& node = _stack[_ply];
			return node.pv;
		}

		ShekTable& getShekTable() {
			return _shekTable;
		}

		ShekStat checkShek() const {
			return _shekTable.check(_board);
		}

		Node& getCurrentNode() {
			return _stack[_ply];
		}

		const Node& getCurrentNode() const {
			return _stack[_ply];
		}

		bool isPriorMove(const Move& move) const {
			auto& curr = _stack[_ply];
			return curr.hash == move ||
				curr.killer1 == move || curr.killer2 == move;
		}

		void setHash(const Move& move) {
			auto& curr = _stack[_ply];
			curr.hash = move;
		}

		const Move& getHash() const {
			auto& curr = _stack[_ply];
			return curr.hash;
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

		void setCapture1(const Move& move, const Value& value) {
			auto& curr = _stack[_ply];
			curr.capture1 = move;
			curr.cvalue1 = value;
		}

		void setCapture2(const Move& move, const Value& value) {
			auto& curr = _stack[_ply];
			curr.capture2 = move;
			curr.cvalue2 = value;
		}

		const Move& getCapture1() const {
			auto& curr = _stack[_ply];
			return curr.capture1;
		}

		const Move& getCapture2() const {
			auto& curr = _stack[_ply];
			return curr.capture2;
		}

		Value getCapture1Value() const {
			auto& curr = _stack[_ply];
			return curr.cvalue1;
		}

		Value getCapture2Value() const {
			auto& curr = _stack[_ply];
			return curr.cvalue2;
		}

		const Pv& __debug__getNextPv() const {
			auto& next = _stack[_ply+1];
			return next.pv;
		}

		std::string __debug__getPath() const;

		bool __debug__matchPath(const char* path ) const;

		const Move& __debug__getPreFrontMove() const {
			assert(_ply >= 2);
			return _stack[_ply-1].move;
		}

	};

}

#endif // __SUNFISH_TREE__
