/* Learn.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "./Learn.h"
#include "core/move/MoveGenerator.h"
#include "core/record/CsaReader.h"
#include "core/util/FileList.h"
#include "core/util/Timer.h"
#include "core/def.h"
#include "logger/Logger.h"
#include <algorithm>
#include <cmath>
#include <ctime>

#define CONF_KIFU               "kifu"
#define CONF_DEPTH              "depth"
#define CONF_THREADS            "threads"

#define CONFPATH                "learn.conf"

#define NUMBER_OF_SIBLING_NODES 8
#define MINI_BATCH_COUNT        20

namespace sunfish {

namespace {

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
    CONSTEXPR float b = 1.8f;
    float s = sigmoid(a * x);
    return (1.0f - s) * s * b;
  }

  inline float norm(float x) {
    CONSTEXPR float n = 0.01f;
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
Learn::Learn() : _rgen(static_cast<unsigned>(time(NULL))) {
  _config.addDef(CONF_KIFU, "");
  _config.addDef(CONF_DEPTH, "3");
  _config.addDef(CONF_THREADS, "1");
}

/**
 * 特徴抽出を行いパラメータを更新します。
 */
bool Learn::adjust(Board board, Move move0) {
  Value val0;
  Pv pv0;
  Move tmpMove;

  // 合法手生成
  Moves moves;
  MoveGenerator::generate(board, moves);

  if (moves.size() < 2) {
    return true;
  }

  // シャッフル
  std::shuffle(moves.begin(), moves.end(), _rgen);

  // 棋譜の手
  {
    // 探索
    board.makeMove(move0);
    _searcher.idsearch(board, tmpMove);
    board.unmakeMove(move0);

    // PV と評価値
    const auto& info = _searcher.getInfo();
    const auto& pv = info.pv;
    val0 = -info.eval;
    pv0.copy(pv);
  }

  // その他の手
  int nmove = 0;
  float gsum = 0;
  for (auto& move : moves) {
    // 探索
    bool valid = board.makeMove(move);
    if (!valid) { continue; }
    _searcher.idsearch(board, tmpMove);
    board.unmakeMove(move);

    // PV と評価値
    const auto& info = _searcher.getInfo();
    const auto& pv = info.pv;
    Value val = -info.eval;

    // 詰みは除外
    if (val <= -Value::Mate || val >= Value::Mate) {
      continue;
    }

    // leaf 局面
    Board leaf = getPvLeaf(board, move, pv);

    // 特徴抽出
    float g = gradient(val.int32() - val0.int32());
    g = g * (1.0f / MINI_BATCH_COUNT);
    _w.extract<float, true>(leaf, g);
    gsum += g;

    nmove++;
    if (nmove >= NUMBER_OF_SIBLING_NODES) {
      break;
    }
  }

  {
    // leaf 局面
    Board leaf = getPvLeaf(board, move0, pv0);

    // 特徴抽出
    _w.extract<float, true>(leaf, -gsum);
  }

  if (++_miniBatchCount >= MINI_BATCH_COUNT) {
    auto& eval = _searcher.getEvaluator();

    // 値更新
    float max = 0.0f;
    float magnitude = 0.0f;
    for (int i = 0; i < KPP_ALL; i++) {
      _w._t->kpp[0][i] += norm(_w._t->kpp[0][i]);
      eval._t->kpp[0][i] = _w._t->kpp[0][i];
      _u._t->kpp[0][i] += _w._t->kpp[0][i] * _count;
      max = std::max(max, std::abs(_w._t->kpp[0][i]));
      magnitude += std::abs(_w._t->kpp[0][i]);
    }
    for (int i = 0; i < KKP_ALL; i++) {
      _w._t->kkp[0][0][i] += norm(_w._t->kkp[0][0][i]);
      eval._t->kkp[0][0][i] = _w._t->kkp[0][0][i];
      _u._t->kkp[0][0][i] += _w._t->kkp[0][0][i] * _count;
      max = std::max(max, std::abs(_w._t->kkp[0][0][i]));
      magnitude += std::abs(_w._t->kkp[0][0][i]);
    }

    Loggers::message << "max=" << max << " magnitude=" << magnitude;

    _count++;
    _miniBatchCount = 0;
  }

  return true;
}

/**
 * 棋譜ファイルを読み込んで学習します。
 */
bool Learn::readCsa(const char* path) {
  Loggers::message << "load: [" << path << "]";

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

    bool ok = adjust(record.getBoard(), move);

    if (!ok) {
      return false;
    }

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

  Timer timer;
  timer.set();

  // 探索設定
  auto searchConfig = _searcher.getConfig();
  searchConfig.maxDepth = _config.getInt(CONF_DEPTH);
  searchConfig.workerSize = _config.getInt(CONF_THREADS);
  searchConfig.treeSize = Searcher::standardTreeSize(searchConfig.workerSize);
  searchConfig.enableLimit = false;
  searchConfig.enableTimeManagement = false;
  searchConfig.ponder = false;
  searchConfig.logging = false;
  _searcher.setConfig(searchConfig);

  // csa ファイルを列挙
  FileList fileList;
  std::string dir = _config.getString(CONF_KIFU);
  fileList.enumerate(dir.c_str(), "csa");

  // 初期化
  _count = 1;
  _miniBatchCount = 0;
  _w.init();
  _u.init();

  for (const auto& filename : fileList) {
    readCsa(filename.c_str());
  }

  // 平均を取る
  auto& eval = _searcher.getEvaluator();
  uint16_t max = 0;
  uint64_t magnitude = 0ull;
  for (int i = 0; i < KPP_ALL; i++) {
    eval._t->kpp[0][i] = _w._t->kpp[0][i] - _u._t->kpp[0][i] / _count;
    max = std::max(max, (uint16_t)std::abs(eval._t->kpp[0][i]));
    magnitude += std::abs(eval._t->kpp[0][i]);
  }
  for (int i = 0; i < KKP_ALL; i++) {
    eval._t->kkp[0][0][i] = _w._t->kkp[0][0][i] - _u._t->kkp[0][0][i] / _count;
    max = std::max(max, (uint16_t)std::abs(eval._t->kkp[0][0][i]));
    magnitude += std::abs(eval._t->kkp[0][0][i]);
  }

  Loggers::message << "max=" << max << " magnitude=" << magnitude;

  // 重みベクトルを保存
  eval.writeFile();

  double elapsed = timer.get();
  Loggers::message << "elapsed: " << elapsed;

  return true;
}

}

#endif // NLEARN
