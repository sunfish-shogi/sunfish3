/* Tree.cpp
 * 
 * Kubo Ryosuke
 */

#include "Tree.h"
#include "logger/Logger.h"

#define ENABLE_SHEK_PRESET 1

namespace sunfish {

Tree::Tree() : ply_(0), checkHistCount_(0) {
  shekTable_.init();
}

void Tree::init(int id, const Board& board, Evaluator& eval, const std::vector<Move>& record) {
  ply_ = 0;
  board_ = board;
#ifndef NDEBUG
  board_.validate();
#endif
  stack_[0].valuePair = eval.evaluate(board);
  stack_[0].checking = board_.isChecking();
  stack_[0].pv.init();
  stack_[0].killer1 = Move::empty();
  stack_[0].killer2 = Move::empty();
  stack_[1].killer1 = Move::empty();
  stack_[1].killer2 = Move::empty();

  Board tmpBoard = board;
  for (int i = (int)record.size()-1; i >= 0; i--) {
#if ENABLE_SHEK_PRESET
    bool ok = tmpBoard.unmakeMove(record[i]);
    assert(ok);
    shekTable_.set(tmpBoard);
#endif
    checkHist_[i].check = tmpBoard.isChecking();
    checkHist_[i].hash = tmpBoard.getHash();
  }
  checkHistCount_ = record.size();

  tlp_.treeId = id;
  tlp_.used = false;
}

void Tree::release(const std::vector<Move>& record) {
  clearStack();

  // SHEK
  Board board = board_;
  for (int i = (int)record.size()-1; i >= 0; i--) {
#if ENABLE_SHEK_PRESET
    bool ok = board.unmakeMove(record[i]);
    assert(ok);
    shekTable_.unset(board);
#endif
  }

#ifndef NDEBUG
  // SHEK のテーブルが元に戻っているかチェックする。
  if (!shekTable_.isAllCleared()) {
    Loggers::warning << "SHEK table has some pending record.";
  }
#endif
}

void Tree::sort(const Moves::iterator begin) {
  auto& moves = stack_[ply_].moves;
  auto& values = sortValues_;
  auto beginIndex = begin - moves.begin();
  auto endIndex = moves.size();

  assert(beginIndex >= 0);
  assert(endIndex >= beginIndex);

  values[endIndex] = INT_MIN;
  for (int i = (int)endIndex - 2; i >= beginIndex; i--) {
    auto tmove = moves[i];
    auto tvalue = sortValues_[i];
    int j = i + 1;
    for (; sortValues_[j] > tvalue; j++) {
      moves[j-1] = moves[j];
      values[j-1] = values[j];
    }
    moves[j-1] = tmove;
    values[j-1] = tvalue;
  }
}

RepStatus Tree::getCheckRepStatus() const {
  uint64_t hash = board_.getHash();
  bool checkSelf = true;
  bool checkEnemy = stack_[ply_].checking;
  bool isSelf = true;
  for (int i = checkHistCount_-1; i >= 0; i--) {
    if (hash == checkHist_[i].hash) {
      if (checkEnemy) {
        return RepStatus::Win;
      } else if (checkSelf) {
        return RepStatus::Lose;
      } else {
        return RepStatus::Draw;
      }
    } else {
      if (isSelf) {
        checkSelf = checkSelf & checkHist_[i].check;
      } else {
        checkEnemy = checkEnemy & checkHist_[i].check;
      }
    }
    isSelf = !isSelf;
  }
  return RepStatus::None;
}

std::string Tree::debug__getPath() const {
  std::ostringstream oss;
  bool isFirst = true;
  for (int ply = 0; ply < ply_; ply++) {
    const auto& move = *(stack_[ply].ite - 1);
    bool black = (ply_ - ply) % 2 == 0 ? board_.isBlack() : !board_.isBlack();
    if (isFirst) {
      isFirst = false;
    } else {
      oss << ' ';
    }
    oss << move.toStringCsa(black);
  }
  return oss.str();
}

bool Tree::debug__matchPath(const char* path) const {
  return debug__getPath() == path;
}

void Tree::clearStack() {
  while (ply_ >= 1) {
    auto& curr = stack_[ply_];
    ply_--;
    if (!curr.move.isEmpty()) {
      board_.unmakeMove(curr.move);
      shekTable_.unset(board_);
    } else {
      board_.unmakeNullMove();
    }
  }
}

void Tree::fastCopy(Tree& parent) {
  clearStack();

  for (int ply = 1; ply <= parent.ply_; ply++) {
    Move move = parent.stack_[ply].move;
    if (!move.isEmpty()) {
      shekTable_.set(board_);
      board_.makeMove(move);
    } else {
      board_.makeNullMove();
    }
    ply_++;

    auto& curr = stack_[ply_];
    auto& parentCurr = parent.stack_[ply_];
    curr.move = move;
    curr.checking = parentCurr.checking;
    curr.valuePair = parentCurr.valuePair;

    auto& child = stack_[ply_+1];
    auto& parentChild = parent.stack_[ply_+1];
    child.killer1 = parentChild.killer1;
    child.killer2 = parentChild.killer2;
    child.nocap1 = parentChild.nocap1;
    child.nocap2 = parentChild.nocap2;
  }

  checkHistCount_ = parent.checkHistCount_;
  for (int i = 0; i < checkHistCount_; i++) {
    checkHist_[i].check = parent.checkHist_[i].check;
    checkHist_[i].hash =  parent.checkHist_[i].hash;
  }

  assert(parent.ply_ == ply_);
  assert(parent.board_.getHash() == board_.getHash());
}

} // namespace sunfish
