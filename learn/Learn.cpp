/* Learn.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "./Learn.h"
#include "core/move/MoveGenerator.h"
#include "core/record/CsaReader.h"
#include "core/util/FileList.h"
#include "core/def.h"
#include "logger/Logger.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>

#define CONF_KIFU               "kifu"
#define CONF_DEPTH              "depth"
#define CONF_THREADS            "threads"

#define CONFPATH                "learn.conf"

#define SEARCH_WINDOW           256
#define NUMBER_OF_SIBLING_NODES 16
#define MINI_BATCH_COUNT        256
#define MINI_BATCH_SCALE        ((1.0f * ValuePair::PositionalScale) / (NUMBER_OF_SIBLING_NODES * MINI_BATCH_COUNT))

#define ENABLE_THREAD_PAIRING   0

namespace sunfish {

namespace {

  void initSearcherConfig(Searcher& searcher, int snt) {
    auto searchConfig = searcher.getConfig();
    searchConfig.workerSize = snt;
    searchConfig.treeSize = Searcher::standardTreeSize(snt);
    searchConfig.enableLimit = false;
    searchConfig.enableTimeManagement = false;
    searchConfig.ponder = false;
    searchConfig.logging = false;
    searcher.setConfig(searchConfig);
  }

  void setSearcherDepth(Searcher& searcher, int depth) {
    auto searchConfig = searcher.getConfig();
    searchConfig.maxDepth = depth;
    searcher.setConfig(searchConfig);
  }

  Board getPVLeaf(const Board& root, const Move& rmove, const PV& pv) {
    Board board = root;
    board.makeMoveIrr(rmove);
    for (int d = 0; d < pv.size(); d++) {
      Move move = pv.get(d).move;
      if (move.isEmpty() || !board.makeMove(move)) {
        break;
      }
    }
    return board;
  }

  inline float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
  }

  inline float gradient(float x) {
    CONSTEXPR float a = 0.025f;
    CONSTEXPR float b = 32.0f * MINI_BATCH_SCALE;
    float s = sigmoid(a * x);
    return (1.0f * s - s * s) * (4.0f * b);
  }

  inline float norm(float x) {
    CONSTEXPR float n = 0.01f * MINI_BATCH_SCALE;
    if (x > 0.0f) {
      return -n;
    } else if (x < 0.0f) {
      return n;
    } else {
      return 0.0f;
    }
  }

}

/**
 * コンストラクタ
 */
Learn::Learn() {
  config_.addDef(CONF_KIFU, "");
  config_.addDef(CONF_DEPTH, "3");
  config_.addDef(CONF_THREADS, "1");
}

/**
 * 勾配を計算します.
 *
 */
void Learn::genGradient(int wn, const Job& job) {
  Board board(job.board);
  Move move0 = job.move;
  Value val0;
  PV pv0;
  Move tmpMove;

  bool black = board.isBlack();

  // 合法手生成
  Moves moves;
  MoveGenerator::generate(board, moves);

  if (moves.size() < 2) {
    return;
  }

  // シャッフル
  std::shuffle(moves.begin(), moves.end(), rgens_[wn]);

  searchers_[wn]->clearHistory();

  // 棋譜の手
  {
    // 探索
    board.makeMove(move0);
    setSearcherDepth(*searchers_[wn], config_.getInt(CONF_DEPTH));
    searchers_[wn]->idsearch(board, tmpMove);
    board.unmakeMove(move0);

    // PV と評価値
    const auto& info = searchers_[wn]->getInfo();
    const auto& pv = info.pv;
    val0 = -info.eval;
    pv0.copy(pv);

    // 詰みは除外
    if (val0 <= -Value::Mate || val0 >= Value::Mate) {
      return;
    }
  }

  // 棋譜の手の評価値から window を決定
  Value alpha = -val0 - SEARCH_WINDOW;
  Value beta = -val0 + SEARCH_WINDOW;

  // その他の手
  int nmove = 0;
  float gsum = 0;
  for (auto& move : moves) {
    // 探索
    CONSTEXPR int reduction = 1;
    bool valid = board.makeMove(move);
    if (!valid) { continue; }
    setSearcherDepth(*searchers_[wn], config_.getInt(CONF_DEPTH) - reduction);
    searchers_[wn]->idsearch(board, tmpMove, alpha, beta);
    board.unmakeMove(move);

    // PV と評価値
    const auto& info = searchers_[wn]->getInfo();
    const auto& pv = info.pv;
    Value val = -info.eval;

    // window を外れた場合は除外
    if (val <= alpha || val >= beta) {
      continue;
    }

    // leaf 局面
    Board leaf = getPVLeaf(board, move, pv);

    // 特徴抽出
    float g = gradient(val.int32() - val0.int32());
    g = g * (black ? 1 : -1);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      g_.extract<float, true>(leaf, -g);
    }
    gsum += g;

    nmove++;
    if (nmove >= NUMBER_OF_SIBLING_NODES) {
      break;
    }
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);

    // leaf 局面
    Board leaf = getPVLeaf(board, move0, pv0);

    // 特徴抽出
    g_.extract<float, true>(leaf, gsum);
  }
}

