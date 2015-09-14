/* Searcher.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_SEARCHER__
#define SUNFISH_SEARCHER__

#include "mate/Mate.h"
#include "mate/MateHistory.h"
#include "SearchInfo.h"
#include "eval/EvaluateTable.h"
#include "tree/Tree.h"
#include "history/History.h"
#include "tt/TT.h"
#include "time/TimeManager.h"
#include "core/record/Record.h"
#include "core/util/Timer.h"
#include "core/util/Random.h"
#include <algorithm>
#include <mutex>
#include <atomic>
#include <climits>

namespace sunfish {

class Tree;
struct Worker;

class Gains {
private:

  uint16_t gains_[Piece::Num][Square::N];

public:

  void clear() {
    memset(&gains_[0][0], 0, sizeof(gains_));
  }

  void update(const Move& move, Value gain) {
    assert(move.piece().index() < Piece::Num);
    assert(move.to().index() >= 0);
    assert(move.to().index() < Square::N);
    assert(gain.int32() < (1<<16));
    assert(gain.int32() > -(1<<16));
    uint16_t& ref = gains_[move.piece().index()][move.to().index()];
    ref = std::max(ref - 1, gain.int32());
  }

  Value get(const Move& move) {
    assert(move.piece().index() < Piece::Num);
    assert(move.to().index() >= 0);
    assert(move.to().index() < Square::N);
    Value g = gains_[move.piece().index()][move.to().index()];
    return g;
  }

};

class Searcher {
public:

  static CONSTEXPR_CONST int Depth1Ply = 8;

  /** 探索設定 */
  struct Config {
    int32_t maxDepth;
    int32_t treeSize;
    int32_t workerSize;
    float limitSeconds;
    bool enableLimit;
    bool enableTimeManagement;
    bool threadPooling;
    bool ponder;
    bool logging;
#if !defined(NLEARN)
    bool learning;
#endif
  };

  static const int DefaultMaxDepth = 7;

private:

  Config config_;
  SearchInfo info_;
  Timer timer_;

  /** tree */
  Tree* trees_;

  /** worker */
  Worker* workers_;

  /** 評価関数 */
  Evaluator eval_;

  /** history heuristic */
  History history_;

  /** transposition table */
  TT tt_;

  /** gains */
  Gains gains_;

  /** mate table */
  MateTable<18> mateTable_;

  /** mate history */
  MateHistory mateHistory_;

  /** values of child node of root node */
  int rootValues_[1024];

  int rootDepth_;

  std::mutex splitMutex_;

  /** 中断フラグ */
  std::atomic<bool> forceInterrupt_;

  /** 実行中フラグ */
  std::atomic<bool> isRunning_;

  /** 思考時間制御 */
  TimeManager timeManager_;

  /** record */
  std::vector<Move> record_;

  Random random_;

  /**
   * 設定の初期化
   */
  void initConfig() {
    config_.maxDepth = DefaultMaxDepth;
    config_.treeSize = 1;
    config_.workerSize = 1;
    config_.enableLimit = true;
    config_.limitSeconds = 10.0;
    config_.enableTimeManagement = true;
    config_.threadPooling = true;
    config_.ponder = false;
    config_.logging = true;
#if !defined(NLEARN)
    config_.learning = false;
#endif
  }

  /**
   * tree の確保
   */
  void allocateTrees();

  /**
   * worker の確保
   */
  void allocateWorkers();

  /**
   * tree の再確保
   */
  void reallocateTrees();

  /**
   * worker の再確保
   */
  void reallocateWorkers();

  /**
   * tree の解放
   */
  void releaseTrees();

  /**
   * worker の解放
   */
  void releaseWorkers();

  /**
   * worker の取得
   */
  Worker& getWorker(Tree& tree);

  void mergeInfo();

  /**
   * 前処理
   */
  void before(const Board& initialBoard, bool fastStart);

  /**
   * 後処理
   */
  void after();

  /**
   * 探索中断判定
   */
  bool isInterrupted(Tree& tree);

  /**
   * get see value
   */
  template <bool shallow>
  Value searchSee(const Board& board, const Move& move, Value alpha, Value beta);

  /**
   * sort moves by see
   */
  void sortSee(Tree& tree, int offset, Value standPat, Value alpha, bool enableKiller, bool estimate, bool exceptSmallCapture, bool isQuies);

  /**
   * except prior moves
   */
  void exceptPriorMoves(Tree& tree);

  /**
   * pick best move by history
   */
  bool pickOneHistory(Tree& tree);

  /**
   * sort moves by history
   */
  void sortHistory(Tree& tree);

  /**
   * update history
   */
  void updateHistory(Tree& tree, int depth, const Move& move);

  /**
   * get LMR depth
   */
  int getReductionDepth(bool improving, int depth, int moveCount, const Move& move, bool isNullWindow);

  /**
   * get next move
   */
  bool nextMove(Tree& tree);

  /**
   * get next move
   */
  bool nextMoveQuies(Tree& tree, int ply, Value standPat, Value alpha);

  /**
   * store PV-nodes to TT
   */
  void storePV(Tree& tree, const PV& pv, int ply);

  bool isNeedMateSearch(Tree& tree, bool black, int depth);

  void updateMateHistory(Tree& tree, bool black, bool mate);

  /**
   * quiesence search
   */
  Value qsearch(Tree& tree, bool black, int qply, Value alpha, Value beta);

  /**
   * update killer move
   */
  void updateKiller(Tree& tree, const Move& move);

  /**
   * nega-max search
   */
  Value search(Tree& tree, bool black, int depth, Value alpha, Value beta, NodeStat stat = NodeStat::Default);

  /**
   * split
   */
  bool split(Tree& parent, bool black, int depth, Value alpha, Value beta, Move best, Value standPat, NodeStat stat, bool improving);

  void searchTlp(Tree& tree);

  void shutdownSiblings(Tree& parent);

  /**
   * search on root node
   */
  Value searchRoot(Tree& tree, int depth, Value alpha, Value beta, Move& best,
      bool breakOnFailLow = false, bool forceFullWindow = false);

  /**
   * aspiration search
   * @return {負けたか中断された場合にfalseを返します。}
   */
  bool searchAsp(int depth, Move& best, Value baseAlpha, Value baseBeta, Value& valueRef);

  void showPV(int depth, const PV& pv, const Value& value);

  void showEndOfIterate(int depth);

  void generateMovesOnRoot();

  /**
   * iterative deepening search from root node
   * @return {負けたか深さ1で中断された場合にfalseを返します。}
   */
  bool idsearch(Move& best, Value alpha, Value beta);

