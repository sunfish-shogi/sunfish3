/* network.cpp
 * 
 * Kubo Ryosuke
 */

#include "config.h"
#include "network/CsaClient.h"
#include <fstream>

using namespace sunfish;

int network() {

  // logger settings
  std::ofstream fout("network.log", std::ios::out | std::ios::app);
  if (fout) {
    Loggers::error.addStream(fout, true, true);
    Loggers::warning.addStream(fout, true, true);
    Loggers::message.addStream(fout, true, true);
    Loggers::send.addStream(fout, true, true);
    Loggers::receive.addStream(fout, true, true);
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

  CsaClient csaClient;

  csaClient.execute();

  return 0;

}
