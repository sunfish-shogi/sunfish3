/* Evaluator.cpp
 *
 * Kubo Ryosuke
 */

#include "Evaluator.h"
#include "core/util/Random.h"
#include "logger/Logger.h"
#include <fstream>
#include <cstdlib>

#define DEFAULT_FV_FILENAME "eval.bin"
#define FVBIN_FILENAME      "fv.bin"

#define ENABLE_DIFF         1
#define ENABLE_KPP          1
#define ENABLE_HASHTABLE    1

namespace {

using namespace sunfish;

inline int kpp_index(int x, int y) {
  assert(x >= y);
  return x*(x+1)/2+y;
}

inline int kpp_index(int x) {
  return x*(x+1)/2+x;
}

inline int kpp_index_safe(int x, int y) {
  return x >= y ? kpp_index(x, y) : kpp_index(y, x);
}

int posIndexBPawn[] = {
  -1,  0,  1,  2,  3,  4,  5,  6,  7,
  -1,  8,  9, 10, 11, 12, 13, 14, 15,
  -1, 16, 17, 18, 19, 20, 21, 22, 23,
  -1, 24, 25, 26, 27, 28, 29, 30, 31,
  -1, 32, 33, 34, 35, 36, 37, 38, 39,
  -1, 40, 41, 42, 43, 44, 45, 46, 47,
  -1, 48, 49, 50, 51, 52, 53, 54, 55,
  -1, 56, 57, 58, 59, 60, 61, 62, 63,
  -1, 64, 65, 66, 67, 68, 69, 70, 71,
};

int posIndexWPawn[] = {
   0,  1,  2,  3,  4,  5,  6,  7, -1,
   8,  9, 10, 11, 12, 13, 14, 15, -1,
  16, 17, 18, 19, 20, 21, 22, 23, -1,
  24, 25, 26, 27, 28, 29, 30, 31, -1,
  32, 33, 34, 35, 36, 37, 38, 39, -1,
  40, 41, 42, 43, 44, 45, 46, 47, -1,
  48, 49, 50, 51, 52, 53, 54, 55, -1,
  56, 57, 58, 59, 60, 61, 62, 63, -1,
  64, 65, 66, 67, 68, 69, 70, 71, -1,
};

int posIndexBKnight[] = {
  -1, -1,  0,  1,  2,  3,  4,  5,  6,
  -1, -1,  7,  8,  9, 10, 11, 12, 13,
  -1, -1, 14, 15, 16, 17, 18, 19, 20,
  -1, -1, 21, 22, 23, 24, 25, 26, 27,
  -1, -1, 28, 29, 30, 31, 32, 33, 34,
  -1, -1, 35, 36, 37, 38, 39, 40, 41,
  -1, -1, 42, 43, 44, 45, 46, 47, 48,
  -1, -1, 49, 50, 51, 52, 53, 54, 55,
  -1, -1, 56, 57, 58, 59, 60, 61, 62,
};

int posIndexWKnight[] = {
   0,  1,  2,  3,  4,  5,  6, -1, -1,
   7,  8,  9, 10, 11, 12, 13, -1, -1,
  14, 15, 16, 17, 18, 19, 20, -1, -1,
  21, 22, 23, 24, 25, 26, 27, -1, -1,
  28, 29, 30, 31, 32, 33, 34, -1, -1,
  35, 36, 37, 38, 39, 40, 41, -1, -1,
  42, 43, 44, 45, 46, 47, 48, -1, -1,
  49, 50, 51, 52, 53, 54, 55, -1, -1,
  56, 57, 58, 59, 60, 61, 62, -1, -1,
};

// sunfish to bonanza
int posS2B[] = {
   0,  9, 18, 27, 36, 45, 54, 63, 72,
   1, 10, 19, 28, 37, 46, 55, 64, 73,
   2, 11, 20, 29, 38, 47, 56, 65, 74,
   3, 12, 21, 30, 39, 48, 57, 66, 75,
   4, 13, 22, 31, 40, 49, 58, 67, 76,
   5, 14, 23, 32, 41, 50, 59, 68, 77,
   6, 15, 24, 33, 42, 51, 60, 69, 78,
   7, 16, 25, 34, 43, 52, 61, 70, 79,
   8, 17, 26, 35, 44, 53, 62, 71, 80,
};

} // namespace

#define POS_INDEX_BPAWN(pos)   (posIndexBPawn[(pos)])
#define POS_INDEX_WPAWN(pos)   (posIndexWPawn[(pos)])
#define POS_INDEX_BKNIGHT(pos) (posIndexBKnight[(pos)])
#define POS_INDEX_WKNIGHT(pos) (posIndexWKnight[(pos)])
#define POS_INDEX_BNORMAL(pos) (pos)
#define POS_INDEX_WNORMAL(pos) (pos)

namespace sunfish {

/**
 * ファイルからパラメータを読み込みます。
 * @param filename
 */
template <class T>
bool Feature<T>::readFile(const char* filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::in);

  if (!file) {
    return false;
  }

  file.read((char*)t_->kpp, sizeof(t_->kpp));
  file.read((char*)t_->kkp, sizeof(t_->kkp));


  file.close();

  return true;
}

/**
 * ファイルにパラメータを書き出します。
 * @param filename
 */
template <class T>
bool Feature<T>::writeFile(const char* filename) const {
  std::ofstream file(filename, std::ios::binary | std::ios::out);

  if (!file) {
    return false;
  }

  file.write((const char*)t_->kpp, sizeof(t_->kpp));
  file.write((const char*)t_->kkp, sizeof(t_->kkp));

  file.close();

  return true;
}

int posInv(int table[], int in) {
  POSITION_EACH(pos) {
    if (table[pos] == in) {
      return pos;
    }
  }
  return -1;
}

/**
 * KPP のインデクスを Bonanza の並びに変換します。
 */
