/* Searcher.cpp
 *
 * Kubo Ryosuke
 */

#include "Searcher.h"
#include "tree/Worker.h"
#include "tree/NodeStat.h"
#include "see/See.h"
#include "core/def.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"
#include <iomanip>
#include <algorithm>
#include <cmath>

#define ENABLE_HIST_REUSE             1
#define ENABLE_LMR                    1
#define ENABLE_SMOOTH_FUT             1
#define ENABLE_RAZORING               1
#define ENABLE_MOVE_COUNT_PRUNING     1
#define ENABLE_PROBCUT                1
#define ENABLE_HASH_MOVE              1
#define ENABLE_KILLER_MOVE            1
#define ENABLE_SHEK                   1
#define ENABLE_MATE_1PLY              1
#define ENABLE_MATE_3PLY              1
#define ENABLE_MATE_HISTORY           1
#define ENABLE_STORE_PV               1
#define ENABLE_SINGULAR_EXTENSION     1
#define SHALLOW_SEE                   0 // should be 0

#define ENABLE_MOVE_COUNT_EXPT        0
#define ENABLE_FUT_EXPT               0
#define ENABLE_RAZOR_EXPT             0
#define ENABLE_PROBCUT_EXPT           0
#define ENABLE_MATE_HIST_EXPT         0

#define ENABLE_ROOT_MOVES_SHUFFLE     0

// debugging flags
#define DEBUG_ROOT_MOVES              0
#define DEBUG_TREE                    0
#define DEBUG_NODE                    0

#define ITERATE_INFO_THRESHOLD        3 // must be greater than or equal to 2

#if ENABLE_MATE_3PLY && !ENABLE_MATE_1PLY
# error
#endif

namespace {

CONSTEXPR_CONST int MAIN_TREEID = 0;
CONSTEXPR_CONST int MAIN_WORKER_ID = 0;

} // namespace

namespace sunfish {

namespace expt {

class PruningCounter {
  uint64_t succ_[64];
  uint64_t fail_[64];
  int index(int depth) const {
    return std::min(std::max(depth / Searcher::Depth1Ply, 0), 64);
  }
public:
  void clear() {
    memset(succ_, 0, sizeof(succ_));
    memset(fail_, 0, sizeof(fail_));
  }
  void succ(int depth) {
    succ_[index(depth)]++;
  }
  void fail(int depth) {
    fail_[index(depth)]++;
  }
  uint64_t succ(int depth) const {
    return succ_[index(depth)];
  }
  uint64_t fail(int depth) const {
    return fail_[index(depth)];
  }
  void print() const {
    for (int i = 0; i < 64; i++) {
      uint64_t s = succ_[i];
      uint64_t f = fail_[i];
      if (s != 0 || f != 0) {
        float r = (float)f / (s + f) * 100.0;
        Loggers::warning << "  " << i << ": " << f << "/" << (s+f) << " (" << r << "%)";
      }
    }
  }
};

#if ENABLE_MOVE_COUNT_EXPT
uint64_t count_sum;
uint64_t count_num;
PruningCounter move_count_based_pruning;
#endif
#if ENABLE_FUT_EXPT
PruningCounter futility_pruning;
#endif
#if ENABLE_RAZOR_EXPT
PruningCounter razoring;
#endif
#if ENABLE_PROBCUT_EXPT
PruningCounter probcut;
#endif
#if ENABLE_MATE_HIST_EXPT
PruningCounter mate_hist;
PruningCounter mate_hist_n;
#endif

} // namespace expt

namespace search_param {

#if ENABLE_SMOOTH_FUT
  CONSTEXPR_CONST int FUT_DEPTH = Searcher::Depth1Ply * 9;
#else
  CONSTEXPR_CONST int FUT_DEPTH = Searcher::Depth1Ply * 3;
#endif
  CONSTEXPR_CONST int EXT_CHECK = Searcher::Depth1Ply;
  CONSTEXPR_CONST int EXT_ONEREP = Searcher::Depth1Ply * 1 / 2;
  CONSTEXPR_CONST int EXT_RECAP = Searcher::Depth1Ply * 1 / 4;
  CONSTEXPR_CONST int EXT_RECAP2 = Searcher::Depth1Ply * 1 / 2;
  CONSTEXPR_CONST int REC_THRESHOLD = Searcher::Depth1Ply * 3;
  CONSTEXPR_CONST int RAZOR_DEPTH = Searcher::Depth1Ply * 4;
  CONSTEXPR_CONST int QUIES_RELIEVE_PLY = 7;

}

namespace search_func {

  inline int recDepth(int depth) {
    return (depth < Searcher::Depth1Ply * 9 / 2 ?
            Searcher::Depth1Ply * 3 / 2 :
            depth - Searcher::Depth1Ply * 3);
  }

  inline int nullDepth(int depth) {
    return (depth < Searcher::Depth1Ply * 26 / 4 ? depth - Searcher::Depth1Ply * 12 / 4 :
            (depth <= Searcher::Depth1Ply * 30 / 4 ? Searcher::Depth1Ply * 14 / 4 : depth - Searcher::Depth1Ply * 16 / 4));
  }

  inline int futilityMargin(int depth, int count) {
#if ENABLE_SMOOTH_FUT
    return (depth < Searcher::Depth1Ply * 3 ? 400 :
            140 / Searcher::Depth1Ply * std::max(depth, 0)) - 8 * count;
#else
    return 400 - 8 * count;
#endif
  }

  inline int razorMargin(int depth) {
    return 256 + 64 / Searcher::Depth1Ply * std::max(depth, 0);
  }

  inline int futilityMoveCounts(bool improving, int depth) {
    int d = depth + (improving ? Searcher::Depth1Ply * 1 / 4 : 0);
    int x = (d * d) / (Searcher::Depth1Ply * Searcher::Depth1Ply);
    int a = improving ? 9 : 6;
    return 2 + x * a / 10;
  }

