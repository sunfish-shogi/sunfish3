/* ConsoleManager.cpp
 *
 * Kubo Ryosuke
 */

#include "ConsoleManager.h"
#include "core/record/CsaWriter.h"
#include "core/record/CsaReader.h"
#include "core/move/MoveGenerator.h"
#include "logger/Logger.h"
#include <utility>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>

#define COMMAND_NUM ((int)Command::num__)

namespace {

using namespace sunfish;

struct CommandSet {
  const char* shortStr;
  const char* longStr;
  Command command;
  const char* description;
};

/**
 * コンソールで利用可能なコマンド一覧
 */
const CommandSet commandSet[COMMAND_NUM] = {
  { "q", "quit", Command::Quit, "quit." },
  { "h", "help", Command::Help, "show this help." },
  { "p", "prev", Command::Prev, "go to previous position." },
  { "n", "next", Command::Next, "go to next position." },
  { "t", "top", Command::Top, "go to top of this record." },
  { "e", "end", Command::End, "go to end of this record." },
  { "s", "search", Command::Search, "search from current position." },
  { "m", "moves", Command::Moves, "show legal moves." },
  { "b", "book", Command::Book, "probe book." },
  { nullptr, "clear-tt", Command::ClearTT, "clear TT." },
  { nullptr, "clear-history", Command::ClearHistory, "clear history." },
};

}

