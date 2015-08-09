/* SquareTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "core/base/Square.h"

using namespace sunfish;

TEST(SquareTest, test) {
  {
    Square sq76(7, 6);
    ASSERT_EQ(P76, sq76);
  }

  {
    Square sq55(5, 5);
    ASSERT_EQ(P54, sq55.up());
    ASSERT_EQ(P56, sq55.down());
    ASSERT_EQ(P65, sq55.left());
    ASSERT_EQ(P45, sq55.right());
    ASSERT_EQ(P53, sq55.up(2));
    ASSERT_EQ(P57, sq55.down(2));
    ASSERT_EQ(P75, sq55.left(2));
    ASSERT_EQ(P35, sq55.right(2));
  }

  {
    Square sq34(3, 4);
    ASSERT_EQ(P76, sq34.reverse());
    ASSERT_EQ(P74, sq34.sym());
  }

  {
    Square sq55(5, 5);
    ASSERT_EQ(P56, sq55.next());

    Square sq59(5, 9);
    ASSERT_EQ(P41, sq59.next());

    Square sq19(1, 9);
    ASSERT_EQ(Square::End, sq19.next());
  }

  {
    Square sq55(5, 5);
    ASSERT_EQ(P45, sq55.nextRightDown());

    Square sq15(1, 5);
    ASSERT_EQ(P96, sq15.nextRightDown());

    Square sq19(1, 9);
    ASSERT_EQ(Square::End, sq19.nextRightDown());
  }
}

#endif // !defined(NDEBUG)