  inline uint64_t excludedHash(const Move& move) {
    CONSTEXPR_CONST uint64_t key = 0xc7ebffd8801628a7llu;
    uint32_t m = Move::serialize(move);
    return key ^ m;
  }

}

/**
 * コンストラクタ
 */
Searcher::Searcher()
: trees_(nullptr)
, workers_(nullptr)
, forceInterrupt_(false)
, isRunning_(false) {
  initConfig();
  history_.init();
}

/**
 * コンストラクタ
 */
Searcher::Searcher(Evaluator& eval)
: trees_(nullptr)
, workers_(nullptr)
, eval_(eval)
, forceInterrupt_(false)
, isRunning_(false) {
  initConfig();
  history_.init();
}

/**
 * デストラクタ
 */
Searcher::~Searcher() {
  releaseTrees();
  releaseWorkers();
}

/**
 * tree の確保
 */
void Searcher::allocateTrees() {
  if (trees_ == nullptr) {
    trees_ = new Tree[config_.treeSize];
  }
}

/**
 * worker の確保
 */
void Searcher::allocateWorkers() {
  if (workers_ == nullptr) {
    workers_ = new Worker[config_.workerSize];
  }
}

/**
 * tree の再確保
 */
void Searcher::reallocateTrees() {
  if (trees_ != nullptr) {
    delete[] trees_;
  }
  trees_ = new Tree[config_.treeSize];
}

/**
 * worker の再確保
 */
void Searcher::reallocateWorkers() {
  if (workers_ != nullptr) {
    delete[] workers_;
  }
  workers_ = new Worker[config_.workerSize];
}

/**
 * tree の解放
 */
void Searcher::releaseTrees() {
  if (trees_ != nullptr) {
    delete[] trees_;
    trees_ = nullptr;
  }
}

/**
 * worker の解放
 */
void Searcher::releaseWorkers() {
  if (workers_ != nullptr) {
    delete[] workers_;
    workers_ = nullptr;
  }
}

/**
 * worker の取得
 */
Worker& Searcher::getWorker(Tree& tree) {
  return workers_[tree.getTlp().workerId];
}

void Searcher::mergeInfo() {
  memset(&info_, 0, sizeof(SearchInfoBase));
  for (int id = 0; id < config_.workerSize; id++) {
    auto& worker = workers_[id];
    info_.failHigh                   += worker.info.failHigh;
    info_.failHighFirst              += worker.info.failHighFirst;
    info_.failHighIsHash             += worker.info.failHighIsHash;
    info_.failHighIsKiller1          += worker.info.failHighIsKiller1;
    info_.failHighIsKiller2          += worker.info.failHighIsKiller2;
    info_.hashProbed                 += worker.info.hashProbed;
    info_.hashHit                    += worker.info.hashHit;
    info_.hashExact                  += worker.info.hashExact;
    info_.hashLower                  += worker.info.hashLower;
    info_.hashUpper                  += worker.info.hashUpper;
    info_.hashStore                  += worker.info.hashStore;
    info_.hashNew                    += worker.info.hashNew;
    info_.hashUpdate                 += worker.info.hashUpdate;
    info_.hashCollision              += worker.info.hashCollision;
    info_.hashReject                 += worker.info.hashReject;
    info_.mateProbed                 += worker.info.mateProbed;
    info_.mateHit                    += worker.info.mateHit;
    info_.expand                     += worker.info.expand;
    info_.expandHashMove             += worker.info.expandHashMove;
    info_.shekProbed                 += worker.info.shekProbed;
    info_.shekSuperior               += worker.info.shekSuperior;
    info_.shekInferior               += worker.info.shekInferior;
    info_.shekEqual                  += worker.info.shekEqual;
    info_.nullMovePruning            += worker.info.nullMovePruning;
    info_.nullMovePruningTried       += worker.info.nullMovePruningTried;
    info_.futilityPruning            += worker.info.futilityPruning;
    info_.extendedFutilityPruning    += worker.info.extendedFutilityPruning;
    info_.moveCountPruning           += worker.info.moveCountPruning;
    info_.razoring                   += worker.info.razoring;
    info_.razoringTried              += worker.info.razoringTried;
    info_.probcut                    += worker.info.probcut;
    info_.probcutTried               += worker.info.probcutTried;
    info_.singular                   += worker.info.singular;
    info_.singularChecked            += worker.info.singularChecked;
    info_.expanded                   += worker.info.expanded;
    info_.checkExtension             += worker.info.checkExtension;
    info_.onerepExtension            += worker.info.onerepExtension;
    info_.recapExtension             += worker.info.recapExtension;
    info_.split                      += worker.info.split;
    info_.node                       += worker.info.node;
    info_.qnode                      += worker.info.qnode;
  }
}

/**
 * 前処理
 */
void Searcher::before(const Board& initialBoard, bool fastStart) {

#if ENABLE_MOVE_COUNT_EXPT
  expt::move_count_based_pruning.clear();
#endif
#if ENABLE_FUT_EXPT
  expt::futility_pruning.clear();
#endif
#if ENABLE_RAZOR_EXPT
  expt::razoring.clear();
#endif
#if ENABLE_PROBCUT_EXPT
  expt::probcut.clear();
#endif
#if ENABLE_MATE_HIST_EXPT
  expt::mate_hist.clear();
  expt::mate_hist_n.clear();
#endif

  if (isRunning_.load()) {
    Loggers::error << __FILE_LINE__ << ": Searcher is already running!!!";
  }

  allocateTrees();
  allocateWorkers();

  // tree の初期化
  for (int id = 0; id < config_.treeSize; id++) {
    auto& tree = trees_[id];
    tree.init(id, initialBoard, eval_, record_);
  }
  idleTreeCount_.store(config_.treeSize - 1);

  // worker の初期化
  for (int id = 0; id < config_.workerSize; id++) {
    auto& worker = workers_[id];
    worker.init(id, this);
    if (id != MAIN_WORKER_ID) {
      worker.startOnChildThread();
    }
  }
  idleWorkerCount_.store(config_.workerSize - 1);

  // 最初の tree を確保
  auto& tree0 = trees_[MAIN_TREEID];
  auto& worker0 = workers_[MAIN_WORKER_ID];
  tree0.use(MAIN_WORKER_ID);
  worker0.startOnCurrentThread(MAIN_TREEID);

  // timer 初期化
  timer_.set();

  forceInterrupt_.store(false);
  isRunning_.store(true);

  timeManager_.init();

  if (fastStart) {
    return;
  }

  // transposition table
  tt_.evolve(); // 世代更新

  // hisotory heuristic
#if ENABLE_HIST_REUSE
  history_.reduce();
#else
  history_.init();
#endif

  // mate
  mateHistory_.clear();

  // gains
  gains_.clear();
}

/**
 * 後処理
 */
void Searcher::after() {

#if ENABLE_MOVE_COUNT_EXPT
  Loggers::warning << "move count based pruning:";
  expt::move_count_based_pruning.print();
#endif
#if ENABLE_FUT_EXPT
  Loggers::warning << "futility pruning:";
  expt::futility_pruning.print();
#endif
#if ENABLE_RAZOR_EXPT
  Loggers::warning << "razoring:";
  expt::razoring.print();
#endif
#if ENABLE_PROBCUT_EXPT
  Loggers::warning << "probcut:";
  expt::probcut.print();
#endif
#if ENABLE_MATE_HIST_EXPT
  Loggers::warning << "mate history:";
  expt::mate_hist.print();
  Loggers::warning << "mate history (n):";
  expt::mate_hist_n.print();
#endif

  if (!isRunning_.load()) {
    Loggers::error << __FILE_LINE__ << ": Searcher is not running???";
  }

  // worker の停止
  for (int id = 1; id < config_.workerSize; id++) {
    auto& worker = workers_[id];
    if (config_.threadPooling) {
      worker.sleep();
    } else {
      worker.stop();
    }
  }

  // tree の解放
  for (int id = 0; id < config_.treeSize; id++) {
    auto& tree = trees_[id];
    tree.release(record_);
  }

  // 探索情報集計
  mergeInfo();

  // 探索情報収集
  auto& tree0 = trees_[0];
  info_.time = timer_.get();
  info_.nps = (info_.node + info_.qnode) / info_.time;
  info_.move = tree0.getPV().get(0).move;
  info_.pv.copy(tree0.getPV());

  isRunning_.store(false);
  forceInterrupt_.store(false);
}

std::string Searcher::getInfoString() const {
  auto format = [](int64_t value) {
    std::ostringstream oss;
    oss << std::setw(8) << (value);
    return oss.str();
  };
  auto format2 = [](int64_t value, int64_t total) {
    std::ostringstream oss;
    oss << std::setw(8) << (value) << '/' << std::setw(8) << (total)
      << " (" << std::setw(5) << std::fixed << std::setprecision(1)<< ((float)(value) / ((total)!=0?(total):1) * 100.0) << "%)";
    return oss.str();
  };

  std::vector<std::pair<std::string, std::string>> lines;
  lines.emplace_back("nodes          ", format (info_.node));
  lines.emplace_back("quies-nodes    ", format (info_.qnode));
  lines.emplace_back("all-nodes      ", format ((info_.node + info_.qnode)));
  lines.emplace_back("time           ", format (info_.time));
  lines.emplace_back("nps            ", format (std::ceil(info_.nps)));
  lines.emplace_back("eval           ", format (info_.eval.int32()));
  lines.emplace_back("split          ", format (info_.split));
  lines.emplace_back("fail high first", format2(info_.failHighFirst, info_.failHigh));
  lines.emplace_back("fail high hash ", format2(info_.failHighIsHash, info_.failHigh));
  lines.emplace_back("fail high kill1", format2(info_.failHighIsKiller1, info_.failHigh));
  lines.emplace_back("fail high kill2", format2(info_.failHighIsKiller2, info_.failHigh));
  lines.emplace_back("expand hash    ", format2(info_.expandHashMove, info_.expand));
  lines.emplace_back("hash hit       ", format2(info_.hashHit, info_.hashProbed));
  lines.emplace_back("hash extract   ", format2(info_.hashExact, info_.hashProbed));
  lines.emplace_back("hash lower     ", format2(info_.hashLower, info_.hashProbed));
  lines.emplace_back("hash upper     ", format2(info_.hashUpper, info_.hashProbed));
  lines.emplace_back("hash new       ", format2(info_.hashNew, info_.hashStore));
  lines.emplace_back("hash update    ", format2(info_.hashUpdate, info_.hashStore));
  lines.emplace_back("hash collide   ", format2(info_.hashCollision, info_.hashStore));
  lines.emplace_back("hash reject    ", format2(info_.hashReject, info_.hashStore));
  lines.emplace_back("mate hit       ", format2(info_.mateHit, info_.mateProbed));
  lines.emplace_back("shek superior  ", format2(info_.shekSuperior, info_.shekProbed));
  lines.emplace_back("shek inferior  ", format2(info_.shekInferior, info_.shekProbed));
  lines.emplace_back("shek equal     ", format2(info_.shekEqual, info_.shekProbed));
  lines.emplace_back("null mv pruning", format2(info_.nullMovePruning, info_.nullMovePruningTried));
  lines.emplace_back("fut pruning    ", format (info_.futilityPruning));
  lines.emplace_back("ext fut pruning", format (info_.extendedFutilityPruning));
  lines.emplace_back("mov cnt pruning", format (info_.moveCountPruning));
  lines.emplace_back("razoring       ", format2(info_.razoring, info_.razoringTried));
  lines.emplace_back("probcut        ", format2(info_.probcut, info_.probcutTried));
  lines.emplace_back("singular       ", format2(info_.singular, info_.singularChecked));
  lines.emplace_back("check extension", format2(info_.checkExtension, info_.expanded));
  lines.emplace_back("1rep extension ", format2(info_.onerepExtension, info_.expanded));
  lines.emplace_back("recap extension", format2(info_.recapExtension, info_.expanded));

  const int columns = 2;
  int rows = (lines.size() + columns - 1) / columns;
  int maxLength[columns];
  memset(maxLength, 0, sizeof(int) * columns);

  for (int row = 0; row < rows; row++) {
    for (int column = 0; column < columns; column++) {
      int index = column * rows + row;
      if (index >= (int)lines.size()) { continue; }
      int length = lines[index].first.length() + lines[index].second.length();
      maxLength[column] = std::max(maxLength[column], length);
    }
  }

  std::ostringstream oss;
  oss << "Search Info:\n";
  for (int row = 0; row < rows; row++) {
    oss << "  ";
    for (int column = 0; column < columns; column++) {
      int index = column * rows + row;
      if (index >= (int)lines.size()) { continue; }
      int length = lines[index].first.length() + lines[index].second.length();
      int padding = maxLength[column] - length + 1;
      oss << " * " << lines[index].first << ":" << lines[index].second;
      bool isLastColumn = column == columns - 1;
      if (isLastColumn) { break; }
      for (int i = 0; i < padding; i++) {
        oss << ' ';
      }
    }
    oss << '\n';
  }

  return oss.str();
}

/**
 * SHEK と千日手検出のための過去の棋譜をクリアします。
 */
void Searcher::clearRecord() {
  record_.clear();
}

/**
 * SHEK と千日手検出のために過去の棋譜をセットします。
 */
void Searcher::setRecord(const Record& record) {
  for (unsigned i = 0; i < record.getCount(); i++) {
    record_.push_back(record.getMoveAt(i));
  }
}

/**
 * 探索中断判定
 */
inline bool Searcher::isInterrupted(Tree& tree) {
  if (tree.getTlp().shutdown.load()) {
  }
  if (forceInterrupt_.load()) {
    return true;
  }
  if (config_.enableLimit && timer_.get() >= config_.limitSeconds) {
    return true;
  }
  return false;
}

/**
 * 探索を強制的に打ち切ります。
 */
void Searcher::forceInterrupt() {
  forceInterrupt_.store(true);
}

/**
 * get see value
 */
template <bool shallow>
Value Searcher::searchSee(const Board& board, const Move& move, Value alpha, Value beta) {
  See see;
  return see.search<shallow>(board, move, alpha, beta);
}

/**
 * sort moves by see
 */
void Searcher::sortSee(Tree& tree, int offset, Value standPat, Value alpha, bool enableKiller, bool estimate, bool exceptSmallCapture, bool isQuies) {
  const auto& board = tree.getBoard();
  auto& node = tree.getCurrentNode();
  auto& worker = getWorker(tree);
#if !ENABLE_KILLER_MOVE
  assert(node.killer1.isEmpty());
  assert(node.killer2.isEmpty());
#endif // ENABLE_KILLER_MOVE
  assert(offset == 0 || offset == 1);
  assert(tree.getNextMove() + offset <= tree.getEnd());
  if (enableKiller) {
    node.capture1 = Move::empty();
    node.capture2 = Move::empty();
    node.cvalue1 = -Value::Inf;
    node.cvalue2 = -Value::Inf;
  }

  for (auto ite = tree.getNextMove() + offset; ite != tree.getEnd(); ) {
    const Move& move = *ite;
    Value value;

    if (exceptSmallCapture) {
      auto captured = tree.getBoard().getBoardPiece(move.to()).kindOnly();
      if ((captured == Piece::Pawn && !move.promote()) ||
          (captured.isEmpty() && move.piece() != Piece::Pawn)) {
        ite = tree.getMoves().remove(ite);
        continue;
      }
    }

    if (isQuies) {
      // futility pruning
      if (standPat + tree.estimate(move, eval_) + gains_.get(move) <= alpha) {
        worker.info.futilityPruning++;
        ite = tree.getMoves().remove(ite);
        continue;
      }
    }

#if SHALLOW_SEE
    value = searchSee<true>(board, move, -1, Value::PieceInf);
#else
    value = searchSee<false>(board, move, -1, Value::PieceInf);
#endif
    if (estimate) {
      value += tree.estimate<true>(move, eval_);
    }

    if (enableKiller) {
      if (value > node.cvalue1) {
        node.capture2 = node.capture1;
        node.cvalue2 = node.cvalue1;
        node.capture1 = move;
        node.cvalue1 = value;
      } else if (value > node.cvalue2) {
        node.capture2 = move;
        node.cvalue2 = value;
      }
    }

    if (!isQuies) {
      if ((node.expStat & HashDone) && move == node.hash) {
        ite = tree.getMoves().remove(ite);
        continue;
      }
    }

    if (enableKiller) {
      if (move == node.killer1) {
        node.expStat |= Killer1Added;
        auto captured = board.getBoardPiece(move.to());
        Value kvalue = tree.getKiller1Value() + material::pieceExchange(captured);
        value = Value::max(value, kvalue);
      } else if (move == node.killer2) {
        node.expStat |= Killer2Added;
        auto captured = board.getBoardPiece(move.to());
        Value kvalue = tree.getKiller2Value() + material::pieceExchange(captured);
        value = Value::max(value, kvalue);
      }
    }

    tree.setSortValue(ite, value.int32());

    ite++;
  }

  if (enableKiller) {
    if (!(node.expStat & Killer1Added) && node.killer1 != node.hash
        && tree.getKiller1Value() >= Value::Zero
        && board.isValidMoveStrict(node.killer1)) {
      node.expStat |= Killer1Added;
      auto ite = tree.addMove(node.killer1);
      Value kvalue = tree.getKiller1Value();
      tree.setSortValue(ite, kvalue.int32());
    }
    if (!(node.expStat & Killer2Added) && node.killer2 != node.hash
        && tree.getKiller2Value() >= Value::Zero
        && board.isValidMoveStrict(node.killer2)) {
      node.expStat |= Killer2Added;
      auto ite = tree.addMove(node.killer2);
      Value kvalue = tree.getKiller2Value();
      tree.setSortValue(ite, kvalue.int32());
    }
  }

  tree.sortAfterCurrent(offset);

  if (isQuies) {
    for (auto ite = tree.getNextMove() + offset; ite != tree.getEnd(); ite++) {
      if (tree.getSortValue(ite) < 0) {
        tree.removeAfter(ite);
        break;
      }
    }
  }

}

/**
 * except prior moves
 */
void Searcher::exceptPriorMoves(Tree& tree) {
  auto& node = tree.getCurrentNode();
#if !ENABLE_KILLER_MOVE
  assert(node.killer1.isEmpty());
  assert(node.killer2.isEmpty());
#endif // ENABLE_KILLER_MOVE

  for (auto ite = tree.getNextMove(); ite != tree.getEnd(); ) {
    const Move& move = *ite;

    if ((node.expStat & HashDone) && move == node.hash) {
      ite = tree.getMoves().remove(ite);
      continue;
    }
    if (((node.expStat & Killer1Done) && move == node.killer1) ||
        ((node.expStat & Killer2Done) && move == node.killer2)) {
      ite = tree.getMoves().remove(ite);
      continue;
    }

    ite++;
  }
}

/**
 * pick best move by history
 */
bool Searcher::pickOneHistory(Tree& tree) {
  Moves::iterator best = tree.getEnd();
  uint32_t bestValue = 0;

  for (auto ite = tree.getNextMove(); ite != tree.getEnd(); ) {
    const Move& move = *ite;

    auto key = History::getKey(move);
    auto data = history_.getData(key);
    auto value = History::getRatio(data);
    if (value > bestValue) {
      best = ite;
      bestValue = value;
    }

    ite++;
  }

  if (best != tree.getEnd()) {
    Move temp = *tree.getNextMove();
    *tree.getNextMove() = *best;
    *best = temp;
    return true;
  }

  return false;
}

/**
 * sort moves by history
 */
void Searcher::sortHistory(Tree& tree) {
  for (auto ite = tree.getNextMove(); ite != tree.getEnd(); ) {
    const Move& move = *ite;

    auto key = History::getKey(move);
    auto data = history_.getData(key);
    auto ratio = History::getRatio(data);
    tree.setSortValue(ite, (int32_t)ratio);

    ite++;
  }

  tree.sortAfterCurrent();

}

/**
 * update history
 */
void Searcher::updateHistory(Tree& tree, int depth, const Move& move) {

  CONSTEXPR_CONST int HistPerDepth = 8;
  int value = std::max(depth * HistPerDepth / Depth1Ply, 1);
  const auto& moves = tree.getCurrentNode().histMoves;
  for (auto ite = moves.begin(); ite != moves.end(); ite++) {
    assert(ite != tree.getEnd());
    auto key = History::getKey(*ite);
    if (ite->equals(move)) {
      history_.add(key, value, value);
    } else {
      history_.add(key, value, 0);
    }

  }

}

/**
 * get LMR depth
 */
int Searcher::getReductionDepth(bool improving, int depth, int count, const Move& move, bool isNullWindow) {
  auto key = History::getKey(move);
  auto data = history_.getData(key);
  auto good = History::getGoodCount(data) + 1;
  auto appear = History::getAppearCount(data) + 2;

  assert(good < appear);

  int reduced = 0;

  if (!isNullWindow) {
    if (good * 20 < appear) {
      reduced += Depth1Ply * 3 / 2;
    } else if (good * 7 < appear) {
      reduced += Depth1Ply * 2 / 2;
    } else if (good * 3 < appear) {
      reduced += Depth1Ply * 1 / 2;
    }
  } else {
    if (good * 10 < appear) {
      reduced += Depth1Ply * 4 / 2;
    } else if (good * 6 < appear) {
      reduced += Depth1Ply * 3 / 2;
    } else if (good * 4 < appear) {
      reduced +=  Depth1Ply * 2 / 2;
    } else if (good * 2 < appear) {
      reduced += Depth1Ply * 1 / 2;
    }
  }

  if (!improving && depth < Depth1Ply * 9) {
    if (count >= 24) {
      reduced += Depth1Ply * 4 / 2;
    } else if (count >= 18) {
      reduced += Depth1Ply * 3 / 2;
    } else if (count >= 12) {
      reduced += Depth1Ply * 2 / 2;
    } else if (count >= 6) {
      reduced += Depth1Ply * 1 / 2;
    }
  }

  return reduced;
}

/**
 * get next move
 */
bool Searcher::nextMove(Tree& tree) {

  auto& moves = tree.getMoves();
  auto& node = tree.getCurrentNode();
  auto& worker = getWorker(tree);
  const auto& board = tree.getBoard();

  while (true) {

    if (!tree.isThroughPhase() && tree.getNextMove() != tree.getEnd()) {
      tree.selectNextMove();
      node.count++;

      const Move& move = *tree.getCurrentMove();
      if (move == node.hash) {
        node.expStat |= HashDone;
      } else if (move == node.killer1) {
        node.expStat |= Killer1Done;
      } else if (move == node.killer2) {
        node.expStat |= Killer2Done;
      } else if (move == node.capture1) {
        node.expStat |= Capture1Done;
      } else if (move == node.capture2) {
        node.expStat |= Capture2Done;
      }

      return true;
    }

    switch (node.genPhase) {
    case GenPhase::Hash:
      {
        Move hashMove = node.hash;
        if (!hashMove.isEmpty() && board.isValidMoveStrict(hashMove)) {
          tree.addMove(hashMove);
          tree.setThroughPhase(true);
          worker.info.expandHashMove++;
        }
        node.genPhase = GenPhase::Capture;
      }
      break;

    case GenPhase::Capture:
      tree.setThroughPhase(false);
      if (tree.isChecking()) {
        int offset = moves.end() - tree.getNextMove();
        MoveGenerator::generateEvasion(board, moves);
        sortSee(tree, offset, Value::Zero, Value::Zero, false, true, false, false);
        node.genPhase = GenPhase::End;
        break;

      } else {
        int offset = moves.end() - tree.getNextMove();
        MoveGenerator::generateCap(board, moves);
        sortSee(tree, offset, Value::Zero, Value::Zero, true, false, false, false);
        node.genPhase = GenPhase::History1;
        break;

      }

    case GenPhase::History1:
      node.count = 0;
      MoveGenerator::generateNoCap(board, moves);
      MoveGenerator::generateDrop(board, moves);
      exceptPriorMoves(tree);
      node.genPhase = GenPhase::History2;
      tree.setThroughPhase(true);
      if (pickOneHistory(tree)) {
        tree.selectNextMove();
        return true;
      }
      break;

    case GenPhase::History2:
      node.genPhase = GenPhase::Misc;
      tree.setThroughPhase(true);
      if (pickOneHistory(tree)) {
        tree.selectNextMove();
        return true;
      }
      break;

    case GenPhase::Misc:
      sortHistory(tree);
      node.genPhase = GenPhase::End;
      tree.setThroughPhase(false);
      break;

    case GenPhase::CaptureOnly:
      assert(false);

    case GenPhase::End:
      return false;
    }
  }

}

/**
 * get next move
 */
bool Searcher::nextMoveQuies(Tree& tree, int qply, Value standPat, Value alpha) {

  auto& moves = tree.getMoves();
  auto& node = tree.getCurrentNode();
  const auto& board = tree.getBoard();

  while (true) {

    if (tree.getNextMove() != tree.getEnd()) {
      tree.selectNextMove();
      return true;
    }

    switch (node.genPhase) {
    case GenPhase::Hash: // fall through
    case GenPhase::Capture: // fall through
    case GenPhase::History1:
    case GenPhase::History2:
    case GenPhase::Misc:
      assert(false);
      break;

    case GenPhase::CaptureOnly:
      if (tree.isChecking()) {
        MoveGenerator::generateEvasion(board, moves);
        sortHistory(tree);
        node.genPhase = GenPhase::End;
        break;

      } else {
        MoveGenerator::generateCap(board, moves);
        if (qply >= search_param::QUIES_RELIEVE_PLY) {
          sortSee(tree, 0, standPat, alpha, false, false, true, true);
        } else {
          sortSee(tree, 0, standPat, alpha, false, false, false, true);
        }
        node.genPhase = GenPhase::End;
        break;

      }

    case GenPhase::End:
      return false;
    }
  }

}

/**
 * store PV-nodes to TT
 */
void Searcher::storePV(Tree& tree, const PV& pv, int ply) {
  if (ply >= pv.size()) {
    return;
  }

  int depth = pv.get(ply).depth;
  if (depth <= 0) {
    return;
  }

  const auto& move = pv.get(ply).move;
  if (move.isEmpty()) {
    return;
  }

  if (tree.makeMoveFast(move)) {
    storePV(tree, pv, ply + 1);
    tree.unmakeMoveFast();
  }

  auto hash = tree.getBoard().getHash();
  tt_.entryPV(hash, depth, Move::serialize16(move));
}

bool Searcher::isNeedMateSearch(Tree& tree, bool black, int depth) {
#if ENABLE_MATE_HISTORY
  if (tree.getPly() <= 6 || depth >= 3 * Depth1Ply) {
    return true;
  }

  const Board& board = tree.getBoard();
  Square king = black ? board.getWKingSquare() : board.getBKingSquare();
  Move move = tree.getPreFrontMove();

  uint64_t data = mateHistory_.getData(king, move);
  assert(data <= MateHistory::Max);
  uint32_t mated = MateHistory::getMated(data);
  uint32_t probed = MateHistory::getProbed(data);
  assert(mated <= probed);

  return (mated * 82) + 17 >= probed;
#else
  return true;
#endif
}

void Searcher::updateMateHistory(Tree& tree, bool black, bool mate) {
#if ENABLE_MATE_HISTORY
  if (tree.getPly() <= 1) {
    return;
  }

  const Board& board = tree.getBoard();
  Square king = black ? board.getWKingSquare() : board.getBKingSquare();
  Move move = tree.getPreFrontMove();

  mateHistory_.update(king, move, mate);
#endif
}

/**
 * quiesence search
 */
Value Searcher::qsearch(Tree& tree, bool black, int qply, Value alpha, Value beta) {

#if DEBUG_NODE
  bool debug = false;
#if 0
  if (tree.debug__matchPath("-4233GI +5968OU -8586FU")) {
    std::cout << "#-- debug quies node begin --#" << std::endl;
    std::cout << tree.debug__getPath() << std::endl;
    debug = true;
  }
#endif
#endif

#if DEBUG_TREE
  {
    for (int i = 0; i < tree.getPly(); i++) {
      std::cout << ' ';
    }
    std::cout << tree.debug__getFrontMove().toString() << std::endl;
  }
#endif

  auto& worker = getWorker(tree);
  worker.info.qnode++;

  // stand-pat
  Value standPat = tree.getValue() * (black ? 1 : -1);

  // beta-cut
  if (standPat >= beta) {
    return standPat;
  }

  // スタックサイズの限界
  if (tree.isStackFull()) {
    return standPat;
  }

#if ENABLE_MATE_1PLY || ENABLE_MATE_3PLY
  {
    // search mate in 3 ply
    bool mate = false;
    worker.info.mateProbed++;
    if (mateTable_.get(tree.getBoard().getHash(), mate)) {
      worker.info.mateHit++;
    } else if (isNeedMateSearch(tree, black, 0)) {
# if ENABLE_MATE_3PLY
      mate = Mate::mate3Ply(tree);
# else
      mate = Mate::mate1Ply(tree.getBoard());
# endif
      mateTable_.set(tree.getBoard().getHash(), mate);
      updateMateHistory(tree, black, mate);
#if ENABLE_MATE_HIST_EXPT
      if (mate) {
        expt::mate_hist_n.fail(0);
      } else {
        expt::mate_hist_n.succ(0);
      }
    } else {
      if (ENABLE_MATE_3PLY ? Mate::mate3Ply(tree) : Mate::mate1Ply(tree.getBoard())) {
        expt::mate_hist.fail(0);
      } else {
        expt::mate_hist.succ(0);
      }
#endif
    }
    if (mate) {
# if ENABLE_MATE_3PLY
      return Value::Inf - tree.getPly() - 3;
# else
      return Value::Inf - tree.getPly() - 1;
#endif
    }
  }
#endif

  alpha = Value::max(alpha, standPat);

  // 合法手生成
  auto& moves = tree.getMoves();
  moves.clear();
  tree.initGenPhase(GenPhase::CaptureOnly);

  while (nextMoveQuies(tree, qply, standPat, alpha)) {
    // make move
    if (!tree.makeMove(eval_)) {
      continue;
    }

    // reccursive call
    Value currval;
    currval = -qsearch(tree, !black, qply + 1, -beta, -alpha);

    // unmake move
    tree.unmakeMove();

    // 中断判定
    if (isInterrupted(tree)) {
      return Value::Zero;
    }

    // 値更新
    if (currval > alpha) {
      alpha = currval;
      tree.updatePV(0);

      // beta-cut
      if (currval >= beta) {
        break;
      }
    }
  }

  return alpha;

}

void Searcher::updateKiller(Tree& tree, const Move& move) {
  auto& node = tree.getCurrentNode();
  const auto& board = tree.getBoard();

  Piece captured = board.getBoardPiece(move.to());
  Value capVal = material::pieceExchange(captured);

  if (move == node.capture1) {
    if ((node.expStat & Killer1Done) && move != node.killer1) {
      node.kvalue1 = node.cvalue1 - capVal - 1;
    }
    if ((node.expStat & Killer2Done) && move != node.killer2) {
      node.kvalue2 = node.cvalue1 - capVal - 1;
    }

  } else if (move == node.killer1) {
    if ((node.expStat & Capture1Done) &&
        node.kvalue2 + capVal <= node.cvalue1) {
      node.kvalue2 = node.cvalue1 - capVal + 1;
    }
    Piece captured2 = board.getBoardPiece(node.killer2.to());
    Value capVal2 = material::pieceExchange(captured2);
    if ((node.expStat & Killer2Done) &&
        node.kvalue1 + capVal <= node.kvalue2 + capVal2) {
      node.kvalue1 = node.kvalue2 + capVal2 - capVal + 1;
    }

  } else if (move == node.killer2) {
    if ((node.expStat & Capture1Done) &&
        node.kvalue2 + capVal <= node.cvalue1) {
      node.kvalue2 = node.cvalue1 - capVal + 1;
    }
    Piece captured1 = board.getBoardPiece(node.killer1.to());
    Value capVal1 = material::pieceExchange(captured1);
    if ((node.expStat & Killer1Done) &&
        node.kvalue2 + capVal <= node.kvalue1 + capVal1) {
      node.kvalue2 = node.kvalue1 + capVal1 - capVal + 1;
    }
    Move::swap(node.killer1, node.killer2);
    Value::swap(node.kvalue1, node.kvalue2);

  } else {
    if (node.expStat & Killer1Done) {
      Value val = searchSee<false>(board, move, -1, Value::PieceInf) - capVal + 1;
      node.kvalue1 = Value::min(node.kvalue1, val);
    }
    node.killer2 = node.killer1;
    node.kvalue2 = node.kvalue1;
    node.killer1 = move;
    node.kvalue1 = node.cvalue1 - capVal + 1;

  }

  if (tree.getBoard().getBoardPiece(move.to()).isEmpty() &&
      (!move.promote() || move.piece() == Piece::Silver)) {
    node.nocap2 = node.nocap1;
    node.nocap1 = move;
  }
}

/**
 * nega-max search
 */
Value Searcher::search(Tree& tree, bool black, int depth, Value alpha, Value beta, NodeStat stat) {

#if DEBUG_TREE
  {
    for (int i = 0; i < tree.getPly(); i++) {
      std::cout << ' ';
    }
    std::cout << tree.debug__getFrontMove().toString() << std::endl;
  }
#endif // DEBUG_TREE

#if DEBUG_NODE
  bool debug = false;
#if 0
  if (tree.debug__matchPath("-0095KA")) {
    std::cout << "#-- debug node begin --#" << std::endl;
    std::cout << tree.debug__getPath() << std::endl;
    std::cout << "alpha=" << alpha.int32() << " beta=" << beta.int32() << " depth=" << depth << std::endl;
    debug = true;
  }
#endif
#endif // DEBUG_NODE

  auto& worker = getWorker(tree);

#if ENABLE_SHEK
#if !defined(NLEARN)
  if (!config_.learning)
#endif
  {
    // SHEK
    ShekStat shekStat = tree.checkShek();
    worker.info.shekProbed++;
    switch (shekStat) {
      case ShekStat::Superior:
        // 過去の局面に対して優位な局面
        tree.getCurrentNode().isHistorical = true;
        worker.info.shekSuperior++;
        return Value::Inf - tree.getPly();

      case ShekStat::Inferior:
        // 過去の局面に対して劣る局面
        tree.getCurrentNode().isHistorical = true;
        worker.info.shekInferior++;
        return -Value::Inf + tree.getPly();

      case ShekStat::Equal:
        //  過去の局面に等しい局面
        worker.info.shekEqual++;
        switch(tree.getCheckRepStatus()) {
        case RepStatus::Win:
          tree.getCurrentNode().isHistorical = true;
          return Value::Inf - tree.getPly();

        case RepStatus::Lose:
          tree.getCurrentNode().isHistorical = true;
          return -Value::Inf + tree.getPly();

        case RepStatus::None:
          assert(false);

        default:
          tree.getCurrentNode().isHistorical = true;
          return Value::Zero;
        }

      default:
        break;
    }
  }
#endif // ENABLE_SHEK

  // スタックサイズの限界
  if (tree.isStackFull()) {
    tree.getCurrentNode().isHistorical = true;
    return tree.getValue() * (black ? 1 : -1);
  }

  // 静止探索の結果を返す。
  if (!tree.isChecking() && depth < Depth1Ply) {
    return qsearch(tree, black, 0, alpha, beta);
  }

  // hash
  uint64_t hash = tree.getBoard().getHash();
  if (!tree.getExcluded().isEmpty()) {
    hash ^= search_func::excludedHash(tree.getExcluded());
  }

#if ENABLE_PREFETCH
  // prefetch
  tt_.prefetch(hash);
#endif // ENABLE_PREFETCH

  Value oldAlpha = alpha;

  // distance pruning
  {
    Value value = -Value::Inf + tree.getPly();
    if (value > alpha) {
      if (value >= beta) {
        return value;
      }
      alpha = value;
    } else {
      value = Value::Inf - tree.getPly() - 1;
      if (value <= alpha) {
        return value;
      }
    }
  }

  worker.info.node++;

  bool isNullWindow = (beta == alpha + 1);

  // transposition table
  Move hashMove = Move::empty();
  Value hashValue;
  uint32_t hashValueType;
  int hashDepth;
#if !defined(NLEARN)
  if (!config_.learning)
#endif
  {
    TTE tte;
    worker.info.hashProbed++;
    if (tt_.get(hash, tte)) {
      hashDepth = tte.getDepth();
      if (depth < search_param::REC_THRESHOLD ||
          hashDepth >= search_func::recDepth(depth)) {
        hashValue = tte.getValue(tree.getPly());
        hashValueType = tte.getValueType();

        // 前回の結果で枝刈り
        if (stat.isHashCut() && isNullWindow) {
          // 現在のノードに対して優位な条件の場合
          if (hashDepth >= depth ||
              ((hashValueType == TTE::Lower || hashValueType == TTE::Exact) && hashValue >= Value::Mate) ||
              ((hashValueType == TTE::Upper || hashValueType == TTE::Exact) && hashValue <= -Value::Mate)) {
            if (hashValueType == TTE::Exact) {
              // 確定値
              worker.info.hashExact++;
              return hashValue;
            } else if (hashValueType == TTE::Lower && hashValue >= beta) {
              // 下界値
              worker.info.hashLower++;
              return hashValue;
            } else if (hashValueType == TTE::Upper && hashValue <= alpha) {
              // 上界値
              worker.info.hashUpper++;
              return hashValue;
            }
          }
          // 十分なマージンを加味して beta 値を超える場合
          if ((hashValueType == TTE::Lower || hashValueType == TTE::Exact) &&
              !tree.isChecking() && !tree.isCheckingOnFrontier()) {
            if (depth < search_param::FUT_DEPTH && hashValue >= beta + search_func::futilityMargin(depth, 0)) {
              return beta;
            }
          }
        }

        if (hashValueType == TTE::Upper || hashValueType == TTE::Exact) {
          // alpha 値を割るなら recursion 不要
          if (hashValue <= alpha && hashDepth >= search_func::recDepth(depth)) {
            stat.unsetRecursion();
          }
          // beta を超えないなら null move pruning を省略
          if (hashValue < beta && hashDepth >= search_func::nullDepth(depth)) {
            stat.unsetNullMove();
          }
        }

        // 前回の最善手を取得
        hashMove = Move::deserialize16(tte.getMove(), tree.getBoard());

        if (tte.isMateThreat()) {
          stat.setMateThreat();
        }
      }
      worker.info.hashHit++;
    }
  }

  Value standPat = tree.getValue() * (black ? 1 : -1);
  bool improving = !tree.hasPrefrontierNode() ||
    standPat >= tree.getPrefrontValue() * (black ? 1 : -1);

  bool isFirst = true;
  Move best = Move::empty();
  bool doSingularExtension = false;

#if ENABLE_RAZOR_EXPT
  bool isRazoring = false;
#endif
#if ENABLE_PROBCUT_EXPT
  bool isProbcut = false;
#endif

  if (!tree.isChecking()) {

#if ENABLE_MATE_1PLY || ENABLE_MATE_3PLY
    if (stat.isMate()) {
      // search mate in 3 ply
      bool mate = false;
      worker.info.mateProbed++;
      if (mateTable_.get(tree.getBoard().getHash(), mate)) {
        worker.info.mateHit++;
      } else if (isNeedMateSearch(tree, black, depth)) {
# if ENABLE_MATE_3PLY
        mate = Mate::mate3Ply(tree);
# else
        mate = Mate::mate1Ply(tree.getBoard());
# endif
        mateTable_.set(tree.getBoard().getHash(), mate);
        updateMateHistory(tree, black, mate);
#if ENABLE_MATE_HIST_EXPT
        if (mate) {
          expt::mate_hist_n.fail(0);
        } else {
          expt::mate_hist_n.succ(0);
        }
      } else {
        if (ENABLE_MATE_3PLY ? Mate::mate3Ply(tree) : Mate::mate1Ply(tree.getBoard())) {
          expt::mate_hist.fail(depth);
        } else {
          expt::mate_hist.succ(depth);
        }
#endif
      }
      if (mate) {
# if ENABLE_MATE_3PLY
        alpha = Value::Inf - tree.getPly() - 3;
# else
        alpha = Value::Inf - tree.getPly() - 1;
# endif
        goto hash_store;
      }
    }
#endif

    if (!stat.isMateThreat()) {
#if ENABLE_RAZORING
      // razoring
      if (depth < search_param::RAZOR_DEPTH && hashMove.isEmpty() &&
          alpha > -Value::Mate && beta < Value::Mate) {
        Value razorAlpha = alpha - search_func::razorMargin(depth);
        if (standPat <= razorAlpha) {
          worker.info.razoringTried++;
          Value qval = qsearch(tree, black, 0, razorAlpha, razorAlpha+1);
          if (qval <= razorAlpha) {
#if ENABLE_RAZOR_EXPT
            isRazoring = true;
#else
            worker.info.razoring++;
            return qval;
#endif
          }
        }
      }
#endif

      // null move pruning
      if (isNullWindow && stat.isNullMove() && beta <= standPat && depth >= Depth1Ply * 2) {
        auto newStat = NodeStat().unsetNullMove();
        int newDepth = search_func::nullDepth(depth);

        worker.info.nullMovePruningTried++;

        // make move
        tree.makeNullMove();

        Value currval = -search(tree, !black, newDepth, -beta, -beta+1, newStat);

        // unmake move
        tree.unmakeNullMove();

        // 中断判定
        if (isInterrupted(tree)) {
          return Value::Zero;
        }

        // beta-cut
        if (currval >= beta) {
          tree.updatePVNull(depth);
          tree.getCurrentNode().isHistorical = tree.getChildNode().isHistorical;
          worker.info.nullMovePruning++;
          alpha = beta;
          if (newDepth < Depth1Ply) {
            goto hash_store;
          }
          goto search_end;
        }

        // mate threat
        if (currval <= -Value::Mate) {
          stat.setMateThreat();
        }
      }

#if ENABLE_PROBCUT
      if (isNullWindow && depth >= Depth1Ply * 5 && beta > -Value::Mate && beta < Value::Mate) {
        Value pcBeta = beta + 200;
        int bcDepth = depth - Depth1Ply * 4;

        Value val = search(tree, black, bcDepth, pcBeta-1, pcBeta, stat);

        worker.info.probcutTried++;
        if (val >= pcBeta) {
#if ENABLE_PROBCUT_EXPT
          isProbcut = true;
#else
          worker.info.probcut++;
          return pcBeta;
#endif
        }
      }
#endif
    }
  }

  // recursive iterative-deepening search
  if (hashMove.isEmpty() && stat.isRecursion() && depth >= search_param::REC_THRESHOLD) {
    auto newStat = NodeStat(stat).unsetNullMove().unsetMate().unsetHashCut();
    search(tree, black, search_func::recDepth(depth), alpha, beta, newStat);

    // 中断判定
    if (isInterrupted(tree)) {
      return Value::Zero;
    }

    // ハッシュ表から前回の最善手を取得
    TTE tte;
    if (tt_.get(hash, tte)) {
      hashMove = Move::deserialize16(tte.getMove(), tree.getBoard());
      hashValue = tte.getValue(tree.getPly());
      hashValueType = tte.getValueType();
      hashDepth = tte.getDepth();
    }
  }

#if ENABLE_SINGULAR_EXTENSION
  // singular extension
  if (!hashMove.isEmpty() &&
      tree.getExcluded().isEmpty() &&
      !tree.isChecking() &&
      depth >= Depth1Ply * 8 &&
      hashValue < Value::Mate &&
      hashValue > -Value::Mate &&
      (hashValueType == TTE::Lower || hashValueType == TTE::Exact) &&
      hashDepth >= depth - Depth1Ply * 3 &&
      tree.getBoard().isValidMoveStrict(hashMove) &&
      !tree.getBoard().isCheck(hashMove) &&
      !(stat.isRecapture() && tree.isRecapture(hashMove) &&
        (hashMove == tree.getCapture1() ||
        (hashMove == tree.getCapture2() && tree.getCapture1Value() < tree.getCapture2Value() + 180)))) {
    auto newStat = NodeStat(stat).unsetNullMove().unsetMate().unsetHashCut();
    Value sBeta = hashValue - 4 * depth / Depth1Ply;

    tree.setExcluded(hashMove);
    Value val = search(tree, black, depth / 2, sBeta-1, sBeta, newStat);
    tree.setExcluded(Move::empty());

    worker.info.singularChecked++;
    if (val < sBeta) {
      worker.info.singular++;
      doSingularExtension = true;
    }
  }
#endif

  tree.initGenPhase();
  worker.info.expand++;
#if ENABLE_HASH_MOVE
  tree.setHash(hashMove);
#else
  tree.setHash(Move::empty());
#endif
  while (nextMove(tree)) {
    Move move = *tree.getCurrentMove();

    if (move == tree.getExcluded()) {
      continue;
    }

    worker.info.expanded++;

    // depth
    int newDepth = depth - Depth1Ply;

    // stat
    NodeStat newStat = NodeStat::Default;

    isNullWindow = (beta == alpha + 1);

    const auto& board = tree.getBoard();
    bool isCheckCurr = board.isCheck(move);
    bool isCheckPrev = tree.isChecking();
    bool isCheck = isCheckCurr || isCheckPrev;
    Piece captured = board.getBoardPiece(move.to());
    int count = tree.getCurrentNode().count;

    if (!isCheckCurr && captured.isEmpty() &&
        (!move.promote() || move.piece() == Piece::Silver)) {
      tree.getCurrentNode().histMoves.add(move);
    }

    // extensions
    if (isCheckCurr) {
      // check
      newDepth += search_param::EXT_CHECK;
      worker.info.checkExtension++;

    } else if (isCheckPrev && isFirst && tree.getGenPhase() == GenPhase::End && tree.getNextMove() == tree.getEnd()) {
      // one-reply
      newDepth += search_param::EXT_ONEREP;
      worker.info.onerepExtension++;

    } else if (!isCheckPrev && stat.isRecapture() && tree.isRecapture(move) &&
               (move == tree.getCapture1() ||
                (move == tree.getCapture2() && tree.getCapture1Value() < tree.getCapture2Value() + 180))
               ) {
      // recapture
      Move fmove = tree.getFrontMove();
      if (!move.promote() && fmove.piece() == fmove.captured()) {
        newDepth += search_param::EXT_RECAP2;
      } else {
        newDepth += search_param::EXT_RECAP;
      }
      newStat.unsetRecapture();
      worker.info.recapExtension++;

    } else if (move == hashMove && doSingularExtension) {
      newDepth += Depth1Ply;
    }

    // late move reduction
    int reduced = 0;
#if ENABLE_LMR
    if (!isFirst && !isCheckPrev && newDepth >= Depth1Ply && !stat.isMateThreat() &&
        captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
        !tree.isPriorMove(move)) {
      reduced = getReductionDepth(improving, newDepth, count, move, isNullWindow);
      newDepth -= reduced;
    }
#endif // ENABLE_LMR

#if ENABLE_MOVE_COUNT_EXPT
    bool isMoveCountPrun = false;
#endif

#if ENABLE_MOVE_COUNT_PRUNING
    // move count based pruning
    if (!isCheckPrev && newDepth < Depth1Ply * 12 &&
        alpha > oldAlpha && !stat.isMateThreat() &&
        captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
        !tree.isPriorMove(move) &&
        count >= search_func::futilityMoveCounts(improving, depth)) {
#if ENABLE_MOVE_COUNT_EXPT
      isMoveCountPrun = true;
#else
      isFirst = false;
      worker.info.moveCountPruning++;
      continue;
#endif
    }
#endif

#if ENABLE_FUT_EXPT
    bool isFutPrun = false;
#endif

    // futility pruning
    if (!isCheck && newDepth < search_param::FUT_DEPTH && alpha > -Value::Mate) {
      Value futAlpha = alpha;
      if (newDepth >= Depth1Ply) { futAlpha -= search_func::futilityMargin(newDepth, count); }
      if (standPat + tree.estimate(move, eval_) + gains_.get(move) <= futAlpha) {
#if ENABLE_FUT_EXPT
        isFutPrun = true;
#else
        isFirst = false;
        worker.info.futilityPruning++;
        continue;
#endif
      }
    }

    // prune moves with negative SEE
    if (!isCheck && newDepth < Depth1Ply * 2 && isNullWindow &&
        captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
        !tree.isPriorMove(move)) {
      if (searchSee<true>(board, move, -1, 0) < Value::Zero) {
        isFirst = false;
        continue;
      }
    }

    // make move
    if (!tree.makeMove(eval_)) {
      continue;
    }

    Value newStandPat = tree.getValue() * (black ? 1 : -1);

    // extended futility pruning
    if (!isCheck && alpha > -Value::Mate) {
      if ((newDepth < Depth1Ply && newStandPat <= alpha) ||
          (newDepth < search_param::FUT_DEPTH && newStandPat + search_func::futilityMargin(newDepth, count) <= alpha)) {
#if ENABLE_FUT_EXPT
        isFutPrun = true;
#else
        tree.unmakeMove();
        isFirst = false;
        worker.info.extendedFutilityPruning++;
        continue;
#endif
      }
    }

    // reccursive call
    Value currval;
    if (isFirst) {
      currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);

    } else {
      // nega-scout
      currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);

      if (!isInterrupted(tree) && currval > alpha && reduced > 0) {
        newDepth += reduced;
        currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);
      }

