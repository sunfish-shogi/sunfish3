/* BatchLearning.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "BatchLearning.h"
#include "LearningConfig.h"
#include "LearningTemplates.h"
#include "config/Config.h"
#include "core/move/MoveGenerator.h"
#include "core/record/CsaReader.h"
#include "core/util/FileList.h"
#include "searcher/eval/Material.h"
#include <list>
#include <algorithm>
#include <cstdlib>

#define SEARCH_WINDOW  256
#define NORM           1.0e-3f

#define ENABLE_OVERLAP 1

namespace {

using namespace sunfish;

struct RelativeSquare {
  int8_t file;
  int8_t rank;
};

const RelativeSquare RelativeSquares[] = {
  {-8,-8}, {-7,-8}, {-6,-8}, {-5,-8}, {-4,-8}, {-3,-8}, {-2,-8}, {-1,-8}, { 0,-8}, { 1,-8}, { 2,-8}, { 3,-8}, { 4,-8}, { 5,-8}, { 6,-8}, { 7,-8}, { 8,-8},
  {-8,-7}, {-7,-7}, {-6,-7}, {-5,-7}, {-4,-7}, {-3,-7}, {-2,-7}, {-1,-7}, { 0,-7}, { 1,-7}, { 2,-7}, { 3,-7}, { 4,-7}, { 5,-7}, { 6,-7}, { 7,-7}, { 8,-7},
  {-8,-6}, {-7,-6}, {-6,-6}, {-5,-6}, {-4,-6}, {-3,-6}, {-2,-6}, {-1,-6}, { 0,-6}, { 1,-6}, { 2,-6}, { 3,-6}, { 4,-6}, { 5,-6}, { 6,-6}, { 7,-6}, { 8,-6},
  {-8,-5}, {-7,-5}, {-6,-5}, {-5,-5}, {-4,-5}, {-3,-5}, {-2,-5}, {-1,-5}, { 0,-5}, { 1,-5}, { 2,-5}, { 3,-5}, { 4,-5}, { 5,-5}, { 6,-5}, { 7,-5}, { 8,-5},
  {-8,-4}, {-7,-4}, {-6,-4}, {-5,-4}, {-4,-4}, {-3,-4}, {-2,-4}, {-1,-4}, { 0,-4}, { 1,-4}, { 2,-4}, { 3,-4}, { 4,-4}, { 5,-4}, { 6,-4}, { 7,-4}, { 8,-4},
  {-8,-3}, {-7,-3}, {-6,-3}, {-5,-3}, {-4,-3}, {-3,-3}, {-2,-3}, {-1,-3}, { 0,-3}, { 1,-3}, { 2,-3}, { 3,-3}, { 4,-3}, { 5,-3}, { 6,-3}, { 7,-3}, { 8,-3},
  {-8,-2}, {-7,-2}, {-6,-2}, {-5,-2}, {-4,-2}, {-3,-2}, {-2,-2}, {-1,-2}, { 0,-2}, { 1,-2}, { 2,-2}, { 3,-2}, { 4,-2}, { 5,-2}, { 6,-2}, { 7,-2}, { 8,-2},
  {-8,-1}, {-7,-1}, {-6,-1}, {-5,-1}, {-4,-1}, {-3,-1}, {-2,-1}, {-1,-1}, { 0,-1}, { 1,-1}, { 2,-1}, { 3,-1}, { 4,-1}, { 5,-1}, { 6,-1}, { 7,-1}, { 8,-1},
  {-8, 0}, {-7, 0}, {-6, 0}, {-5, 0}, {-4, 0}, {-3, 0}, {-2, 0}, {-1, 0},          { 1, 0}, { 2, 0}, { 3, 0}, { 4, 0}, { 5, 0}, { 6, 0}, { 7, 0}, { 8, 0},
  {-8, 1}, {-7, 1}, {-6, 1}, {-5, 1}, {-4, 1}, {-3, 1}, {-2, 1}, {-1, 1}, { 0, 1}, { 1, 1}, { 2, 1}, { 3, 1}, { 4, 1}, { 5, 1}, { 6, 1}, { 7, 1}, { 8, 1},
  {-8, 2}, {-7, 2}, {-6, 2}, {-5, 2}, {-4, 2}, {-3, 2}, {-2, 2}, {-1, 2}, { 0, 2}, { 1, 2}, { 2, 2}, { 3, 2}, { 4, 2}, { 5, 2}, { 6, 2}, { 7, 2}, { 8, 2},
  {-8, 3}, {-7, 3}, {-6, 3}, {-5, 3}, {-4, 3}, {-3, 3}, {-2, 3}, {-1, 3}, { 0, 3}, { 1, 3}, { 2, 3}, { 3, 3}, { 4, 3}, { 5, 3}, { 6, 3}, { 7, 3}, { 8, 3},
  {-8, 4}, {-7, 4}, {-6, 4}, {-5, 4}, {-4, 4}, {-3, 4}, {-2, 4}, {-1, 4}, { 0, 4}, { 1, 4}, { 2, 4}, { 3, 4}, { 4, 4}, { 5, 4}, { 6, 4}, { 7, 4}, { 8, 4},
  {-8, 5}, {-7, 5}, {-6, 5}, {-5, 5}, {-4, 5}, {-3, 5}, {-2, 5}, {-1, 5}, { 0, 5}, { 1, 5}, { 2, 5}, { 3, 5}, { 4, 5}, { 5, 5}, { 6, 5}, { 7, 5}, { 8, 5},
  {-8, 6}, {-7, 6}, {-6, 6}, {-5, 6}, {-4, 6}, {-3, 6}, {-2, 6}, {-1, 6}, { 0, 6}, { 1, 6}, { 2, 6}, { 3, 6}, { 4, 6}, { 5, 6}, { 6, 6}, { 7, 6}, { 8, 6},
  {-8, 7}, {-7, 7}, {-6, 7}, {-5, 7}, {-4, 7}, {-3, 7}, {-2, 7}, {-1, 7}, { 0, 7}, { 1, 7}, { 2, 7}, { 3, 7}, { 4, 7}, { 5, 7}, { 6, 7}, { 7, 7}, { 8, 7},
  {-8, 8}, {-7, 8}, {-6, 8}, {-5, 8}, {-4, 8}, {-3, 8}, {-2, 8}, {-1, 8}, { 0, 8}, { 1, 8}, { 2, 8}, { 3, 8}, { 4, 8}, { 5, 8}, { 6, 8}, { 7, 8}, { 8, 8},
};

bool isValidSquare(Piece piece, Square square) {
  if ((piece == Piece::BPawn || piece == Piece::BLance) && !square.isPawnMovable<true>()) {
    return false;
  }

  if ((piece == Piece::BKnight) && !square.isKnightMovable<true>()) {
    return false;
  }

  if ((piece == Piece::WPawn || piece == Piece::WLance) && !square.isPawnMovable<false>()) {
    return false;
  }

  if ((piece == Piece::WKnight) && !square.isKnightMovable<false>()) {
    return false;
  }

  return true;
}

template <class T>
void overlap(Piece piece1, Piece piece2, int index1, int index2, T&& func) {
  SQUARE_EACH(king) {
    int kingFile = king.getFile();
    int kingRank = king.getRank();

    int file1 = kingFile + RelativeSquares[index1].file;
    int rank1 = kingRank + RelativeSquares[index1].rank;

    int file2 = kingFile + RelativeSquares[index2].file;
    int rank2 = kingRank + RelativeSquares[index2].rank;

    if (!Square::isValidFile(file1) || !Square::isValidRank(rank1)) {
      continue;
    }

    if (!Square::isValidFile(file2) || !Square::isValidRank(rank2)) {
      continue;
    }

    Square square1(file1, rank1);
    Square square2(file2, rank2);

    if (!isValidSquare(piece1, square1) || !isValidSquare(piece2, square2)) {
      return;
    }

    func(king, square1, square2);
  }
}

std::string trainingDataFileName(uint32_t wn) {
  std::ostringstream oss;
  oss << "training" << wn << ".dat";
  return oss.str();
}

void setSearcherDepth(Searcher& searcher, int depth) {
  auto searchConfig = searcher.getConfig();
  searchConfig.maxDepth = depth;
  searcher.setConfig(searchConfig);
}

inline float gain() {
  return 7.0f / SEARCH_WINDOW;
}

inline float sigmoid(float x) {
  return 1.0 / (1.0 + std::exp(x * -gain()));
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

namespace sunfish {

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
 * ジョブを拾います。
 */
