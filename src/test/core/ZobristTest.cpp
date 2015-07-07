/* ZobristTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

TEST(ZobristTest, boardHashTest) {

  {
    std::string src = "\
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n\
P2 * -HI *  *  *  *  * -KA * \n\
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n\
P8 * +KA *  *  *  *  * +HI * \n\
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board, board2;
    CsaReader::readBoard(iss, board);

    Move move1(Piece::Pawn, P77, P76, false);
    Move move2(Piece::Pawn, P33, P34, false);

    // make move
    board.makeMove(move1);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // make move
    board.makeMove(move2);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // unmake move
    board.unmakeMove(move2);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // unmake move
    board.unmakeMove(move1);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

  }

  {
    std::string src = "\
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n\
P2 * -HI *  *  *  *  * -KA * \n\
P3-FU-FU-FU-FU-FU-FU * -FU-FU\n\
P4 *  *  *  *  *  * -FU *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  * +FU *  *  *  *  *  * \n\
P7+FU+FU * +FU+FU+FU+FU+FU+FU\n\
P8 * +KA *  *  *  *  * +HI * \n\
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board, board2;
    CsaReader::readBoard(iss, board);

    Move move1(Piece::Bishop, P88, P22, true);
    Move move2(Piece::Silver, P31, P22, false);
    Move move3(Piece::Bishop, P45);
    Move move4(Piece::Bishop, P85);

    // make move
    board.makeMove(move1);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // make move
    board.makeMove(move2);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // make move
    board.makeMove(move3);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // make move
    board.makeMove(move4);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // unmake move
    board.unmakeMove(move4);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // unmake move
    board.unmakeMove(move3);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // unmake move
    board.unmakeMove(move2);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

    // unmake move
    board.unmakeMove(move1);

    board2 = board;
    board2.refreshHash();

    ASSERT_EQ(board.getHash(), board2.getHash());

  }

}

#endif // !defined(NDEBUG)