int convertKppIndex4FvBin(int index) {
  struct BoardInfo {
    int begin;
    int end;
    int offset;
    int* table;
  };

  static const BoardInfo biList[] = {
    { KPP_BBPAWN,   KPP_BWPAWN,   -9,  posIndexBPawn },
    { KPP_BWPAWN,   KPP_BBLANCE,  0,   posIndexWPawn },
    { KPP_BBLANCE,  KPP_BWLANCE,  -9,  posIndexBPawn },
    { KPP_BWLANCE,  KPP_BBKNIGHT, 0,   posIndexWPawn },
    { KPP_BBKNIGHT, KPP_BWKNIGHT, -18, posIndexBKnight },
    { KPP_BWKNIGHT, KPP_BBSILVER, 0,   posIndexWKnight },
    { KPP_BBSILVER, KPP_BWSILVER, 0,   nullptr },
    { KPP_BWSILVER, KPP_BBGOLD,   0,   nullptr },
    { KPP_BBGOLD,   KPP_BWGOLD,   0,   nullptr },
    { KPP_BWGOLD,   KPP_BBBISHOP, 0,   nullptr },
    { KPP_BBBISHOP, KPP_BWBISHOP, 0,   nullptr },
    { KPP_BWBISHOP, KPP_BBHORSE,  0,   nullptr },
    { KPP_BBHORSE,  KPP_BWHORSE,  0,   nullptr },
    { KPP_BWHORSE,  KPP_BBROOK,   0,   nullptr },
    { KPP_BBROOK,   KPP_BWROOK,   0,   nullptr },
    { KPP_BWROOK,   KPP_BBDRAGON, 0,   nullptr },
    { KPP_BBDRAGON, KPP_BWDRAGON, 0,   nullptr },
    { KPP_BWDRAGON, KPP_MAX,      0,   nullptr },
  };

  if (index < KPP_BBPAWN) {
    return index;
  }

  for (unsigned i = 0; i < sizeof(biList)/sizeof(biList[0]); i++) {
    const auto& bi = biList[i];
    if (bi.begin <= index && index < bi.end) {
      int pos = index - bi.begin;
      assert(pos >= 0);
      assert(pos < 81);
      if (bi.table != nullptr) {
        pos = posInv(bi.table, pos);
        assert(pos >= 0);
        assert(pos < 81);
      }
      int result = bi.begin + posS2B[pos] + bi.offset;
      assert(bi.begin <= result);
      assert(result < bi.end);
      return result;
    }
  }
  assert(false);
  return 0;
}

/**
 * KKP のインデクスを Bonanza の並びに変換します。
 */
int convertKkpIndex4FvBin(int index) {
  struct BoardInfo {
    int begin;
    int end;
    int offset;
    int* table;
  };

  static const BoardInfo biList[] = {
    { KKP_BPAWN,   KKP_BLANCE,   -9,  posIndexBPawn },
    { KKP_BLANCE,  KKP_BKNIGHT,  -9,  posIndexBPawn },
    { KKP_BKNIGHT, KKP_BSILVER, -18, posIndexBKnight },
    { KKP_BSILVER, KKP_BGOLD,     0,   nullptr },
    { KKP_BGOLD,   KKP_BBISHOP,   0,   nullptr },
    { KKP_BBISHOP, KKP_BHORSE,    0,   nullptr },
    { KKP_BHORSE,  KKP_BROOK,     0,   nullptr },
    { KKP_BROOK,   KKP_BDRAGON,   0,   nullptr },
    { KKP_BDRAGON, KKP_MAX,       0,   nullptr },
  };

  if (index < KKP_BPAWN) {
    return index;
  }

  for (unsigned i = 0; i < sizeof(biList)/sizeof(biList[0]); i++) {
    const auto& bi = biList[i];
    if (bi.begin <= index && index < bi.end) {
      int pos = index - bi.begin;
      assert(pos >= 0);
      assert(pos < 81);
      if (bi.table != nullptr) {
        pos = posInv(bi.table, pos);
      }
      assert(pos >= 0);
      assert(pos < 81);
      int result = bi.begin + posS2B[pos] + bi.offset;
      assert(bi.begin <= result);
      assert(result < bi.end);
      return result;
    }
  }
  assert(false);
  return 0;
}

/**
 * 盤上の駒の種類から KKP のインデクスを取得します。
 */
int kkpBoardIndex(Piece piece, const Position& pos) {
  switch (piece) {
    case Piece::BPawn:      case Piece::WPawn:      return KKP_BPAWN + posIndexBPawn[pos];
    case Piece::BLance:     case Piece::WLance:     return KKP_BLANCE + posIndexBPawn[pos];
    case Piece::BKnight:    case Piece::WKnight:    return KKP_BKNIGHT + posIndexBKnight[pos];
    case Piece::BSilver:    case Piece::WSilver:    return KKP_BSILVER + pos;
    case Piece::BGold:      case Piece::WGold:      return KKP_BGOLD + pos;
    case Piece::BBishop:    case Piece::WBishop:    return KKP_BBISHOP + pos;
    case Piece::BRook:      case Piece::WRook:      return KKP_BROOK + pos;
    case Piece::BTokin:     case Piece::WTokin:     return KKP_BGOLD + pos;
    case Piece::BProLance:  case Piece::WProLance:  return KKP_BGOLD + pos;
    case Piece::BProKnight: case Piece::WProKnight: return KKP_BGOLD + pos;
    case Piece::BProSilver: case Piece::WProSilver: return KKP_BGOLD + pos;
    case Piece::BHorse:     case Piece::WHorse:     return KKP_BHORSE + pos;
    case Piece::BDragon:    case Piece::WDragon:    return KKP_BDRAGON + pos;
  }

  assert(false);
  return 0;

}

/**
 * 持ち駒の種類から KKP のインデクスを取得します。
 */
int kkpHandIndex(Piece piece) {
  switch (piece) {
    case Piece::BPawn:      case Piece::WPawn:      return KKP_HPAWN;
    case Piece::BLance:     case Piece::WLance:     return KKP_HLANCE;
    case Piece::BKnight:    case Piece::WKnight:    return KKP_HKNIGHT;
    case Piece::BSilver:    case Piece::WSilver:    return KKP_HSILVER;
    case Piece::BGold:      case Piece::WGold:      return KKP_HGOLD;
    case Piece::BBishop:    case Piece::WBishop:    return KKP_HBISHOP;
    case Piece::BRook:      case Piece::WRook:      return KKP_HROOK;
    case Piece::BTokin:     case Piece::WTokin:     return KKP_HPAWN;
    case Piece::BProLance:  case Piece::WProLance:  return KKP_HLANCE;
    case Piece::BProKnight: case Piece::WProKnight: return KKP_HKNIGHT;
    case Piece::BProSilver: case Piece::WProSilver: return KKP_HSILVER;
    case Piece::BHorse:     case Piece::WHorse:     return KKP_HBISHOP;
    case Piece::BDragon:    case Piece::WDragon:    return KKP_HROOK;
  }

  assert(false);
  return 0;

}

/**
 * 盤上の先手の駒の種類から KPP のインデクスを取得します。
 */
