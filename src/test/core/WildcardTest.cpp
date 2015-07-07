/* WildcardTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "core/util/Wildcard.h"

using namespace sunfish;

TEST(Wildcard, test) {

  Wildcard test1("test*;");
  ASSERT_EQ(true, test1.match("test;"));
  ASSERT_EQ(true, test1.match("test_abcde;"));
  ASSERT_EQ(true, test1.match("test_abcde;;"));
  ASSERT_EQ(false, test1.match("test_abcde; "));
  ASSERT_EQ(false, test1.match(" test_abcde;"));

  Wildcard test2("*");
  ASSERT_EQ(true, test2.match(""));
  ASSERT_EQ(true, test2.match("1"));
  ASSERT_EQ(true, test2.match("12"));
  ASSERT_EQ(true, test2.match("sunfish*shogi"));

  Wildcard test3("?");
  ASSERT_EQ(false, test3.match(""));
  ASSERT_EQ(true, test3.match("x"));
  ASSERT_EQ(true, test3.match("?"));
  ASSERT_EQ(false, test3.match(""));

  Wildcard test4("???");
  ASSERT_EQ(false, test4.match(""));
  ASSERT_EQ(false, test4.match("1"));
  ASSERT_EQ(false, test4.match("12"));
  ASSERT_EQ(true, test4.match("123"));
  ASSERT_EQ(false, test4.match("1234"));

  Wildcard test5("???*");
  ASSERT_EQ(false, test5.match(""));
  ASSERT_EQ(false, test5.match("12"));
  ASSERT_EQ(true, test5.match("123"));
  ASSERT_EQ(true, test5.match("1234"));

  Wildcard test6("*???");
  ASSERT_EQ(false, test6.match(""));
  ASSERT_EQ(false, test6.match("12"));
  ASSERT_EQ(true, test6.match("123"));
  ASSERT_EQ(true, test6.match("1234"));

  Wildcard test7("\\?");
  ASSERT_EQ(false, test7.match(""));
  ASSERT_EQ(true, test7.match("?"));
  ASSERT_EQ(false, test7.match("1"));

  Wildcard test8("?\\??");
  ASSERT_EQ(true, test8.match("???"));
  ASSERT_EQ(true, test8.match("x?x"));
  ASSERT_EQ(false, test8.match("xxx"));

  Wildcard test9("#command ?*=*;");
  ASSERT_EQ(true, test9.match("#command key=value;"));
  ASSERT_EQ(true, test9.match("#command key=3;"));
  ASSERT_EQ(true, test9.match("#command key=;"));
  ASSERT_EQ(false, test9.match("#command =;"));
  ASSERT_EQ(false, test9.match("#command key=value"));
  ASSERT_EQ(false, test9.match("#command key="));

  Wildcard test10("test**sunfish");
  ASSERT_EQ(true, test10.match("testsunfish"));
  ASSERT_EQ(true, test10.match("test sunfish"));
  ASSERT_EQ(true, test10.match("test - sunfish"));
  ASSERT_EQ(false, test10.match(" test - sunfish"));
  ASSERT_EQ(false, test10.match("test - sunfish "));

  Wildcard test11("test*?*sunfish");
  ASSERT_EQ(false, test11.match("testsunfish"));
  ASSERT_EQ(true, test11.match("test sunfish"));
  ASSERT_EQ(true, test11.match("test - sunfish"));

}

#endif // !defined(NDEBUG)
