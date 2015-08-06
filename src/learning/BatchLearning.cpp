/* BatchLearning.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "BatchLearning.h"
#include "./LearningConfig.h"
#include "config/Config.h"
#include "core/move/MoveGenerator.h"
#include "core/record/CsaReader.h"
#include "core/util/FileList.h"
#include <list>
#include <cstdlib>

#define TRAINING_DAT  "training.dat"

#define SEARCH_WINDOW 256
#define NORM          1.0e-2f

namespace sunfish {

namespace {

void setSearcherDepth(Searcher& searcher, int depth) {
  auto searchConfig = searcher.getConfig();
  searchConfig.maxDepth = depth;
  searcher.setConfig(searchConfig);
}

inline float gain() {
  return -7.0f / SEARCH_WINDOW;
}

inline float sigmoid(float x) {
  return 1.0 / (1.0 + std::exp(x * gain()));
}

inline float dsigmoid(float x) {
  float s = sigmoid(x);
  return (s - s * s) * gain();
}

inline float loss(float x) {
  return sigmoid(x);
}

inline float gradient(float x) {
  return dsigmoid(x);
}

inline float norm(float x) {
  if (x > 0.0f) {
    return -NORM;
  } else if (x < 0.0f) {
    return NORM;
  } else {
    return 0.0f;
  }
}

} // namespace

bool BatchLearning::openTrainingData() {
  trainingData_.reset(new std::ofstream);
  trainingData_->open(TRAINING_DAT, std::ios::binary | std::ios::out);

  if (!trainingData_) {
    Loggers::error << "open error!! [" << TRAINING_DAT << "]";
    return false;
  }

  return true;
}

void BatchLearning::closeTrainingData() {
  trainingData_->close();
}

/**
 * プログレスバーの表示を更新します。
 */
void BatchLearning::updateProgress() {
  int cmax = 50;

  std::cout << "\r";
  for (int c = 0; c < cmax; c++) {
    if (c * totalJobs_ <= cmax * completedJobs_) {
      std::cout << '#';
    } else {
      std::cout << ' ';
    }
  }
  float percentage = (float)completedJobs_ / totalJobs_ * 100.0f;
  std::cout << " [" << percentage << "%]";
  std::cout << std::flush;
}

/**
 * プログレスバーの表示を終了します。
 */
void BatchLearning::closeProgress() {
  std::cout << "\n";
  std::cout << std::flush;
}

/**
 * 訓練データを生成します。
 */
void BatchLearning::generateTraningData(int wn, Board board, Move move0) {
  // 合法手生成
  Moves moves;
  MoveGenerator::generate(board, moves);

  if (moves.size() < 2) {
    return;
  }

  Value val0;
  Move tmpMove;
  std::list<PV> list;

  // ヒストリのクリア
  searchers_[wn]->clearHistory();

  {
    // 探索
    board.makeMove(move0);
    setSearcherDepth(*searchers_[wn], config_.getInt(LCONF_DEPTH));
    searchers_[wn]->idsearch(board, tmpMove);
    board.unmakeMove(move0);

    // PV と評価値
    const auto& info = searchers_[wn]->getInfo();
    const auto& pv = info.pv;
    val0 = -info.eval;

    // 詰みは除外
    if (val0 <= -Value::Mate || val0 >= Value::Mate) {
      return;
    }

    list.push_back({});
    list.back().set(move0, 0, pv);
  }

  totalMoves_++;

  // 棋譜の手の評価値から window を決定
  Value alpha = val0 - SEARCH_WINDOW;
  Value beta = val0 + SEARCH_WINDOW;

  for (auto& move : moves) {
    // 探索
    bool valid = board.makeMove(move);
    if (!valid) { continue; }
    setSearcherDepth(*searchers_[wn], config_.getInt(LCONF_DEPTH));
    searchers_[wn]->idsearch(board, tmpMove, -beta, -alpha);
    board.unmakeMove(move);

    // PV と評価値
    const auto& info = searchers_[wn]->getInfo();
    const auto& pv = info.pv;
    Value val = -info.eval;

    if (val <= alpha) {
      continue;
    }

    if (val >= beta) {
      outOfWindLoss_++;
      continue;
    }

    list.push_back({});
    list.back().set(move, 0, pv);
  }

  // 書き出し
  if (!list.empty()) {
    std::lock_guard<std::mutex> lock(mutex_);

    // ルート局面
    CompactBoard cb = board.getCompactBoard();
    trainingData_->write(reinterpret_cast<char*>(&cb), sizeof(cb));

    for (const auto& pv : list) {
      // 手順の長さ
      uint8_t length = static_cast<uint8_t>(pv.size()) + 1;
      trainingData_->write(reinterpret_cast<char*>(&length), sizeof(length));

      // 手順
      for (size_t i = 0; i < pv.size(); i++) {
        uint16_t m = Move::serialize16(pv.get(i).move);
        trainingData_->write(reinterpret_cast<char*>(&m), sizeof(m));
      }
    }

    // 終端
    uint8_t n = 0;
    trainingData_->write(reinterpret_cast<char*>(&n), sizeof(n));
  }
}

