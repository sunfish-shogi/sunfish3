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

TEST(MoveGeneratorTest, testNocap) {
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
}

TEST(MoveGeneratorTest, testCap) {
	{
		// 先手の駒 歩から桂まで
		std::string src =
"P1 *  *  *  *  *  *  *  *  * \n"
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
"P1 *  *  *  *  * +KA *  *  * \n"
"P2+KI *  *  *  *  *  * +HI * \n"
"P3 *  * +GI *  *  *  *  *  * \n"
"P4+GI *  *  *  *  *  *  *  * \n"
"P5 *  * +KA *  *  *  *  *  * \n"
"P6 *  *  *  *  *  * +HI *  * \n"
"P7 *  *  *  *  *  *  *  *  * \n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  * -TO *  * \n"
"P+\n"
"P-\n"
"+\n";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);

		Moves moves;
		MoveGenerator::generateCap(board, moves);
		ASSERT_EQ(39, moves.size());
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
"P9 * +KE *  *  *  *  *  *  * \n"
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
"P1 *  *  *  *  *  *  *  *  * \n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3 *  *  *  *  *  *  *  *  * \n"
"P4 *  *  *  *  *  * -HI * +KY\n"
"P5 *  * -KA *  *  *  *  *  * \n"
"P6-GI *  *  *  *  *  *  *  * \n"
"P7 *  * -GI *  *  *  *  *  * \n"
"P8-KI *  *  *  *  *  * -HI * \n"
"P9 * +KE *  *  * -KA *  *  * \n"
"P+\n"
"P-\n"
"-\n";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);

		Moves moves;
		MoveGenerator::generateCap(board, moves);
		ASSERT_EQ(39, moves.size());
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
"P1 *  *  *  *  *  *  * +KA * \n"
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
"P1 *  *  *  *  *  *  *  *  * \n"
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
"P1 *  *  *  *  *  *  *  *  * \n"
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
"P1 *  *  *  *  *  *  *  *  * \n"
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
"P1-KY *  *  *  *  *  *  *  * \n"
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
"P9 *  *  *  *  *  *  *  *  * \n"
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

#endif // !defined(NDEBUG)
