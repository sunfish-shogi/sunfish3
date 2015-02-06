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

TEST(MoveGeneratorTest, testNoCap) {
	Board board;
	board.init(Board::Handicap::Even);
	{
		Moves moves;
		MoveGenerator::generateNoCap(board, moves);
		ASSERT_EQ(30, moves.size());
	}
	// 76歩
	board.makeMoveIrr(Move(Piece::Pawn, P77, P76, false));
	{
		Moves moves;
		MoveGenerator::generateNoCap(board, moves);
		ASSERT_EQ(30, moves.size());
	}
	// 34歩
	board.makeMoveIrr(Move(Piece::Pawn, P33, P34, false));
	{
		Moves moves;
		MoveGenerator::generateNoCap(board, moves);
		ASSERT_EQ(36, moves.size());
	}
}

TEST(MoveGeneratorTest, testCap) {
	Board board;
	Moves moves;
	board.init(Board::Handicap::Even);
	{
		MoveGenerator::generateCap(board, moves);
		ASSERT_EQ(0, moves.size());
	}
	// 76歩
	board.makeMoveIrr(Move(Piece::Pawn, P77, P76, false));
	{
		Moves moves;
		MoveGenerator::generateCap(board, moves);
		ASSERT_EQ(0, moves.size());
	}
	// 34歩
	board.makeMoveIrr(Move(Piece::Pawn, P33, P34, false));
	{
		Moves moves;
		MoveGenerator::generateCap(board, moves);
		ASSERT_EQ(1, moves.size());
	}
}

TEST(MoveGeneratorTest, testDrop) {
	Board board;
	Moves moves;
	board.init(Board::Handicap::Even);
	{
		Moves moves;
		MoveGenerator::generateDrop(board, moves);
		ASSERT_EQ(0, moves.size());
	}
	board.makeMoveIrr(Move(Piece::Pawn, P77, P76, false));
	board.makeMoveIrr(Move(Piece::Pawn, P33, P34, false));
	board.makeMoveIrr(Move(Piece::Pawn, P27, P26, false));
	board.makeMoveIrr(Move(Piece::Pawn, P83, P84, false));
	board.makeMoveIrr(Move(Piece::Pawn, P26, P25, false));
	board.makeMoveIrr(Move(Piece::Pawn, P84, P85, false));
	board.makeMoveIrr(Move(Piece::Gold, P69, P78, false));
	board.makeMoveIrr(Move(Piece::Gold, P41, P32, false));
	board.makeMoveIrr(Move(Piece::Pawn, P25, P24, false));
	board.makeMoveIrr(Move(Piece::Pawn, P23, P24, false));
	board.makeMoveIrr(Move(Piece::Rook, P28, P24, false));
	board.makeMoveIrr(Move(Piece::Pawn, P85, P86, false));
	board.makeMoveIrr(Move(Piece::Pawn, P87, P86, false));
	board.makeMoveIrr(Move(Piece::Rook, P82, P86, false));
	{
		Moves moves;
		MoveGenerator::generateDrop(board, moves);
		ASSERT_EQ(10, moves.size());
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

}

#endif // !defined(NDEBUG)