      if (!isInterrupted(tree) && currval > alpha && currval < beta && !isNullWindow) {
        currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);
      }

    }

    // unmake move
    tree.unmakeMove();

    // 中断判定
    if (isInterrupted(tree)) {
      return Value::Zero;
    }

    // update gain
    gains_.update(move, newStandPat - standPat - tree.estimate(move, eval_));

    tree.getCurrentNode().isHistorical |= tree.getChildNode().isHistorical;

    // 値更新
    if (currval > alpha) {
#if ENABLE_MOVE_COUNT_EXPT
      if (isMoveCountPrun) { expt::move_count_based_pruning.fail(newDepth); }
#endif
#if ENABLE_FUT_EXPT
      if (isFutPrun) { expt::futility_pruning.fail(newDepth); }
#endif

      alpha = currval;
      best = move;
      if (!isNullWindow) {
        tree.updatePV(depth);
      }

      // beta-cut
      if (currval >= beta) {
        tree.getCurrentNode().isHistorical = tree.getChildNode().isHistorical;
        worker.info.failHigh++;
        if (isFirst) {
          worker.info.failHighFirst++;
        }
        if (move == tree.getHash()) {
          worker.info.failHighIsHash++;
        } else if (move == tree.getKiller1()) {
          worker.info.failHighIsKiller1++;
        } else if (move == tree.getKiller2()) {
          worker.info.failHighIsKiller2++;
        }
        break;
      }
    } else {
#if ENABLE_MOVE_COUNT_EXPT
      if (isMoveCountPrun) { expt::move_count_based_pruning.succ(newDepth); }
#endif
#if ENABLE_FUT_EXPT
      if (isFutPrun) { expt::futility_pruning.succ(newDepth); }
#endif
    }

    // split
    if ((depth >= Depth1Ply * 4 || (depth >= Depth1Ply * 3 && rootDepth_ < Depth1Ply * 12)) &&
        (!isCheckPrev || tree.getEnd() - tree.getCurrentMove() >= 4) &&
        idleWorkerCount_.load() >= 1 && idleTreeCount_.load() >= 2) {
      if (split(tree, black, depth, alpha, beta, best, standPat, stat, improving)) {
        worker.info.split++;
        if (isInterrupted(tree)) {
          return Value::Zero;
        }
        if (tree.getTlp().alpha > alpha) {
          alpha = tree.getTlp().alpha;
          best = tree.getTlp().best;
        }
        break;
      }
    }

    isFirst = false;

  }

  if (!best.isEmpty() && !tree.isChecking()) {
#if ENABLE_KILLER_MOVE
    updateKiller(tree, best);
#endif // ENABLE_KILLER_MOVE
    if (tree.getBoard().getBoardPiece(best.to()).isEmpty() &&
        (!best.promote() || best.piece() == Piece::Silver)) {
      updateHistory(tree, depth, best);
    }
  }

