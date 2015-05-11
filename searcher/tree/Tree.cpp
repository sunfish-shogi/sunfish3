/* Tree.cpp
 * 
 * Kubo Ryosuke
 */

#include "Tree.h"
#include "logger/Logger.h"

#define ENABLE_SHEK_PRESET 1

namespace sunfish {

Tree::Tree() : _ply(0), _checkHistCount(0) {
  _shekTable.init();
}

void Tree::init(int id, const Board& board, Evaluator& eval, const std::vector<Move>& record) {
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

  _tlp.treeId = id;
  _tlp.used = false;
}

void Tree::release(const std::vector<Move>& record) {
  clearStack();

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
    Loggers::warning << "SHEK table has some pending record.";
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

void Tree::clearStack() {
  while (_ply >= 1) {
    auto& curr = _stack[_ply];
    _ply--;
    if (!curr.move.isEmpty()) {
      _board.unmakeMove(curr.move);
      _shekTable.unset(_board);
    } else {
      _board.unmakeNullMove();
    }
  }
}

void Tree::fastCopy(Tree& parent) {
  clearStack();

  for (int ply = 1; ply <= parent._ply; ply++) {
    Move move = parent._stack[ply].move;
    if (!move.isEmpty()) {
      _shekTable.set(_board);
      _board.makeMove(move);
    } else {
      _board.makeNullMove();
    }
    _ply++;

    auto& curr = _stack[_ply];
    auto& parentCurr = parent._stack[_ply];
    curr.move = move;
    curr.checking = parentCurr.checking;
    curr.valuePair = parentCurr.valuePair;

    auto& child = _stack[_ply+1];
    auto& parentChild = parent._stack[_ply+1];
    child.killer1 = parentChild.killer1;
    child.killer2 = parentChild.killer2;
    child.nocap1 = parentChild.nocap1;
    child.nocap2 = parentChild.nocap2;
  }

  _checkHistCount = parent._checkHistCount;
  for (int i = 0; i < _checkHistCount; i++) {
    _checkHist[i].check = parent._checkHist[i].check;
    _checkHist[i].hash =  parent._checkHist[i].hash;
  }

  assert(parent._ply == _ply);
  assert(parent._board.getHash() == _board.getHash());
}

} // namespace sunfish
