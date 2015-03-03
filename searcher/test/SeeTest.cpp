/* SeeTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../see/See.h"
#include "core/record/CsaReader.h"

using namespace sunfish;

TEST(SeeTest, testGenerateAttackers) {

	Evaluator eval;
	See see;

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  * -FU *  *  *  * \n\
P5 *  *  *  * -KY+KI *  *  * \n\
P6 *  *  *  * +FU *  *  *  * \n\
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
		Move move(Piece::Gold, P45, P55, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().pawnEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().pawnEx);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  * -KY *  *  *  * \n\
P5 *  *  *  * -FU+KI *  *  * \n\
P6 *  *  *  * +KY *  *  *  * \n\
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
		Move move(Piece::Gold, P45, P55, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().lanceEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().lanceEx);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  * -KE * -KE *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  * -FU *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +KE * +KE *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Knight, P47, P55, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 2);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().knightEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().knightEx);
		ASSERT_EQ(see.getWhiteList()[1].attacker->value.int32(), eval.table().knightEx);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  * -GI * +GI *  *  * \n\
P5 *  *  *  * -FU *  *  *  * \n\
P6 *  *  * +GI * -GI *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
-\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Silver, P46, P55, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().silverEx);
		ASSERT_EQ(see.getBlackList()[1].attacker->value.int32(), eval.table().silverEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().silverEx);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  * -KI-KI *  *  * \n\
P5 *  *  *  * -FU *  *  *  * \n\
P6 *  *  * +GI+KI+KI *  *  * \n\
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
		Move move(Piece::Silver, P66, P55, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 2);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().goldEx);
		ASSERT_EQ(see.getBlackList()[1].attacker->value.int32(), eval.table().goldEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().goldEx);
		ASSERT_EQ(see.getWhiteList()[1].attacker->value.int32(), eval.table().goldEx);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  * -KA *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  * -FU *  *  *  * \n\
P6 *  *  *  *  * +GI *  *  * \n\
P7 *  * +KA *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Silver, P46, P55, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().bishopEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().bishopEx);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  * -HI *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  * -FU *  *  *  * \n\
P6 *  *  *  *  * +GI *  *  * \n\
P7 *  *  *  * +HI *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Silver, P46, P55, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().rookEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().rookEx);
	}

	{
		std::string src = "\
P1 * -FU *  * -OU *  *  *  * \n\
P2+GI * +GI *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9-KI *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Silver, P72, P81, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 0);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().silverEx);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  * -KA * \n\
P4 *  *  *  *  *  * -GI *  * \n\
P5 *  *  *  *  * -FU *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  * +HI+KE *  * \n\
P8 *  * -UM *  *  *  *  *  * \n\
P9 *  *  *  * +OU+KY *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Rook, P47, P45, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 3);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().lanceEx);
		ASSERT_EQ(see.getBlackList()[1].attacker->value.int32(), eval.table().knightEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().silverEx);
		ASSERT_EQ(see.getWhiteList()[1].attacker->value.int32(), eval.table().bishopEx);
		ASSERT_EQ(see.getWhiteList()[2].attacker->value.int32(), eval.table().horseEx);
	}

	{
		std::string src = "\
P1 *  * -KE *  *  *  *  *  * \n\
P2 *  *  * -KI-KI * -OU *  * \n\
P3 * -RY-KI-GI *  * +FU *  * \n\
P4 *  *  * +FU+GI *  *  *  * \n\
P5 *  *  *  * +KE+UM *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  * +KY *  *  * +KA * \n\
P8 *  *  *  *  * +OU *  *  * \n\
P9 * +KY * +RY *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Pawn, P64, P63, true);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 6);
		ASSERT_EQ(see.getWhiteNum(), 5);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().lanceEx);
		ASSERT_EQ(see.getBlackList()[1].attacker->value.int32(), eval.table().knightEx);
		ASSERT_EQ(see.getBlackList()[2].attacker->value.int32(), eval.table().silverEx);
		ASSERT_EQ(see.getBlackList()[3].attacker->value.int32(), eval.table().bishopEx);
		ASSERT_EQ(see.getBlackList()[4].attacker->value.int32(), eval.table().horseEx);
		ASSERT_EQ(see.getBlackList()[5].attacker->value.int32(), eval.table().dragonEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().knightEx);
		ASSERT_EQ(see.getWhiteList()[1].attacker->value.int32(), eval.table().goldEx);
		ASSERT_EQ(see.getWhiteList()[2].attacker->value.int32(), eval.table().goldEx);
		ASSERT_EQ(see.getWhiteList()[3].attacker->value.int32(), eval.table().goldEx);
		ASSERT_EQ(see.getWhiteList()[4].attacker->value.int32(), eval.table().dragonEx);
		ASSERT_EQ(see.getBlackList()[0].attacker->dependOn == nullptr, true);
		ASSERT_EQ(see.getBlackList()[1].attacker->dependOn == nullptr, true);
		ASSERT_EQ(see.getBlackList()[2].attacker->dependOn == nullptr, true);
		ASSERT_EQ(see.getBlackList()[3].attacker->dependOn == see.getBlackList()[4].attacker, true);
		ASSERT_EQ(see.getBlackList()[4].attacker->dependOn == see.getBlackList()[2].attacker, true);
		ASSERT_EQ(see.getBlackList()[5].attacker->dependOn == see.getBlackList()[0].attacker, true);
	}

	{
		std::string src = "\
P1 *  *  *  * -OU-KY *  *  * \n\
P2 *  *  *  *  *  * -KE *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  * -FU *  *  * \n\
P5 *  *  *  * +OU+KY *  *  * \n\
P6 *  *  *  *  *  * +KE *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::Lance, P45, P44, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 2);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().knightEx);
		ASSERT_EQ(see.getBlackList()[1].attacker->value.int32(), eval.PieceInfEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().lanceEx);
		ASSERT_EQ(see.getWhiteList()[1].attacker->value.int32(), eval.table().knightEx);
	}

}

TEST(SeeTest, testSearch) {

	Evaluator eval;
	See see;

	{
		std::string src = "\
P1 *  *  *  * -OU *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  * -GI *  * \n\
P5 *  *  *  *  * -FU *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  * +HI+KE *  * \n\
P8 *  * -UM *  *  *  *  *  * \n\
P9 *  *  *  * +OU *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);

		// 飛車で歩を取った場合
		Move capByRook(Piece::Rook, P47, P45, false);
		Value exact = see.search(eval, board, capByRook, -Evaluator::PieceInf, Evaluator::PieceInf);
		Value correct = (+ eval.table().pawnEx
										 - eval.table().rookEx
										 + eval.table().silverEx
										 - eval.table().knightEx);
		ASSERT_EQ(correct.int32(), exact.int32());

		// 桂馬で歩を取った場合
		Move capByKnight(Piece::Knight, P37, P45, false);
		exact = see.search(eval, board, capByKnight, -Evaluator::PieceInf, Evaluator::PieceInf);
		correct = (+ eval.table().pawnEx
							 - eval.table().knightEx);
		ASSERT_EQ(correct.int32(), exact.int32());
	}

	{
		std::string src = "\
P1 *  *  *  * -OU-KY *  *  * \n\
P2 *  *  *  *  *  * -KE *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  * -FU *  *  * \n\
P5 *  *  *  * +OU+KY *  *  * \n\
P6 *  *  *  *  *  * +KE *  * \n\
P7 *  *  *  *  *  *  *  *  * \n\
P8 *  *  *  *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);

		Move move(Piece::Lance, P45, P44, false);
		Value exact = see.search(eval, board, move, -Evaluator::PieceInf, Evaluator::PieceInf);
		Value correct = (+ eval.table().pawnEx
										 - eval.table().lanceEx
										 + eval.table().lanceEx
										 - eval.table().knightEx
										 + eval.table().knightEx);
		ASSERT_EQ(correct.int32(), exact.int32());
	}

}

#endif // !defined(NDEBUG)