template <bool blackPiece>
int kppBoardIndex(Piece piece, const Position& pos) {
  switch (piece) {
    case Piece::BPawn:      case Piece::WPawn:      return (blackPiece ? KPP_BBPAWN   : KPP_BWPAWN) + (blackPiece ? posIndexBPawn[pos] : posIndexWPawn[pos]);
    case Piece::BLance:     case Piece::WLance:     return (blackPiece ? KPP_BBLANCE  : KPP_BWLANCE) + (blackPiece ? posIndexBPawn[pos] : posIndexWPawn[pos]);
    case Piece::BKnight:    case Piece::WKnight:    return (blackPiece ? KPP_BBKNIGHT : KPP_BWKNIGHT) + (blackPiece ? posIndexBKnight[pos] : posIndexWKnight[pos]);
    case Piece::BSilver:    case Piece::WSilver:    return (blackPiece ? KPP_BBSILVER : KPP_BWSILVER) + pos;
    case Piece::BGold:      case Piece::WGold:      return (blackPiece ? KPP_BBGOLD   : KPP_BWGOLD) + pos;
    case Piece::BBishop:    case Piece::WBishop:    return (blackPiece ? KPP_BBBISHOP : KPP_BWBISHOP) + pos;
    case Piece::BRook:      case Piece::WRook:      return (blackPiece ? KPP_BBROOK   : KPP_BWROOK) + pos;
    case Piece::BTokin:     case Piece::WTokin:     return (blackPiece ? KPP_BBGOLD   : KPP_BWGOLD) + pos;
    case Piece::BProLance:  case Piece::WProLance:  return (blackPiece ? KPP_BBGOLD   : KPP_BWGOLD) + pos;
    case Piece::BProKnight: case Piece::WProKnight: return (blackPiece ? KPP_BBGOLD   : KPP_BWGOLD) + pos;
    case Piece::BProSilver: case Piece::WProSilver: return (blackPiece ? KPP_BBGOLD   : KPP_BWGOLD) + pos;
    case Piece::BHorse:     case Piece::WHorse:     return (blackPiece ? KPP_BBHORSE  : KPP_BWHORSE) + pos;
    case Piece::BDragon:    case Piece::WDragon:    return (blackPiece ? KPP_BBDRAGON : KPP_BWDRAGON) + pos;
  }

  assert(false);
  return 0;
}

/**
 * 先手の持ち駒の種類から KPP のインデクスを取得します。
 */
template <bool blackPiece>
int kppHandIndex(Piece piece) {
  switch (piece) {
    case Piece::BPawn:      case Piece::WPawn:      return blackPiece ? KPP_HBPAWN   : KPP_HWPAWN;
    case Piece::BLance:     case Piece::WLance:     return blackPiece ? KPP_HBLANCE  : KPP_HWLANCE;
    case Piece::BKnight:    case Piece::WKnight:    return blackPiece ? KPP_HBKNIGHT : KPP_HWKNIGHT;
    case Piece::BSilver:    case Piece::WSilver:    return blackPiece ? KPP_HBSILVER : KPP_HWSILVER;
    case Piece::BGold:      case Piece::WGold:      return blackPiece ? KPP_HBGOLD   : KPP_HWGOLD;
    case Piece::BBishop:    case Piece::WBishop:    return blackPiece ? KPP_HBBISHOP : KPP_HWBISHOP;
    case Piece::BRook:      case Piece::WRook:      return blackPiece ? KPP_HBROOK   : KPP_HWROOK;
    case Piece::BTokin:     case Piece::WTokin:     return blackPiece ? KPP_HBPAWN   : KPP_HWPAWN;
    case Piece::BProLance:  case Piece::WProLance:  return blackPiece ? KPP_HBLANCE  : KPP_HWLANCE;
    case Piece::BProKnight: case Piece::WProKnight: return blackPiece ? KPP_HBKNIGHT : KPP_HWKNIGHT;
    case Piece::BProSilver: case Piece::WProSilver: return blackPiece ? KPP_HBSILVER : KPP_HWSILVER;
    case Piece::BHorse:     case Piece::WHorse:     return blackPiece ? KPP_HBBISHOP : KPP_HWBISHOP;
    case Piece::BDragon:    case Piece::WDragon:    return blackPiece ? KPP_HBROOK   : KPP_HWROOK;
  }

  assert(false);
  return 0;

}

