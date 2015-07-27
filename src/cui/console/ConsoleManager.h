/*
 * ConsoleManager.h
 *
 * Kubo Ryosuke
 */

#ifndef CONSOLEMANAGER_H_
#define CONSOLEMANAGER_H_

#include "core/record/Record.h"
#include "searcher/Searcher.h"
#include "book/Book.h"

namespace sunfish {

/**
 * コンソールコマンド
 */
enum class Command : int {
  Empty = 0,
  Quit,
  Help,
  Prev,
  Next,
  Top,
  End,
  Search,
  Moves,
  Book,
  ClearTT,
  ClearHistory,

  num__,

  Unknown,
};

/**
 * ConsoleManager
 */
class ConsoleManager {
public:

  struct Config {
    bool autoBlack;
    bool autoWhite;
    int maxDepth;
    int limitSeconds;
    int worker;
    std::string inFileName;
    std::string outFileName;
  };

private:

  enum class CommandResult {
    Quit, Changed, None
  };

  Searcher searcher_;
  Book book_;
  Record record_;
  Command prevCommand_;
  Config config_;

  void showHelp();

  Command parseCommand(const char* str);

  bool inputMove(const char* str, const Board& board, Move& move) const;

  void printBoard(const Board& board);

  bool isAuto() const {
    return (record_.getBoard().isBlack() && config_.autoBlack)
        || (record_.getBoard().isWhite() && config_.autoWhite);
  }

  bool search(bool withMakeMove);

  void showMoves() const;

  void probeBook() const;

  CommandResult inputCommand();

public:

  ConsoleManager() {
    config_ = getDefaultConfig();
    prevCommand_ = Command::Empty;
  }
  ConsoleManager(const ConsoleManager&) = delete;
  ConsoleManager(ConsoleManager&&) = delete;

  void setAutoBlack(bool enable) {
    config_.autoBlack = enable;
  }
  bool getAutoBlack() const {
    return config_.autoBlack;
  }

  void setAutoWhite(bool enable) {
    config_.autoWhite = enable;
  }
  bool getAutoWhite() const {
    return config_.autoWhite;
  }

  void setMaxDepth(int depth) {
    config_.maxDepth = depth;
  }
  int getMaxDepth() const  {
    return config_.maxDepth;
  }

  void setLimitSeconds(int limitSeconds) {
    config_.limitSeconds = limitSeconds;
  }
  int getLimitSeconds() const {
    return config_.limitSeconds;
  }

  void setWorker(int worker) {
    config_.worker = worker;
  }
  int getWorker() const {
    return config_.worker;
  }

  void setInFileName(const std::string& inFileName) {
    config_.inFileName = inFileName;
  }
  const std::string& getInFileName() const {
    return config_.inFileName;
  }

  void setOutFileName(const std::string& outFileName) {
    config_.outFileName = outFileName;
  }
  const std::string& getOutFileName() const {
    return config_.outFileName;
  }

  void setConfig(Config config) {
    config_ = std::move(config);
  }
  const Config& getConfig() const {
    return config_;
  }
  static Config getDefaultConfig();
  static Searcher::Config buildSearcherConfig(Searcher::Config searcherConfigOrg, const Config& config) {
    Searcher::Config searcherConfig = std::move(searcherConfigOrg);
    searcherConfig.maxDepth = config.maxDepth;
    searcherConfig.limitSeconds = config.limitSeconds;
    searcherConfig.workerSize = config.worker;
    searcherConfig.treeSize = Searcher::standardTreeSize(config.worker);
    return searcherConfig;
  }

  bool play();

};

} // namespace sunfish

#endif // CONSOLEMANAGER_H_
