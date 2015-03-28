/* Tree.cpp
 * 
 * Kubo Ryosuke
 */

#include "Tree.h"

#define ENABLE_SHEK_PRESET						1

namespace sunfish {

	void Tree::init(const Board& board, Evaluator& eval, const std::vector<Move>& record) {
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

		Board tmpBoard = board;
		for (int i = (int)record.size()-1; i >= 0; i--) {
#if ENABLE_SHEK_PRESET
			bool ok = tmpBoard.unmakeMove(record[i]);
			assert(ok);
			_shekTable.set(tmpBoard);
#endif
			_checkHist[i].check = tmpBoard.isChecking();
			_checkHist[i].hash = tmpBoard.getHash();
		}
		_checkHistCount = record.size();
	}

	void Tree::release(const std::vector<Move>& record) {
		// SHEK
		Board board = _board;
		for (int i = (int)record.size()-1; i >= 0; i--) {
#if ENABLE_SHEK_PRESET
			bool ok = board.unmakeMove(record[i]);
			assert(ok);
			_shekTable.unset(board);
#endif
		}

#ifndef NDEBUG
		// SHEK のテーブルが元に戻っているかチェックする。
		if (!_shekTable.isAllCleared()) {
			std::cout << "SHEK table has some pending record." << std::endl;
		}
#endif
	}

	void Tree::sort(const Moves::iterator begin) {
		auto& moves = _stack[_ply].moves;
		auto& values = _sortValues;
		auto beginIndex = begin - moves.begin();
		auto endIndex = moves.size();

		assert(beginIndex >= 0);
		assert(endIndex >= beginIndex);

		values[endIndex] = INT_MIN;
		for (int i = (int)endIndex - 2; i >= beginIndex; i--) {
			auto tmove = moves[i];
			auto tvalue = _sortValues[i];
			int j = i + 1;
    	for (; _sortValues[j] > tvalue; j++) {
				moves[j-1] = moves[j];
				values[j-1] = values[j];
    	}
			moves[j-1] = tmove;
			values[j-1] = tvalue;
		}
	}

	RepStatus Tree::getCheckRepStatus() const {
		uint64_t hash = _board.getHash();
		bool checkSelf = true;
		bool checkEnemy = _stack[_ply].checking;
		bool isSelf = true;
		for (int i = _checkHistCount-1; i >= 0; i--) {
			if (hash == _checkHist[i].hash) {
				if (checkEnemy) {
					return RepStatus::Win;
				} else if (checkSelf) {
					return RepStatus::Lose;
				} else {
					return RepStatus::Draw;
				}
			} else {
				if (isSelf) {
					checkSelf = checkSelf & _checkHist[i].check;
				} else {
					checkEnemy = checkEnemy & _checkHist[i].check;
				}
			}
			isSelf = !isSelf;
		}
		return RepStatus::None;
	}

	std::string Tree::__debug__getPath() const {
		std::ostringstream oss;
		bool isFirst = true;
		for (int ply = 0; ply < _ply; ply++) {
			const auto& move = *(_stack[ply].ite - 1);
			bool black = (_ply - ply) % 2 == 0 ? _board.isBlack() : !_board.isBlack();
			if (isFirst) {
				isFirst = false;
			} else {
				oss << ' ';
			}
			oss << move.toStringCsa(black);
		}
		return oss.str();
	}

	bool Tree::__debug__matchPath(const char* path) const {
		return __debug__getPath() == path;
	}

}
