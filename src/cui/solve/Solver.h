/* Solver.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_SOLVER__
#define SUNFISH_SOLVER__

#include "../console/ConsoleManager.h"
#include "searcher/Searcher.h"
#include "core/record/Record.h"
#include <vector>
#include <string>

namespace sunfish {

class Solver {
public:

  enum class ErrorCode : unsigned {
    FILE_READ_ERROR,
    INVALID_RECORD_LENGTH,
  };

  struct Error {
    std::string filename;
    ErrorCode errorCode;
  };

  struct Summary {
    int correct;
    int incorrect;
    int total;
    int ignore;
    Summary() : correct(0), incorrect(0), total(0), ignore(0) {}
  };

private:

  Searcher searcher_;

  Record record_;

  std::vector<std::string> problems_;

  std::vector<Error> errors_;

  Summary summary_;

  void solve(const std::string& problem);

  void solve(const Board& board, const Move& correct);

public:

  void clear() {
    problems_.clear();
    problems_.shrink_to_fit();
    errors_.clear();
    errors_.shrink_to_fit();
    summary_ = Summary();
  }

  void setProblems(const std::vector<std::string>& problems);

  void solve();

  bool hasError() const {
    return !errors_.empty();
  }

  const std::vector<Error>& getErrors() const {
    return errors_;
  }

  const Summary& getSummary() const {
    return summary_;
  }

  void setConfig(const ConsoleManager::Config& config) {
    auto searcherConfig = ConsoleManager::buildSearcherConfig(searcher_.getConfig(), config);
    searcher_.setConfig(searcherConfig);
  }

};

} // namespace sunfish

#endif // SUNFISH_SOLVER__
