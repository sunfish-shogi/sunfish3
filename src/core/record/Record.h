/* Record.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_RECORD__
#define SUNFISH_RECORD__

#include "../board/Board.h"
#include "../move/Move.h"
#include <vector>
#include <string>

namespace sunfish {

struct RecordInfo {
  std::string title;
  std::string blackName;
  std::string whiteName;
  int timeLimitHour;
  int timeLimitMinutes;
  int timeLimitReadoff;
};

class Record {
private:

  Board board_;
  std::vector<Move> moves_;
  unsigned count_;

public:

  Record() : count_(0) {
  }
  Record(const Board& board) : board_(board), count_(0) {
  }
  Record(Board&& board) : board_(std::move(board)), count_(0) {
  }

  void init(const Board& board) {
    board_ = board;
    moves_.clear();
    moves_.shrink_to_fit();
    count_ = 0;
  }
  void init(Board::Handicap handicap) {
    board_.init(handicap);
    moves_.clear();
    moves_.shrink_to_fit();
    count_ = 0;
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
    return board_;
  }

  /**
   * 初期局面を取得します。
   */
  Board getInitialBoard() const;

  /**
   * 先手番かチェックします。
   */
  bool isBlack() const {
    return board_.isBlack();
  }

  /**
   * 先手番かチェックします。
   */
  bool isBlackAt(int i) const {
    return isBlack() ^ (((int)count_ - i) % 2 != 0);
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
    return moves_[i];
  }

  /**
   * 指し手を返します。
   */
  Move getMove() const {
    return count_ >= 1 ? getMoveAt(count_-1) : Move::empty();
  }

  /**
   * 次の指し手を返します。
   */
  Move getNextMove() const {
    return count_ < getTotalCount() ? getMoveAt(count_) : Move::empty();
  }

  /**
   * 総手数を返します。
   */
  unsigned getTotalCount() const {
    return (unsigned)moves_.size();
  }

  /**
   * 現在の手数を返します。
   */
  unsigned getCount() const {
    return count_;
  }

};

} // namespace sunfish

#endif //SUNFISH_RECORD__