hash_store:
  if (
#if !defined(NLEARN)
      !config_.learning &&
#endif
      !tree.getCurrentNode().isHistorical) {
    TTStatus status = tt_.entry(hash, oldAlpha, beta, alpha, depth, tree.getPly(), Move::serialize16(best), stat);
    switch (status) {
      case TTStatus::New: worker.info.hashNew++; break;
      case TTStatus::Update: worker.info.hashUpdate++; break;
      case TTStatus::Collide: worker.info.hashCollision++; break;
      case TTStatus::Reject: worker.info.hashReject++; break;
      default: break;
    }
    worker.info.hashStore++;
  }

#if ENABLE_RAZOR_EXPT
  if (oldAlpha != alpha) {
    if (isRazoring) { expt::razoring.fail(depth); }
  } else {
    if (isRazoring) { expt::razoring.succ(depth); }
  }
#endif
#if ENABLE_PROBCUT_EXPT
  if (alpha >= beta) {
    if (isProbcut) { expt::probcut.succ(depth); }
  } else {
    if (isProbcut) { expt::probcut.fail(depth); }
  }
#endif

search_end:
  return alpha;

}

void Searcher::releaseTree(int tid) {
  auto& tree = trees_[tid];
  auto& parent = trees_[tree.getTlp().parentTreeId];
  tree.unuse();
  idleTreeCount_.fetch_add(1);
  parent.getTlp().childCount.fetch_sub(1);
}

