/* Solver.cpp
 *
 * Kubo Ryosuke
 */

#include "Solver.h"
#include "logger/Logger.h"
#include "core/record/CsaReader.h"

namespace sunfish {

void Solver::setProblems(const std::vector<std::string>& problems) {
  problems_.clear();
  problems_.reserve(problems.size());
  problems_ = problems;
}

void Solver::solve() {
  for (const auto& problem : problems_) {
    solve(problem);
  }

  float correctRatio = (float)summary_.correct / summary_.total;
  float incorrectRatio = (float)summary_.incorrect / summary_.total;
  float ignoreRatio = (float)summary_.ignore / summary_.total;

  Loggers::message << "Complete!!";
  Loggers::message << "";
  Loggers::message << "Summary:";
  Loggers::message << "  total     : " << summary_.total;
  Loggers::message << "  correct   : " << summary_.correct << " (" << correctRatio * 100.0 << "%)";
  Loggers::message << "  incorrect : " << summary_.incorrect << " (" << incorrectRatio * 100.0 << "%)";
  Loggers::message << "  ignore    : " << summary_.ignore << " (" << ignoreRatio * 100.0 << "%)";
}

void Solver::solve(const std::string& problem) {

  Loggers::message << "[" << problem << "]";

  // 棋譜を読み込む
  if (!CsaReader::read(problem, record_)) {
    errors_.push_back({ problem, ErrorCode::FILE_READ_ERROR });
    summary_.ignore++;
    Loggers::message << "read error: [" << problem << "]";
    return;
  }

  // 不正な手数
  if (record_.getTotalCount() == 0) {
    errors_.push_back({ problem, ErrorCode::INVALID_RECORD_LENGTH });
    summary_.ignore++;
    Loggers::message << "invalid record: [" << problem << "]";
    return;
  }

  // 序盤30手を飛ばす
  int size = (int)record_.getTotalCount();
  int startPos = std::max(0, std::min(30, size-30));

  while ((int)record_.getCount() > startPos && record_.unmakeMove())
    ;

  while (true) {
    Move correct = record_.getNextMove();
    if (correct.isEmpty()) {
      break;
    }
    solve(record_.getBoard(), correct);
    if (!record_.makeMove()) {
      break;
    }
  }

}

void Solver::solve(const Board& board, const Move& correct) {

  Loggers::message << board.toStringCsa();

  Move answer;
  bool ok = searcher_.idsearch(board, answer);

  bool result;
  if (ok && answer == correct) {
    summary_.correct++;
    result = true;
  } else {
    summary_.incorrect++;
    result = false;
  }
  summary_.total++;

  bool black = board.isBlack();
  Loggers::message << "correct: " << correct.toStringCsa(black);
  Loggers::message << "answer : " << (ok ? answer.toStringCsa(black) : "Resign");
  Loggers::message << "result : " << (result ? "Good" : "Bad");
  Loggers::message << "";

}

} // namespace sunfish
