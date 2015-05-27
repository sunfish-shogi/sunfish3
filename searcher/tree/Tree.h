/* Tree.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TREE__
#define __SUNFISH_TREE__

#include "Pv.h"
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
    Move nocap1;
    Move nocap2;
    Move excluded;
    bool isThroughPhase;
    bool checking;
  };

  struct CheckHist {
    bool check;
    uint64_t hash;
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

  /** 開始局面からの王手履歴 */
  CheckHist _checkHist[1024];
  int _checkHistCount;

  std::mutex _mutex;

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
  } _tlp;

  void clearStack();

  void fastCopy(Tree& parent);

public:

  Tree();

  void init(int id, const Board& board, Evaluator& eval, const std::vector<Move>& record);
  void release(const std::vector<Move>& record);

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

  const Move& getPreFrontMove() const {
    assert(_ply >= 2);
    return _stack[_ply-1].move;
  }

  bool isRecapture(const Move& move) const {
    assert(_ply >= 1);
    const auto& fmove = _stack[_ply].move;
    return !fmove.isEmpty() && fmove.to() == move.to() &&
      (fmove.isCapturing() || (fmove.promote() && fmove.piece() != Piece::Silver));
  }

  bool isRecaptureOnFrontier() const {
    assert(_ply >= 2);
    const auto& move = _stack[_ply].move;
    const auto& fmove = _stack[_ply-1].move;
    return !fmove.isEmpty() && fmove.to() == move.to() &&
      (fmove.isCapturing() || (fmove.promote() && fmove.piece() != Piece::Silver));
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

  void sortAfterCurrent(int offset = 0) {
    sort(_stack[_ply].ite + offset);
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
    auto& node = _stack[_ply];
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
    auto& node = _stack[_ply];
    return node.valuePair.value();
  }

  Value getFrontValue() const {
    auto& node = _stack[_ply-1];
    return node.valuePair.value();
  }

  Value getPrefrontValue() const {
    auto& node = _stack[_ply-2];
    return node.valuePair.value();
  }

  const ValuePair& getValuePair() const {
    auto& node = _stack[_ply];
    return node.valuePair;
  }

  bool hasPrefrontierNode() const {
    return _ply >= 2;
  }

  template <bool positionalOnly = false>
  Value estimate(const Move& move, Evaluator& eval) const {
    return eval.estimate<positionalOnly>(_board, move);
  }

  bool makeMove(Evaluator& eval) {
    assert(_stack[_ply].ite != _stack[_ply].moves.begin());
    // frontier node
    auto& front = _stack[_ply];
    // move
    Move& move = *(front.ite-1);
    move.unsetCaptured();
    // SHEK
    _shekTable.set(_board);
    // check history
    _checkHist[_checkHistCount].check = front.checking;
    _checkHist[_checkHistCount].hash = _board.getHash();
    _checkHistCount++;
    bool checking = _board.isCheck(move);
    // make move
    if (!_board.makeMove(move)) {
      _shekTable.unset(_board);
      _checkHistCount--;
      return false;
    }
    _ply++;
    // current node
    auto& curr = _stack[_ply];
    curr.move = move;
    curr.checking = checking;
    assert(checking == _board.isChecking());
    curr.pv.init();
    curr.valuePair = eval.evaluateDiff(_board, front.valuePair, move);
    // child node
    auto& child = _stack[_ply+1];
    child.killer1 = Move::empty();
    child.killer2 = Move::empty();
    child.nocap1 = Move::empty();
    child.nocap2 = Move::empty();
    child.excluded = Move::empty();
    return true;
  }

  void unmakeMove() {
    auto& curr = _stack[_ply];
    _ply--;
    assert(_stack[_ply].ite != _stack[_ply].moves.begin());
    _board.unmakeMove(curr.move);
    _shekTable.unset(_board);
    _checkHistCount--;
  }

  void makeNullMove() {
    // check history
    _checkHist[_checkHistCount].check = false;
    _checkHist[_checkHistCount].hash = _board.getHash();
    _checkHistCount++;
    // make move
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
    child.nocap1 = Move::empty();
    child.nocap2 = Move::empty();
    child.excluded = Move::empty();
  }

  void unmakeNullMove() {
    _ply--;
    _board.unmakeNullMove();
    _checkHistCount--;
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
    auto& move = *getCurrentMove();
    curr.pv.set(move, depth, next.pv);
  }

  void updatePv(int depth, Tree& child) {
    auto& curr = _stack[_ply];
    auto& next = child._stack[_ply+1];
    auto& move = *child.getCurrentMove();
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
      curr.nocap1 == move || curr.nocap2 == move;
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

  const Move& getExcluded() const {
    auto& curr = _stack[_ply];
    return curr.excluded;
  }

  void setExcluded(const Move& move) {
    auto& curr = _stack[_ply];
    curr.excluded = move;
  }

  const Pv& __debug__getNextPv() const {
    auto& next = _stack[_ply+1];
    return next.pv;
  }

  RepStatus getCheckRepStatus() const;

  std::string __debug__getPath() const;

  bool __debug__matchPath(const char* path ) const;

  void use(int wid) {
    _tlp.used = true;
    _tlp.workerId = wid;
    _tlp.parentTreeId = InvalidId;
    _tlp.shutdown.store(false);
  }

  void use(Tree& parent, int wid) {
    fastCopy(parent);
    _tlp.used = true;
    _tlp.workerId = wid;
    _tlp.parentTreeId = parent._tlp.treeId;
    _tlp.shutdown.store(false);
  }

  void unuse() {
    _tlp.used = false;
  }

  std::mutex& getMutex() {
    return _mutex;
  }

  Tlp& getTlp() {
    return _tlp;
  }

};

} // namespace sunfish

#endif // __SUNFISH_TREE__
