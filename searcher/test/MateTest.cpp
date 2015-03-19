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

}

#endif // !defined(NDEBUG)