void BatchLearning::work(uint32_t wn) {
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
      // worker number の制約がある場合は一致するものだけを拾う
      if (job.wn != InvalidWorkerNumber && job.wn != wn) {
        continue;
      }
      jobQueue_.pop();
      activeCount_++;
    }

    job.method(wn);

    completedJobs_++;
    activeCount_--;

    if (job.type == JobType::GenerateTrainingData) {
      std::lock_guard<std::mutex> lock(mutex_);
      updateProgress();
    }
  }
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
 * 訓練データを生成します。
 */
void BatchLearning::generateTrainingData(uint32_t wn, Board board, Move move0) {
  int depth = config_.getInt(LCONF_DEPTH);

  // 合法手生成
  Moves moves;
  MoveGenerator::generate(board, moves);

  if (moves.size() < 2) {
    return;
  }

  Value val0;
  Move tmpMove;
  std::list<PV> list;

  auto& to = threadObjects_[wn];
  auto& searcher = *(to.searcher);
  auto& outTrainingData = to.outTrainingData;

  // ヒストリのクリア
  searcher.clearHistory();

  {
    int newDepth = depth;
    if (board.isCheck(move0)) {
      newDepth += 1;
    }

    // 探索
    board.makeMove(move0);
    setSearcherDepth(searcher, newDepth);
    searcher.search(board, tmpMove);
    board.unmakeMove(move0);

    // PV と評価値
    const auto& info = searcher.getInfo();
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
    if (move == move0) {
      continue;
    }

    int newDepth = depth;
    if (board.isCheck(move)) {
      newDepth += 1;
    }

    // 探索
    bool valid = board.makeMove(move);
    if (!valid) { continue; }
    setSearcherDepth(searcher, newDepth);
    searcher.search(board, tmpMove, -beta, -alpha, true);
    board.unmakeMove(move);

    // PV と評価値
    const auto& info = searcher.getInfo();
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
    outTrainingData->write(reinterpret_cast<char*>(&cb), sizeof(cb));

    for (const auto& pv : list) {
      // 手順の長さ
      uint8_t length = static_cast<uint8_t>(pv.size()) + 1;
      outTrainingData->write(reinterpret_cast<char*>(&length), sizeof(length));

      // 手順
      for (int i = 0; i < pv.size(); i++) {
        uint16_t m = Move::serialize16(pv.get(i).move);
        outTrainingData->write(reinterpret_cast<char*>(&m), sizeof(m));
      }
    }

    // 終端
    uint8_t n = 0;
    outTrainingData->write(reinterpret_cast<char*>(&n), sizeof(n));
  }
}

