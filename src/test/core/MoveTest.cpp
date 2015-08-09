/* MoveTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "core/move/Move.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

TEST(MoveTest, test) {
  {
    Move move(Piece::BPawn, S57, S56, false);
    ASSERT_EQ(S57, move.from().index());
    ASSERT_EQ(S56, move.to().index());
    ASSERT_EQ(false, move.promote());
    ASSERT_EQ(Piece::Pawn, move.piece());
    ASSERT_EQ(false, move.isHand());
  }

  {
    Move move(Piece::WPawn, S53, S54, false);
    ASSERT_EQ(S53, move.from().index());
    ASSERT_EQ(S54, move.to().index());
    ASSERT_EQ(false, move.promote());
    ASSERT_EQ(Piece::Pawn, move.piece());
    ASSERT_EQ(false, move.isHand());
  }

  {
    Move move(Piece::BPawn, S54, S53, true);
    ASSERT_EQ(S54, move.from().index());
    ASSERT_EQ(S53, move.to().index());
    ASSERT_EQ(true, move.promote());
    ASSERT_EQ(Piece::Pawn, move.piece());
    ASSERT_EQ(false, move.isHand());
  }

  {
    Move move(Piece::BPawn, S55);
    ASSERT_EQ(S55, move.to().index());
    ASSERT_EQ(false, move.promote());
    ASSERT_EQ(Piece::Pawn, move.piece());
    ASSERT_EQ(true, move.isHand());
  }
}

TEST(MoveTest, testSerialization) {
  {
    Move in(Piece::Pawn, S77, S76, false);
    uint32_t data = Move::serialize(in);
    Move out = Move::deserialize(data);
    ASSERT_EQ(in, out);
  }

  {
    Move in(Piece::Knight, S33, S41, true);
    uint32_t data = Move::serialize(in);
    Move out = Move::deserialize(data);
    ASSERT_EQ(in, out);
  }

  {
    Move in(Piece::Rook, S45, S75, false, Piece::Pawn);
    uint32_t data = Move::serialize(in);
    Move out = Move::deserialize(data);
    ASSERT_EQ(in, out);
  }

  {
    Move in(Piece::Pawn, S54);
    uint32_t data = Move::serialize(in);
    Move out = Move::deserialize(data);
    ASSERT_EQ(in, out);
  }
}

TEST(MoveTest, testSerialization16) {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  * +KE *  * \n\
P4 *  *  *  * +FU *  *  *  * \n\
P5 *  * -FU *  * +HI *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  * +FU *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
  std::istringstream iss(src);
  Board board;
  CsaReader::readBoard(iss, board);

  {
    Move in(Piece::Pawn, S77, S76, false);
    uint16_t data = Move::serialize16(in);
    Move out = Move::deserialize16(data, board);
    ASSERT_EQ(in, out);
  }

  {
    Move in(Piece::Knight, S33, S41, true);
    uint16_t data = Move::serialize16(in);
    Move out = Move::deserialize16(data, board);
    ASSERT_EQ(in, out);
  }

  {
    Move in(Piece::Rook, S45, S75, false, Piece::Pawn);
    uint16_t data = Move::serialize16(in);
    Move out = Move::deserialize16(data, board);
    ASSERT_EQ(in, out);
  }

  {
    Move in(Piece::Pawn, S54);
    uint16_t data = Move::serialize16(in);
    Move out = Move::deserialize16(data, board);
    ASSERT_EQ(in, out);
  }

  {
    Move in = Move::empty();
    uint16_t data = Move::serialize16(in);
    Move out = Move::deserialize16(data, board);
    ASSERT_EQ(in, out);
  }
}

#endif // !defined(NDEBUG)