/**
 * split
 */
bool Searcher::split(Tree& parent, bool black, int depth, Value alpha, Value beta, Move best, Value standPat, NodeStat stat, bool improving) {
  int currTreeId = Tree::InvalidId;

  {
    std::lock_guard<std::mutex> lock(splitMutex_);

    if (idleTreeCount_.load() <= 1 || idleWorkerCount_.load() == 0) {
      return false;
    }

    // カレントスレッドに割り当てる tree を決定
    for (int tid = 1; tid < config_.treeSize; tid++) {
      auto& tree = trees_[tid];
      if (!tree.getTlp().used) {
        currTreeId = tid;
        tree.use(parent, parent.getTlp().workerId);
        idleTreeCount_.fetch_sub(1);
        break;
      }
    }

    // 他の worker と tree を確保
    int childCount = 1;
    for (int wid = 0; wid < config_.workerSize; wid++) {
      auto& worker = workers_[wid];
      if (!worker.job.load()) {
        for (int tid = 1; tid < config_.treeSize; tid++) {
          auto& tree = trees_[tid];
          if (!tree.getTlp().used) {
            tree.use(parent, wid);
            idleTreeCount_.fetch_sub(1);

            worker.setJob(tid);
            idleWorkerCount_.fetch_sub(1);

            childCount++;

            if (idleTreeCount_.load() == 0 || idleWorkerCount_.load() == 0) {
              goto lab_assign_end;
            }

            break;
          }
        }
      }
    }
    lab_assign_end:
      ;

    parent.getTlp().childCount.store(childCount);
    parent.getTlp().black     = black;
    parent.getTlp().depth     = depth;
    parent.getTlp().alpha     = alpha;
    parent.getTlp().beta      = beta;
    parent.getTlp().best      = best;
    parent.getTlp().standPat  = standPat;
    parent.getTlp().stat      = stat;
    parent.getTlp().improving = improving;
  }

  auto& tree = trees_[currTreeId];
  auto& worker = workers_[parent.getTlp().workerId];
  worker.swapTree(currTreeId);
  searchTlp(tree);
  worker.swapTree(parent.getTlp().treeId);

  {
    std::lock_guard<std::mutex> lock(splitMutex_);
    tree.unuse();
    idleTreeCount_.fetch_add(1);
    parent.getTlp().childCount.fetch_sub(1);
  }

  if (!tree.getTlp().shutdown.load()) {
    // suspend
    worker.waitForJob(&parent);
  } else {
    // wait for brothers
    while (true) {
      if (parent.getTlp().childCount.load() == 0) {
        break;
      }
      std::this_thread::yield();
    }
  }

  return true;
}

