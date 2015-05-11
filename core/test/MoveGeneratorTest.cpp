/* MoveGeneratorTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../move/MoveGenerator.h"
#include "../record/CsaReader.h"

using namespace sunfish;

#if 0
namespace {
  void debugPrint(const Moves& moves) {
    std::ostringstream oss;
    for (auto ite = moves.begin(); ite != moves.end(); ite++) {
      oss << ite->toString() << ", ";
    }
    Loggers::debug << oss.str();
  }
}
#endif

TEST(MoveGeneratorTest, test) {
  {
    std::string src =
"P1-KY-KE-GI-KI * -KI * -KE-KY\n"
"P2 * -OU *  *  *  *  * -HI * \n"
"P3-FU-FU-FU-FU-FU * -KA * -FU\n"
"P4 *  *  *  * -GI-FU-FU+KA * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6+FU * +FU * +FU *  *  *  * \n"
"P7 * +FU+GI+FU * +FU+FU * +FU\n"
"P8 *  * +OU * +KI *  * +HI * \n"
"P9+KY+KE * +KI *  * +GI+KE+KY\n"
"P+00FU\n"
"P-00FU\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generate(board, moves);
    ASSERT_EQ(46, moves.size());
  }
}

TEST(MoveGeneratorTest, testNoCap) {
  {
    std::string src =
"P1-KY-KE-GI-KI * -KI * -KE-KY\n"
"P2 * -OU *  *  *  *  * -HI * \n"
"P3-FU-FU-FU-FU-FU * -KA * -FU\n"
"P4 *  *  *  * -GI-FU-FU+KA * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6+FU * +FU * +FU *  *  *  * \n"
"P7 * +FU+GI+FU * +FU+FU * +FU\n"
"P8 *  * +OU * +KI *  * +HI * \n"
"P9+KY+KE * +KI *  * +GI+KE+KY\n"
"P+00FU\n"
"P-00FU\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateNoCap(board, moves);
    ASSERT_EQ(40, moves.size());
  }

  {
    // 先手の駒 銀から飛車まで
    std::string src =
"P1 *  *  *  * -OU+KA *  *  * \n"
"P2+KI *  *  *  *  *  * +HI * \n"
"P3 *  * +GI *  *  *  *  *  * \n"
"P4+GI *  *  *  *  *  *  *  * \n"
"P5 *  * +KA *  *  *  *  *  * \n"
"P6 *  *  *  *  *  * +HI *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  * +OU * -TO *  * \n"
"P+\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateNoCap(board, moves);
    ASSERT_EQ(43, moves.size());
  }
}

TEST(MoveGeneratorTest, testCap) {
  {
    // 先手の駒 歩から桂まで
    std::string src =
"P1 *  *  * -OU *  *  *  *  * \n"
"P2 * +OU *  * -FU *  *  *  * \n"
"P3 *  *  * +FU *  *  *  *  * \n"
"P4 *  * +FU *  *  * -KI+KE * \n"
"P5 * +FU *  *  *  * +KE *  * \n"
"P6 *  *  *  * +KY+KY * +KE * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCap(board, moves);
    ASSERT_EQ(12, moves.size());
  }

  {
    // 先手の駒 銀から飛車まで
    std::string src =
"P1 *  *  *  * -OU+KA *  *  * \n"
"P2+KI *  *  *  *  *  * +HI * \n"
"P3 *  * +GI *  *  *  *  *  * \n"
"P4+GI *  *  *  *  *  *  *  * \n"
"P5 *  * +KA *  *  *  *  *  * \n"
"P6 *  *  *  *  *  * +HI *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  * +OU * -TO *  * \n"
"P+\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCap(board, moves);
    ASSERT_EQ(32, moves.size());
  }

  {
    // 後手の駒 歩から桂まで
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4-FU *  *  * -KY-KY * -KE * \n"
"P5+UM-FU *  *  *  * -KE *  * \n"
"P6 *  * -FU *  *  *  * -KE * \n"
"P7 *  *  * -FU *  *  *  *  * \n"
"P8 * -OU *  * +FU *  *  *  * \n"
"P9 * +KE * +OU *  *  *  *  * \n"
"P+\n"
"P-\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCap(board, moves);
    ASSERT_EQ(13, moves.size());
  }

  {
    // 後手の駒 銀から飛車まで
    std::string src =
"P1 *  *  *  * -OU *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  * -HI * +KY\n"
"P5 *  * -KA *  *  *  *  *  * \n"
"P6-GI *  *  *  *  *  *  *  * \n"
"P7 *  * -GI *  *  *  *  *  * \n"
"P8-KI *  *  *  *  *  * -HI * \n"
"P9 * +KE *  * +OU-KA *  *  * \n"
"P+\n"
"P-\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCap(board, moves);
    ASSERT_EQ(32, moves.size());
  }
}

TEST(MoveGeneratorTest, testDrop) {
  {
    // 先手の手
    std::string src =
"P1+TO-KE *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  * -OU *  *  * -FU * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  * +FU * -KI *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  * +FU *  *  *  * +FU * \n"
"P8 *  * +OU *  *  *  *  *  * \n"
"P9 *  *  *  * -KY * -RY *  * \n"
"P+00FU00FU00KY00KE00KA\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // FUx45 KYx63 KEx54 KAx70
    Moves moves;
    MoveGenerator::generateDrop(board, moves);
    ASSERT_EQ(45+63+54+70, moves.size());
  }

  {
    // 後手の手
    std::string src =
"P1 *  *  *  * +KY * +RY *  * \n"
"P2 *  * -OU *  *  *  *  *  * \n"
"P3 *  * -FU *  *  *  * -FU * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  * -FU * +KI *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  * +OU *  *  * +FU * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9-TO+KE *  *  *  *  *  *  * \n"
"P+\n"
"P-00FU00FU00KY00KE00KA\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // FUx45 KYx63 KEx54 KAx70
    Moves moves;
    MoveGenerator::generateDrop(board, moves);
    ASSERT_EQ(45+63+54+70, moves.size());
  }
}

TEST(MoveGeneratorTest, testEvasion) {
  {
    // 歩による王手
    std::string src =
"P1 *  *  *  *  * -OU * +KA * \n"
"P2 *  *  * +RY *  *  *  *  * \n"
"P3 * +UM *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  * +KI-FU *  *  *  *  * \n"
"P6 *  *  * +OU+GI *  *  *  * \n"
"P7 *  *  *  * +KE *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  * +KY *  *  *  *  * \n"
"P+00FU\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 玉x5 その他x6
    Moves moves;
    MoveGenerator::generateEvasion(board, moves);
    ASSERT_EQ(11, moves.size());
  }

  {
    // 香車による王手
    std::string src =
"P1 *  *  *  * -OU *  *  *  * \n"
"P2 *  *  * -KY *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  * +KI *  *  *  * \n"
"P7 *  *  *  * +FU *  *  *  * \n"
"P8 *  * -GI+OU *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+00FU\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 65金 66金 玉x7 合駒x5
    Moves moves;
    MoveGenerator::generateEvasion(board, moves);
    ASSERT_EQ(14, moves.size());
  }

  {
    // 角による王手
    std::string src =
"P1 *  *  *  * -OU *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  * -KA * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  * +RY *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 * -FU+FU *  *  *  *  *  * \n"
"P8 *  * +OU *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  * +KY * \n"
"P+00FU00FU00KA\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 45竜 56竜 67竜 23香不成 23香成 玉x7 合駒2種x4
    Moves moves;
    MoveGenerator::generateEvasion(board, moves);
    ASSERT_EQ(20, moves.size());
  }

  {
    // 竜による王手
    std::string src =
"P1 *  *  *  * -OU *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  * +UM *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  * +GI *  *  *  *  *  * \n"
"P8 *  * +OU *  *  * -RY *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+00FU00FU00KA\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 38馬 68銀 玉x7 合駒2種x3
    Moves moves;
    MoveGenerator::generateEvasion(board, moves);
    ASSERT_EQ(15, moves.size());
  }

  {
    // 両王手
    std::string src =
"P1-KY *  *  * -OU *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 * +FU+FU-UM *  *  *  *  * \n"
"P7 * +GI *  *  *  *  *  *  * \n"
"P8 * +OU *  *  *  * -HI *  * \n"
"P9+KY+KE * -GI *  *  *  *  * \n"
"P+00FU00FU00KI00KE\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 玉x5
    Moves moves;
    MoveGenerator::generateEvasion(board, moves);
    ASSERT_EQ(5, moves.size());
  }

  {
    // 跳び駒の近接からの王手
    std::string src =
"P1 *  *  * -KI *  *  *  *  * \n"
"P2 *  *  *  * -OU+RY *  *  * \n"
"P3 *  *  * -FU *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  * +OU *  *  *  * \n"
"P+\n"
"P-\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 玉x6
    Moves moves;
    MoveGenerator::generateEvasion(board, moves);
    ASSERT_EQ(6, moves.size());
  }

  {
    // 跳び駒の近接からの王手(2)
    std::string src =
"P1-KY-KE-GI-KI *  *  * -KE-KY\n"
"P2 * -OU *  * -KI-HI *  *  * \n"
"P3-FU-FU-FU-FU-FU *  * -FU-FU\n"
"P4 *  *  *  * -GI * -FU+FU * \n"
"P5 *  *  *  *  * -FU *  *  * \n"
"P6+FU * +FU * +FU *  *  *  * \n"
"P7 * +FU-UM+FU * +FU+FU * +FU\n"
"P8 * +KA * +OU+KI *  * +HI * \n"
"P9+KY+KE * +KI *  * +GI+KE+KY\n"
"P+\n"
"P-00GI\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 角 桂 玉x5
    Moves moves;
    MoveGenerator::generateEvasion(board, moves);
    ASSERT_EQ(7, moves.size());
  }

}

TEST(MoveGeneratorTest, testCheck) {
  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  * -FU *  *  *  * \n"
"P4 *  *  * +GI * -OU *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  * +GI *  *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  * +OU *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+00FU00GI\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(9, moves.size());
    ASSERT_EQ(Move(Piece::Silver, P55), moves[0]);
    ASSERT_EQ(Move(Piece::Silver, P45), moves[1]);
    ASSERT_EQ(Move(Piece::Silver, P33), moves[2]);
    ASSERT_EQ(Move(Piece::Silver, P35), moves[3]);
    ASSERT_EQ(Move(Piece::Silver, P64, P53, false), moves[4]);
    ASSERT_EQ(Move(Piece::Silver, P64, P55, false), moves[5]);
    ASSERT_EQ(Move(Piece::Silver, P56, P55, false), moves[6]);
    ASSERT_EQ(Move(Piece::Silver, P56, P45, false), moves[7]);
    ASSERT_EQ(Move(Piece::Pawn, P45), moves[8]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  * -OU *  *  * \n"
"P4 *  *  *  *  * +KE *  *  * \n"
"P5 *  *  *  *  * +FU *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  * +OU *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(0, moves.size());
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  * +KI *  *  *  * \n"
"P4 *  *  *  *  * -OU *  *  * \n"
"P5 *  *  *  *  *  *  * +KI * \n"
"P6 *  *  *  *  * +FU *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  * +OU *  *  * +KE * \n"
"P9 *  *  *  *  * +KY+KY *  * \n"
"P+00FU00KE\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 45歩 56桂 36桂打 36桂 54金 43金 34金 35金
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(8, moves.size());
    ASSERT_EQ(Move(Piece::Gold, P53, P54, false), moves[0]);
    ASSERT_EQ(Move(Piece::Gold, P53, P43, false), moves[1]);
    ASSERT_EQ(Move(Piece::Gold, P25, P34, false), moves[2]);
    ASSERT_EQ(Move(Piece::Gold, P25, P35, false), moves[3]);
    ASSERT_EQ(Move(Piece::Knight, P56), moves[4]);
    ASSERT_EQ(Move(Piece::Knight, P36), moves[5]);
    ASSERT_EQ(Move(Piece::Knight, P28, P36, false), moves[6]);
    ASSERT_EQ(Move(Piece::Pawn, P46, P45, false), moves[7]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  * -FU-OU *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  * -FU *  *  * \n"
"P8 *  *  * +OU *  *  *  *  * \n"
"P9 *  *  *  *  * +KY+KY *  * \n"
"P+00KY00KI\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 45香 46香 47香 55金 43金 45金 34金 35金
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(8, moves.size());
    ASSERT_EQ(Move(Piece::Gold, P55), moves[0]);
    ASSERT_EQ(Move(Piece::Gold, P43), moves[1]);
    ASSERT_EQ(Move(Piece::Gold, P45), moves[2]);
    ASSERT_EQ(Move(Piece::Gold, P34), moves[3]);
    ASSERT_EQ(Move(Piece::Gold, P35), moves[4]);
    ASSERT_EQ(Move(Piece::Lance, P45), moves[5]);
    ASSERT_EQ(Move(Piece::Lance, P46), moves[6]);
    ASSERT_EQ(Move(Piece::Lance, P49, P47, false), moves[7]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  * -FU-OU *  *  * \n"
"P4 *  * +KA *  *  *  *  *  * \n"
"P5 *  * +KA *  *  *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  * -FU *  *  * \n"
"P8 *  *  * +OU *  *  *  *  * \n"
"P9 *  *  *  *  * +KY+KY *  * \n"
"P+\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 47香 33香成 53角成 52角成 65角
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(5, moves.size());
    ASSERT_EQ(Move(Piece::Lance, P49, P47, false), moves[0]);
    ASSERT_EQ(Move(Piece::Lance, P39, P33, true), moves[1]);
    ASSERT_EQ(Move(Piece::Bishop, P74, P65, false), moves[2]);
    ASSERT_EQ(Move(Piece::Bishop, P74, P52, true), moves[3]);
    ASSERT_EQ(Move(Piece::Bishop, P75, P53, true), moves[4]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  * -KE * \n"
"P2 *  *  *  * -GI *  *  *  * \n"
"P3 *  *  *  * +FU-OU *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  * +KA *  *  *  * +FU * \n"
"P6 *  * -FU *  *  * +HI * +HI\n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  * +OU *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+00KA\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 65角 54角 32角 34角 46飛 32飛成 33飛成 13飛成
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(8, moves.size());
    ASSERT_EQ(Move(Piece::Bishop, P65), moves[0]);
    ASSERT_EQ(Move(Piece::Bishop, P54), moves[1]);
    ASSERT_EQ(Move(Piece::Bishop, P32), moves[2]);
    ASSERT_EQ(Move(Piece::Bishop, P34), moves[3]);
    ASSERT_EQ(Move(Piece::Rook, P36, P46, false), moves[4]);
    ASSERT_EQ(Move(Piece::Rook, P36, P32, true), moves[5]);
    ASSERT_EQ(Move(Piece::Rook, P36, P33, true), moves[6]);
    ASSERT_EQ(Move(Piece::Rook, P16, P13, true), moves[7]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  * -FU *  * -OU * -FU * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  * +RY * -FU+UM *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  * +OU *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+00HI\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 63飛 53飛 41飛 42飛 44飛 33飛 53馬 44馬 34馬 25馬 63竜 54竜 45竜
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(13, moves.size());
    ASSERT_EQ(Move(Piece::Rook, P63), moves[0]);
    ASSERT_EQ(Move(Piece::Rook, P53), moves[1]);
    ASSERT_EQ(Move(Piece::Rook, P41), moves[2]);
    ASSERT_EQ(Move(Piece::Rook, P42), moves[3]);
    ASSERT_EQ(Move(Piece::Rook, P44), moves[4]);
    ASSERT_EQ(Move(Piece::Rook, P33), moves[5]);
    ASSERT_EQ(Move(Piece::Horse, P35, P53, false), moves[6]);
    ASSERT_EQ(Move(Piece::Horse, P35, P44, false), moves[7]);
    ASSERT_EQ(Move(Piece::Horse, P35, P34, false), moves[8]);
    ASSERT_EQ(Move(Piece::Horse, P35, P25, false), moves[9]);
    ASSERT_EQ(Move(Piece::Dragon, P65, P63, false), moves[10]);
    ASSERT_EQ(Move(Piece::Dragon, P65, P54, false), moves[11]);
    ASSERT_EQ(Move(Piece::Dragon, P65, P45, false), moves[12]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  * +NG *  *  *  * \n"
"P3 *  *  *  *  * -OU *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  * +TO+NY+NK *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  * +OU *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 54と 44と 54成香 44成香 34成香 44成桂 34成桂 53成銀 42成銀
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(9, moves.size());
    ASSERT_EQ(Move(Piece::ProSilver, P52, P53, false), moves[0]);
    ASSERT_EQ(Move(Piece::ProSilver, P52, P42, false), moves[1]);
    ASSERT_EQ(Move(Piece::Tokin, P55, P54, false), moves[2]);
    ASSERT_EQ(Move(Piece::Tokin, P55, P44, false), moves[3]);
    ASSERT_EQ(Move(Piece::ProLance, P45, P54, false), moves[4]);
    ASSERT_EQ(Move(Piece::ProLance, P45, P44, false), moves[5]);
    ASSERT_EQ(Move(Piece::ProLance, P45, P34, false), moves[6]);
    ASSERT_EQ(Move(Piece::ProKnight, P35, P44, false), moves[7]);
    ASSERT_EQ(Move(Piece::ProKnight, P35, P34, false), moves[8]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  * -OU *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  * -GI *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  * -GI * +OU *  *  * \n"
"P7 *  *  *  * +FU *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-00FU00GI\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(9, moves.size());
    ASSERT_EQ(Move(Piece::Silver, P55), moves[0]);
    ASSERT_EQ(Move(Piece::Silver, P45), moves[1]);
    ASSERT_EQ(Move(Piece::Silver, P35), moves[2]);
    ASSERT_EQ(Move(Piece::Silver, P37), moves[3]);
    ASSERT_EQ(Move(Piece::Silver, P66, P55, false), moves[4]);
    ASSERT_EQ(Move(Piece::Silver, P66, P57, false), moves[5]);
    ASSERT_EQ(Move(Piece::Silver, P54, P55, false), moves[6]);
    ASSERT_EQ(Move(Piece::Silver, P54, P45, false), moves[7]);
    ASSERT_EQ(Move(Piece::Pawn, P45), moves[8]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  * -OU *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  * -FU *  *  * \n"
"P6 *  *  *  *  * -KE *  *  * \n"
"P7 *  *  *  *  * +OU *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(0, moves.size());
  }

  {
    std::string src =
"P1 *  *  *  *  * -KY-KY *  * \n"
"P2 *  *  * -OU *  *  * -KE * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  * -FU *  *  * \n"
"P5 *  *  *  *  *  *  * -KI * \n"
"P6 *  *  *  *  * +OU *  *  * \n"
"P7 *  *  *  * -KI *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-00FU00KE\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 45歩 54桂 34桂打 34桂 56金 47金 36金 35金
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(8, moves.size());
    ASSERT_EQ(Move(Piece::Gold, P57, P56, false), moves[0]);
    ASSERT_EQ(Move(Piece::Gold, P57, P47, false), moves[1]);
    ASSERT_EQ(Move(Piece::Gold, P25, P35, false), moves[2]);
    ASSERT_EQ(Move(Piece::Gold, P25, P36, false), moves[3]);
    ASSERT_EQ(Move(Piece::Knight, P54), moves[4]);
    ASSERT_EQ(Move(Piece::Knight, P34), moves[5]);
    ASSERT_EQ(Move(Piece::Knight, P22, P34, false), moves[6]);
    ASSERT_EQ(Move(Piece::Pawn, P44, P45, false), moves[7]);
  }

  {
    std::string src =
"P1 *  *  *  *  * -KY-KY *  * \n"
"P2 *  *  * -OU *  *  *  *  * \n"
"P3 *  *  *  *  * +FU *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  *  *  * +FU+OU *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-00KY00KI\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 45香 44香 43香 55金 47金 45金 36金 35金
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(8, moves.size());
    ASSERT_EQ(Move(Piece::Gold, P55), moves[0]);
    ASSERT_EQ(Move(Piece::Gold, P45), moves[1]);
    ASSERT_EQ(Move(Piece::Gold, P47), moves[2]);
    ASSERT_EQ(Move(Piece::Gold, P35), moves[3]);
    ASSERT_EQ(Move(Piece::Gold, P36), moves[4]);
    ASSERT_EQ(Move(Piece::Lance, P44), moves[5]);
    ASSERT_EQ(Move(Piece::Lance, P45), moves[6]);
    ASSERT_EQ(Move(Piece::Lance, P41, P43, false), moves[7]);
  }

  {
    std::string src =
"P1 *  *  *  *  * -KY-KY *  * \n"
"P2 *  *  * -OU *  *  *  *  * \n"
"P3 *  *  *  *  * +FU *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  * -KA *  *  *  *  *  * \n"
"P6 *  * -KA *  *  *  *  *  * \n"
"P7 *  *  *  * +FU+OU *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 43香 37香成 57角成 58角成 65角
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(5, moves.size());
    ASSERT_EQ(Move(Piece::Lance, P41, P43, false), moves[0]);
    ASSERT_EQ(Move(Piece::Lance, P31, P37, true), moves[1]);
    ASSERT_EQ(Move(Piece::Bishop, P75, P57, true), moves[2]);
    ASSERT_EQ(Move(Piece::Bishop, P76, P65, false), moves[3]);
    ASSERT_EQ(Move(Piece::Bishop, P76, P58, true), moves[4]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  * -OU *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  * +FU *  *  * -HI * -HI\n"
"P5 *  * -KA *  *  *  * +FU * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  * -FU+OU *  *  * \n"
"P8 *  *  *  * +GI *  *  *  * \n"
"P9 *  *  *  *  *  *  * +KE * \n"
"P-00KA\n"
"P+\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 65角 56角 38角 36角 44飛 38飛成 37飛成 17飛成
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(8, moves.size());
    ASSERT_EQ(Move(Piece::Bishop, P65), moves[0]);
    ASSERT_EQ(Move(Piece::Bishop, P56), moves[1]);
    ASSERT_EQ(Move(Piece::Bishop, P36), moves[2]);
    ASSERT_EQ(Move(Piece::Bishop, P38), moves[3]);
    ASSERT_EQ(Move(Piece::Rook, P34, P44, false), moves[4]);
    ASSERT_EQ(Move(Piece::Rook, P34, P37, true), moves[5]);
    ASSERT_EQ(Move(Piece::Rook, P34, P38, true), moves[6]);
    ASSERT_EQ(Move(Piece::Rook, P14, P17, true), moves[7]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  * -OU *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  * -RY * +FU-UM *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  * +FU *  * +OU * +FU * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-00HI\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 67飛 57飛 49飛 48飛 46飛 37飛 57馬 46馬 36馬 25馬 67竜 56竜 45竜
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(13, moves.size());
    ASSERT_EQ(Move(Piece::Rook, P67), moves[0]);
    ASSERT_EQ(Move(Piece::Rook, P57), moves[1]);
    ASSERT_EQ(Move(Piece::Rook, P46), moves[2]);
    ASSERT_EQ(Move(Piece::Rook, P48), moves[3]);
    ASSERT_EQ(Move(Piece::Rook, P49), moves[4]);
    ASSERT_EQ(Move(Piece::Rook, P37), moves[5]);
    ASSERT_EQ(Move(Piece::Horse, P35, P57, false), moves[6]);
    ASSERT_EQ(Move(Piece::Horse, P35, P46, false), moves[7]);
    ASSERT_EQ(Move(Piece::Horse, P35, P36, false), moves[8]);
    ASSERT_EQ(Move(Piece::Horse, P35, P25, false), moves[9]);
    ASSERT_EQ(Move(Piece::Dragon, P65, P67, false), moves[10]);
    ASSERT_EQ(Move(Piece::Dragon, P65, P56, false), moves[11]);
    ASSERT_EQ(Move(Piece::Dragon, P65, P45, false), moves[12]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  * -OU *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  * -TO-NY-NK *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  * +OU *  *  * \n"
"P8 *  *  *  * -NG *  *  *  * \n"
"P9 *  *  *  *  *  *  *  *  * \n"
"P+\n"
"P-\n"
"-\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    // 56と 46と 56成香 46成香 36成香 46成桂 36成桂 57成銀 48成銀
    Moves moves;
    MoveGenerator::generateCheck(board, moves);
    ASSERT_EQ(9, moves.size());
    ASSERT_EQ(Move(Piece::Tokin, P55, P56, false), moves[0]);
    ASSERT_EQ(Move(Piece::Tokin, P55, P46, false), moves[1]);
    ASSERT_EQ(Move(Piece::ProSilver, P58, P57, false), moves[2]);
    ASSERT_EQ(Move(Piece::ProSilver, P58, P48, false), moves[3]);
    ASSERT_EQ(Move(Piece::ProLance, P45, P56, false), moves[4]);
    ASSERT_EQ(Move(Piece::ProLance, P45, P46, false), moves[5]);
    ASSERT_EQ(Move(Piece::ProLance, P45, P36, false), moves[6]);
    ASSERT_EQ(Move(Piece::ProKnight, P35, P46, false), moves[7]);
    ASSERT_EQ(Move(Piece::ProKnight, P35, P36, false), moves[8]);
  }
}

TEST(MoveGeneratorTest, testCheckLight) {
  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  * -OU *  *  *  * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  * +OU *  *  *  *  * \n"
"P+00KY00KA00HI\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCheckLight(board, moves);
    ASSERT_EQ(23, moves.size());
    ASSERT_EQ(Move(Piece::Lance, P56), moves[0]);
    ASSERT_EQ(Move(Piece::Lance, P57), moves[1]);
    ASSERT_EQ(Move(Piece::Lance, P58), moves[2]);
    ASSERT_EQ(Move(Piece::Bishop, P64), moves[3]);
    ASSERT_EQ(Move(Piece::Bishop, P73), moves[4]);
    ASSERT_EQ(Move(Piece::Bishop, P66), moves[5]);
    ASSERT_EQ(Move(Piece::Bishop, P77), moves[6]);
    ASSERT_EQ(Move(Piece::Bishop, P44), moves[7]);
    ASSERT_EQ(Move(Piece::Bishop, P33), moves[8]);
    ASSERT_EQ(Move(Piece::Bishop, P46), moves[9]);
    ASSERT_EQ(Move(Piece::Bishop, P37), moves[10]);
    ASSERT_EQ(Move(Piece::Rook, P65), moves[11]);
    ASSERT_EQ(Move(Piece::Rook, P75), moves[12]);
    ASSERT_EQ(Move(Piece::Rook, P85), moves[13]);
    ASSERT_EQ(Move(Piece::Rook, P45), moves[14]);
    ASSERT_EQ(Move(Piece::Rook, P35), moves[15]);
    ASSERT_EQ(Move(Piece::Rook, P25), moves[16]);
    ASSERT_EQ(Move(Piece::Rook, P54), moves[17]);
    ASSERT_EQ(Move(Piece::Rook, P53), moves[18]);
    ASSERT_EQ(Move(Piece::Rook, P52), moves[19]);
    ASSERT_EQ(Move(Piece::Rook, P56), moves[20]);
    ASSERT_EQ(Move(Piece::Rook, P57), moves[21]);
    ASSERT_EQ(Move(Piece::Rook, P58), moves[22]);
  }

  {
    std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  * -FU *  *  * \n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  * -OU * \n"
"P6 *  *  *  *  *  *  *  *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  * +FU * \n"
"P9 *  *  * +OU *  *  *  *  * \n"
"P+00KY00KA00HI\n"
"P-\n"
"+\n";
    std::istringstream iss(src);
    Board board;
    CsaReader::readBoard(iss, board);

    Moves moves;
    MoveGenerator::generateCheckLight(board, moves);
    ASSERT_EQ(16, moves.size());
    ASSERT_EQ(Move(Piece::Lance, P26), moves[0]);
    ASSERT_EQ(Move(Piece::Lance, P27), moves[1]);
    ASSERT_EQ(Move(Piece::Bishop, P34), moves[2]);
    ASSERT_EQ(Move(Piece::Bishop, P36), moves[3]);
    ASSERT_EQ(Move(Piece::Bishop, P47), moves[4]);
    ASSERT_EQ(Move(Piece::Bishop, P14), moves[5]);
    ASSERT_EQ(Move(Piece::Bishop, P16), moves[6]);
    ASSERT_EQ(Move(Piece::Rook, P35), moves[7]);
    ASSERT_EQ(Move(Piece::Rook, P45), moves[8]);
    ASSERT_EQ(Move(Piece::Rook, P55), moves[9]);
    ASSERT_EQ(Move(Piece::Rook, P15), moves[10]);
    ASSERT_EQ(Move(Piece::Rook, P24), moves[11]);
    ASSERT_EQ(Move(Piece::Rook, P23), moves[12]);
    ASSERT_EQ(Move(Piece::Rook, P22), moves[13]);
    ASSERT_EQ(Move(Piece::Rook, P26), moves[14]);
    ASSERT_EQ(Move(Piece::Rook, P27), moves[15]);
  }
}

#endif // !defined(NDEBUG)
