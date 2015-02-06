/* CsaReaderTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../record/CsaReader.h"
#include <sstream>

using namespace sunfish;

TEST(CsaReaderTest, test) {
	std::string src = "\
P1-KY-KE-GI-KI-OU *  * -KE-KY\n\
P2 *  *  *  *  *  *  *  *  * \n\
P3-FU * -FU-FU-FU-FU *  * -FU\n\
P4 *  *  *  *  *  *  *  *  * \n\
P5 *  *  *  *  *  *  *  *  * \n\
P6 *  *  *  *  *  *  *  *  * \n\
P7+FU *  * +FU+FU+FU+FU * +FU\n\
P8 *  *  *  *  *  *  *  *  * \n\
P9+KY+KE *  * +OU+KI+GI+KE+KY\n\
P+00FU00FU00KA22UM24HI78KI79GI76FU\n\
P-00FU00FU31GI32KI86HI34FU\n\
-\n\
-3122GI\n\
+0077KA\n\
-8689RY\n\
+7722UM\n\
-3222KI\n\
+2422RY\n\
-8979RY\n\
+7879KI\n\
-0077KA\n";
	std::istringstream iss(src);
	Record record;

	bool result = CsaReader::read(iss, record);

	ASSERT_EQ(true, result);

	std::string correct = "\
KyKeGiKiOu    KeKy\n\
              ry  \n\
Fu  FuFuFuFu    Fu\n\
            Fu    \n\
                  \n\
    fu            \n\
fu  Kafufufufu  fu\n\
                  \n\
ky  ki  oukigikeky\n\
black: fu2 gi ki hi\n\
white: fu2 ke gi ka\n\
next: black\n\
";
	ASSERT_EQ(correct, record.getBoard().toString(false));
}

#endif // !defined(NDEBUG)