public:

  std::atomic<int> idleTreeCount_;
  std::atomic<int> idleWorkerCount_;

  /**
   * コンストラクタ
   */
  Searcher();

  /**
   * コンストラクタ
   */
  Searcher(Evaluator& eval);

  Searcher(const Searcher&) = delete;
  Searcher(Searcher&&) = delete;

  /**
   * デストラクタ
   */
  ~Searcher();

  /**
   * 設定を反映します。
   */
  void setConfig(const Config& config) {
    auto org = config_;
    config_ = config;
    if (config_.treeSize != org.treeSize) {
      reallocateTrees();
    }
    if (config_.workerSize != org.workerSize) {
      reallocateWorkers();
    }
  }

  /**
   * 設定を取得します。
   */
  const Config& getConfig() const {
    return config_;
  }

  /**
   * 探索情報を取得します。
   */
  const SearchInfo& getInfo() const {
    return info_;
  }

  std::string getInfoString() const;

  /**
   * SHEK と千日手検出のための過去の棋譜をクリアします。
   */
  void clearRecord();

  /**
   * SHEK と千日手検出のために過去の棋譜をセットします。
   */
  void setRecord(const Record& record);

  /**
   * 探索を強制的に打ち切ります。
   */
  void forceInterrupt();

  /**
   * 探索中かチェックします。
   */
  bool isRunning() const {
    return isRunning_.load();
  }

  /**
   * 指定した局面に対して探索を実行します。
   * @return {負けたいか中断された場合にfalseを返します。}
   */
  bool search(const Board& initialBoard, Move& best,
      Value alpha = -Value::Mate, Value beta = Value::Mate,
      bool fastStart = false);

  /**
   * 指定した局面に対して反復深化探索を実行します。
   * @return {負けたか深さ1で中断された場合にfalseを返します。}
   */
  bool idsearch(const Board& initialBoard, Move& best,
      Value alpha = -Value::Mate, Value beta = Value::Mate,
      bool fastStart = false);

  /**
   * Evaluator を取得します。
   */
  Evaluator& getEvaluator() {
    return eval_;
  }

  /**
   * TT をクリアします。
   */
  void clearTT() {
    tt_.init();
  }

  /**
   * historyをクリアします。
   */
  void clearHistory() {
    history_.init();
  }

  std::mutex& getSplitMutex() {
    return splitMutex_;
  }

  void addIdleWorker() {
    idleWorkerCount_.fetch_add(1);
  }

  void reduceIdleWorker() {
    idleWorkerCount_.fetch_sub(1);
  }

  void releaseTree(int tid);

  void searchTlp(int tid) {
    auto& tree = trees_[tid];
    searchTlp(tree);
  }

  static int standardTreeSize(int workerSize) {
    return workerSize * 4 - 3;
  }

};

} // namespace sunfish

#endif //SUNFISH_SEARCHER__
