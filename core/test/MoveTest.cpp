/* MoveTest.cpp
 *
 * Kubo Ryosuke
 */

 #if !defined(NDEBUG)

#include "test/Test.h"
#include "../move/Move.h"

using namespace sunfish;

TEST(MoveTest, test) {
	{
		Move move(Piece::BPawn, P57, P56, false);
		ASSERT_EQ(P57, move.from());
		ASSERT_EQ(P56, move.to());
		ASSERT_EQ(false, move.promote());
		ASSERT_EQ(Piece::Pawn, move.piece());
		ASSERT_EQ(false, move.isHand());
	}

	{
		Move move(Piece::WPawn, P53, P54, false);
		ASSERT_EQ(P53, move.from());
		ASSERT_EQ(P54, move.to());
		ASSERT_EQ(false, move.promote());
		ASSERT_EQ(Piece::Pawn, move.piece());
		ASSERT_EQ(false, move.isHand());
	}

	{
		Move move(Piece::BPawn, P54, P53, true);
		ASSERT_EQ(P54, move.from());
		ASSERT_EQ(P53, move.to());
		ASSERT_EQ(true, move.promote());
		ASSERT_EQ(Piece::Pawn, move.piece());
		ASSERT_EQ(false, move.isHand());
	}

	{
		Move move(Piece::BPawn, P55);
		ASSERT_EQ(P55, move.to());
		ASSERT_EQ(false, move.promote());
		ASSERT_EQ(Piece::Pawn, move.piece());
		ASSERT_EQ(true, move.isHand());
	}
}

#endif // !defined(NDEBUG)
