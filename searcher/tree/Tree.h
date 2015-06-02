/* Tree.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_TREE__
#define SUNFISH_TREE__

#include "PV.h"
#include "NodeStat.h"
#include "../eval/Evaluator.h"
#include "../shek/ShekTable.h"
#include "core/move/Moves.h"
#include "core/def.h"
#include <atomic>
#include <mutex>
#include <vector>
#include <cstdint>
#include <cassert>

namespace sunfish {

enum class GenPhase : int {
  Hash,
  Capture,
  History1,
  History2,
  Misc,
  CaptureOnly,
  End,
};

enum class RepStatus : int {
  None, Win, Lose, Draw
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

  static CONSTEXPR int StackSize = 64;
  static CONSTEXPR int InvalidId = -1;

private:

  struct Node {
    Move move;
    Moves moves;
    Moves histMoves;
    GenPhase genPhase;
    ExpStat expStat;
    Moves::iterator ite;
    int count;
    PV pv;
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
    Move nocap1;
    Move nocap2;
    Move excluded;
    bool isThroughPhase;
    bool checking;
    bool isHistorical;
  };

  struct CheckHist {
    bool check;
    uint64_t hash;
  };

  /** stack */
  Node stack_[StackSize];

  /** SHEK table */
  ShekTable shekTable_;

  /** 局面 */
  Board board_;

  /** ルート局面からの手数 */
  int ply_;

  /** ソートキー */
  int32_t sortValues_[1024];

  /** 開始局面からの王手履歴 */
  CheckHist checkHist_[1024];
  int checkHistCount_;

  std::mutex mutex_;

  struct Tlp {
    int treeId;
    int parentTreeId;
    int workerId;
    bool used;
    std::atomic<int> childCount;
    std::atomic<bool> shutdown;
    bool black;
    int depth;
    Value alpha;
    Value beta;
    Move best;
    Value standPat;
    NodeStat stat;
    bool improving;
  } tlp_;

  void clearStack();

  void fastCopy(Tree& parent);

