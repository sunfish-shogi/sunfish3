/* problem.cpp
 * 
 * Kubo Ryosuke
 */

#include "config.h"
#include "logger/Logger.h"
#include "solve/Solver.h"
#include "console/ConsoleManager.h"
#include <vector>
#include <fstream>

using namespace sunfish;

int solve(const std::vector<std::string>& problems, const ConsoleManager::Config& config) {

  // logger settings
  std::ofstream fout("problem.log", std::ios::out);
  if (fout) {
    Loggers::message.addStream(fout, true, true);
  }
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

  Solver solver;
  solver.setProblems(problems);
  solver.setConfig(config);
  solver.solve();

  return solver.hasError() ? 1 : 0;

}