void Searcher::searchTlp(Tree& tree) {
  {
    std::lock_guard<std::mutex> lock(splitMutex_);
  }

  auto& parent = trees_[tree.getTlp().parentTreeId];
  auto& worker = getWorker(tree);

  bool black = parent.getTlp().black;
  int depth = parent.getTlp().depth;
  Value beta = parent.getTlp().beta;
  Value standPat = parent.getTlp().standPat;
  NodeStat stat = parent.getTlp().stat;
  bool improving = parent.getTlp().improving;

  tree.initGenPhase();

  while (true) {
    Move move;
    Value alpha;
    const auto& board = tree.getBoard();
    bool isCheckCurr;
    bool isCheckPrev = tree.isChecking();
    bool isCheck;
    bool isPriorMove;
    Piece captured;
    int count;

    {
      std::lock_guard<std::mutex> lock(parent.getMutex());

      if (!nextMove(parent)) {
        return;
      }

      move = *parent.getCurrentMove();
      count = parent.getCurrentNode().count;
      alpha = parent.getTlp().alpha;
      captured = board.getBoardPiece(move.to());
      isCheckCurr = board.isCheck(move);
      isCheck = isCheckCurr || isCheckPrev;
      isPriorMove = parent.isPriorMove(move);

      if (!isCheckCurr && captured.isEmpty() &&
          (!move.promote() || move.piece() == Piece::Silver)) {
        parent.getCurrentNode().histMoves.add(move);
      }
    }

    tree.addMove(move);
    tree.selectNextMove();

    // depth
    int newDepth = depth - Depth1Ply;

    // stat
    NodeStat newStat = NodeStat::Default;

    bool isNullWindow = (beta == alpha + 1);

    // extensions
    if (isCheckCurr) {
      // check
      newDepth += search_param::EXT_CHECK;
      worker.info.checkExtension++;

    } else if (!isCheckPrev && stat.isRecapture() && parent.isRecapture(move) &&
               (move == parent.getCapture1() ||
                (move == parent.getCapture2() && parent.getCapture1Value() < parent.getCapture2Value() + 180))
               ) {
      // recapture
      Move fmove = parent.getFrontMove();
      if (!move.promote() && fmove.piece() == fmove.captured()) {
        newDepth += search_param::EXT_RECAP2;
      } else {
        newDepth += search_param::EXT_RECAP;
      }
      newStat.unsetRecapture();
      worker.info.recapExtension++;

    }

    // late move reduction
    int reduced = 0;
#if ENABLE_LMR
    if (!isCheckPrev && newDepth >= Depth1Ply && !stat.isMateThreat() &&
        captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
        !isPriorMove) {
      reduced = getReductionDepth(improving, newDepth, count, move, isNullWindow);
      newDepth -= reduced;
    }
#endif // ENABLE_LMR

#if ENABLE_MOVE_COUNT_PRUNING
    // move count based pruning
    if (!isCheckPrev && newDepth < Depth1Ply * 12 &&
        /*alpha > oldAlpha &&*/ !stat.isMateThreat() &&
        captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
        !tree.isPriorMove(move) &&
        count >= search_func::futilityMoveCounts(improving, depth)) {
      worker.info.moveCountPruning++;
      continue;
    }
#endif

    // futility pruning
    if (!isCheck && newDepth < search_param::FUT_DEPTH && alpha > -Value::Mate) {
      Value futAlpha = alpha;
      if (newDepth >= Depth1Ply) { futAlpha -= search_func::futilityMargin(newDepth, count); }
      if (standPat + tree.estimate(move, eval_) + gains_.get(move) <= futAlpha) {
        worker.info.futilityPruning++;
        continue;
      }
    }

    if (newDepth < Depth1Ply * 2 && isNullWindow && !isCheck &&
        captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver) &&
        !isPriorMove) {
      if (searchSee<true>(board, move, -1, 0) < Value::Zero) {
        continue;
      }
    }

    if (!tree.makeMove(eval_)) {
      continue;
    }

    Value newStandPat = tree.getValue() * (black ? 1 : -1);

    // extended futility pruning
    if (!isCheck && alpha > -Value::Mate) {
      if ((newDepth < Depth1Ply && newStandPat <= alpha) ||
          (newDepth < search_param::FUT_DEPTH && newStandPat + search_func::futilityMargin(newDepth, count) <= alpha)) {
        tree.unmakeMove();
        worker.info.extendedFutilityPruning++;
        continue;
      }
    }

    // nega-scout
    Value currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);

    if (!isInterrupted(tree) && currval > alpha && reduced > 0) {
      newDepth += reduced;
      currval = -search(tree, !black, newDepth, -alpha-1, -alpha, newStat);
    }

    if (!isInterrupted(tree) && currval > alpha && currval < beta && !isNullWindow) {
      currval = -search(tree, !black, newDepth, -beta, -alpha, newStat);
    }

    // unmake move
    tree.unmakeMove();

    // 中断判定
    if (isInterrupted(tree)) {
      return;
    }

    // update gain
    gains_.update(move, newStandPat - standPat - tree.estimate(move, eval_));

    {
      std::lock_guard<std::mutex> lock(parent.getMutex());

      if (tree.getTlp().shutdown.load()) {
        return;
      }

      // 値更新
      if (currval > parent.getTlp().alpha) {
        parent.getTlp().alpha = currval;
        parent.getTlp().best = move;
        if (!isNullWindow) {
          parent.updatePV(depth, tree);
        }

        // beta-cut
        if (currval >= parent.getTlp().beta) {
          parent.getCurrentNode().isHistorical = tree.getChildNode().isHistorical;
          worker.info.failHigh++;
          if (move == parent.getHash()) {
            worker.info.failHighIsHash++;
          } else if (move == parent.getKiller1()) {
            worker.info.failHighIsKiller1++;
          } else if (move == parent.getKiller2()) {
            worker.info.failHighIsKiller2++;
          }
          shutdownSiblings(parent);
        }
      }
    }
  }
}

