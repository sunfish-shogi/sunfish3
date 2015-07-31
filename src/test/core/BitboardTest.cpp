/* BitboardTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "core/board/Bitboard.h"

using namespace sunfish;

TEST(BitboardTest, test) {
  {
    Bitboard bb;
    bb.init();
    ASSERT_EQ(Bitboard(0x00ll, 0x00ll), bb);
  }

  {
    Bitboard bb;
    bb.init();
    bb.set(P55);
    ASSERT_EQ(Bitboard(P55), bb);
    ASSERT(bb.check(P55));
  }

  {
    Bitboard bb;
    bb.init();
    bb.set(P91);
    ASSERT_EQ(Bitboard(P91), bb);
    ASSERT(bb.check(P91));
  }

  {
    Bitboard bb;
    bb.init();
    bb.set(P19);
    ASSERT_EQ(Bitboard(P19), bb);
    ASSERT(bb.check(P19));
  }

  {
    Bitboard bb;
    bb.init();
    bb.set(P38);
    ASSERT_EQ(Bitboard(P38), bb);
    ASSERT(bb.check(P38));
  }

  {
    Bitboard bb;
    bb.init();
    bb.set(P84);
    ASSERT_EQ(Bitboard(P84), bb);
    ASSERT(bb.check(P84));
  }

  {
    Bitboard bb;
    bb.init();
    ASSERT_EQ(Bitboard(P97), bb.copyWithSet(P97));
    ASSERT_EQ(Bitboard(P77), bb.copyWithSet(P77));
    ASSERT_EQ(Bitboard(P48), bb.copyWithSet(P48));
    ASSERT_EQ(Bitboard(P12), bb.copyWithSet(P12));
  }

  {
    ASSERT_EQ(Bitboard::file(1).toString2D(),
      "000000001\n"
      "000000001\n"
      "000000001\n"
      "000000001\n"
      "000000001\n"
      "000000001\n"
      "000000001\n"
      "000000001\n"
      "000000001\n");
    ASSERT_EQ(Bitboard::file(2).toString2D(),
      "000000010\n"
      "000000010\n"
      "000000010\n"
      "000000010\n"
      "000000010\n"
      "000000010\n"
      "000000010\n"
      "000000010\n"
      "000000010\n");
    ASSERT_EQ(Bitboard::file(3).toString2D(),
      "000000100\n"
      "000000100\n"
      "000000100\n"
      "000000100\n"
      "000000100\n"
      "000000100\n"
      "000000100\n"
      "000000100\n"
      "000000100\n");
    ASSERT_EQ(Bitboard::file(4).toString2D(),
      "000001000\n"
      "000001000\n"
      "000001000\n"
      "000001000\n"
      "000001000\n"
      "000001000\n"
      "000001000\n"
      "000001000\n"
      "000001000\n");
    ASSERT_EQ(Bitboard::file(5).toString2D(),
      "000010000\n"
      "000010000\n"
      "000010000\n"
      "000010000\n"
      "000010000\n"
      "000010000\n"
      "000010000\n"
      "000010000\n"
      "000010000\n");
    ASSERT_EQ(Bitboard::file(6).toString2D(),
      "000100000\n"
      "000100000\n"
      "000100000\n"
      "000100000\n"
      "000100000\n"
      "000100000\n"
      "000100000\n"
      "000100000\n"
      "000100000\n");
    ASSERT_EQ(Bitboard::file(7).toString2D(),
      "001000000\n"
      "001000000\n"
      "001000000\n"
      "001000000\n"
      "001000000\n"
      "001000000\n"
      "001000000\n"
      "001000000\n"
      "001000000\n");
    ASSERT_EQ(Bitboard::file(8).toString2D(),
      "010000000\n"
      "010000000\n"
      "010000000\n"
      "010000000\n"
      "010000000\n"
      "010000000\n"
      "010000000\n"
      "010000000\n"
      "010000000\n");
    ASSERT_EQ(Bitboard::file(9).toString2D(),
      "100000000\n"
      "100000000\n"
      "100000000\n"
      "100000000\n"
      "100000000\n"
      "100000000\n"
      "100000000\n"
      "100000000\n"
      "100000000\n");
    ASSERT_EQ(Bitboard::notFile(1).toString2D(),
      "111111110\n"
      "111111110\n"
      "111111110\n"
      "111111110\n"
      "111111110\n"
      "111111110\n"
      "111111110\n"
      "111111110\n"
      "111111110\n");
    ASSERT_EQ(Bitboard::notFile(2).toString2D(),
      "111111101\n"
      "111111101\n"
      "111111101\n"
      "111111101\n"
      "111111101\n"
      "111111101\n"
      "111111101\n"
      "111111101\n"
      "111111101\n");
    ASSERT_EQ(Bitboard::notFile(3).toString2D(),
      "111111011\n"
      "111111011\n"
      "111111011\n"
      "111111011\n"
      "111111011\n"
      "111111011\n"
      "111111011\n"
      "111111011\n"
      "111111011\n");
    ASSERT_EQ(Bitboard::notFile(4).toString2D(),
      "111110111\n"
      "111110111\n"
      "111110111\n"
      "111110111\n"
      "111110111\n"
      "111110111\n"
      "111110111\n"
      "111110111\n"
      "111110111\n");
    ASSERT_EQ(Bitboard::notFile(5).toString2D(),
      "111101111\n"
      "111101111\n"
      "111101111\n"
      "111101111\n"
      "111101111\n"
      "111101111\n"
      "111101111\n"
      "111101111\n"
      "111101111\n");
    ASSERT_EQ(Bitboard::notFile(6).toString2D(),
      "111011111\n"
      "111011111\n"
      "111011111\n"
      "111011111\n"
      "111011111\n"
      "111011111\n"
      "111011111\n"
      "111011111\n"
      "111011111\n");
    ASSERT_EQ(Bitboard::notFile(7).toString2D(),
      "110111111\n"
      "110111111\n"
      "110111111\n"
      "110111111\n"
      "110111111\n"
      "110111111\n"
      "110111111\n"
      "110111111\n"
      "110111111\n");
    ASSERT_EQ(Bitboard::notFile(8).toString2D(),
      "101111111\n"
      "101111111\n"
      "101111111\n"
      "101111111\n"
      "101111111\n"
      "101111111\n"
      "101111111\n"
      "101111111\n"
      "101111111\n");
    ASSERT_EQ(Bitboard::notFile(9).toString2D(),
      "011111111\n"
      "011111111\n"
      "011111111\n"
      "011111111\n"
      "011111111\n"
      "011111111\n"
      "011111111\n"
      "011111111\n"
      "011111111\n");
  }

  {
    ASSERT_EQ(BPawnMovable.toString2D(),
      "000000000\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n");
    ASSERT_EQ(BLanceMovable.toString2D(),
      "000000000\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n");
    ASSERT_EQ(BKnightMovable.toString2D(),
      "000000000\n"
      "000000000\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n");
    ASSERT_EQ(WPawnMovable.toString2D(),
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "000000000\n");
    ASSERT_EQ(WLanceMovable.toString2D(),
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "000000000\n");
    ASSERT_EQ(WKnightMovable.toString2D(),
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "000000000\n"
      "000000000\n");
    ASSERT_EQ(BPromotable.toString2D(),
      "111111111\n"
      "111111111\n"
      "111111111\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n");
    ASSERT_EQ(WPromotable.toString2D(),
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "111111111\n"
      "111111111\n"
      "111111111\n");
    ASSERT_EQ(BPromotable2.toString2D(),
      "111111111\n"
      "111111111\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n");
    ASSERT_EQ(WPromotable2.toString2D(),
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "000000000\n"
      "111111111\n"
      "111111111\n");
  }
}

TEST(BitboardTest, testShift) {
  ASSERT_EQ(Bitboard(P54), Bitboard(P55).up());
  ASSERT_EQ(Bitboard(P56), Bitboard(P55).down());
  ASSERT_EQ(Bitboard(P65), Bitboard(P55).left());
  ASSERT_EQ(Bitboard(P45), Bitboard(P55).right());
  ASSERT_EQ(Bitboard(P45), Bitboard(P35).left());

  ASSERT_EQ(Bitboard(P53), Bitboard(P55).up(2));
  ASSERT_EQ(Bitboard(P57), Bitboard(P55).down(2));
  ASSERT_EQ(Bitboard(P75), Bitboard(P55).left(2));
  ASSERT_EQ(Bitboard(P35), Bitboard(P55).right(2));
  ASSERT_EQ(Bitboard(P55), Bitboard(P35).left(2));

  ASSERT_EQ(Bitboard(P25), Bitboard(P85).right(6));
  ASSERT_EQ(Bitboard(P85), Bitboard(P25).left(6));
}

TEST(BitboardTest, testIterate) {
  Bitboard bb;
  Square sq;
  bb.init();
  bb.set(P95);
  bb.set(P41);
  bb.set(P48);
  bb.set(P27);

  ASSERT_EQ(P95, bb.getFirst());
  ASSERT_EQ(P27, bb.getLast());
  sq = bb.pickFirst();
  ASSERT_EQ(P95, sq.index());

  ASSERT_EQ(P41, bb.getFirst());
  ASSERT_EQ(P27, bb.getLast());
  sq = bb.pickFirst();
  ASSERT_EQ(P41, sq.index());

  ASSERT_EQ(P48, bb.getFirst());
  ASSERT_EQ(P27, bb.getLast());
  sq = bb.pickFirst();
  ASSERT_EQ(P48, sq.index());

  ASSERT_EQ(P27, bb.getFirst());
  ASSERT_EQ(P27, bb.getLast());
  sq = bb.pickFirst();
  ASSERT_EQ(P27, sq.index());

  ASSERT_EQ(Square::Invalid, bb.getFirst());
  ASSERT_EQ(Square::Invalid, bb.getLast());
  sq = bb.pickFirst();
  ASSERT(sq.isInvalid());
}

#endif // !defined(NDEBUG)