template <class T>
template <class U, bool update>
U Feature<T>::extract(const Board& board, U delta) {
  U positional = 0;
  auto bking = board.getBKingPosition();
  auto wking = board.getWKingPosition();
  auto bkingR = bking.reverse();
  auto wkingR = wking.reverse();

  int num = 14;
  int bList[52]; // 52 = 40(総駒数) - 2(玉) + 14(駒台)
  int wList[52];
  int nTemp;
  int wTemp[34]; // 34 = 18(と金) + 16(成香, 成桂, 成銀, 金)

#define ON_HAND(piece, pieceL, index) { \
  int count = board.getBlackHand(Piece::piece); \
  if (update) { \
    t_->kkp[bking][wking][KKP_H ## pieceL+count] += ValueType(delta); \
  } else { \
    positional += t_->kkp[bking][wking][KKP_H ## pieceL+count]; \
  } \
  bList[index] = KPP_HB ## pieceL + count; \
  wList[index+1] = KPP_HW ## pieceL + count; \
  count = board.getWhiteHand(Piece::piece); \
  if (update) { \
    t_->kkp[wkingR][bkingR][KKP_H ## pieceL+count] -= ValueType(delta); \
  } else { \
    positional -= t_->kkp[wkingR][bkingR][KKP_H ## pieceL+count]; \
  } \
  bList[index+1] = KPP_HW ## pieceL + count; \
  wList[index] = KPP_HB ## pieceL + count; \
}

  ON_HAND(Pawn, PAWN, 0);
  ON_HAND(Lance, LANCE, 2);
  ON_HAND(Knight, KNIGHT, 4);
  ON_HAND(Silver, SILVER, 6);
  ON_HAND(Gold, GOLD, 8);
  ON_HAND(Bishop, BISHOP, 10);
  ON_HAND(Rook, ROOK, 12);

#undef ON_HAND

#define ON_BOARD(blackBB, whiteBB, pieceL, POS_INDEX_B, POS_INDEX_W) { \
  nTemp = 0; \
  auto bb = (blackBB); \
  BB_EACH_OPE(pos, bb, { \
    if (update) { \
      t_->kkp[bking][wking][KKP_B ## pieceL+POS_INDEX_B(pos)] += ValueType(delta); \
    } else { \
      positional += t_->kkp[bking][wking][KKP_B ## pieceL+POS_INDEX_B(pos)]; \
    } \
    bList[num++] = KPP_BB ## pieceL + POS_INDEX_B(pos); \
    wTemp[nTemp++] = KPP_BW ## pieceL + POS_INDEX_W(pos.reverse()); \
  }); \
  bb = (whiteBB); \
  BB_EACH_OPE(pos, bb, { \
    if (update) { \
      t_->kkp[wkingR][bkingR][KKP_B ## pieceL+POS_INDEX_B(pos.reverse())] -= ValueType(delta); \
    } else { \
      positional -= t_->kkp[wkingR][bkingR][KKP_B ## pieceL+POS_INDEX_B(pos.reverse())]; \
    } \
    bList[num++] = KPP_BW ## pieceL + POS_INDEX_W(pos); \
    wTemp[nTemp++] = KPP_BB ## pieceL + POS_INDEX_B(pos.reverse()); \
  }); \
  for (int i = 0; i < nTemp; i++) { wList[num-i-1] = wTemp[i]; } \
}

  ON_BOARD(board.getBPawn(), board.getWPawn(), PAWN, POS_INDEX_BPAWN, POS_INDEX_WPAWN);
  ON_BOARD(board.getBLance(),board.getWLance(), LANCE, POS_INDEX_BPAWN, POS_INDEX_WPAWN);
  ON_BOARD(board.getBKnight(), board.getWKnight(), KNIGHT, POS_INDEX_BKNIGHT, POS_INDEX_WKNIGHT);
  ON_BOARD(board.getBSilver(), board.getWSilver(), SILVER, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBGold() | board.getBTokin() | board.getBProLance() | board.getBProKnight() | board.getBProSilver(),
           board.getWGold() | board.getWTokin() | board.getWProLance() | board.getWProKnight() | board.getWProSilver(), GOLD,
           POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBBishop(), board.getWBishop(), BISHOP, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBHorse(), board.getWHorse(), HORSE, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBRook(), board.getWRook(), ROOK, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBDragon(), board.getWDragon(), DRAGON, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);

#undef ON_BOARD

#if ENABLE_KPP
  for (int i = 0; i < num; i++) {
    int bx = bList[i];
    int wx = wList[i];
    for (int j = 0; j <= i; j++) {
      int by = bList[j];
      int wy = wList[j];
      assert(by <= bx);
      assert(wy <= wx);
      if (update) {
        t_->kpp[bking][kpp_index(bx, by)] += ValueType(delta);
        t_->kpp[wkingR][kpp_index(wx, wy)] -= ValueType(delta);
      } else {
        positional += t_->kpp[bking][kpp_index(bx, by)];
        positional -= t_->kpp[wkingR][kpp_index(wx, wy)];
      }
    }
  }
#endif // ENABLE_KPP

  return positional;
}

template int32_t Feature<float>::extract<int32_t, false>(const Board& board, int32_t delta);
template float Feature<float>::extract<float, true>(const Board& board, float delta);

Evaluator::Evaluator(InitType initType /*= InitType::File*/) {
  switch (initType) {
  case InitType::File:
    init();
    if (!readFile()) {
      if (convertFromFvBin()) {
        writeFile();
      }
    }
    break;

  case InitType::Zero:
    init();
    break;

  case InitType::Random:
    initRandom();
    break;

  default:
    assert(false);
    break;
  }
}

Evaluator::Evaluator(Evaluator& ref) : Feature<int16_t>(ref) {
}

void Evaluator::init() {
  memset(t_->kpp, 0, sizeof(t_->kpp));
  memset(t_->kkp, 0, sizeof(t_->kkp));
}

void Evaluator::initRandom() {
  Random random;
  for (int i = 0; i < KPP_ALL; i++) {
    t_->kpp[0][i] = (int16_t)(random.getInt32() % 21) - 10;
  }
  for (int i = 0; i < KKP_ALL; i++) {
    t_->kkp[0][0][i] = (int16_t)(random.getInt32() % 21) - 10;
  }
}

/**
 * ファイルからパラメータを読み込みます。
 */
bool Evaluator::readFile() {
  return Feature<int16_t>::readFile(DEFAULT_FV_FILENAME);
}

/**
 * ファイルにパラメータを書き出します。
 */
bool Evaluator::writeFile() const {
  return Feature<int16_t>::writeFile(DEFAULT_FV_FILENAME);
}

/**
 * fv.bin があれば読み込んで並べ替えを行います。
 */
bool Evaluator::convertFromFvBin() {

  std::shared_ptr<Table> fvbin_ptr = readFvBin();

  if (!fvbin_ptr) {
    return false;
  }

  convertFromFvBin(fvbin_ptr.get());

  return true;

}

std::shared_ptr<Evaluator::Table> Evaluator::readFvBin() {
  std::ifstream file(FVBIN_FILENAME, std::ios::binary | std::ios::in);

  if (!file) {
    return std::shared_ptr<Table>();
  }

  std::shared_ptr<Table> fvbin_ptr = std::make_shared<Table>();
  auto fvbin = fvbin_ptr.get();

  file.read((char*)fvbin->kpp, sizeof(t_->kpp));
  file.read((char*)fvbin->kkp, sizeof(t_->kkp));

  file.close();

  return fvbin_ptr;

}

void Evaluator::convertFromFvBin(Table* fvbin) {

#define CONV_ERROR_CHECK 1

#if CONV_ERROR_CHECK
  for (int i = 0; i < KPP_ALL; i++) {
    t_->kpp[0][i] = (int16_t)0x7fff;
  }
  for (int i = 0; i < KKP_ALL; i++) {
    t_->kkp[0][0][i] = (int16_t)0x7fff;
  }
#endif

  // king-piece-piece
  POSITION_EACH(kingPos) {
    int king = kingPos;
    int bona = posS2B[king];

    for (int x = 0; x < KPP_MAX; x++) {
      int bx = convertKppIndex4FvBin(x);
      for (int y = 0; y <= x; y++) {
        int by = convertKppIndex4FvBin(y);
        int index = kpp_index(x, y);
        int bonaIndex = bx >= by ? kpp_index(bx, by) : kpp_index(by, bx);
        t_->kpp[king][index] = fvbin->kpp[bona][bonaIndex];
      }
    }
  }

  // king-king-piece
  POSITION_EACH(bkingPos) {
    POSITION_EACH(wkingPos) {
      int bking = bkingPos;
      int wking = wkingPos;
      int bbona = posS2B[bking];
      int wbona = posS2B[wking];

      for (int index = 0; index < KKP_MAX; index++) {
        int bonaIndex = convertKkpIndex4FvBin(index);
        t_->kkp[bking][wking][index] = fvbin->kkp[bbona][wbona][bonaIndex];
      }
    }
  }

#if CONV_ERROR_CHECK
  for (int i = 0; i < KPP_ALL; i++) {
    if (t_->kpp[0][i] == (int16_t)0x7fff) {
      Loggers::error << "error kpp " << i;
    }
  }
  for (int i = 0; i < KKP_ALL; i++) {
    if (t_->kkp[0][0][i] == (int16_t)0x7fff) {
      Loggers::error << "error kkp " << i;
    }
  }
#endif

}

/**
 * 局面の駒割りを算出します。
 * @param board
 */
Value Evaluator::evaluateMaterial_(const Board& board) const {
  Value material = Value::Zero;

  material += material::Pawn * board.getBPawn().count();
  material += material::Lance * board.getBLance().count();
  material += material::Knight * board.getBKnight().count();
  material += material::Silver * board.getBSilver().count();
  material += material::Gold * board.getBGold().count();
  material += material::Bishop * board.getBBishop().count();
  material += material::Rook * board.getBRook().count();
  material += material::Tokin * board.getBTokin().count();
  material += material::Pro_lance * board.getBProLance().count();
  material += material::Pro_knight * board.getBProKnight().count();
  material += material::Pro_silver * board.getBProSilver().count();
  material += material::Horse * board.getBHorse().count();
  material += material::Dragon * board.getBDragon().count();

  material -= material::Pawn * board.getWPawn().count();
  material -= material::Lance * board.getWLance().count();
  material -= material::Knight * board.getWKnight().count();
  material -= material::Silver * board.getWSilver().count();
  material -= material::Gold * board.getWGold().count();
  material -= material::Bishop * board.getWBishop().count();
  material -= material::Rook * board.getWRook().count();
  material -= material::Tokin * board.getWTokin().count();
  material -= material::Pro_lance * board.getWProLance().count();
  material -= material::Pro_knight * board.getWProKnight().count();
  material -= material::Pro_silver * board.getWProSilver().count();
  material -= material::Horse * board.getWHorse().count();
  material -= material::Dragon * board.getWDragon().count();

  material += material::Pawn * board.getBlackHand(Piece::Pawn);
  material += material::Lance * board.getBlackHand(Piece::Lance);
  material += material::Knight * board.getBlackHand(Piece::Knight);
  material += material::Silver * board.getBlackHand(Piece::Silver);
  material += material::Gold * board.getBlackHand(Piece::Gold);
  material += material::Bishop * board.getBlackHand(Piece::Bishop);
  material += material::Rook * board.getBlackHand(Piece::Rook);

  material -= material::Pawn * board.getWhiteHand(Piece::Pawn);
  material -= material::Lance * board.getWhiteHand(Piece::Lance);
  material -= material::Knight * board.getWhiteHand(Piece::Knight);
  material -= material::Silver * board.getWhiteHand(Piece::Silver);
  material -= material::Gold * board.getWhiteHand(Piece::Gold);
  material -= material::Bishop * board.getWhiteHand(Piece::Bishop);
  material -= material::Rook * board.getWhiteHand(Piece::Rook);

  return material;
}

/**
 * 局面の評価値を算出します。
 * @param board
 */
Value Evaluator::evaluate_(const Board& board) {
  Value positional = extract<int32_t, false>(board, 0);

#if ENABLE_HASHTABLE
  evaluateCache_.set(board.getNoTurnHash(), positional);
#endif

  return positional;
}

/**
 * 指定した指し手による評価値の変化値を算出します。
 * @param board 着手後の局面を指定します。
 * @param prevValuePair
 * @param move
 */
template <bool black>
ValuePair Evaluator::evaluateDiff_(const Board& board, const ValuePair& prevValuePair, const Move& move) {

  Value material = prevValuePair.material();;
  Value positional;
  auto piece = move.piece();
  auto captured = move.captured();

  assert(board.isBlack() != black);

  // ハッシュ表から引く
#if ENABLE_HASHTABLE
  if (evaluateCache_.get(board.getNoTurnHash(), positional)) {
    if (!captured.isEmpty()) {
      if (black) {
        material += material::pieceExchange(captured);
      } else {
        material -= material::pieceExchange(captured);
      }
    }
    if (move.promote()) {
      if (black) {
        material += material::piecePromote(piece);
      } else {
        material -= material::piecePromote(piece);
      }
    }
    return ValuePair(material, positional);
  }
#endif

  // 玉の移動の場合は差分計算不可
  if (!ENABLE_DIFF || piece == Piece::King) {
    if (!captured.isEmpty()) {
      if (black) {
        material += material::pieceExchange(captured);
      } else {
        material -= material::pieceExchange(captured);
      }
    }
#if !ENABLE_DIFF
    if (move.promote()) {
      if (black) {
        material += material::piecePromote(piece);
      } else {
        material -= material::piecePromote(piece);
      }
    }
#endif
    return ValuePair(material, evaluate_(board));
  }

  positional = prevValuePair.positional();
  bool isHand = move.isHand();
  auto to = move.to();
  bool isProm = move.promote();

  auto bking = board.getBKingPosition();
  auto wking = board.getWKingPosition();
  auto bkingR = bking.reverse();
  auto wkingR = wking.reverse();

  int kppIndexFromB;
  int kppIndexFromW;
  int kppIndexToB;
  int kppIndexToW;
  int kppIndexCapturedB;
  int kppIndexCapturedW;
  int kppIndexCapHandB;
  int kppIndexCapHandW;

  assert(piece != Piece::King);

  // 移動元
  if (isHand) {
    // 持ち駒を打った場合
    if (black) {
      int num = board.getBlackHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional += t_->kkp[bking][wking][kkpIndex];
      positional -= t_->kkp[bking][wking][kkpIndex+1];
      kppIndexFromB = kppHandIndex<true>(piece) + num;
      kppIndexFromW = kppHandIndex<false>(piece) + num;
    } else {
      int num = board.getWhiteHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional -= t_->kkp[wkingR][bkingR][kkpIndex];
      positional += t_->kkp[wkingR][bkingR][kkpIndex+1];
      kppIndexFromB = kppHandIndex<false>(piece) + num;
      kppIndexFromW = kppHandIndex<true>(piece) + num;
    }
  } else {
    // 盤上の駒を動かした場合
    auto from = move.from();
    if (black) {
      positional -= t_->kkp[bking][wking][kkpBoardIndex(piece, from)];
      kppIndexFromB = kppBoardIndex<true>(piece, from);
      kppIndexFromW = kppBoardIndex<false>(piece, from.reverse());
    } else {
      positional += t_->kkp[wkingR][bkingR][kkpBoardIndex(piece, from.reverse())];
      kppIndexFromB = kppBoardIndex<false>(piece, from);
      kppIndexFromW = kppBoardIndex<true>(piece, from.reverse());
    }
  }

  // 移動先
  if (isProm) {
    // 駒が成った場合
    if (black) {
      material += material::piecePromote(piece);
      auto promoted = piece.promote();
      positional += t_->kkp[bking][wking][kkpBoardIndex(promoted, to)];
      kppIndexToB = kppBoardIndex<true>(promoted, to);
      kppIndexToW = kppBoardIndex<false>(promoted, to.reverse());
    } else {
      material -= material::piecePromote(piece);
      auto promoted = piece.promote();
      positional -= t_->kkp[wkingR][bkingR][kkpBoardIndex(promoted, to.reverse())];
      kppIndexToB = kppBoardIndex<false>(promoted, to);
      kppIndexToW = kppBoardIndex<true>(promoted, to.reverse());
    }
  } else {
    // 成らなかった場合
    if (black) {
      positional += t_->kkp[bking][wking][kkpBoardIndex(piece, to)];
      kppIndexToB = kppBoardIndex<true>(piece, to);
      kppIndexToW = kppBoardIndex<false>(piece, to.reverse());
    } else {
      positional -= t_->kkp[wkingR][bkingR][kkpBoardIndex(piece, to.reverse())];
      kppIndexToB = kppBoardIndex<false>(piece, to);
      kppIndexToW = kppBoardIndex<true>(piece, to.reverse());
    }
  }

  // 駒を取った場合
  if (!captured.isEmpty()) {
    if (black) {
      material += material::pieceExchange(captured);
      positional += t_->kkp[wkingR][bkingR][kkpBoardIndex(captured, to.reverse())];
      kppIndexCapturedB = kppBoardIndex<false>(captured, to);
      kppIndexCapturedW = kppBoardIndex<true>(captured, to.reverse());
    } else {
      material -= material::pieceExchange(captured);
      positional -= t_->kkp[bking][wking][kkpBoardIndex(captured, to)];
      kppIndexCapturedB = kppBoardIndex<true>(captured, to);
      kppIndexCapturedW = kppBoardIndex<false>(captured, to.reverse());
    }
    auto hand = captured.hand();
    if (black) {
      int num = board.getBlackHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional += t_->kkp[bking][wking][kkpIndex];
      positional -= t_->kkp[bking][wking][kkpIndex-1];
      kppIndexCapHandB = kppHandIndex<true>(hand) + num;
      kppIndexCapHandW = kppHandIndex<false>(hand) + num;
    } else {
      int num = board.getWhiteHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional -= t_->kkp[wkingR][bkingR][kkpIndex];
      positional += t_->kkp[wkingR][bkingR][kkpIndex-1];
      kppIndexCapHandB = kppHandIndex<false>(hand) + num;
      kppIndexCapHandW = kppHandIndex<true>(hand) + num;
    }
  }

  int num = 14;
  int bList[52]; // 52 = 40(総駒数) - 2(玉) + 14(駒台)
  int wList[52];
  int nTemp;
  int wTemp[34]; // 34 = 18(と金) + 16(成香, 成桂, 成銀, 金)

#define ON_HAND(piece, pieceL, index) { \
  int count = board.getBlackHand(Piece::piece); \
  bList[index] = KPP_HB ## pieceL + count; \
  wList[index+1] = KPP_HW ## pieceL + count; \
  count = board.getWhiteHand(Piece::piece); \
  bList[index+1] = KPP_HW ## pieceL + count; \
  wList[index] = KPP_HB ## pieceL + count; \
}

  ON_HAND(Pawn, PAWN, 0);
  ON_HAND(Lance, LANCE, 2);
  ON_HAND(Knight, KNIGHT, 4);
  ON_HAND(Silver, SILVER, 6);
  ON_HAND(Gold, GOLD, 8);
  ON_HAND(Bishop, BISHOP, 10);
  ON_HAND(Rook, ROOK, 12);

#undef ON_HAND

#define ON_BOARD(blackBB, whiteBB, pieceL, POS_INDEX_B, POS_INDEX_W) { \
  nTemp = 0; \
  auto bb = (blackBB); \
  BB_EACH_OPE(pos, bb, { \
    bList[num++] = KPP_BB ## pieceL + POS_INDEX_B(pos); \
    wTemp[nTemp++] = KPP_BW ## pieceL + POS_INDEX_W(pos.reverse()); \
  }); \
  bb = (whiteBB); \
  BB_EACH_OPE(pos, bb, { \
    bList[num++] = KPP_BW ## pieceL + POS_INDEX_W(pos); \
    wTemp[nTemp++] = KPP_BB ## pieceL + POS_INDEX_B(pos.reverse()); \
  }); \
  for (int i = 0; i < nTemp; i++) { wList[num-i-1] = wTemp[i]; } \
}

  ON_BOARD(board.getBPawn(), board.getWPawn(), PAWN, POS_INDEX_BPAWN, POS_INDEX_WPAWN);
  ON_BOARD(board.getBLance(),board.getWLance(), LANCE, POS_INDEX_BPAWN, POS_INDEX_WPAWN);
  ON_BOARD(board.getBKnight(), board.getWKnight(), KNIGHT, POS_INDEX_BKNIGHT, POS_INDEX_WKNIGHT);
  ON_BOARD(board.getBSilver(), board.getWSilver(), SILVER, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBGold() | board.getBTokin() | board.getBProLance() | board.getBProKnight() | board.getBProSilver(),
           board.getWGold() | board.getWTokin() | board.getWProLance() | board.getWProKnight() | board.getWProSilver(), GOLD,
           POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBBishop(), board.getWBishop(), BISHOP, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBHorse(), board.getWHorse(), HORSE, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBRook(), board.getWRook(), ROOK, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);
  ON_BOARD(board.getBDragon(), board.getWDragon(), DRAGON, POS_INDEX_BNORMAL, POS_INDEX_WNORMAL);

#undef ON_BOARD

#if ENABLE_KPP
  if (isHand) {
    // 持ち駒を打った場合
    for (int i = 0; i < num; i++) {
      int b = bList[i];
      int w = wList[i];
      // 持ち駒の現在の数
      positional += t_->kpp[bking][kpp_index_safe(b, kppIndexFromB)];
      positional -= t_->kpp[wkingR][kpp_index_safe(w, kppIndexFromW)];
      // 持ち駒の元の数
      positional -= t_->kpp[bking][kpp_index_safe(b, kppIndexFromB+1)];
      positional += t_->kpp[wkingR][kpp_index_safe(w, kppIndexFromW+1)];
      // 移動先の駒
      positional += t_->kpp[bking][kpp_index_safe(b, kppIndexToB)];
      positional -= t_->kpp[wkingR][kpp_index_safe(w, kppIndexToW)];
      // 現在の局面では存在しないはずのインデクス
      assert(b != kppIndexFromB+1);
      assert(w != kppIndexFromW+1);
    }
    // 2重に足している特徴の組み合わせ(ループ内で足しすぎているので引く)
    positional -= t_->kpp[bking][kpp_index_safe(kppIndexFromB, kppIndexToB)];
    positional += t_->kpp[wkingR][kpp_index_safe(kppIndexFromW, kppIndexToW)];
    // 前の局面に存在しなかった特徴の組み合わせ(ループ内で引きすぎている分を足す)
    positional += t_->kpp[bking][kpp_index(kppIndexFromB+1, kppIndexFromB)];
    positional += t_->kpp[bking][kpp_index_safe(kppIndexFromB+1, kppIndexToB)];
    positional -= t_->kpp[wkingR][kpp_index(kppIndexFromW+1, kppIndexFromW)];
    positional -= t_->kpp[wkingR][kpp_index_safe(kppIndexFromW+1, kppIndexToW)];
    // 前の局面にしか存在しない特徴の組み合わせ(現局面には存在しないので引く)
    positional -= t_->kpp[bking][kpp_index(kppIndexFromB+1, kppIndexFromB+1)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexFromW+1, kppIndexFromW+1)];
  } else if (captured.isEmpty()) {
    // 駒を取らずに盤上の駒を移動した場合
    for (int i = 0; i < num; i++) {
      int b = bList[i];
      int w = wList[i];
      // 移動元の駒
      positional -= t_->kpp[bking][kpp_index_safe(b, kppIndexFromB)];
      positional += t_->kpp[wkingR][kpp_index_safe(w, kppIndexFromW)];
      // 移動先の駒
      positional += t_->kpp[bking][kpp_index_safe(b, kppIndexToB)];
      positional -= t_->kpp[wkingR][kpp_index_safe(w, kppIndexToW)];
      // 現在の局面では存在しないはずのインデクス
      assert(b != kppIndexFromB);
      assert(w != kppIndexFromW);
    }
    // 前の局面に存在しなかった特徴の組み合わせ(ループ内で引きすぎている分を足す)
    positional += t_->kpp[bking][kpp_index_safe(kppIndexFromB, kppIndexToB)];
    positional -= t_->kpp[wkingR][kpp_index_safe(kppIndexFromW, kppIndexToW)];
    // 前の局面にしか存在しない特徴の組み合わせ(現局面には存在しないので引く)
    positional -= t_->kpp[bking][kpp_index(kppIndexFromB, kppIndexFromB)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexFromW, kppIndexFromW)];
  } else {
    // 駒を取った場合
    for (int i = 0; i < num; i++) {
      int b = bList[i];
      int w = wList[i];
      // 移動元の駒
      positional -= t_->kpp[bking][kpp_index_safe(b, kppIndexFromB)];
      positional += t_->kpp[wkingR][kpp_index_safe(w, kppIndexFromW)];
      // 移動先の駒
      positional += t_->kpp[bking][kpp_index_safe(b, kppIndexToB)];
      positional -= t_->kpp[wkingR][kpp_index_safe(w, kppIndexToW)];
      // 取られた駒
      positional -= t_->kpp[bking][kpp_index_safe(b, kppIndexCapturedB)];
      positional += t_->kpp[wkingR][kpp_index_safe(w, kppIndexCapturedW)];
      // 持ち駒の現在の数
      positional += t_->kpp[bking][kpp_index_safe(b, kppIndexCapHandB)];
      positional -= t_->kpp[wkingR][kpp_index_safe(w, kppIndexCapHandW)];
      // 持ち駒の元の数
      positional -= t_->kpp[bking][kpp_index_safe(b, kppIndexCapHandB-1)];
      positional += t_->kpp[wkingR][kpp_index_safe(w, kppIndexCapHandW-1)];
      // 現在の局面では存在しないはずのインデクス
      assert(b != kppIndexFromB);
      assert(w != kppIndexFromW);
      assert(b != kppIndexCapturedB);
      assert(w != kppIndexCapturedW);
      assert(b != kppIndexCapHandB-1);
      assert(w != kppIndexCapHandW-1);
    }
    // 2重に足している特徴の組み合わせ(ループ内で足しすぎているので引く)
    positional -= t_->kpp[bking][kpp_index(kppIndexToB, kppIndexCapHandB)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexToW, kppIndexCapHandW)];
    // 前の局面に存在しなかった特徴の組み合わせ(ループ内で引きすぎている分を足す)
    positional += t_->kpp[bking][kpp_index_safe(kppIndexFromB, kppIndexToB)];
    positional += t_->kpp[bking][kpp_index(kppIndexFromB, kppIndexCapHandB)];
    positional += t_->kpp[bking][kpp_index_safe(kppIndexCapturedB, kppIndexToB)];
    positional += t_->kpp[bking][kpp_index(kppIndexCapturedB, kppIndexCapHandB)];
    positional += t_->kpp[bking][kpp_index(kppIndexToB, kppIndexCapHandB-1)];
    positional += t_->kpp[bking][kpp_index(kppIndexCapHandB, kppIndexCapHandB-1)];
    positional -= t_->kpp[wkingR][kpp_index_safe(kppIndexFromW, kppIndexToW)];
    positional -= t_->kpp[wkingR][kpp_index(kppIndexFromW, kppIndexCapHandW)];
    positional -= t_->kpp[wkingR][kpp_index_safe(kppIndexCapturedW, kppIndexToW)];
    positional -= t_->kpp[wkingR][kpp_index(kppIndexCapturedW, kppIndexCapHandW)];
    positional -= t_->kpp[wkingR][kpp_index(kppIndexToW, kppIndexCapHandW-1)];
    positional -= t_->kpp[wkingR][kpp_index(kppIndexCapHandW, kppIndexCapHandW-1)];
    // 前の局面にしか存在しない特徴の組み合わせ(現局面には存在しないので引く)
    positional -= t_->kpp[bking][kpp_index(kppIndexFromB, kppIndexFromB)];
    positional -= t_->kpp[bking][kpp_index_safe(kppIndexFromB, kppIndexCapturedB)];
    positional -= t_->kpp[bking][kpp_index(kppIndexFromB, kppIndexCapHandB-1)];
    positional -= t_->kpp[bking][kpp_index(kppIndexCapturedB, kppIndexCapturedB)];
    positional -= t_->kpp[bking][kpp_index(kppIndexCapturedB, kppIndexCapHandB-1)];
    positional -= t_->kpp[bking][kpp_index(kppIndexCapHandB-1, kppIndexCapHandB-1)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexFromW, kppIndexFromW)];
    positional += t_->kpp[wkingR][kpp_index_safe(kppIndexFromW, kppIndexCapturedW)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexFromW, kppIndexCapHandW-1)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexCapturedW, kppIndexCapturedW)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexCapturedW, kppIndexCapHandW-1)];
    positional += t_->kpp[wkingR][kpp_index(kppIndexCapHandW-1, kppIndexCapHandW-1)];
  }
