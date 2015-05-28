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
#define MINI_BATCH_COUNT        128
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

  Board getPvLeaf(const Board& root, const Move& rmove, const Pv& pv) {
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
    return (1.0f * s - s * s) * b;
  }

  inline float norm(float x) {
    CONSTEXPR float n = 0.005f * MINI_BATCH_SCALE;
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
  _config.addDef(CONF_KIFU, "");
  _config.addDef(CONF_DEPTH, "3");
  _config.addDef(CONF_THREADS, "1");
}

/**
 * 勾配を計算します.
 *
 */
void Learn::genGradient(int wn, const Job& job) {
  Board board(job.board);
  Move move0 = job.move;
  Value val0;
  Pv pv0;
  Move tmpMove;

  bool black = board.isBlack();

  // 合法手生成
  Moves moves;
  MoveGenerator::generate(board, moves);

  if (moves.size() < 2) {
    return;
  }

  // シャッフル
  std::shuffle(moves.begin(), moves.end(), _rgens[wn]);

  // 棋譜の手
  {
    // 探索
    board.makeMove(move0);
    setSearcherDepth(*_searchers[wn], _config.getInt(CONF_DEPTH));
    _searchers[wn]->idsearch(board, tmpMove);
    board.unmakeMove(move0);

    // PV と評価値
    const auto& info = _searchers[wn]->getInfo();
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
    setSearcherDepth(*_searchers[wn], _config.getInt(CONF_DEPTH) - reduction);
    _searchers[wn]->idsearch(board, tmpMove, alpha, beta);
    board.unmakeMove(move);

    // PV と評価値
    const auto& info = _searchers[wn]->getInfo();
    const auto& pv = info.pv;
    Value val = -info.eval;

    // window を外れた場合は除外
    if (val <= alpha || val >= beta) {
      continue;
    }

    // leaf 局面
    Board leaf = getPvLeaf(board, move, pv);

    // 特徴抽出
    float g = gradient(val.int32() - val0.int32());
    g = g * (black ? 1 : -1);
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _g.extract<float, true>(leaf, -g);
    }
    gsum += g;

    nmove++;
    if (nmove >= NUMBER_OF_SIBLING_NODES) {
      break;
    }
  }

  {
    std::lock_guard<std::mutex> lock(_mutex);

    // leaf 局面
    Board leaf = getPvLeaf(board, move0, pv0);

    // 特徴抽出
    _g.extract<float, true>(leaf, gsum);
  }
}

/**
 * ジョブを拾います。
 */
void Learn::work(int wn) {
  while (!_shutdown) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    Job job;

    // dequeue
    {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_jobQueue.empty()) {
        continue;
      }
      job = _jobQueue.front();
      _jobQueue.pop();
      _activeCount++;
    }

    genGradient(wn, job);

    _activeCount--;
  }
}

/**
 * ミニバッチを実行します。
 */