/**
 * 訓練データを生成します。
 */
void BatchLearning::generateTrainingDataOnWorker(uint32_t wn, const std::string& path) {
  Record record;
  if (!CsaReader::read(path, record)) {
    Loggers::error << "Could not read csa file. [" << path << "]";
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

    generateTrainingData(wn, record.getBoard(), move);

    // 1手進める
    if (!record.makeMove()) {
      break;
    }
  }
}

/**
 * 訓練データ作成を開始します。
 */
bool BatchLearning::generateTrainingData() {
  // open training data files
  for (uint32_t wn = 0; wn < nt_; wn++) {
    auto& to = threadObjects_[wn];
    auto& outTrainingData = to.outTrainingData;
    std::string fname = trainingDataFileName(wn);

    outTrainingData.reset(new std::ofstream);
    outTrainingData->open(fname, std::ios::binary | std::ios::out);

    if (!*outTrainingData) {
      Loggers::error << "open error!! [" << fname << "]";
      return false;
    }
  }

  // enumerate .csa files
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
    // push jobs
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& path : fileList) {
      using namespace std::placeholders;
      jobQueue_.push({
        JobType::GenerateTrainingData,
        std::bind(std::mem_fn(&BatchLearning::generateTrainingDataOnWorker), this, _1, path),
        InvalidWorkerNumber
      });
    }
  }

  waitForWorkers();

  // close progress bar
  closeProgress();

  // close training data files
  size_t size = 0;
  for (uint32_t wn = 0; wn < nt_; wn++) {
    auto& to = threadObjects_[wn];
    auto& outTrainingData = to.outTrainingData;
    size += outTrainingData->tellp();
    outTrainingData->close();
  }
  Loggers::message << "training_data_size=" << size;

  return true;
}

