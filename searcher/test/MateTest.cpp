/* MateTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../mate/Mate.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

TEST(MateTest, testMate1Ply) {

  {
    // 頭金(詰)
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 頭金(詰)
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  * -FU *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-00KI\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 頭銀(不詰)
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00GI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 頭銀(不詰)
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  * -FU *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-00GI\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  * +FU * -FU *  *  * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00HI\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  * +FU *  *  *  *  * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00HI\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  * +HI+FU * -FU *  *  * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  * +RY+FU * -FU *  *  * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  * +TO+FU * -FU *  *  * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  * +NG * \n\
P4 *  *  *  *  *  *  *  * -OU\n\
P5 *  *  *  *  *  *  * +FU-FU\n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  * -FU+FU\n\
P6 *  *  *  *  *  *  *  * +OU\n\
P7 *  *  *  *  *  *  * -NG * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+00FU\n\
P-00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  * +NG * \n\
P4 *  *  *  *  *  *  *  * -OU\n\
P5 *  *  *  *  *  *  * +FU * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  * -FU * \n\
P6 *  *  *  *  *  *  *  * +OU\n\
P7 *  *  *  *  *  *  * -NG * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+00FU\n\
P-00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  * -HI * \n\
P3 *  *  *  * +FU *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KI\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  * -FU *  *  *  * \n\
P8 * +HI *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-00KI\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  * +TO *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  * -TO *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  * -FU-OU-FU *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +FU * +FU *  *  * \n\
P4 *  *  *  * -FU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +HI *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  * -FU-OU-FU *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +FU * +FU *  *  * \n\
P4 *  *  *  * -FU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +HI *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  * -FU-OU-FU *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +FU * +KI *  *  * \n\
P4 *  *  *  * -FU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +HI *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 詰
    std::string src = "\
P1 *  *  * -FU-OU-FU *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +FU * +KI *  *  * \n\
P4 *  *  *  * -FU * +HI *  * \n\
P5 *  *  *  * -KA *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 不詰
    std::string src = "\
P1 *  *  * -FU-OU-FU *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +FU * +KI *  *  * \n\
P4 *  *  *  * -FU * +HI *  * \n\
P5 *  *  *  * -UM *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰
    std::string src = "\
P1-KY * +KI * -OU *  * -KE * \n\
P2 *  *  *  * -HI *  *  *  * \n\
P3-FU * -KE+TO *  *  * -FU * \n\
P4 * +FU-FU * -FU * -GI *  * \n\
P5+FU * +GI *  * -FU *  * -FU\n\
P6 *  * +HI * +FU * +FU * +GI\n\
P7 *  * +KE * -KA+FU+KI+FU * \n\
P8 *  *  *  *  *  *  *  * +KY\n\
P9+KY *  *  *  *  * +KI+KE+OU\n\
P+00FU00FU00FU00GI\n\
P-00FU00FU00KY00KI00KI00KA\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰
    std::string src = "\
P1-KY * -KI *  * -OU-KA+HI-KY\n\
P2 *  *  * -GI *  *  * -GI * \n\
P3-FU *  * -FU * +KE-KE-FU-FU\n\
P4 *  *  *  *  *  * -KE *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  * -FU *  * -FU *  *  * \n\
P7+FU+FU-KI-KA *  * +FU * +FU\n\
P8 *  *  * -GI * +OU+GI *  * \n\
P9+KY *  *  *  *  *  * +HI+KY\n\
P+00KI00KI00FU00FU00FU00FU00FU00FU\n\
P-00KE00FU00FU\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰(53角 中合)
    std::string src = "\
P1 *  *  * -FU-OU-FU *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +KI *  *  *  *  * \n\
P4 *  *  * +KA-FU *  *  *  * \n\
P5 *  *  *  *  *  * -KA *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +KY *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰(53歩 中合)
    std::string src = "\
P1 *  *  * -FU-OU-FU *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * +KI *  *  *  *  * \n\
P4 *  *  * +KA-FU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +KY *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 不詰(53桂 移動合)
    std::string src = "\
P1 *  *  * -FU-OU-KE *  *  * \n\
P2 *  *  *  *  * -FU *  *  * \n\
P3 *  *  * +KI *  *  *  *  * \n\
P4 *  *  *  * -FU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +KY *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

  {
    // 突き歩詰
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  * -KY\n\
P3 *  *  *  *  *  *  *  * -OU\n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  * +RY+FU\n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(true, mate);
  }

  {
    // 打ち歩詰(不詰)
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  * -KY\n\
P3 *  *  *  *  *  *  *  * -OU\n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  * +RY * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00FU\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    bool mate = Mate::mate1Ply(board);
    ASSERT_EQ(false, mate);
  }

}

TEST(MateTest, testMate3Ply) {
  Tree tree;
  Evaluator eval(Evaluator::InitType::Zero);

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  * -OU *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  * +FU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KI00KI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(true, mate);
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  * -OU *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  * +FU *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KI00GI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(false, mate);
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  * -FU *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +OU *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-00KI00KI\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(true, mate);
  }

  {
    std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  * -FU *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  * +OU *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-00KI00GI\n\
-\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(false, mate);
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  * -GI+KA\n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  * -OU * \n\
P4 *  *  *  *  *  * +FU *  * \n\
P5 *  *  *  *  *  *  * -FU-FU\n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(false, mate);
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  * -GI+KA\n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  * -OU * \n\
P4 *  *  *  *  *  * +FU *  * \n\
P5 *  *  *  *  *  *  * -FU-FU\n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(false, mate);
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  * -GI+KA\n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  * -OU * \n\
P4 *  *  *  *  *  * +FU * -FU\n\
P5 *  *  *  *  *  *  * -FU * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(true, mate);
  }

  {
    std::string src = "\
P1 *  *  * -FU-OU * -KI *  * \n\
P2 *  *  * -KY *  *  *  *  * \n\
P3 *  *  * +KA * -FU *  *  * \n\
P4 *  *  *  *  *  * +FU *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * -RY *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00HI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(true, mate);
  }

  {
    std::string src = "\
P1 *  *  * -FU-OU * -KI *  * \n\
P2 *  *  * -KY *  *  *  *  * \n\
P3 *  *  * +KA-FU-FU *  *  * \n\
P4 *  *  *  *  *  * +FU *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * -RY *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00HI\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(false, mate);
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  * -KI * \n\
P3 *  *  *  *  * +HI *  *  * \n\
P4 *  *  *  *  *  * -FU-OU-KY\n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  * +GI * -FU\n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KY00KA\n\
P-\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(true, mate);
  }

  {
    std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  * -KI * \n\
P3 *  *  *  *  * +HI *  *  * \n\
P4 *  *  *  *  *  * -FU-OU-KY\n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  * +GI * -FU\n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+00KY00KA\n\
P-00FU\n\
+\n\
";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);
    tree.init(0, board, eval, std::vector<Move>());

    bool mate = Mate::mate3Ply(tree);
    ASSERT_EQ(false, mate);
  }

}

#endif // !defined(NDEBUG)