void Searcher::shutdownSiblings(Tree& parent) {
  std::lock_guard<std::mutex> lock(splitMutex_);

  for (int id = 0; id < config_.treeSize; id++) {
    int parentId = trees_[id].getTlp().parentTreeId;
    while (parentId != Tree::InvalidId) {
      if (parentId == parent.getTlp().treeId) {
        trees_[id].getTlp().shutdown.store(true);
        break;
      }
      parentId = trees_[parentId].getTlp().parentTreeId;
    }
  }
}

/**
 * search on root node
 */
Value Searcher::searchRoot(Tree& tree, int depth, Value alpha, Value beta, Move& best,
    bool breakOnFailLow /*= false*/, bool forceFullWindow /*= false*/) {
  const auto& board = tree.getBoard();
  bool black = board.isBlack();
  bool isFirst = true;
  Value oldAlpha = alpha;

  rootDepth_ = depth;

  while (nextMove(tree)) {
    Move move = *tree.getCurrentMove();

    // depth
    int newDepth = depth - Depth1Ply;

    bool isCheckPrev = board.isChecking();
    bool isCheckCurr = board.isCheck(move);
    Piece captured = board.getBoardPiece(move.to());

    // extensions
    if (isCheckCurr) {
      // check
      newDepth += search_param::EXT_CHECK;
      info_.checkExtension++;
    }

    // late move reduction
    int reduced = 0;
#if ENABLE_LMR
    if (!isFirst && newDepth >= Depth1Ply * 2 && !isCheckPrev &&
        captured.isEmpty() && (!move.promote() || move.piece() == Piece::Silver)) {
      reduced = Depth1Ply;
      newDepth -= reduced;
    }
#endif // ENABLE_LMR

    // make move
    bool ok = tree.makeMove(eval_);
    assert(ok);

    Value currval;

    if (forceFullWindow) {
      currval = -search(tree, !black, newDepth, -beta, -oldAlpha);

    } else if (isFirst) {
      // full window search
      currval = -search(tree, !black, newDepth, -beta, -alpha);

    } else {
      // nega-scout
      currval = -search(tree, !black, newDepth, -alpha-1, -alpha);

      if (!isInterrupted(tree) && currval >= alpha + 1 && reduced != 0) {
        // full depth
        newDepth += reduced;
        currval = -search(tree, !black, newDepth, -alpha-1, -alpha);
      }

      if (!isInterrupted(tree) && currval >= alpha + 1) {
        // full window search
        currval = -search(tree, !black, newDepth, -beta, -alpha);
      }
    }

    // unmake move
    tree.unmakeMove();

    // 中断判定
    if (isInterrupted(tree)) {
      return alpha;
    }

    if (breakOnFailLow && isFirst && currval <= alpha && currval > -Value::Mate) {
      return currval;
    }

    // ソート用に値をセット
    auto index = tree.getIndexByMove(move);
    if (forceFullWindow || currval > alpha) {
      rootValues_[index] = currval.int32();
    } else {
      rootValues_[index] = -Value::Inf;
    }

    timeManager_.addMove(move, currval);

    // 値更新
    if (currval > alpha) {
      // update alpha
      alpha = currval;
      best = move;
      tree.updatePV(depth);

      if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD || currval >= Value::Mate) {
        showPV(depth / Depth1Ply, tree.getPV(), black ? currval : -currval);
      }
      info_.lastDepth = depth / Depth1Ply;

      // beta-cut or update best move
      if (alpha >= beta) {
        return alpha;
      }
    }

    isFirst = false;
  }

  return alpha;
}