/**
 * 勾配ベクトルを生成します。
 */
bool BatchLearning::generateGradient(uint32_t wn) {
  std::string fname = trainingDataFileName(wn);
  std::ifstream inTrainingData(fname);
  if (!inTrainingData) {
    Loggers::error << "open error!! [" << fname << "]";
    return false;
  }

  float loss0 = 0.0f;
  std::unique_ptr<FVM> gm0(new FVM);
  std::unique_ptr<FV> g0(new FV);

  gm0->init();
  g0->init();

  while (true) {
    // ルート局面
    CompactBoard cb;
    inTrainingData.read(reinterpret_cast<char*>(&cb), sizeof(cb));
    if (inTrainingData.eof()) {
      break;
    }

    const Board root(cb);
    const bool black = root.isBlack();

    auto readPV = [&inTrainingData](Board& board) {
      // 手順の長さ
      uint8_t length;
      inTrainingData.read(reinterpret_cast<char*>(&length), sizeof(length));
      if (length == 0) {
        return false;
      }
      length--;

      // 手順
      bool ok = true;
      for (uint8_t i = 0; i < length; i++) {
        uint16_t m;
        inTrainingData.read(reinterpret_cast<char*>(&m), sizeof(m));
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

      float g = gradient(diff);
      g = black ? g : -g;

      loss0 += loss(diff);
      gm0->extract(board0, g);
      gm0->extract(board, -g);
      g0->extract<float, true>(board0, g);
      g0->extract<float, true>(board, -g);
    }
  }

  inTrainingData.close();

  {
    std::lock_guard<std::mutex> lock(mutex_);

    loss_ += loss0;

    gm_.pawn       += gm0->pawn;
    gm_.lance      += gm0->lance;
    gm_.knight     += gm0->knight;
    gm_.silver     += gm0->silver;
    gm_.gold       += gm0->gold;
    gm_.bishop     += gm0->bishop;
    gm_.rook       += gm0->rook;
    gm_.tokin      += gm0->tokin;
    gm_.pro_lance  += gm0->pro_lance;
    gm_.pro_knight += gm0->pro_knight;
    gm_.pro_silver += gm0->pro_silver;
    gm_.horse      += gm0->horse;
    gm_.dragon     += gm0->dragon;

    for (int i = 0; i < KPP_ALL; i++) {
      ((FV::ValueType*)g_.t_->kpp)[i] += ((FV::ValueType*)g0->t_->kpp)[i];
    }

    for (int i = 0; i < KKP_ALL; i++) {
      ((FV::ValueType*)g_.t_->kkp)[i] += ((FV::ValueType*)g0->t_->kkp)[i];
    }
  }

  return true;
}

/**
 * 勾配ベクトルを生成します。
 */
bool BatchLearning::generateGradient() {
  std::atomic<bool> ok(true);

  gm_.init();
  g_.init();

  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (uint32_t wn = 0; wn < nt_; wn++) {
      jobQueue_.push({
        JobType::GenerateGradient,
        [this, &ok](uint32_t wn) {
          ok = generateGradient(wn) && ok;
        },
        wn
      });
    }
  }

  waitForWorkers();

  return ok;
}

/**
 * 相対座標が同じである要素同士を重ねあわせます。
 */
void BatchLearning::overlapParameters(int index1, int index2) {
  // king-piece-piece
  PIECE_EACH(piece1) {
    PIECE_EACH(piece2) {
      FV::ValueType sum = 0.0f;
      int num = 0;

      overlap(piece1, piece2, index1, index2,
          [&sum, &num, this, piece1, piece2](Square king, Square square1, Square square2) {
        int x = kkpBoardIndex(piece1, square1);
        int y = kkpBoardIndex(piece2, square2);
        if (x >= y) {
          sum += g_.t_->kpp[king.index()][kpp_index(x, y)];
          num++;
        }
      });

      overlap(piece1, piece2, index1, index2,
          [sum, num, this, piece1, piece2](Square king, Square square1, Square square2) {
        int x = kkpBoardIndex(piece1, square1);
        int y = kkpBoardIndex(piece2, square2);
        if (x >= y) {
          g_.t_->kpp[king.index()][kpp_index(x, y)] += sum / (FV::ValueType)num;
        }
      });
    }
  }

  // king-king-piece
  PIECE_KIND_EACH(piece) {
    FV::ValueType sum = 0.0f;
    int num = 0;

    overlap(Piece::WKing, piece, index1, index2,
        [&sum, &num, this, piece](Square king, Square square1, Square square2) {
      sum += g_.t_->kkp[king.index()][square1.index()][kkpBoardIndex(piece, square2)];
      num++;
    });

    overlap(Piece::WKing, piece, index1, index2,
        [sum, num, this, piece](Square king, Square square1, Square square2) {
      g_.t_->kkp[king.index()][square1.index()][kkpBoardIndex(piece, square2)] += sum / (FV::ValueType)num;
    });
  }
}

