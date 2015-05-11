/* EvaluateTableTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../eval/EvaluateTable.h"

using namespace sunfish;

TEST(SeeTest, testEvaluateTable) {
  EvaluateTable<21> table;

  uint64_t hash1 = 0x8f70ac3b2973d012LL;
  uint64_t hash2 = 0x9b2185ef180294ceLL;
  Value value1 = 1234;
  Value result1;
  Value result2;

  table.set(hash1, value1);

  bool ok1 = table.get(hash1, result1);
  bool ok2 = table.get(hash2, result2);

  ASSERT_EQ(true, ok1);
  ASSERT_EQ(value1.int32(), result1.int32());
  ASSERT_EQ(false, ok2);
}

#endif // !defined(NDEBUG)

