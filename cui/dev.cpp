/* dev.cpp
 * 
 * Kubo Ryosuke
 */

#include "logger/Logger.h"
#include "core/dev/MoveGeneratorExpr.h"
#include "core/dev/CodeGenerator.h"
#include "core/dev/MoveGenChecker.h"
#include <fstream>

#if !defined(NDEBUG)

using namespace sunfish;

static void initLoggers() {
	Loggers::error.addStream(std::cerr, "\x1b[31m", "\x1b[39m");
	Loggers::warning.addStream(std::cerr, "\x1b[33m", "\x1b[39m");
	Loggers::message.addStream(std::cerr);
	Loggers::send.addStream(std::cerr, true, true, "\x1b[34m", "\x1b[39m");
	Loggers::receive.addStream(std::cerr, true, true, "\x1b[35m", "\x1b[39m");
	Loggers::debug.addStream(std::cerr, "\x1b[36m", "\x1b[39m");
	Loggers::test.addStream(std::cerr, "\x1b[32m", "\x1b[39m");
	Loggers::develop.addStream(std::cerr, "\x1b[32m", "\x1b[39m");
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
