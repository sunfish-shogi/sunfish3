/* MoveGeneratorExpr.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "MoveGeneratorExpr.h"
#include "../move/MoveGenerator.h"
#include "../record/CsaReader.h"
#include "../util/Timer.h"
#include "logger/Logger.h"
#include <sstream>
#include <iomanip>

namespace sunfish {

void MoveGeneratorExpr::testSpeed() {
  Record record;
  MoveGenerator gen;
  Moves moves;

  std::string csa = "\
P1-KY *  *  *  *  *  * -KE-KY\n\
P2 *  *  *  *  * +TO * -KI-OU\n\
P3 *  * -KE-FU * +GI *  *  * \n\
P4-FU * -FU *  *  *  * +FU-FU\n\
P5 *  *  * +FU *  * +GI-FU * \n\
P6 * +FU+FU-KA *  * +FU * +FU\n\
P7+FU *  *  *  *  * +KI+GI * \n\
P8+HI *  *  *  *  *  *  *  * \n\
P9+KY+KE *  *  *  * -KA+OU+KY\n\
P+00KI00HI\n\
P-00FU00FU00FU00FU00FU00KE00GI00KI\n\
-\n\
";
  std::istringstream iss(csa);
  std::ostringstream oss;

  CsaReader::read(iss, record);

  Loggers::debug << record.getBoard().toString();

  moves.clear();
  gen.generate(record.getBoard(), moves);
  for (auto ite = moves.begin(); ite != moves.end(); ite++) {
    oss << (*ite).toString() << ", ";
  }
  Loggers::debug << oss.str();
  Loggers::debug << moves.size() << " moves";

  Timer timer;
  timer.set();

  int count = 10 * 1000 * 1000;
  for (int i = 0; i < count; i++) {
    moves.clear();
    gen.generate(record.getBoard(), moves);
  }

  float elapsed = timer.get();
  float speed = (float)count / elapsed;
  Loggers::develop << "elapsed: " << elapsed << "[sec]";
  Loggers::develop << "count  : " << count;
  Loggers::develop << "speed  : " << std::fixed << std::setprecision(2) << speed << "[1/sec]";
}

} // namespace sunfish

#endif // !defined(NDEBUG)