public:

  Tree();

  void init(int id, const Board& board, Evaluator& eval, const std::vector<Move>& record);
  void release(const std::vector<Move>& record);

  int getPly() const {
    return ply_;
  }

  bool isStackFull() const {
    // killer や mate で判定を省略するため余裕を設ける。
    assert(ply_ <= StackSize - 8);
    return ply_ >= StackSize - 8;
  }

  Moves& getMoves() {
    return stack_[ply_].moves;
  }

  GenPhase& getGenPhase() {
    return stack_[ply_].genPhase;
  }

  bool isThroughPhase() const {
    return stack_[ply_].isThroughPhase;
  }

  void setThroughPhase(bool b) {
    stack_[ply_].isThroughPhase = b;
  }

  const Move& getFrontMove() const {
    assert(ply_ >= 1);
    return stack_[ply_].move;
  }

  const Move& getPreFrontMove() const {
    assert(ply_ >= 2);
    return stack_[ply_-1].move;
  }

  bool isRecapture(const Move& move) const {
    assert(ply_ >= 1);
    const auto& fmove = stack_[ply_].move;
    return !fmove.isEmpty() && fmove.to() == move.to() &&
      (fmove.isCapturing() || (fmove.promote() && fmove.piece() != Piece::Silver));
  }

  bool isRecaptureOnFrontier() const {
    assert(ply_ >= 2);
    const auto& move = stack_[ply_].move;
    const auto& fmove = stack_[ply_-1].move;
    return !fmove.isEmpty() && fmove.to() == move.to() &&
      (fmove.isCapturing() || (fmove.promote() && fmove.piece() != Piece::Silver));
  }

  Moves::iterator getNextMove() {
    return stack_[ply_].ite;
  }

  Moves::iterator getCurrentMove() {
    assert(stack_[ply_].ite != stack_[ply_].moves.begin());
    return stack_[ply_].ite - 1;
  }

  Moves::iterator getBegin() {
    return stack_[ply_].moves.begin();
  }

  Moves::iterator getEnd() {
    return stack_[ply_].moves.end();
  }

  Moves::iterator selectFirstMove() {
    return stack_[ply_].ite = stack_[ply_].moves.begin();
  }

  Moves::iterator selectNextMove() {
    assert(stack_[ply_].ite != stack_[ply_].moves.end());
    return stack_[ply_].ite++;
  }

  Moves::iterator selectPreviousMove() {
    assert(stack_[ply_].ite != stack_[ply_].moves.begin());
    return stack_[ply_].ite--;
  }

  Moves::iterator addMove(const Move& move) {
    stack_[ply_].moves.add(move);
    return stack_[ply_].moves.end() - 1;
  }

  void rejectPreviousMove() {
    assert(stack_[ply_].ite != stack_[ply_].moves.begin());
    stack_[ply_].ite--;
    stack_[ply_].moves.removeStable(stack_[ply_].ite);
  }

  void removeAfter(const Moves::iterator ite) {
    return stack_[ply_].moves.removeAfter(ite);
  }

  int getIndexByIterator(const Moves::iterator ite) const {
    return (int)(ite - stack_[ply_].moves.begin());
  }

  int getIndexByMove(const Move& move) const {
    const auto& moves = stack_[ply_].moves;
    for (int i = 0; i < moves.size(); i++) {
      if (moves[i].equals(move)) {
        return i;
      }
    }
    return -1;
  }

  void setSortValue(const Moves::iterator ite, int32_t value) {
    auto index = getIndexByIterator(ite);
    sortValues_[index] = value;
  }

  int32_t getSortValue(const Moves::iterator ite) {
    auto index = getIndexByIterator(ite);
    return sortValues_[index];
  }

  void setSortValues(const int32_t* sortValues) {
    unsigned size = stack_[ply_].moves.size();
    memcpy(sortValues_, sortValues, sizeof(int32_t) * size);
  }

  void sort(const Moves::iterator begin);

  void sortAll() {
    sort(stack_[ply_].moves.begin());
  }

  void sortAfterCurrent(int offset = 0) {
    sort(stack_[ply_].ite + offset);
  }

  bool isChecking() {
    return stack_[ply_].checking;
  }

  bool isCheckingOnFrontier() {
    return ply_ >= 1 ? stack_[ply_-1].checking : false;
  }

  const Board& getBoard() const {
    return board_;
  }

  void initGenPhase(GenPhase phase = GenPhase::Hash) {
    auto& node = stack_[ply_];
    node.moves.clear();
    node.histMoves.clear();
    node.genPhase = phase;
    node.expStat = 0x00;
    node.isThroughPhase = false;
    node.ite = node.moves.begin();
    node.count = 0;
    node.capture1 = Move::empty();
    node.capture2 = Move::empty();
    node.cvalue1 = -Value::Inf;
    node.cvalue2 = -Value::Inf;
  }

  void resetGenPhase() {
    auto& node = stack_[ply_];
    node.genPhase = GenPhase::End;
    node.expStat = 0x00;
    node.isThroughPhase = false;
    node.ite = node.moves.begin();
    node.count = 0;
    node.capture1 = Move::empty();
    node.capture2 = Move::empty();
    node.cvalue1 = -Value::Inf;
    node.cvalue2 = -Value::Inf;
  }

  Value getValue() const {
    auto& node = stack_[ply_];
    return node.valuePair.value();
  }

  Value getFrontValue() const {
    auto& node = stack_[ply_-1];
    return node.valuePair.value();
  }

  Value getPrefrontValue() const {
    auto& node = stack_[ply_-2];
    return node.valuePair.value();
  }

  const ValuePair& getValuePair() const {
    auto& node = stack_[ply_];
    return node.valuePair;
  }

  bool hasPrefrontierNode() const {
    return ply_ >= 2;
  }

  template <bool positionalOnly = false>
  Value estimate(const Move& move, Evaluator& eval) const {
    return eval.estimate<positionalOnly>(board_, move);
  }

  bool makeMove(Evaluator& eval) {
    assert(stack_[ply_].ite != stack_[ply_].moves.begin());
    // frontier node
    auto& front = stack_[ply_];
    // move
    Move& move = *(front.ite-1);
    move.unsetCaptured();
    // SHEK
    shekTable_.set(board_);
    // check history
    checkHist_[checkHistCount_].check = front.checking;
    checkHist_[checkHistCount_].hash = board_.getHash();
    checkHistCount_++;
    bool checking = board_.isCheck(move);
    // make move
    if (!board_.makeMove(move)) {
      shekTable_.unset(board_);
      checkHistCount_--;
      return false;
    }
    ply_++;
    // current node
    auto& curr = stack_[ply_];
    curr.move = move;
    curr.checking = checking;
    assert(checking == board_.isChecking());
    curr.pv.init();
    curr.valuePair = eval.evaluateDiff(board_, front.valuePair, move);
    // child node
    auto& child = stack_[ply_+1];
    child.killer1 = Move::empty();
    child.killer2 = Move::empty();
    child.nocap1 = Move::empty();
    child.nocap2 = Move::empty();
    child.excluded = Move::empty();
    child.isHistorical = false;
    return true;
  }

  void unmakeMove() {
    auto& curr = stack_[ply_];
    ply_--;
    assert(stack_[ply_].ite != stack_[ply_].moves.begin());
    board_.unmakeMove(curr.move);
    shekTable_.unset(board_);
    checkHistCount_--;
  }

  void makeNullMove() {
    // check history
    checkHist_[checkHistCount_].check = false;
    checkHist_[checkHistCount_].hash = board_.getHash();
    checkHistCount_++;
    // make move
    board_.makeNullMove();
    ply_++;
    // current node
    auto& curr = stack_[ply_];
    curr.move.setEmpty();
    curr.checking = false;
    assert(!board_.isChecking());
    curr.pv.init();
    // frontier node
    auto& front = stack_[ply_-1];
    curr.valuePair = front.valuePair;
    // child node
    auto& child = stack_[ply_+1];
    child.killer1 = Move::empty();
    child.killer2 = Move::empty();
    child.nocap1 = Move::empty();
    child.nocap2 = Move::empty();
    child.excluded = Move::empty();
    child.isHistorical = false;
  }

  void unmakeNullMove() {
    ply_--;
    board_.unmakeNullMove();
    checkHistCount_--;
  }

  bool makeMoveFast(const Move& move) {
    Move mtemp = move;
    mtemp.unsetCaptured();
    if (board_.makeMove(mtemp)) {
      ply_++;
      auto& curr = stack_[ply_];
      curr.move = mtemp;
      return true;
    }
    return false;
  }

  void unmakeMoveFast() {
    auto& curr = stack_[ply_];
    ply_--;
    board_.unmakeMove(curr.move);
  }

  void updatePV(int depth) {
    auto& curr = stack_[ply_];
    auto& next = stack_[ply_+1];
    auto& move = *getCurrentMove();
    curr.pv.set(move, depth, next.pv);
  }

  void updatePV(int depth, Tree& child) {
    auto& curr = stack_[ply_];
    auto& next = child.stack_[ply_+1];
    auto& move = *child.getCurrentMove();
    curr.pv.set(move, depth, next.pv);
  }

  void updatePVNull(int depth) {
    auto& curr = stack_[ply_];
    auto& next = stack_[ply_+1];
    curr.pv.set(Move::empty(), depth, next.pv);
  }

  const PV& getPV() const {
    auto& node = stack_[ply_];
    return node.pv;
  }

  ShekTable& getShekTable() {
    return shekTable_;
  }

  ShekStat checkShek() const {
    return shekTable_.check(board_);
  }

  Node& getCurrentNode() {
    return stack_[ply_];
  }

  const Node& getCurrentNode() const {
    return stack_[ply_];
  }

  Node& getChildNode() {
    return stack_[ply_+1];
  }

  const Node& getChildNode() const {
    return stack_[ply_+1];
  }

  bool isPriorMove(const Move& move) const {
    auto& curr = stack_[ply_];
    return curr.hash == move ||
      curr.nocap1 == move || curr.nocap2 == move;
  }

  void setHash(const Move& move) {
    auto& curr = stack_[ply_];
    curr.hash = move;
  }

  const Move& getHash() const {
    auto& curr = stack_[ply_];
    return curr.hash;
  }

  const Move& getKiller1() const {
    auto& curr = stack_[ply_];
    return curr.killer1;
  }

  const Move& getKiller2() const {
    auto& curr = stack_[ply_];
    return curr.killer2;
  }

  Value getKiller1Value() const {
    auto& curr = stack_[ply_];
    return curr.kvalue1;
  }

  Value getKiller2Value() const {
    auto& curr = stack_[ply_];
    return curr.kvalue2;
  }

  const Move& getCapture1() const {
    auto& curr = stack_[ply_];
    return curr.capture1;
  }

  const Move& getCapture2() const {
    auto& curr = stack_[ply_];
    return curr.capture2;
  }

  Value getCapture1Value() const {
    auto& curr = stack_[ply_];
    return curr.cvalue1;
  }

  Value getCapture2Value() const {
    auto& curr = stack_[ply_];
    return curr.cvalue2;
  }

  const Move& getExcluded() const {
    auto& curr = stack_[ply_];
    return curr.excluded;
  }

  void setExcluded(const Move& move) {
    auto& curr = stack_[ply_];
    curr.excluded = move;
  }

  const PV& debug__getNextPV() const {
    auto& next = stack_[ply_+1];
    return next.pv;
  }

  RepStatus getCheckRepStatus() const;

  std::string debug__getPath() const;

  bool debug__matchPath(const char* path ) const;

  void use(int wid) {
    tlp_.used = true;
    tlp_.workerId = wid;
    tlp_.parentTreeId = InvalidId;
    tlp_.shutdown.store(false);
  }

  void use(Tree& parent, int wid) {
    fastCopy(parent);
    tlp_.used = true;
    tlp_.workerId = wid;
    tlp_.parentTreeId = parent.tlp_.treeId;
    tlp_.shutdown.store(false);
  }

  void unuse() {
    tlp_.used = false;
  }

  std::mutex& getMutex() {
    return mutex_;
  }

  Tlp& getTlp() {
    return tlp_;
  }

};

} // namespace sunfish

#endif // SUNFISH_TREE__
