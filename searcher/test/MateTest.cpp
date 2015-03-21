/* MateTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../Mate.h"
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

}

#endif // !defined(NDEBUG)
