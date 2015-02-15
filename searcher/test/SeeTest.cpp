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
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  * -GI *  * \n\
P5 *  *  *  *  * -FU *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  * +HI+KE *  * \n\
P8 *  * -UM *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);
		Move move(Piece::BRook, P47, P45, false);

		see.generateAttackers(eval, board, move);

		ASSERT_EQ(see.getBlackNum(), 1);
		ASSERT_EQ(see.getWhiteNum(), 2);
		ASSERT_EQ(see.getBlackList()[0].attacker->value.int32(), eval.table().knightEx);
		ASSERT_EQ(see.getWhiteList()[0].attacker->value.int32(), eval.table().silverEx);
		ASSERT_EQ(see.getWhiteList()[1].attacker->value.int32(), eval.table().horseEx);
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
		Move move(Piece::BPawn, P64, P63, true);

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

}

TEST(SeeTest, testSearch) {

	Evaluator eval;
	See see;

	{
		std::string src = "\
P1 *  *  *  *  *  *  *  *  * \n\
P2 *  *  *  *  *  *  *  *  * \n\
P3 *  *  *  *  *  *  *  *  * \n\
P4 *  *  *  *  *  * -GI *  * \n\
P5 *  *  *  *  * -FU *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7 *  *  *  *  * +HI+KE *  * \n\
P8 *  * -UM *  *  *  *  *  * \n\
P9 *  *  *  *  *  *  *  *  * \n\
P+\n\
P-\n\
+\n\
";
		std::istringstream iss(src);
		Board board;
		CsaReader::readBoard(iss, board);

		// 飛車で歩を取った場合
		Move capByRook(Piece::BRook, P47, P45, false);
		Value exact = see.search(eval, board, capByRook);
		Value correct = (+ eval.table().pawnEx
										 - eval.table().rookEx
										 + eval.table().silverEx
										 - eval.table().knightEx);
		ASSERT_EQ(correct.int32(), exact.int32());

		// 桂馬で歩を取った場合
		Move capByKnight(Piece::BKnight, P37, P45, false);
		exact = see.search(eval, board, capByKnight);
		correct = (+ eval.table().pawnEx
							 - eval.table().knightEx);
		ASSERT_EQ(correct.int32(), exact.int32());
	}

}

#endif // !defined(NDEBUG)
