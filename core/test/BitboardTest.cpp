/* BitboardTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../board/Bitboard.h"

using namespace sunfish;

TEST(BitboardTest, test) {
	Bitboard bb;
	bb.init();
	bb.set(P55);
	ASSERT_EQ(Bitboard::mask(P55), bb);
	ASSERT(bb.check(P55));
}

TEST(BitboardTest, testShift) {
	ASSERT_EQ(Bitboard::mask(P54), Bitboard::mask(P55).up());
	ASSERT_EQ(Bitboard::mask(P56), Bitboard::mask(P55).down());
	ASSERT_EQ(Bitboard::mask(P65), Bitboard::mask(P55).left());
	ASSERT_EQ(Bitboard::mask(P45), Bitboard::mask(P55).right());
	ASSERT_EQ(Bitboard::mask(P45), Bitboard::mask(P35).left());

	ASSERT_EQ(Bitboard::mask(P53), Bitboard::mask(P55).up(2));
	ASSERT_EQ(Bitboard::mask(P57), Bitboard::mask(P55).down(2));
	ASSERT_EQ(Bitboard::mask(P75), Bitboard::mask(P55).left(2));
	ASSERT_EQ(Bitboard::mask(P35), Bitboard::mask(P55).right(2));
	ASSERT_EQ(Bitboard::mask(P55), Bitboard::mask(P35).left(2));

	ASSERT_EQ(Bitboard::mask(P25), Bitboard::mask(P85).right(6));
	ASSERT_EQ(Bitboard::mask(P85), Bitboard::mask(P25).left(6));
}

TEST(BitboardTest, testIterate) {
	Bitboard bb;
	Position pos;
	bb.init();
	bb.set(P95);
	bb.set(P41);
	bb.set(P48);
	bb.set(P27);

	ASSERT_EQ(P95, bb.getFirst());
	ASSERT_EQ(P27, bb.getLast());
	pos = bb.pickFirst();
	ASSERT_EQ(P95, pos);

	ASSERT_EQ(P41, bb.getFirst());
	ASSERT_EQ(P27, bb.getLast());
	pos = bb.pickFirst();
	ASSERT_EQ(P41, pos);

	ASSERT_EQ(P48, bb.getFirst());
	ASSERT_EQ(P27, bb.getLast());
	pos = bb.pickFirst();
	ASSERT_EQ(P48, pos);

	ASSERT_EQ(P27, bb.getFirst());
	ASSERT_EQ(P27, bb.getLast());
	pos = bb.pickFirst();
	ASSERT_EQ(P27, pos);

	ASSERT_EQ(Position::Invalid, bb.getFirst());
	ASSERT_EQ(Position::Invalid, bb.getLast());
	pos = bb.pickFirst();
	ASSERT(pos.isInvalid());
}

#endif // !defined(NDEBUG)
