/* dev.cpp
 * 
 * Kubo Ryosuke
 */

#include "config.h"
#include "logger/Logger.h"
#include "core/dev/MoveGeneratorExpr.h"
#include "core/dev/CodeGenerator.h"
#include "core/dev/MoveGenChecker.h"
#include <fstream>

#if !defined(NDEBUG)

using namespace sunfish;

static void initLoggers() {
  Loggers::error.addStream(std::cerr, ESC_SEQ_COLOR_RED, ESC_SEQ_COLOR_RESET);
  Loggers::warning.addStream(std::cerr, ESC_SEQ_COLOR_YELLOW, ESC_SEQ_COLOR_RESET);
  Loggers::message.addStream(std::cerr);
  Loggers::send.addStream(std::cerr, true, true, ESC_SEQ_COLOR_BLUE, ESC_SEQ_COLOR_RESET);
  Loggers::receive.addStream(std::cerr, true, true, ESC_SEQ_COLOR_MAGENTA, ESC_SEQ_COLOR_RESET);
  Loggers::debug.addStream(std::cerr, ESC_SEQ_COLOR_CYAN, ESC_SEQ_COLOR_RESET);
  Loggers::test.addStream(std::cerr, ESC_SEQ_COLOR_GREEN, ESC_SEQ_COLOR_RESET);
  Loggers::develop.addStream(std::cerr, ESC_SEQ_COLOR_GREEN, ESC_SEQ_COLOR_RESET);
}

// 指し手生成速度計測
int exprMoveGenSpeed() {
  initLoggers();

  MoveGeneratorExpr expr;
  expr.testSpeed();

  return 0;
}

// ハッシュ表生成
int generateZobrist() {
  CodeGenerator gen;
  gen.generateZobrist();

  return 0;
}

int checkMoveGen() {
  MoveGenChecker checker;
  bool ok = checker.check();

  return ok ? 0 : 1;
}

#endif //!defined(NDEBUG)