#endif // ENABLE_KPP

#if ENABLE_HASHTABLE
  evaluateCache_.set(board.getNoTurnHash(), positional);
#endif

  return ValuePair(material, positional);

}
template ValuePair Evaluator::evaluateDiff_<true>(const Board&, const ValuePair&, const Move&);
template ValuePair Evaluator::evaluateDiff_<false>(const Board&, const ValuePair&, const Move&);

/**
 * 評価値の変化を推定します。
 * @param board
 * @param move
 */
template <bool black, bool isKing, bool positionalOnly>
Value Evaluator::estimate_(const Board& board, const Move& move) {

  Value value;

#if ENABLE_HASHTABLE
  uint64_t hash = board.getHash() ^ (uint64_t)Move::serialize(move);
  if (estimateCache_.get(hash, value)) {
    return value;
  }
#endif

  Value material = 0;
  Value positional = 0;
  auto piece = move.piece();
  auto captured = board.getBoardPiece(move.to());

  assert(board.isBlack() == black);

  bool isHand = move.isHand();
  auto to = move.to();
  bool isProm = move.promote();

  auto bking = board.getBKingPosition();
  auto wking = board.getWKingPosition();
  auto bkingR = bking.reverse();
  auto wkingR = wking.reverse();

  // 移動元
  if (!isKing && isHand) {
    // 持ち駒を打った場合
    if (black) {
      int num = board.getBlackHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional += t_->kkp[bking][wking][kkpIndex];
      positional -= t_->kkp[bking][wking][kkpIndex+1];
      int kppIndexB = kppHandIndex<true>(piece) + num;
      positional += t_->kpp[bking][kpp_index(kppIndexB)];
      positional -= t_->kpp[bking][kpp_index(kppIndexB+1)];
      int kppIndexW = kppHandIndex<false>(piece) + num;
      positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
      positional += t_->kpp[wkingR][kpp_index(kppIndexW+1)];
    } else {
      int num = board.getWhiteHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional -= t_->kkp[wkingR][bkingR][kkpIndex];
      positional += t_->kkp[wkingR][bkingR][kkpIndex+1];
      int kppIndexB = kppHandIndex<false>(piece) + num;
      positional += t_->kpp[bking][kpp_index(kppIndexB)];
      positional -= t_->kpp[bking][kpp_index(kppIndexB+1)];
      int kppIndexW = kppHandIndex<true>(piece) + num;
      positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
      positional += t_->kpp[wkingR][kpp_index(kppIndexW+1)];
    }
  } else {
    // 盤上の駒を動かした場合
    auto from = move.from();
    if (black) {
      if (!isKing) {
        positional -= t_->kkp[bking][wking][kkpBoardIndex(piece, from)];
        int kppIndexB = kppBoardIndex<true>(piece, from);
        positional += t_->kpp[bking][kpp_index(kppIndexB)];
        int kppIndexW = kppBoardIndex<false>(piece, from.reverse());
        positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
      }
    } else {
      if (!isKing) {
        positional += t_->kkp[wkingR][bkingR][kkpBoardIndex(piece, from.reverse())];
        int kppIndexB = kppBoardIndex<false>(piece, from);
        positional += t_->kpp[bking][kpp_index(kppIndexB)];
        int kppIndexW = kppBoardIndex<true>(piece, from.reverse());
        positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
      }
    }
  }

  // 移動先
  if (!isKing && isProm) {
    // 駒が成った場合
    if (black) {
      if (!positionalOnly) { material += material::piecePromote(piece); }
      auto promoted = piece.promote();
      positional += t_->kkp[bking][wking][kkpBoardIndex(promoted, to)];
      int kppIndexB = kppBoardIndex<true>(promoted, to);
      positional += t_->kpp[bking][kpp_index(kppIndexB)];
      int kppIndexW = kppBoardIndex<false>(promoted, to.reverse());
      positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
    } else {
      if (!positionalOnly) { material -= material::piecePromote(piece); }
      auto promoted = piece.promote();
      positional -= t_->kkp[wkingR][bkingR][kkpBoardIndex(promoted, to.reverse())];
      int kppIndexB = kppBoardIndex<false>(promoted, to);
      positional -= t_->kpp[bking][kpp_index(kppIndexB)];
      int kppIndexW = kppBoardIndex<true>(promoted, to.reverse());
      positional += t_->kpp[wkingR][kpp_index(kppIndexW)];
    }
  } else {
    // 成らなかった場合
    if (black) {
      if (!isKing) {
        positional += t_->kkp[bking][wking][kkpBoardIndex(piece, to)];
        int kppIndexB = kppBoardIndex<true>(piece, to);
        positional += t_->kpp[bking][kpp_index(kppIndexB)];
        int kppIndexW = kppBoardIndex<false>(piece, to.reverse());
        positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
      }
    } else {
      if (!isKing) {
        positional -= t_->kkp[wkingR][bkingR][kkpBoardIndex(piece, to.reverse())];
        int kppIndexB = kppBoardIndex<false>(piece, to);
        positional -= t_->kpp[bking][kpp_index(kppIndexB)];
        int kppIndexW = kppBoardIndex<true>(piece, to.reverse());
        positional += t_->kpp[wkingR][kpp_index(kppIndexW)];
      }
    }
  }

  // 駒を取った場合
  if (!captured.isEmpty()) {
    if (black) {
      if (!positionalOnly) { material += material::pieceExchange(captured); }
      positional += t_->kkp[wkingR][bkingR][kkpBoardIndex(captured, to.reverse())];
      int kppIndexB = kppBoardIndex<false>(captured, to);
      positional += t_->kpp[bking][kpp_index(kppIndexB)];
      int kppIndexW = kppBoardIndex<true>(captured, to.reverse());
      positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
    } else {
      if (!positionalOnly) { material -= material::pieceExchange(captured); }
      positional -= t_->kkp[bking][wking][kkpBoardIndex(captured, to)];
      int kppIndexB = kppBoardIndex<true>(captured, to);
      positional -= t_->kpp[bking][kpp_index(kppIndexB)];
      int kppIndexW = kppBoardIndex<false>(captured, to.reverse());
      positional += t_->kpp[wkingR][kpp_index(kppIndexW)];
    }
    auto hand = captured.hand();
    if (black) {
      int num = board.getBlackHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional += t_->kkp[bking][wking][kkpIndex];
      positional -= t_->kkp[bking][wking][kkpIndex-1];
      int kppIndexB = kppHandIndex<true>(hand) + num;
      positional += t_->kpp[bking][kpp_index(kppIndexB)];
      positional -= t_->kpp[bking][kpp_index(kppIndexB-1)];
      int kppIndexW = kppHandIndex<false>(hand) + num;
      positional -= t_->kpp[wkingR][kpp_index(kppIndexW)];
      positional += t_->kpp[wkingR][kpp_index(kppIndexW-1)];
    } else {
      int num = board.getWhiteHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional -= t_->kkp[wkingR][bkingR][kkpIndex];
      positional += t_->kkp[wkingR][bkingR][kkpIndex-1];
      int kppIndexB = kppHandIndex<false>(hand) + num;
      positional -= t_->kpp[bking][kpp_index(kppIndexB)];
      positional += t_->kpp[bking][kpp_index(kppIndexB-1)];
      int kppIndexW = kppHandIndex<true>(hand) + num;
      positional += t_->kpp[wkingR][kpp_index(kppIndexW)];
      positional -= t_->kpp[wkingR][kpp_index(kppIndexW-1)];
    }
  }

  auto valuePair = ValuePair(material, positional);
  value = black ? valuePair.value() : -valuePair.value();

#if ENABLE_HASHTABLE
  estimateCache_.set(hash, value);
#endif

  return value;
}
template Value Evaluator::estimate_<true, true, true>(const Board& board, const Move& move);
template Value Evaluator::estimate_<true, false, true>(const Board& board, const Move& move);
template Value Evaluator::estimate_<false, true, true>(const Board& board, const Move& move);
template Value Evaluator::estimate_<false, false, true>(const Board& board, const Move& move);
template Value Evaluator::estimate_<true, true, false>(const Board& board, const Move& move);
template Value Evaluator::estimate_<true, false, false>(const Board& board, const Move& move);
template Value Evaluator::estimate_<false, true, false>(const Board& board, const Move& move);
template Value Evaluator::estimate_<false, false, false>(const Board& board, const Move& move);

template class Feature<int16_t>;
template class Feature<float>;

} // namespace sunfish