bool Learn::miniBatch() {

  if (_jobs.size() < MINI_BATCH_COUNT) {
    return false;
  }

  Loggers::message << "mini-bach (" << _count << ")";

  {
    std::lock_guard<std::mutex> lock(_mutex);

    for (int i = 0; i < MINI_BATCH_COUNT; i++) {
      _jobQueue.push(_jobs.back());
      _jobs.pop_back();
    }
  }

  // キューが空になるのを待つ
  while (true) {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_jobQueue.empty() && _activeCount == 0) {
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // 値更新
  float max = 0.0f;
  float magnitude = 0.0f;
  for (int i = 0; i < KPP_ALL; i++) {
    float g = _g._t->kpp[0][i] + norm(_w._t->kpp[0][i]);
    _g._t->kpp[0][i] = 0.0f;
    _w._t->kpp[0][i] += g;
    _u._t->kpp[0][i] += g * _count;
    _eval._t->kpp[0][i] = _w._t->kpp[0][i];
    max = std::max(max, std::abs(_w._t->kpp[0][i]));
    magnitude += std::abs(_w._t->kpp[0][i]);
  }
  for (int i = 0; i < KKP_ALL; i++) {
    float g = _g._t->kkp[0][0][i] + norm(_w._t->kkp[0][0][i]);
    _g._t->kkp[0][0][i] = 0.0f;
    _w._t->kkp[0][0][i] += g;
    _u._t->kkp[0][0][i] += g * _count;
    _eval._t->kkp[0][0][i] = _w._t->kkp[0][0][i];
    max = std::max(max, std::abs(_w._t->kkp[0][0][i]));
    magnitude += std::abs(_w._t->kkp[0][0][i]);
  }

  Loggers::message << "max=" << max << " magnitude=" << magnitude;

  float elapsed = _timer.get();
  Loggers::message << "elapsed: " << elapsed;

  _count++;

  // TT を初期化
  for (uint32_t wn = 0; wn < _nt; wn++) {
    _searchers[wn]->clearTT();
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

    _jobs.push_back({ record.getBoard().getCheepBoard(), move });

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
  if (!_config.read(CONFPATH)) {
    return false;
  }
  Loggers::message << _config.toString();

  _timer.set();

  // csa ファイルを列挙
  FileList fileList;
  std::string dir = _config.getString(CONF_KIFU);
  fileList.enumerate(dir.c_str(), "csa");

  // 初期化
	_eval.init();
  _count = 1;
  _g.init();
  _w.init();
  _u.init();

  // 学習スレッド数
  _nt = _config.getInt(CONF_THREADS);

  // 探索スレッド数
#if ENABLE_THREAD_PAIRING
  int snt = _nt >= 4 ? 2 : 1;
#else
  int snt = 1;
#endif
  _nt = _nt / snt;

  // Searcher生成
  uint32_t seed = static_cast<uint32_t>(time(NULL));
  _rgens.clear();
  _searchers.clear();
  for (uint32_t wn = 0; wn < _nt; wn++) {
    _rgens.emplace_back(seed);
    seed = _rgens.back()();
    _searchers.emplace_back(new Searcher(_eval));
    initSearcherConfig(*_searchers.back().get(), snt);
  }

  // 棋譜の取り込み
  size_t count = 0;
  for (const auto& filename : fileList) {
    readCsa(++count, fileList.size(), filename.c_str());
  }

  // 訓練データのシャッフル
  std::shuffle(_jobs.begin(), _jobs.end(), _rgens[0]);

  _activeCount = 0;

  // ワーカースレッド生成
  _shutdown = false;
  _threads.clear();
  for (uint32_t wn = 0; wn < _nt; wn++) {
    _threads.emplace_back(std::bind(std::mem_fn(&Learn::work), this, wn));
  }

  // 学習処理の実行
  while (true) {
    bool ok = miniBatch();
    if (!ok) {
      break;
    }
  }

  // ワーカースレッド停止
  _shutdown = true;
  for (uint32_t wn = 0; wn < _nt; wn++) {
    _threads[wn].join();
  }

  Loggers::message << "publishing..";

  // 平均を取る
  uint16_t max = 0u;
  uint64_t magnitude = 0ull;
  uint32_t nonZero = 0u;
  for (int i = 0; i < KPP_ALL; i++) {
    _eval._t->kpp[0][i] = _w._t->kpp[0][i] - _u._t->kpp[0][i] / _count;
    max = std::max(max, (uint16_t)std::abs(_eval._t->kpp[0][i]));
    magnitude += std::abs(_eval._t->kpp[0][i]);
    nonZero += _eval._t->kpp[0][i] != 0 ? 1 : 0;
  }
  for (int i = 0; i < KKP_ALL; i++) {
    _eval._t->kkp[0][0][i] = _w._t->kkp[0][0][i] - _u._t->kkp[0][0][i] / _count;
    max = std::max(max, (uint16_t)std::abs(_eval._t->kkp[0][0][i]));
    magnitude += std::abs(_eval._t->kkp[0][0][i]);
    nonZero += _eval._t->kkp[0][0][i] != 0 ? 1 : 0;
  }

  Loggers::message << "[final] max=" << max << " magnitude=" << magnitude;
  Loggers::message << "[final] nonZero=" << nonZero << " zero=" << (KPP_ALL + KKP_ALL);

  // 重みベクトルを保存
  _eval.writeFile();

  float elapsed = _timer.get();
  Loggers::message << "[final] elapsed: " << elapsed;

  return true;
}

}

#endif // NLEARN
