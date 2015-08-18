/* BatchLearning.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_BATCHLEARNING__
#define SUNFISH_BATCHLEARNING__

#ifndef NLEARN

#include "./FV.h"
#include "core/util/Timer.h"
#include "core/util/Random.h"
#include "searcher/Searcher.h"
#include <fstream>
#include <queue>
#include <vector>
#include <thread>
#include <string>
#include <atomic>
#include <mutex>
#include <cstdint>

namespace sunfish {

class Config;
class Board;
class Move;

class BatchLearning {
private:

  struct Job {
    std::string path;
  };

  Timer timer_;

  const Config& config_;

  Evaluator eval_;

  FVM gm_;

  FV g_;

  std::queue<Job> jobQueue_;

  std::atomic<uint32_t> completedJobs_;

  uint32_t totalJobs_;

  std::atomic<uint32_t> totalMoves_;

  std::atomic<uint32_t> outOfWindLoss_;

  float loss_;

  Evaluator::ValueType max_;

  uint64_t magnitude_;

  std::vector<std::unique_ptr<Searcher>> searchers_;

  std::vector<std::thread> threads_;

  uint32_t nt_;

  std::atomic<bool> shutdown_;

	std::atomic<int> activeCount_;

  std::mutex mutex_;

  std::unique_ptr<std::ofstream> trainingData_;

  Random rand_;

  bool openTrainingData();
  void closeTrainingData();

  void updateProgress();
  void closeProgress();

  void generateTraningData(int wn, Board board, Move move0);
  void generateTraningData(int wn, const Job& job);
  void work(int wn);

  bool generateJobs();
  void waitForWorkers();
  bool generateGradient();
  void updateParameters();
  void updateMaterial();
  bool iterate();

public:

  BatchLearning(const Config& config)
    : config_(config) {
  }

  /**
   * 機械学習を実行します。
   */
  bool run();

};

} // namespace sunfish

#endif // NLEARN

#endif // SUNFISH_BATCHLEARNING__