/**
 * ジョブを拾います。
 */
void Learn::work(int wn) {
  while (!shutdown_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    Job job;

    // dequeue
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (jobQueue_.empty()) {
        continue;
      }
      job = jobQueue_.front();
      jobQueue_.pop();
      activeCount_++;
    }

    genGradient(wn, job);

    activeCount_--;
  }
}

/**
 * ミニバッチを実行します。
 */
bool Learn::miniBatch() {

  if (jobs_.size() < MINI_BATCH_COUNT) {
    return false;
  }

  Loggers::message << "mini-bach (" << count_ << ")";

  {
    std::lock_guard<std::mutex> lock(mutex_);

    for (int i = 0; i < MINI_BATCH_COUNT; i++) {
      jobQueue_.push(jobs_.back());
      jobs_.pop_back();
    }
  }

  // キューが空になるのを待つ
  while (true) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (jobQueue_.empty() && activeCount_ == 0) {
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // 値更新
  float max = 0.0f;
  float magnitude = 0.0f;
  for (int i = 0; i < KPP_ALL; i++) {
    float g = g_.t_->kpp[0][i] + norm(w_.t_->kpp[0][i]);
    g_.t_->kpp[0][i] = 0.0f;
    w_.t_->kpp[0][i] += g;
    u_.t_->kpp[0][i] += g * count_;
    eval_.t_->kpp[0][i] = w_.t_->kpp[0][i];
    max = std::max(max, std::abs(w_.t_->kpp[0][i]));
    magnitude += std::abs(w_.t_->kpp[0][i]);
  }
  for (int i = 0; i < KKP_ALL; i++) {
    float g = g_.t_->kkp[0][0][i] + norm(w_.t_->kkp[0][0][i]);
    g_.t_->kkp[0][0][i] = 0.0f;
    w_.t_->kkp[0][0][i] += g;
    u_.t_->kkp[0][0][i] += g * count_;
    eval_.t_->kkp[0][0][i] = w_.t_->kkp[0][0][i];
    max = std::max(max, std::abs(w_.t_->kkp[0][0][i]));
    magnitude += std::abs(w_.t_->kkp[0][0][i]);
  }

  Loggers::message << "max=" << max << " magnitude=" << magnitude;

  float elapsed = timer_.get();
  Loggers::message << "elapsed: " << elapsed;
  Loggers::message << "unprocessed jobs: " << jobs_.size();

  count_++;

  // ハッシュ表を初期化
  eval_.clearCache();
  for (uint32_t wn = 0; wn < nt_; wn++) {
    searchers_[wn]->clearTT();
    searchers_[wn]->clearSeeCache(); // 駒割りを学習しないなら関係ない
  }

  return true;
}

/**
 * 棋譜ファイルを読み込んで学習します。
 */
bool Learn::readCsa(size_t count, size_t total, const char* path) {
  Loggers::message << "loading (" << count << "/" << total << "): [" << path << "]";

  Record record;
  if (!CsaReader::read(path, record)) {
    Loggers::warning << "Could not read csa file. [" << path << "]";
    return false;
  }

  // 棋譜の先頭へ
  while (record.unmakeMove())
    ;

  while (true) {
    // 次の1手を取得
    Move move = record.getNextMove();
    if (move.isEmpty()) {
      break;
    }

    jobs_.push_back({ record.getBoard().getCompactBoard(), move });

    // 1手進める
    if (!record.makeMove()) {
      break;
    }
  }

  return true;
}

/**
 * 機械学習を実行します。
 */
bool Learn::run() {
  // 設定読み込み
  if (!config_.read(CONFPATH)) {
    return false;
  }
  Loggers::message << config_.toString();

  timer_.set();

  // csa ファイルを列挙
  FileList fileList;
  std::string dir = config_.getString(CONF_KIFU);
  fileList.enumerate(dir.c_str(), "csa");

  // 初期化
  eval_.init();
  count_ = 1;
  g_.init();
  w_.init();
  u_.init();

  // 学習スレッド数
  nt_ = config_.getInt(CONF_THREADS);

  // 探索スレッド数
#if ENABLE_THREAD_PAIRING
  int snt = nt_ >= 4 ? 2 : 1;
#else
  int snt = 1;
#endif
  nt_ = nt_ / snt;

  // Searcher生成
  uint32_t seed = static_cast<uint32_t>(time(NULL));
  rgens_.clear();
  searchers_.clear();
  for (uint32_t wn = 0; wn < nt_; wn++) {
    rgens_.emplace_back(seed);
    seed = rgens_.back()();
    searchers_.emplace_back(new Searcher(eval_));
    initSearcherConfig(*searchers_.back().get(), snt);
  }

  // 棋譜の取り込み
  size_t count = 0;
  for (const auto& filename : fileList) {
    readCsa(++count, fileList.size(), filename.c_str());
  }

  // 訓練データのシャッフル
  std::shuffle(jobs_.begin(), jobs_.end(), rgens_[0]);

  activeCount_ = 0;

  // ワーカースレッド生成
  shutdown_ = false;
  threads_.clear();
  for (uint32_t wn = 0; wn < nt_; wn++) {
    threads_.emplace_back(std::bind(std::mem_fn(&Learn::work), this, wn));
  }

  // 学習処理の実行
  while (true) {
    bool ok = miniBatch();
    if (!ok) {
      break;
    }
  }

  // ワーカースレッド停止
  shutdown_ = true;
  for (uint32_t wn = 0; wn < nt_; wn++) {
    threads_[wn].join();
  }

  Loggers::message << "publishing..";

  // 平均を取る
  uint16_t max = 0u;
  uint64_t magnitude = 0ull;
  uint32_t nonZero = 0u;
  for (int i = 0; i < KPP_ALL; i++) {
    eval_.t_->kpp[0][i] = w_.t_->kpp[0][i] - u_.t_->kpp[0][i] / count_;
    max = std::max(max, (uint16_t)std::abs(eval_.t_->kpp[0][i]));
    magnitude += std::abs(eval_.t_->kpp[0][i]);
    nonZero += eval_.t_->kpp[0][i] != 0 ? 1 : 0;
  }
  for (int i = 0; i < KKP_ALL; i++) {
    eval_.t_->kkp[0][0][i] = w_.t_->kkp[0][0][i] - u_.t_->kkp[0][0][i] / count_;
    max = std::max(max, (uint16_t)std::abs(eval_.t_->kkp[0][0][i]));
    magnitude += std::abs(eval_.t_->kkp[0][0][i]);
    nonZero += eval_.t_->kkp[0][0][i] != 0 ? 1 : 0;
  }

  Loggers::message << "[final] max=" << max << " magnitude=" << magnitude;
  Loggers::message << "[final] nonZero=" << nonZero << " zero=" << (KPP_ALL + KKP_ALL);

  // 重みベクトルを保存
  eval_.writeFile();

  float elapsed = timer_.get();
  Loggers::message << "[final] elapsed: " << elapsed;

  return true;
}

}

#endif // NLEARN
