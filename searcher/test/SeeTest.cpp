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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::PawnEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::PawnEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::LanceEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::LanceEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 2);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::KnightEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::KnightEx);
		ASSERT_EQ(see.getWhiteList()[1]->value.int32(), material::KnightEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::SilverEx);
		ASSERT_EQ(see.getBlackList()[1]->value.int32(), material::SilverEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::SilverEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 2);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::GoldEx);
		ASSERT_EQ(see.getBlackList()[1]->value.int32(), material::GoldEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::GoldEx);
		ASSERT_EQ(see.getWhiteList()[1]->value.int32(), material::GoldEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::BishopEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::BishopEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 1);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::RookEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::RookEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 0);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::SilverEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 3);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::LanceEx);
		ASSERT_EQ(see.getBlackList()[1]->value.int32(), material::KnightEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::SilverEx);
		ASSERT_EQ(see.getWhiteList()[1]->value.int32(), material::BishopEx);
		ASSERT_EQ(see.getWhiteList()[2]->value.int32(), material::HorseEx);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 6);
		ASSERT_EQ(see.getWhiteNum(), 5);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::LanceEx);
		ASSERT_EQ(see.getBlackList()[1]->value.int32(), material::KnightEx);
		ASSERT_EQ(see.getBlackList()[2]->value.int32(), material::SilverEx);
		ASSERT_EQ(see.getBlackList()[3]->value.int32(), material::BishopEx);
		ASSERT_EQ(see.getBlackList()[4]->value.int32(), material::HorseEx);
		ASSERT_EQ(see.getBlackList()[5]->value.int32(), material::DragonEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::KnightEx);
		ASSERT_EQ(see.getWhiteList()[1]->value.int32(), material::GoldEx);
		ASSERT_EQ(see.getWhiteList()[2]->value.int32(), material::GoldEx);
		ASSERT_EQ(see.getWhiteList()[3]->value.int32(), material::GoldEx);
		ASSERT_EQ(see.getWhiteList()[4]->value.int32(), material::DragonEx);
		ASSERT_EQ(see.getBlackList()[0]->dependOn == nullptr, true);
		ASSERT_EQ(see.getBlackList()[1]->dependOn == nullptr, true);
		ASSERT_EQ(see.getBlackList()[2]->dependOn == nullptr, true);
		ASSERT_EQ(see.getBlackList()[3]->dependOn == see.getBlackList()[4], true);
		ASSERT_EQ(see.getBlackList()[4]->dependOn == see.getBlackList()[2], true);
		ASSERT_EQ(see.getBlackList()[5]->dependOn == see.getBlackList()[0], true);
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

		see.generateAttackers(board, move);

		ASSERT_EQ(see.getBlackNum(), 2);
		ASSERT_EQ(see.getWhiteNum(), 2);
		ASSERT_EQ(see.getBlackList()[0]->value.int32(), material::KnightEx);
		ASSERT_EQ(see.getBlackList()[1]->value.int32(), Value::PieceInfEx);
		ASSERT_EQ(see.getWhiteList()[0]->value.int32(), material::LanceEx);
		ASSERT_EQ(see.getWhiteList()[1]->value.int32(), material::KnightEx);
	}

}

TEST(SeeTest, testSearch) {

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
		Value exact = see.search(board, capByRook, -Value::PieceInf, Value::PieceInf);
		Value correct = (+ material::PawnEx
										 - material::RookEx
										 + material::SilverEx
										 - material::KnightEx);
		ASSERT_EQ(correct.int32(), exact.int32());

		// 桂馬で歩を取った場合
		Move capByKnight(Piece::Knight, P37, P45, false);
		exact = see.search(board, capByKnight, -Value::PieceInf, Value::PieceInf);
		correct = (+ material::PawnEx
							 - material::KnightEx);
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
		Value exact = see.search(board, move, -Value::PieceInf, Value::PieceInf);
		Value correct = (+ material::PawnEx
										 - material::LanceEx
										 + material::LanceEx
										 - material::KnightEx
										 + material::KnightEx);
		ASSERT_EQ(correct.int32(), exact.int32());
	}

}

#endif // !defined(NDEBUG)
