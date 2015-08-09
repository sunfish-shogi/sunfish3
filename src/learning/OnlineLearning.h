/* OnlineLearning.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_ONLINELEARNING__
#define SUNFISH_ONLINELEARNING__

#ifndef NLEARN

#include "./FV.h"
#include "core/board/Board.h"
#include "core/move/Move.h"
#include "core/util/Timer.h"
#include "searcher/Searcher.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <random>
#include <queue>
#include <vector>
#include <thread>
#include <cstring>

namespace sunfish {

class Config;
class Searcher;

class OnlineLearning {
private:

  struct Job {
    CompactBoard board;
    Move move;
  };

  Timer timer_;

  const Config& config_;

  Evaluator eval_;

  std::vector<std::mt19937> rgens_;

  std::vector<std::unique_ptr<Searcher>> searchers_;

  uint32_t miniBatchCount_;

  uint32_t miniBatchScale_;

  std::atomic<uint32_t> errorCount_;

  float errorSum_;

  FV g_;

  FV w_;

  FV u_;

  std::vector<Job> jobs_;

  std::queue<Job> jobQueue_;

  std::vector<std::thread> threads_;

  uint32_t nt_;

  std::atomic<bool> shutdown_;

	std::atomic<int> activeCount_;

  std::mutex mutex_;

  void analyzeEval();

  void genGradient(int wn, const Job& job);

  void work(int wn);

  bool miniBatch();

  /**
   * 棋譜ファイルを読み込んで学習します。
   */
  bool readCsa(size_t count, size_t total, const char* path);

public:

  /**
   * コンストラクタ
   */
  OnlineLearning(const Config& config)
    : config_(config),
      eval_(Evaluator::InitType::Zero) {
  }

  /**
   * 機械学習を実行します。
   */
  bool run();

};

} // namespace sunfish

#endif // NLEARN

#endif // SUNFISH_ONLINELEARNING__
