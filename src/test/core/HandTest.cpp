/* HandTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "core/board/Hand.h"

using namespace sunfish;

TEST(HandTest, test) {
  Hand hand;

  ASSERT_EQ(0, hand.get(Piece::Pawn));
  hand.inc(Piece::Pawn);
  ASSERT_EQ(1, hand.get(Piece::Pawn));
  hand.inc(Piece::Pawn);
  hand.inc(Piece::Pawn);
  ASSERT_EQ(3, hand.get(Piece::Pawn));
  hand.dec(Piece::Pawn);
  ASSERT_EQ(2, hand.get(Piece::Pawn));

  ASSERT_EQ(0, hand.get(Piece::Rook));
  hand.inc(Piece::Rook);
  ASSERT_EQ(1, hand.get(Piece::Rook));

  ASSERT_EQ(0, hand.get(Piece::Knight));
  hand.inc(Piece::ProKnight);
  ASSERT_EQ(1, hand.get(Piece::Knight));
}

TEST(HandTest, testUnsafe) {
  Hand hand;

  ASSERT_EQ(0, hand.get(Piece::Pawn));
  hand.inc(Piece::Pawn);
  ASSERT_EQ(1, hand.get(Piece::Pawn));
  hand.inc(Piece::Pawn);
  hand.inc(Piece::Pawn);
  ASSERT_EQ(3, hand.get(Piece::Pawn));
  hand.dec(Piece::Pawn);
  ASSERT_EQ(2, hand.get(Piece::Pawn));

  ASSERT_EQ(0, hand.get(Piece::Rook));
  hand.inc(Piece::Rook);
  ASSERT_EQ(1, hand.get(Piece::Rook));
}

#endif // !defined(NDEBUG)