/**
 * 相対座標が同じである要素同士を重ねあわせます。
 */
void BatchLearning::overlapParameters(uint32_t wn) {
  int begin = wn;
  int step = nt_;
  int rslen = sizeof(RelativeSquares) / sizeof(RelativeSquares[0]);

  for (int i = begin; i < rslen; i += step) {
    for (int j = 0; j < rslen; j++) {
      if (i == j) {
        continue;
      }

      int maxFile = std::max({ (int8_t)0, RelativeSquares[i].file, RelativeSquares[j].file});
      int maxRank = std::max({ (int8_t)0, RelativeSquares[i].rank, RelativeSquares[j].rank});
      int minFile = std::min({ (int8_t)0, RelativeSquares[i].file, RelativeSquares[j].file});
      int minRank = std::min({ (int8_t)0, RelativeSquares[i].rank, RelativeSquares[j].rank});
      int width = maxFile - minFile + 1;
      int height = maxRank - minRank + 1;
      if (width > Square::FileN || height > Square::RankN) {
        continue;
      }

      overlapParameters(i, j);
    }
  }
}

/**
 * 相対座標が同じである要素同士を重ねあわせます。
 */
void BatchLearning::overlapParameters() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (uint32_t wn = 0; wn < nt_; wn++) {
      jobQueue_.push({
        JobType::OverlapParam,
        [this](uint32_t wn) {
          overlapParameters(wn);
        },
        wn
      });
    }
  }

  waitForWorkers();
}

void BatchLearning::updateParameter(uint32_t wn,
    FV::ValueType& g, Evaluator::ValueType& e,
    Evaluator::ValueType& max, uint64_t& magnitude) {
  auto& to = threadObjects_[wn];
  auto& r = *(to.rand);

  g += norm(e);
  if (g > 0.0f) {
    e += r.getBit() + r.getBit();
  } else if (g < 0.0f) {
    e -= r.getBit() + r.getBit();
  }

  Evaluator::ValueType abs = std::abs(e);
  max = std::max(max, abs);
  magnitude = magnitude + abs;
};

/**
 * パラメータを更新します。
 */
void BatchLearning::updateParameters(uint32_t wn) {
  int begin = wn;
  int step = nt_;

  auto& to = threadObjects_[wn];
  auto& max = to.max;
  auto& magnitude = to.magnitude;

  for (int i = begin; i < KPP_ALL; i += step) {
    updateParameter(wn, ((FV::ValueType*)g_.t_->kpp)[i],
      ((Evaluator::ValueType*)eval_.t_->kpp)[i], max, magnitude);
  }

  for (int i = begin; i < KKP_ALL; i += step) {
    updateParameter(wn, ((FV::ValueType*)g_.t_->kkp)[i],
      ((Evaluator::ValueType*)eval_.t_->kkp)[i], max, magnitude);
  }
}

/**
 * パラメータを更新します。
 */
void BatchLearning::updateParameters() {
  LearningTemplates::symmetrize(g_, [](float& a, float& b) {
      a = b = a + b;
  });

  updateMaterial();

  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (uint32_t wn = 0; wn < nt_; wn++) {
      threadObjects_[wn].max = 0;
      threadObjects_[wn].magnitude = 0;
      jobQueue_.push({
        JobType::UpdateParam,
        [this](uint32_t wn) {
          updateParameters(wn);
        },
        wn
      });
    }
  }

  waitForWorkers();

  max_ = 0;
  magnitude_ = 0;
  for (uint32_t wn = 0; wn < nt_; wn++) {
    max_ += threadObjects_[wn].max;
    magnitude_ += threadObjects_[wn].magnitude;
  }

  LearningTemplates::symmetrize(eval_, [](Evaluator::ValueType& a, Evaluator::ValueType& b) {
      a = b;
  });

  // ハッシュ表を初期化
  eval_.clearCache();
  // transposition table は SearchConfig::learning で無効にしている
  //for (uint32_t wn = 0; wn < nt_; wn++) {
  //  searchers_[wn]->clearTT();
  //}
}

