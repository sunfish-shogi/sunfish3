/* test.cpp
 * 
 * Kubo Ryosuke
 */

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
	Loggers::error.addStream(std::cerr, "\x1b[31m", "\x1b[39m");
	Loggers::warning.addStream(std::cerr, "\x1b[33m", "\x1b[39m");
	Loggers::message.addStream(std::cerr, "\x1b[32m", "\x1b[39m");
	Loggers::send.addStream(std::cerr, true, true, "\x1b[34m", "\x1b[39m");
	Loggers::receive.addStream(std::cerr, true, true, "\x1b[35m", "\x1b[39m");
	Loggers::debug.addStream(std::cerr, "\x1b[36m", "\x1b[39m");
	Loggers::develop.addStream(std::cerr, "\x1b[37m", "\x1b[39m");

	// execute
	bool result = TestSuite::test();

	if (result) {
		Loggers::message << "The test is succeeded.";
	} else {
		Loggers::error << "The test is failed.";
	}
	Loggers::message << "Look 'test.log'.";

	// return value
	return result ? 0 : 1;

}

#endif //!defined(NDEBUG)
