/* PositionTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../base/Position.h"

using namespace sunfish;

TEST(PositionTest, test) {
	{
		Position pos76(7, 6);
		ASSERT_EQ(P76, pos76);
	}

	{
		Position pos55(5, 5);
		ASSERT_EQ(P54, pos55.up());
		ASSERT_EQ(P56, pos55.down());
		ASSERT_EQ(P65, pos55.left());
		ASSERT_EQ(P45, pos55.right());
		ASSERT_EQ(P53, pos55.up(2));
		ASSERT_EQ(P57, pos55.down(2));
		ASSERT_EQ(P75, pos55.left(2));
		ASSERT_EQ(P35, pos55.right(2));
	}

	{
		Position pos55(5, 5);
		ASSERT_EQ(P56, pos55.next());

		Position pos59(5, 9);
		ASSERT_EQ(P41, pos59.next());

		Position pos19(1, 9);
		ASSERT_EQ(Position::End, pos19.next());
	}

	{
		Position pos55(5, 5);
		ASSERT_EQ(P45, pos55.nextRightDown());

		Position pos15(1, 5);
		ASSERT_EQ(P96, pos15.nextRightDown());

		Position pos19(1, 9);
		ASSERT_EQ(Position::End, pos19.nextRightDown());
	}
}

#endif // !defined(NDEBUG)
