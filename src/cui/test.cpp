/* test.cpp
 * 
 * Kubo Ryosuke
 */

#include "config.h"
#include "logger/Logger.h"
#include "test/Test.h"
#include <fstream>

#if !defined(NDEBUG)

using namespace sunfish;

int test() {

  // logger settings
  std::ofstream fout("test.log", std::ios::out);
  if (fout) {
    Loggers::debug.addStream(fout);
    Loggers::test.addStream(fout);
  }
  Loggers::error.addStream(std::cerr, ESC_SEQ_COLOR_RED, ESC_SEQ_COLOR_RESET);
  Loggers::warning.addStream(std::cerr, ESC_SEQ_COLOR_YELLOW, ESC_SEQ_COLOR_RESET);
  Loggers::message.addStream(std::cerr, ESC_SEQ_COLOR_GREEN, ESC_SEQ_COLOR_RESET);
  Loggers::send.addStream(std::cerr, true, true, ESC_SEQ_COLOR_BLUE, ESC_SEQ_COLOR_RESET);
  Loggers::receive.addStream(std::cerr, true, true, ESC_SEQ_COLOR_MAGENTA, ESC_SEQ_COLOR_RESET);
  Loggers::debug.addStream(std::cerr, ESC_SEQ_COLOR_CYAN, ESC_SEQ_COLOR_RESET);
  Loggers::develop.addStream(std::cerr, ESC_SEQ_COLOR_WHITE, ESC_SEQ_COLOR_RESET);

  // execute
  bool result = TestSuite::test();

  if (result) {
    Loggers::message << "Test passed.";
  } else {
    Loggers::error << "Test failed.";
  }
  Loggers::message << "Look 'test.log'.";

  // return value
  return result ? 0 : 1;

}

#endif //!defined(NDEBUG)