namespace sunfish {

/**
 * デフォルト設定
 */
ConsoleManager::Config ConsoleManager::getDefaultConfig() {
  Config config;
  config.autoBlack = false;
  config.autoWhite = true;
  config.maxDepth = 15;
  config.limitSeconds = 3.0;
  config.worker = 1;
  config.inFileName = "";
  config.outFileName = "console.csa";
  return config;
}

/**
 * ユーザが入力したコマンドを読み込みます。
 */
Command ConsoleManager::parseCommand(const char* str) {
  if (str[0] == '\0') {
    return Command::Empty;
  }
  for (int i = 0; i < COMMAND_NUM; i++) {
    const char* s = commandSet[i].shortStr;
    const char* l = commandSet[i].longStr;
    if ((s != nullptr && 0 == strcmp(str, s)) ||
        (l != nullptr && 0 == strcmp(str, l))) {
      return commandSet[i].command;
    }
  }
  return Command::Unknown;
}

/**
 * ユーザが入力した指し手を読み込みます。
 */
bool ConsoleManager::inputMove(const char* str, const Board& board, Move& move) const {

  // CSA形式で読み込んでみる
  if (CsaReader::readMove(str, record_.getBoard(), move)) {
    return true;
  }

  if (strlen(str) < 4) {
    return false;
  }

  // 移動元
  auto to = Position::parse(&str[2]);
  if (!to.isValid()) {
    return false;
  }

  // 移動先
  auto from = Position::parse(&str[0]);

  if (from.isValid()) {
    // 盤上の駒を動かす手

    // 駒種
    auto piece = board.getBoardPiece(from);
    if (piece.isEmpty()) {
      return false;
    }

    // 成
    auto promote = str[4] == 'n';
    if (promote && piece.isPromoted()) {
      return false;
    }

    move.set(piece, from, to, promote);

    // 合法手チェック
    if (board.isValidMoveStrict(move)) {
      return true;
    }

    // 成を自動付加
    if (!promote && piece.isUnpromoted()) {
      move.setPromote(true);
      if (board.isValidMoveStrict(move)) {
        return true;
      }
    }

  } else {
    // 持ち駒を打つ手

    // 駒種
    auto piece = Piece::parse(&str[0]);
    if (piece.isEmpty()) {
      return false;
    }

    move.set(piece.black(), to);

    // 合法手チェック
    if (board.isValidMoveStrict(move)) {
      return true;
    }

  }

  return false;

}

/**
 * ヘルプを表示します。
 */
void ConsoleManager::showHelp() {
  for (int i = 0; i < COMMAND_NUM; i++) {
    const char* l = commandSet[i].longStr;
    const char* s = commandSet[i].shortStr;
    const char* d = commandSet[i].description;
    if (l != nullptr) { std::cout << l; }
    std::cout << '\t';
    if (s != nullptr) { std::cout << s; }
    std::cout << '\t';
    if (d != nullptr) { std::cout << d; }
    std::cout << std::endl;
  }
}

/**
 * 盤面を表示します。
 */
void ConsoleManager::printBoard(const Board& board) {
  std::cout << "'----------------------------\n";
  std::cout << board.toStringCsa();
  std::cout << "'----------------------------\n";
  std::cout << std::endl;

#if 1
  auto& evaluator = searcher_.getEvaluator();
  auto valuePair = evaluator.evaluate(board);
  std::cout << "Evaluator Info\n";
  std::cout << "  eval: " << valuePair.value().int32() <<
    "(" << valuePair.material().int32() << ","
    " " << valuePair.positional().int32() << ")\n";
  std::cout << std::endl;
#endif
}

/**
 * 探索を行って結果を出力します。
 */
bool ConsoleManager::search(bool withMakeMove) {

  bool ok = false;
  Move move = Move::empty();

  // 定跡検索
  if (!ok) {
    const auto& board = record_.getBoard();
    uint64_t hash = board.getHash();
    BookResult bookResult = book_.selectRandom(hash);
    if (!bookResult.move.isEmpty() && board.isValidMove(bookResult.move)) {
      move = bookResult.move;
      std::cout << "book hit: " << move.toString() << " (" << bookResult.count << "/" << bookResult.total << ")\n";
      std::cout << std::endl;
      ok = true;
    }
  }

  // 探索
  if (move.isEmpty()) {
    std::cout << "searching..\n";
    searcher_.setRecord(record_);
    ok = searcher_.idsearch(record_.getBoard(), move);
    searcher_.clearRecord();
    std::cout << "done.\n";
    std::cout << std::endl;

    std::cout << searcher_.getInfoString();
    std::cout << std::endl;
  }

  if (ok) {

    std::cout << "Result:" << std::endl;
    std::cout << "  " << move.toString() << std::endl;
    std::cout << std::endl;

    if (withMakeMove) {
      // 着手
      record_.makeMove(move);
    }

  } else {

    std::cout << "resign.\n";
    std::cout << std::endl;

    if (withMakeMove) {
      // 着手をマニュアルモードに切り替え
      if (config_.autoBlack) {
        std::cout << "Change mode:\n";
        std::cout << "  black: auto => manual\n";
        std::cout << std::endl;
        config_.autoBlack = false;
      }
      if (config_.autoWhite) {
        std::cout << "Change mode:\n";
        std::cout << "  white: auto => manual\n";
        std::cout << std::endl;
        config_.autoWhite = false;
      }
    }

  }

  return ok;

}

void ConsoleManager::showMoves() const {
  Moves moves;
  MoveGenerator::generate(record_.getBoard(), moves);
  for (auto ite = moves.begin(); ite != moves.end(); ite++) {
    if (record_.getBoard().isValidMoveStrict(*ite)) {
      std::cout << ite->toString() << ' ';
    }
  }
  std::cout << std::endl;
}

void ConsoleManager::probeBook() const {
  uint64_t hash = record_.getBoard().getHash();
  auto p = book_.find(hash);
  if (p == nullptr) {
    std::cout << "(empty)" << std::endl;
    return;
  }

  uint32_t totalCount = p->getCount();
  const auto& bookMoves = p->getMoves();

  struct Element {
    Move move;
    uint32_t count;
  };
  std::vector<Element> moves;
  for (const auto& bookMove : bookMoves) {
    moves.push_back({ bookMove.move, bookMove.count });
  }
  std::sort(moves.begin(), moves.end(), [](const Element& l, const Element& r) {
    return l.count > r.count;
  });
  for (const auto& element : moves) {
    float percentage = (float)element.count / totalCount * 100.0f;
    std::cout << element.move.toString() << "(" << std::fixed << std::setprecision(1) << percentage << "%) ";
  }
  std::cout << std::endl;
}

/**
 * ユーザからのコマンド入力を受け付けます。
 */
ConsoleManager::CommandResult ConsoleManager::inputCommand() {

  char line[1024];

  std::cout << "> ";

  // コマンド受付
  std::cin.getline(line, sizeof(line));
  if (std::cin.eof()) {
    return CommandResult::Quit;
  }

  // コマンドの解析
  Command command = parseCommand(line);

  // quit
  if (command == Command::Quit) {
    return CommandResult::Quit;
  }

  // 入力がないときは直前のコマンドを繰り返す。
  if (command == Command::Empty) {
    command = prevCommand_;
  }

  prevCommand_ = Command::Empty;

  switch (command) {
    case Command::Empty:
      // 入力なし
      return CommandResult::None;

    case Command::Help:
      // ヘルプを表示
      showHelp();
      return CommandResult::None;

    case Command::Prev:
      // 1手戻る。
      if (!record_.unmakeMove()) {
        std::cout << "There is no previous move.\n";
        return CommandResult::None;
      }

      if (config_.autoBlack || config_.autoWhite) {
        // コンピュータの手番はスキップ
        record_.unmakeMove();
      }
      prevCommand_ = Command::Prev;
      return CommandResult::Changed;

    case Command::Next:
      // 1手進む。
      if (!record_.makeMove()) {
        std::cout << "There is no next move.\n";
        return CommandResult::None;
      }

      if (config_.autoBlack || config_.autoWhite) {
        // コンピュータの手番はスキップ
        record_.makeMove();
      }
      prevCommand_ = Command::Next;
      return CommandResult::Changed;

    case Command::Top:
      // 開始局面に戻る。
      if (!record_.unmakeMove()) {
        std::cout << "There is no previous move.\n";
        return CommandResult::None;
      }

      while (record_.unmakeMove())
        ;
      return CommandResult::Changed;

    case Command::End:
      // 最終局面まで進む。
      if (!record_.makeMove()) {
        std::cout << "There is no next move.\n";
        return CommandResult::None;
      }

      while (record_.makeMove())
        ;
      return CommandResult::Changed;

    case Command::Search:
      // 探索
      search(false);
      return CommandResult::None;

    case Command::Moves:
      // 合法手生成
      showMoves();
      return CommandResult::None;

    case Command::Book:
      // 定跡検索
      probeBook();
      return CommandResult::None;

    case Command::ClearTT:
      // TTクリア
      searcher_.clearTT();
      return CommandResult::None;

    case Command::ClearHistory:
      // Historyクリア
      searcher_.clearHistory();
      return CommandResult::None;

    default: {
      // 指し手入力
      Move move;
      if (!inputMove(line, record_.getBoard(), move)) {
      std::cout << "unknown command!!\n";
      return CommandResult::None;
      }

      if (!record_.makeMove(move)) {
        std::cout << "illegal move!!\n";
        return CommandResult::None;
      }

      std::cout << "Move:\n";
      std::cout << "  " << move.toString() << '\n';
      std::cout << std::endl;
      return CommandResult::Changed;
    }
  }

}

/**
 * コンソール上で対局を行います。
 */
bool ConsoleManager::play() {

  // 定跡の読み込み
  if (!book_.readFile()) {
    Loggers::warning << "Could not read book.";
  }

  record_.init(Board::Handicap::Even);

  // 棋譜の読み込み
  if (!config_.inFileName.empty()) {
    CsaReader::read(config_.inFileName, record_);
  }

  auto searcherConfig = buildSearcherConfig(searcher_.getConfig(), config_);
  searcher_.setConfig(searcherConfig);

  printBoard(record_.getBoard());

  while (true) {

    if (!config_.outFileName.empty()) {
      CsaWriter::write(config_.outFileName, record_);
    }

    if (isAuto()) {

      // コンピュータによる着手
      search(true);

      // 盤面表示
      printBoard(record_.getBoard());

    } else {

      // ユーザからのコマンド入力
      CommandResult result = inputCommand();

      if (result == CommandResult::Changed) {
        // 盤面表示
        printBoard(record_.getBoard());

      } else if (result == CommandResult::Quit) {
        // quit
        break;

      }

    }
  }

  return true;
}

} // namespace sunfish