/**
 * aspiration search
 * @return {負けたか中断された場合にfalseを返します。}
 */
bool Searcher::searchAsp(int depth, Move& best, Value baseAlpha, Value baseBeta, Value& valueRef) {
  auto& tree0 = trees_[0];

  Value baseVal = valueRef;
  baseVal = Value::max(baseVal, baseAlpha);
  baseVal = Value::min(baseVal, baseBeta);

  CONSTEXPR_CONST int wmax = 3;
  int lower = 0;
  int upper = 0;
  const Value alphas[wmax] = { baseVal-198, baseVal-793, -Value::Mate };
  const Value betas[wmax] = { baseVal+198, baseVal+793, Value::Mate };

  Value value = alphas[lower];

  while (true) {

    timeManager_.startDepth();

    const Value alpha = Value::max(alphas[lower], baseAlpha);
    const Value beta = Value::min(betas[upper], baseBeta);

    value = searchRoot(tree0, depth, alpha, beta, best, true);

    // 中断判定
    if (isInterrupted(tree0)) {
      return false;
    }

    // 値が確定
    if (value > alpha && value < beta) {
      break;
    }

    bool retry = false;

    if (value <= baseAlpha || value >= baseBeta) {
      break;
    }

    // alpha 値を広げる
    while (value <= alphas[lower] && alphas[lower] > baseAlpha && lower != wmax - 1) {
      lower++;
      assert(lower < wmax);
      retry = true;
      tree0.selectFirstMove();
    }

    // beta 値を広げる
    while (value >= betas[upper] && betas[upper] < baseBeta && upper != wmax - 1) {
      upper++;
      assert(upper < wmax);
      retry = true;
      tree0.selectPreviousMove();
    }

    if (!retry) { break; }
  }

  if (depth >= Depth1Ply * ITERATE_INFO_THRESHOLD ||
      value >= Value::Mate || value <= -Value::Mate) {
    showEndOfIterate(depth / Depth1Ply);
  }
  info_.lastDepth = depth / Depth1Ply;

  tree0.setSortValues(rootValues_);
  tree0.sortAll();

  info_.eval = value;
  valueRef = value;

  if (value <= -Value::Mate) {
    return false;
  }

  if (!config_.ponder && config_.enableTimeManagement) {
    float limit = config_.enableLimit ? config_.limitSeconds : 0.0;
    if (timeManager_.isEasy(limit, timer_.get())) {
      return false;
    }
  }

  return true;

}

void Searcher::showPV(int depth, const PV& pv, const Value& value) {
  if (!config_.logging) {
    return;
  }

  // 探索情報集計
  mergeInfo();

  uint64_t node = info_.node + info_.qnode;

  std::ostringstream oss;
  if (info_.lastDepth == depth) {
    oss << "    ";
  } else {
    oss << std::setw(2) << depth << ": ";
  }
  oss << std::setw(10) << node << ": ";
  oss << pv.toString() << ": ";
  oss << value.int32();
  Loggers::message << oss.str();
}

void Searcher::showEndOfIterate(int depth) {
  if (!config_.logging) {
    return;
  }

  // 探索情報集計
  mergeInfo();

  uint64_t node = info_.node + info_.qnode;
  float seconds = timer_.get();

  std::ostringstream oss;
  if (info_.lastDepth == depth) {
    oss << "    ";
  } else {
    oss << std::setw(2) << depth << ": ";
  }
  oss << std::setw(10) << node;
  oss << ": " << seconds << "sec";
  Loggers::message << oss.str();
}

void Searcher::generateMovesOnRoot() {
  auto& tree0 = trees_[0];

  auto& moves = tree0.getMoves();
  const auto& board = tree0.getBoard();

  // 合法手生成
  tree0.initGenPhase();
  MoveGenerator::generate(board, moves);
  tree0.resetGenPhase();

  // 非合法手除去
  for (auto ite = moves.begin(); ite != moves.end();) {
    if (!board.isValidMove(*ite)) {
      ite = moves.remove(ite);
    } else {
      ++ite;
    }
  }

#if ENABLE_ROOT_MOVES_SHUFFLE
  // シャッフル
  random_.shuffle(moves.begin(), moves.end());
#endif
}

/**
 * 指定した局面に対して探索を実行します。
 * @return {負けたか中断された場合にfalseを返します。}
 */
bool Searcher::search(const Board& initialBoard, Move& best,
                      Value alpha, Value beta, bool fastStart) {
  // 前処理
  before(initialBoard, fastStart);

  auto& tree0 = trees_[0];
  int depth = config_.maxDepth;

  generateMovesOnRoot();

  Value value = searchRoot(tree0, depth * Depth1Ply + Depth1Ply / 2, alpha, beta, best);

  info_.lastDepth = depth;
  info_.eval = value;

  bool result = value > alpha;

  // 中断判定
  if (isInterrupted(tree0)) {
    result = false;
  }

  // 後処理
  after();

  return result;

}

/**
 * iterative deepening search from root node
 * @return {負けたか深さ1で中断された場合にfalseを返します。}
 */
bool Searcher::idsearch(Move& best, Value alpha, Value beta) {
  auto& tree0 = trees_[0];
  bool result = false;

  generateMovesOnRoot();

  Value value = searchRoot(tree0, Depth1Ply, -Value::Inf, Value::Inf, best, false, true);
  tree0.setSortValues(rootValues_);
  tree0.sortAll();

  for (int depth = 1; depth <= config_.maxDepth; depth++) {
    bool cont = searchAsp(depth * Depth1Ply + Depth1Ply / 2, best, alpha, beta, value);

#if DEBUG_ROOT_MOVES
    std::ostringstream oss;
    for (auto ite = tree0.getBegin(); ite != tree0.getEnd(); ite++) {
      oss << ' ' << (*ite).toString() << '[' << tree0.getSortValue(ite) << ']';
    }
    Loggers::debug << oss.str();
#endif

#if ENABLE_STORE_PV
    storePV(tree0, tree0.getPV(), 0);
#endif // ENABLE_STORE_PV

    if (value >= beta) {
      result = true;
      break;
    }

    if (value <= alpha) {
      result = false;
      break;
    }

    if (!cont) {
      break;
    }

    result = true;

    timeManager_.nextDepth();
  }

  return result;

}

/**
 * 指定した局面に対して反復深化探索を実行します。
 * @return {負けたか深さ1で中断された場合にfalseを返します。}
 */
bool Searcher::idsearch(const Board& initialBoard, Move& best,
                        Value alpha, Value beta, bool fastStart) {

  // 前処理
  before(initialBoard, fastStart);

  bool result = idsearch(best, alpha, beta);

  // 後処理
  after();

  return result;

}

} // namespace sunfish
