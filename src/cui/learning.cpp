/* learn.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "config.h"
#include "learning/Learning.h"
#include "searcher/eval/Evaluator.h"
#include "logger/Logger.h"
#include <fstream>

using namespace sunfish;

int learn() {
  std::ofstream fout("learn.log", std::ios::out | std::ios::app);
  if (fout) {
    Loggers::error.addStream(fout, true, true);
    Loggers::warning.addStream(fout, true, true);
    Loggers::message.addStream(fout, true, true);
  }
  Loggers::error.addStream(std::cerr, ESC_SEQ_COLOR_RED, ESC_SEQ_COLOR_RESET);
  Loggers::warning.addStream(std::cerr, ESC_SEQ_COLOR_YELLOW, ESC_SEQ_COLOR_RESET);
  Loggers::message.addStream(std::cerr);
  Loggers::send.addStream(std::cerr, true, true, ESC_SEQ_COLOR_BLUE, ESC_SEQ_COLOR_RESET);
  Loggers::receive.addStream(std::cerr, true, true, ESC_SEQ_COLOR_MAGENTA, ESC_SEQ_COLOR_RESET);
#ifndef NDEBUG
  Loggers::debug.addStream(std::cerr, ESC_SEQ_COLOR_CYAN, ESC_SEQ_COLOR_RESET);
  Loggers::develop.addStream(std::cerr, ESC_SEQ_COLOR_WHITE, ESC_SEQ_COLOR_RESET);
#endif // NDEBUG

  Learning learning;
  bool ok = learning.run();

  return ok ? 0 : 1;
}

int analyzeEvalBin() {
  Loggers::error.addStream(std::cerr, "\x1b[31m", "\x1b[39m");
  Loggers::warning.addStream(std::cerr, "\x1b[33m", "\x1b[39m");
  Loggers::message.addStream(std::cerr);
  Loggers::send.addStream(std::cerr, true, true, "\x1b[34m", "\x1b[39m");
  Loggers::receive.addStream(std::cerr, true, true, "\x1b[35m", "\x1b[39m");
#ifndef NDEBUG
  Loggers::debug.addStream(std::cerr, "\x1b[36m", "\x1b[39m");
  Loggers::develop.addStream(std::cerr, "\x1b[37m", "\x1b[39m");
#endif // NDEBUG

  Evaluator eval;
  eval.readFile();

  Evaluator::ValueType max = 0;
  int64_t magnitude = 0ll;
  int32_t nonZero = 0;

  auto func = [](const Evaluator::ValueType& e,
      Evaluator::ValueType& max, int64_t& magnitude, int32_t& nonZero) {
    max = std::max(max, (Evaluator::ValueType)std::abs(e));
    magnitude += std::abs(e);
    nonZero += e != 0 ? 1 : 0;
  };

  for (int i = 0; i < KPP_ALL; i++) {
    func(eval.t_->kpp[0][i], max, magnitude, nonZero);
  }
  for (int i = 0; i < KKP_ALL; i++) {
    func(eval.t_->kkp[0][0][i], max, magnitude, nonZero);
  }

  Loggers::message << "max=" << max
    << "\tmagnitude=" << magnitude
    << "\tnonZero=" << nonZero
    << "\tzero=" << (KPP_ALL + KKP_ALL - nonZero);

  return 1;
}

#endif // NLEARN