/**
 * 駒割りを更新します。
 */
void BatchLearning::updateMaterial() {
  float* p[13];

  p[0]  = &gm_.pawn;
  p[1]  = &gm_.lance;
  p[2]  = &gm_.knight;
  p[3]  = &gm_.silver;
  p[4]  = &gm_.gold;
  p[5]  = &gm_.bishop;
  p[6]  = &gm_.rook;
  p[7]  = &gm_.tokin;
  p[8]  = &gm_.pro_lance;
  p[9]  = &gm_.pro_knight;
  p[10] = &gm_.pro_silver;
  p[11] = &gm_.horse;
  p[12] = &gm_.dragon;

  // 昇順でソート
  std::sort(p, p + 13, [](float* a, float* b) {
    return (*a) < (*b);
  });

  auto& to = threadObjects_[0];

  // シャッフル
  to.rand->shuffle(p, p + 6);
  to.rand->shuffle(p + 6, p + 13);

  // 更新値を決定
  *p[0]  = *p[1]  = -2.0f;
  *p[2]  = *p[3]  = *p[4]  = -1.0f;
  *p[5]  = *p[6]  = *p[7]  = 0.0f;
  *p[8]  = *p[9]  = *p[10] = 1.0f;
  *p[11] = *p[12] = 2.0f;

  // 値を更新
  material::Pawn       += gm_.pawn;
  material::Lance      += gm_.lance;
  material::Knight     += gm_.knight;
  material::Silver     += gm_.silver;
  material::Gold       += gm_.gold;
  material::Bishop     += gm_.bishop;
  material::Rook       += gm_.rook;
  material::Tokin      += gm_.tokin;
  material::Pro_lance  += gm_.pro_lance;
  material::Pro_knight += gm_.pro_knight;
  material::Pro_silver += gm_.pro_silver;
  material::Horse      += gm_.horse;
  material::Dragon     += gm_.dragon;

  // 交換値を更新
  material::updateEx();
}

/**
 * バッチ学習の反復処理を実行します。
 */
bool BatchLearning::iterate() {
  const int iterateCount = config_.getInt(LCONF_ITERATION);
  int  updateCount = 128;

  for (int i = 0; i < iterateCount; i++) {
    totalMoves_ = 0;
    outOfWindLoss_ = 0;

    if (!generateTrainingData()) {
      return false;
    }

    for (int j = 0; j < updateCount; j++) {
      loss_ = 0.0f;

      if (!generateGradient()) {
        return false;
      }

#if ENABLE_OVERLAP
      overlapParameters();
#endif

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
    material::writeFile();
    eval_.writeFile();

    // キャッシュクリア
    eval_.clearCache();

    updateCount = std::max(updateCount / 2, 16);
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
  for (uint32_t wn = 0; wn < nt_; wn++) {
  }

  activeCount_ = 0;

  // ワーカースレッド生成
  shutdown_ = false;
  threadObjects_.clear();
  for (uint32_t wn = 0; wn < nt_; wn++) {
    threadObjects_.push_back(ThreadObject {
      std::thread(std::bind(std::mem_fn(&BatchLearning::work), this, wn)),
      std::unique_ptr<Searcher>(new Searcher(eval_)),
      std::unique_ptr<Random>(new Random()),
      nullptr, 0, 0,
    });

    auto& to = threadObjects_.back();
    auto& searcher = *(to.searcher);

    auto searchConfig = searcher.getConfig();
    searchConfig.workerSize = 1;
    searchConfig.treeSize = Searcher::standardTreeSize(searchConfig.workerSize);
    searchConfig.enableLimit = false;
    searchConfig.enableTimeManagement = false;
    searchConfig.ponder = false;
    searchConfig.logging = false;
    searchConfig.learning = true;
    searcher.setConfig(searchConfig);
  }

  bool ok = iterate();

  // ワーカースレッド停止
  shutdown_ = true;
  for (uint32_t wn = 0; wn < nt_; wn++) {
    auto& to = threadObjects_[wn];
    to.thread.join();
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
