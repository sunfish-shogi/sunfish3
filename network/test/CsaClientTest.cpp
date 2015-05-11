/* CsaClientTest.cpp
 *
 * Kubo Ryosuke
 */

#if !defined(NDEBUG)

#include "test/Test.h"
#include "../CsaClient.h"

TEST(CsaClient, test) {
  auto flagSets = sunfish::CsaClient::getFlagSets();

  auto probe = [flagSets](const char* str) {
    for (int i = 0; i < sunfish::CsaClient::RECV_NUM; i++) {
      if (flagSets[i].wildcard.match(str)) {
        return flagSets[i].flag;
      }
    }
    return sunfish::CsaClient::RECV_FLAG::RECV_NULL;
  };

  ASSERT_EQ(probe("LOGIN:sunfish OK"), sunfish::CsaClient::RECV_LOGIN_OK);
  ASSERT_EQ(probe("LOGIN:incorrect"), sunfish::CsaClient::RECV_LOGIN_INC);
  ASSERT_EQ(probe("LOGOUT:completed"), sunfish::CsaClient::RECV_LOGOUT);
  ASSERT_EQ(probe("%TORYO"), sunfish::CsaClient::RECV_MOVE_EX);
  ASSERT_EQ(probe("+7776FU"), sunfish::CsaClient::RECV_MOVE_B);
  ASSERT_EQ(probe("+7776FU, foo"), sunfish::CsaClient::RECV_MOVE_B);
  ASSERT_EQ(probe("-3334FU, bar"), sunfish::CsaClient::RECV_MOVE_W);
  ASSERT_EQ(probe("BEGIN Game_Summary"), sunfish::CsaClient::RECV_SUMMARY);;
  ASSERT_EQ(probe("START:foo_bar"), sunfish::CsaClient::RECV_START);
  ASSERT_EQ(probe("REJECT:foo by bar"), sunfish::CsaClient::RECV_REJECT);
  ASSERT_EQ(probe("#WIN"), sunfish::CsaClient::RECV_WIN);
  ASSERT_EQ(probe("#LOSE"), sunfish::CsaClient::RECV_LOSE);
  ASSERT_EQ(probe("#WIN(LOSE)"), sunfish::CsaClient::RECV_WIN_LOSE);
  ASSERT_EQ(probe("#DRAW"), sunfish::CsaClient::RECV_DRAW);
  ASSERT_EQ(probe("#CHUDAN"), sunfish::CsaClient::RECV_INTERRUPT);
  ASSERT_EQ(probe("#SENNICHITE"), sunfish::CsaClient::RECV_REPEAT);
  ASSERT_EQ(probe("#OUTE_SENNICHITE"), sunfish::CsaClient::RECV_CHECK_REP);
  ASSERT_EQ(probe("#ILLEGAL_MOVE"), sunfish::CsaClient::RECV_ILLEGAL);
  ASSERT_EQ(probe("#TIME_UP"), sunfish::CsaClient::RECV_TIME_UP);
  ASSERT_EQ(probe("#RESIGN"), sunfish::CsaClient::RECV_RESIGN);
  ASSERT_EQ(probe("#JISHOGI"), sunfish::CsaClient::RECV_JISHOGI);
  ASSERT_EQ(probe("#MAX_MOVES"), sunfish::CsaClient::RECV_MAX_MOVE);
  ASSERT_EQ(probe("#CENSORED"), sunfish::CsaClient::RECV_CENSORED);

  ASSERT_EQ(probe("LOGIN:sunfish"), sunfish::CsaClient::RECV_NULL);
  ASSERT_EQ(probe("LOGIN:sunfish "), sunfish::CsaClient::RECV_NULL);
  ASSERT_EQ(probe("LOGOUT:hoge "), sunfish::CsaClient::RECV_NULL);
  ASSERT_EQ(probe("START"), sunfish::CsaClient::RECV_NULL);
  ASSERT_EQ(probe("REJECT"), sunfish::CsaClient::RECV_NULL);
  ASSERT_EQ(probe("#HOGE"), sunfish::CsaClient::RECV_NULL);

}

#endif // !defined(NDEBUG)