/**
 * 訓練データを生成します。
 */
void BatchLearning::generateTraningData(int wn, const Job& job) {
  Record record;
  if (!CsaReader::read(job.path, record)) {
    Loggers::error << "Could not read csa file. [" << job.path << "]";
    exit(1);
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

    generateTraningData(wn, record.getBoard(), move);

    // 1手進める
    if (!record.makeMove()) {
      break;
    }
  }
}

/**
 * ジョブを拾います。
 */
void BatchLearning::work(int wn) {
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

    generateTraningData(wn, job);
 
    completedJobs_++;
    activeCount_--;

    {
      std::lock_guard<std::mutex> lock(mutex_);
      updateProgress();
    }
  }
}

/**
 * ジョブを作成します。
 */
bool BatchLearning::generateJobs() {
  FileList fileList;
  std::string dir = config_.getString(LCONF_KIFU);
  fileList.enumerate(dir.c_str(), "csa");

  if (fileList.size() == 0) {
    Loggers::error << "no files.";
    return false;
  }

  completedJobs_ = 0;
  totalJobs_ = fileList.size();

  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& path : fileList) {
      jobQueue_.push({ path });
    }
  }

  return true;
}

/**
 * ワーカーがジョブを終えるまで待機します。
 */
void BatchLearning::waitForWorkers() {
  while (true) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (jobQueue_.empty() && activeCount_ == 0) {
        return;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

/**
 * 勾配ベクトルを生成します。
 */
bool BatchLearning::generateGradient() {
  std::ifstream trainingData;

  trainingData.open(TRAINING_DAT);
  if (!trainingData) {
    Loggers::error << "open error!! [" << TRAINING_DAT << "]";
    return false;
  }

  g_.init();

  while (true) {
    // ルート局面
    CompactBoard cb;
    trainingData.read(reinterpret_cast<char*>(&cb), sizeof(cb));

    if (trainingData.eof()) {
      break;
    }

    const Board root(cb);
    const bool black = root.isBlack();

    auto readPV = [&trainingData](Board& board) {
      // 手順の長さ
      uint8_t length;
      trainingData.read(reinterpret_cast<char*>(&length), sizeof(length));
      if (length == 0) {
        return false;
      }
      length--;

      // 手順
      bool ok = true;
      for (uint8_t i = 0; i < length; i++) {
        uint16_t m;
        trainingData.read(reinterpret_cast<char*>(&m), sizeof(m));
        Move move = Move::deserialize16(m, board);
        if (!ok || move.isEmpty() || !board.makeMove(move)) {
          ok = false;
        }
      }

      return true;
    };

    Board board0 = root;
    readPV(board0);
    Value val0 = eval_.evaluate(board0).value();

    while (true) {
      Board board = root;
      if (!readPV(board)) {
        break;
      }
      Value val = eval_.evaluate(board).value();

      float diff = val.int32() - val0.int32();
      diff = black ? diff : -diff;

      loss_ += loss(diff);

      float g = gradient(diff);
      g = black ? g : -g;
      g_.extract<float, true>(board0, g);
      g_.extract<float, true>(board, -g);
    }
  }

  trainingData.close();
  return true;
}

/**
 * パラメータを更新します。
 */
void BatchLearning::updateParameters() {
  auto update = [this](FV::ValueType& g, Evaluator::ValueType& e,
      Evaluator::ValueType& max, uint64_t& magnitude) {
    g += norm(e);
    if (g > 0.0f) {
      e += rand_.getBit() + rand_.getBit();
    } else if (g < 0.0f) {
      e -= rand_.getBit() + rand_.getBit();
    }
    Evaluator::ValueType abs = std::abs(e);
    max = std::max(max, abs);
    magnitude = magnitude + abs;
  };

  max_ = 0;
  magnitude_ = 0;

  for (int i = 0; i < KPP_ALL; i++) {
    update(((FV::ValueType*)g_.t_->kpp)[i],
           ((Evaluator::ValueType*)eval_.t_->kpp)[i],
           max_, magnitude_);
  }

  for (int i = 0; i < KKP_ALL; i++) {
    update(((FV::ValueType*)g_.t_->kkp)[i],
           ((Evaluator::ValueType*)eval_.t_->kkp)[i],
           max_, magnitude_);
  }

  // ハッシュ表を初期化
  eval_.clearCache();
  for (uint32_t wn = 0; wn < nt_; wn++) {
    searchers_[wn]->clearTT();
  }
}

/**
 * バッチ学習の反復処理を実行します。
 */
bool BatchLearning::iterate() {
  const int iterateCount = config_.getInt(LCONF_ITERATION);
  int  updateCount = 256;

  for (int i = 0; i < iterateCount; i++) {
    if (!openTrainingData()) {
      return false;
    }

    totalMoves_ = 0;
    outOfWindLoss_ = 0;

    if (!generateJobs()) {
      return false;
    }

    waitForWorkers();

    closeTrainingData();
    closeProgress();

    updateCount = std::max(updateCount / 2, 16);

    for (int j = 0; j < updateCount; j++) {
      loss_ = 0.0f;

      if (!generateGradient()) {
        return false;
      }

      updateParameters();

      float elapsed = timer_.get();
      float outOfWindLoss = (float)outOfWindLoss_ / totalMoves_;
      float totalLoss = ((float)outOfWindLoss_ + loss_) / totalMoves_;

      Loggers::message
        << "elapsed=" << elapsed
        << "\titeration=" << i << "," << j
        << "\tout_wind_loss=" << outOfWindLoss
        << "\tloss=" << totalLoss
        << "\tmax=" << max_
        << "\tmagnitude=" << magnitude_;
    }

    // 保存
    eval_.writeFile();

    // キャッシュクリア
    eval_.clearCache();
  }

  return true;
}

/**
 * 学習を実行します。
 */
bool BatchLearning::run() {
  Loggers::message << "begin learning";

  timer_.set();

  // 初期化
  eval_.init();

  // 学習スレッド数
  nt_ = config_.getInt(LCONF_THREADS);

  // Searcher生成
  searchers_.clear();
  for (uint32_t wn = 0; wn < nt_; wn++) {
    searchers_.emplace_back(new Searcher(eval_));

    auto searchConfig = searchers_.back()->getConfig();
    searchConfig.workerSize = 1;
    searchConfig.treeSize = Searcher::standardTreeSize(searchConfig.workerSize);
    searchConfig.enableLimit = false;
    searchConfig.enableTimeManagement = false;
    searchConfig.ponder = false;
    searchConfig.logging = false;
    searchers_.back()->setConfig(searchConfig);
  }

  activeCount_ = 0;

  // ワーカースレッド生成
  shutdown_ = false;
  threads_.clear();
  for (uint32_t wn = 0; wn < nt_; wn++) {
    threads_.emplace_back(std::bind(std::mem_fn(&BatchLearning::work), this, wn));
  }

  bool ok = iterate();

  // ワーカースレッド停止
  shutdown_ = true;
  for (uint32_t wn = 0; wn < nt_; wn++) {
    threads_[wn].join();
  }

  if (!ok) {
    return false;
  }

  Loggers::message << "completed..";

  float elapsed = timer_.get();
  Loggers::message << "elapsed: " << elapsed;
  Loggers::message << "end learning";

  return true;
}

}

#endif // NLEARN
