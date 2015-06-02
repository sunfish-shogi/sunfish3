/* Test.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_TEST__
#define SUNFISH_TEST__

#if !defined(NDEBUG)

#include <typeinfo>
#include <sstream>
#include <string>
#include <map>
#include "logger/Logger.h"

#define TEST(group, name) \
void test_method_ ## group ## name();\
sunfish::test_adder__ __test_adder_ ## group ## name (#group, #name, test_method_ ## group ## name); \
void test_method_ ## group ## name()

#define TEST_RESULT__(reason) throw sunfish::TestError{(reason), __FILE__, __LINE__}

#define ASSERT_EQ(correct, exact) do { \
  const auto correct_ = (correct); \
  const auto exact_ = (exact); \
  if (!((exact_) == (correct_))) {\
    std::ostringstream oss;\
    oss << "\"" << (exact_) << "\" is not equal to \"" << (correct_) << "\".";\
    TEST_RESULT__(oss.str());\
  } \
} while (false)

#define ASSERT(b) do { \
  const auto b_ = (b);\
  if (!b_) { TEST_RESULT__(std::string(#b) + " is not true."); } \
} while (false)

namespace sunfish {

typedef void (*TEST_METHOD)();

/**
 * error
 */
struct TestError {
  std::string reason;
  const char* file;
  unsigned line;
};

class TestSuite {
private:

  std::map<std::string, std::map<std::string, TEST_METHOD>> tests_;

  static TestSuite* getInstance() {
    static TestSuite instance;
    return &instance;
  }

public:

  static void addTest(const char* groupName, const char* methodName, TEST_METHOD method) {
    auto ins = getInstance();
    ins->tests_[groupName][methodName] = method;
  }

  static bool test() {
    auto ins = getInstance();
    const auto& tests = ins->tests_;
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

class test_adder__ {
public:
  test_adder__(const char* groupName, const char* methodName, TEST_METHOD method) {
    TestSuite::addTest(groupName, methodName, method);
  }
};

} // namespace sunfish

#endif // !defined(NDEBUG)

#endif // SUNFISH_TEST__
