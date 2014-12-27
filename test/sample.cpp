#ifdef SUNFISH_TEST_SAMPLE

#include "Test.h"
#include "logger/Logger.h"
#include <fstream>

using namespace sunfish;

// test code
TEST(sample, test) {
	int a = 1;
	ASSERT_EQ(1, a);
	ASSERT(a < 2);
}

int main(int, char**, char**) {

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

	// test
	bool result = TestSuite::test();

	if (result) {
		Loggers::message << "Test was succeeded.";
	} else {
		Loggers::error << "Test was failed.";
	}
	Loggers::message << "Show 'test.log'.";

	// return value
	return result ? 0 : 1;

}

#endif // SUNFISH_TEST_SAMPLE
