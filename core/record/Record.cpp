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
  if (!board_.makeMoveStrict(mtemp)) {
    return false;
  }

  if (moves_.size() <= count_ || moves_[count_] != mtemp) {
    while (moves_.size() > count_) {
      moves_.pop_back();
    }
    moves_.push_back(mtemp);
  }
  count_++;

  return true;

}

/**
 * 1手進めます。
 */
bool Record::makeMove() {

  if (count_ >= moves_.size()) {
    return false;
  }

  Move mtemp = moves_[count_];
  if (!board_.makeMoveStrict(mtemp)) {
    return false;
  }
  count_++;

  return true;

}

/**
 * 1手戻します。
 */
bool Record::unmakeMove() {

  if (count_ == 0) {
    return false;
  }

  const Move& move = moves_[count_-1];
  if (board_.unmakeMove(move)) {
    count_--;
  } else {
    assert(false);
  }

  return true;

}

/**
 * 初期局面を取得します。
 */
Board Record::getInitialBoard() const {
  Board board = board_;
  int count = count_;

  while(count != 0) {
    const Move& move = moves_[count-1];
    board.unmakeMove(move);
    count--;
  }

  return board;
}

} // namespace sunfish
