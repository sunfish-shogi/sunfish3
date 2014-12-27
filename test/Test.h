/* Test.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TEST__
#define __SUNFISH_TEST__

#include <typeinfo>
#include <sstream>
#include <string>
#include <map>
#include "logger/Logger.h"

#define TEST(group, name)														\
void __test_method_ ## group ## name();\
sunfish::__test_adder__ __test_adder_ ## group ## name (#group, #name, __test_method_ ## group ## name); \
void __test_method_ ## group ## name()

#define __TEST_RESULT__(reason)								throw sunfish::TestError{(reason).c_str(), __FILE__, __LINE__}

#define ASSERT_EQ(correct, exact)							do { \
	const auto _correct = (correct); \
	const auto _exact = (exact); \
	if (!((_exact) == (_correct))) {\
		std::ostringstream oss;\
		oss << "\"" << (_exact) << "\" is not equal to \"" << (_correct) << "\".";\
		__TEST_RESULT__(oss.str());\
	} \
} while (false)

#define ASSERT(b)															do { \
	const auto _b = (b);\
	if (!_b) { __TEST_RESULT__(std::string(#b) + " is not true."); } \
} while (false)

namespace sunfish {

	typedef void (*TEST_METHOD)();

	/**
	 * error
	 */
	struct TestError {
		const char* reason;
		const char* file;
		unsigned line;
	};

	class TestSuite {
	private:
	
		std::map<std::string, std::map<std::string, TEST_METHOD>> _tests;

		static TestSuite* getInstance() {
			static TestSuite instance;
			return &instance;
		}

	public:

		static void addTest(const char* groupName, const char* methodName, TEST_METHOD method) {
			auto ins = getInstance();
			ins->_tests[groupName][methodName] = method;
		}

		static bool test() {
			auto ins = getInstance();
			const auto& tests = ins->_tests;
			int totalSuccess = 0;
			int totalFail = 0;

			Loggers::test << "<?xml version=\"1.0\"?>";
			Loggers::test << "<testSuite>";
			Loggers::test << "\t<tests>";

			for (auto it = tests.begin(); it != tests.end(); it++) {

				auto& groupName = it->first;
				auto& methods = it->second;
				int success = 0;
				int fail = 0;

				Loggers::test << "\t\t<test name=\"" << groupName << "\">";
				Loggers::test << "\t\t\t<methods>";

				for (auto im = methods.begin(); im != methods.end(); im++) {

					auto& methodName = im->first;
					auto method = im->second;

					Loggers::test << "\t\t\t\t<method name=\"" << methodName << "\">";
					try {
						method();
						Loggers::test << "\t\t\t\t\t<status>success</status>";
						success++;
					} catch (TestError e) {
						Loggers::test << "\t\t\t\t\t<status>fail</status>";
						Loggers::test << "\t\t\t\t\t<reason>" << e.reason << "</reason>";
						Loggers::test << "\t\t\t\t\t<file>" << e.file << ':' << e.line << "</file>";
						fail++;
					}
					Loggers::test << "\t\t\t\t</method>";

				}

				Loggers::test << "\t\t\t</methods>";
				Loggers::test << "\t\t\t<summary>";
				Loggers::test << "\t\t\t\t<total>" << (success + fail) << "</total>";
				Loggers::test << "\t\t\t\t<success>" << success << "</success>";
				Loggers::test << "\t\t\t\t<fail>" << fail << "</fail>";
				Loggers::test << "\t\t\t</summary>";
				Loggers::test << "\t\t</test>";

				totalSuccess += success;
				totalFail += fail;

			}

			Loggers::test << "\t</tests>";
			Loggers::test << "\t<summary>";
			Loggers::test << "\t\t<total>" << (totalSuccess + totalFail) << "</total>";
			Loggers::test << "\t\t<success>" << totalSuccess << "</success>";
			Loggers::test << "\t\t<fail>" << totalFail << "</fail>";
			Loggers::test << "\t</summary>";
			Loggers::test << "</testSuite>";

			return totalFail == 0;
		}

	};

	class __test_adder__ {
	public:
		__test_adder__(const char* groupName, const char* methodName, TEST_METHOD method) {
			TestSuite::addTest(groupName, methodName, method);
		}
	};

}

#endif // __SUNFISH_TEST__
