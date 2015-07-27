/* profile.cpp
 * 
 * Kubo Ryosuke
 */

#include "config.h"
#include "logger/Logger.h"
#include "console/ConsoleManager.h"
#include "searcher/Searcher.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

int profile(const ConsoleManager::Config& config, bool full) {
  Loggers::error.addStream(std::cerr, ESC_SEQ_COLOR_RED, ESC_SEQ_COLOR_RESET);
  Loggers::warning.addStream(std::cerr, ESC_SEQ_COLOR_YELLOW, ESC_SEQ_COLOR_RESET);
  Loggers::message.addStream(std::cerr);
  Loggers::send.addStream(std::cerr, true, true, ESC_SEQ_COLOR_BLUE, ESC_SEQ_COLOR_RESET);
  Loggers::receive.addStream(std::cerr, true, true, ESC_SEQ_COLOR_MAGENTA, ESC_SEQ_COLOR_RESET);
#ifndef NDEBUG
  Loggers::debug.addStream(std::cerr, ESC_SEQ_COLOR_CYAN, ESC_SEQ_COLOR_RESET);
  Loggers::test.addStream(std::cerr, ESC_SEQ_COLOR_GREEN, ESC_SEQ_COLOR_RESET);
  Loggers::develop.addStream(std::cerr, ESC_SEQ_COLOR_WHITE, ESC_SEQ_COLOR_RESET);
#endif

  Searcher searcher;
  auto searcherConfig = ConsoleManager::buildSearcherConfig(searcher.getConfig(), config);
  searcher.setConfig(searcherConfig);

  const char* data[] = {
R"(
P1-KY-KE * -KI *  *  *  *  * 
P2 * -OU-GI-GI *  *  *  *  * 
P3 * -FU-FU-FU * -FU-KE *  * 
P4-FU *  *  * +GI *  *  * -FU
P5 *  * +GI *  *  * -FU *  * 
P6+FU+HI *  * +UM *  *  *  * 
P7 * +FU+KE+FU+OU * +FU * +KE
P8 *  * +KI *  *  *  *  *  * 
P9+KY *  * -RY *  *  *  * +KY
P+00FU00FU00FU00KI00KI
P-00FU00FU00FU00FU00KY00KA
-
)",
R"(
P1-KY *  *  *  *  * -OU-KE-KY
P2 *  *  *  *  *  * -KI *  * 
P3 *  * +GI *  * -KI * -FU * 
P4 *  *  *  *  * +FU-FU * -FU
P5-FU-HI *  * -FU *  *  *  * 
P6 *  *  *  *  * +HI+FU *  * 
P7+FU-GI * +OU+FU+GI+KE * +FU
P8 *  *  *  *  *  * +KI *  * 
P9 *  *  *  *  *  *  *  * +KY
P+00KA00KE00KE00FU00FU00FU00FU00FU00FU00FU
P-00KA00KI00GI00KY00FU
-
)",
R"(
P1-KY-KE *  *  *  *  * -KE-KY
P2-HI *  *  *  *  * -KI *  * 
P3 *  *  * -KA+NK-KI *  * -OU
P4-FU * -FU+KA-GI-GI-FU+FU-FU
P5 * -FU *  *  *  *  * -FU * 
P6+FU * +FU+FU+GI * +FU * +FU
P7 * +FU+GI * +FU *  *  *  * 
P8 * +OU+KI+KI * +HI *  * +KY
P9+KY+KE *  *  *  *  *  *  * 
P+00FU
P-00FU00FU00FU
-
)",
R"(
P1-KY-HI *  *  *  *  * -KE-OU
P2 *  *  *  *  *  * -KI-GI-KY
P3 *  * +KA * -GI-KI * -FU-FU
P4 * -FU-FU-FU *  * -FU *  * 
P5-FU-KE *  * +GI+FU * +FU+FU
P6 * +GI+FU+FU *  * +FU *  * 
P7+FU+FU * +KI * -FU+KE *  * 
P8 * +OU+KI *  *  *  * +HI * 
P9+KY+KE *  *  * -KA *  * +KY
P+00FU
P-00FU
-
)",
R"(
P1 *  * -HI *  *  *  * -KE-KY
P2-KY *  *  *  *  * -KI-OU * 
P3 *  * -GI *  * -KI * -FU * 
P4-FU-FU-FU-FU-FU-GI-FU * -FU
P5 * -KE *  *  * +KE * +FU * 
P6+FU+GI+FU+FU+FU * +FU * +FU
P7 * +FU * +KI *  *  *  *  * 
P8 * +OU+KI+GI * +HI *  *  * 
P9+KY+KE *  *  *  *  *  * +KY
P+00KA00FU
P-00KA00FU
+
)",
R"(
P1-KY *  *  *  *  * -OU-KE-KY
P2 *  *  *  * -GI * -KI *  * 
P3-FU *  *  * -HI *  * -GI-FU
P4 *  * -FU+KA * -KI * -FU * 
P5 * -FU *  *  * -FU-FU *  * 
P6 *  * +FU-FU+FU *  *  * +FU
P7+FU *  * +OU * +UM+KI *  * 
P8 *  *  *  *  *  *  * +HI * 
P9+KY+KE *  *  *  *  *  * +KY
P+00GI00KE00KE00FU00FU00FU00FU00FU
P-00KI00GI00FU
+
)",
  };

  for (auto p : data) {
    Loggers::message << p;

    std::istringstream iss(p);
    Board board;
    CsaReader::readBoard(iss, board);

    Move move;
    bool ok = searcher.idsearch(board, move);
    if (ok) {
      Loggers::message << move.toString();
    }

    Loggers::message << searcher.getInfoString();

    if (!full) {
      break;
    }
  }

  return 0;
}
