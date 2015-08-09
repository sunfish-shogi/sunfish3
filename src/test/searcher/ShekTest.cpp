/* SeeTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "searcher/shek/ShekTable.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

TEST(ShekTest, test) {
  {
    std::string src =
"P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
"P2 * -HI *  *  *  * -KI-KA * \n"
"P3-FU * -FU-FU-FU-FU-FU-FU-FU\n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
"P8 * +KA+KI *  *  *  * +HI * \n"
"P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
"P+00FU\n"
"P-00FU\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    ShekTable table;

    Move moves[] = {
      { Piece::Pawn, S24 },
      { Piece::Pawn, S23, S24, false },
      { Piece::Rook, S28, S24, false },
      { Piece::Pawn, S23 },
    };

    for (auto& move : moves) {
      table.set(board);
      bool ok = board.makeMove(move);
      ASSERT(ok);
      ShekStat stat = table.check(board);
      ASSERT_EQ((int)ShekStat::None, (int)stat);
    }

    table.set(board);
    Move move = { Piece::Rook, S24, S28, false };
    bool ok = board.makeMove(move);
    ASSERT(ok);
    ShekStat stat = table.check(board);
    ASSERT_EQ((int)ShekStat::Superior, (int)stat);

    Move moves2[] = {
      { Piece::Pawn, S86 },
      { Piece::Pawn, S87, S86, false },
      { Piece::Rook, S82, S86, false },
      { Piece::Pawn, S87 },
    };

    for (auto& move : moves2) {
      table.set(board);
      bool ok = board.makeMove(move);
      ASSERT(ok);
      ShekStat stat = table.check(board);
      ASSERT_EQ((int)ShekStat::None, (int)stat);
    }

    table.set(board);
    move = { Piece::Rook, S86, S82, false };
    ok = board.makeMove(move);
    ASSERT(ok);
    stat = table.check(board);
    ASSERT_EQ((int)ShekStat::Equal, (int)stat);
  }

  {
    std::string src =
"P1-KY-KE-GI-KI-OU *  * -KE-KY\n"
"P2 * -HI *  *  *  * -KI *  * \n"
"P3-FU * -FU-FU-FU-FU-GI * -FU\n"
"P4 *  *  *  *  * -KA-FU *  * \n"
"P5 *  *  *  *  * +FU *  *  * \n"
"P6 *  * +FU+FU *  *  * -FU * \n"
"P7+FU+FU * +GI+FU * +FU * +FU\n"
"P8 * +KA+KI *  * +GI * +HI * \n"
"P9+KY+KE *  * +OU+KI * +KE+KY\n"
"P+00FU\n"
"P-00FU\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Move moves[] = {
      { Piece::Pawn, S26, S27, true },
      { Piece::Rook, S28, S27, false },
      { Piece::Pawn, S26 },
    };

    ShekTable table;
    for (auto& move : moves) {
      table.set(board);
      bool ok = board.makeMove(move);
      ASSERT(ok);
      ShekStat stat = table.check(board);
      ASSERT_EQ((int)ShekStat::None, (int)stat);
    }

    table.set(board);
    Move move = { Piece::Rook, S27, S28, false };
    bool ok = board.makeMove(move);
    ASSERT(ok);
    ShekStat stat = table.check(board);
    ASSERT_EQ((int)ShekStat::Inferior, (int)stat);

    table.set(board);
    move = { Piece::Pawn, S26, S27, true };
    ok = board.makeMove(move);
    ASSERT(ok);
    stat = table.check(board);
    ASSERT_EQ((int)ShekStat::Superior, (int)stat);
  }
}

#endif // !defined(NDEBUG)
