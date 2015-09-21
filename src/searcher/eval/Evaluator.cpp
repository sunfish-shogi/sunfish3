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
#define ENABLE_HASHTABLE    1

namespace {

using namespace sunfish;

const int8_t sqIndexBPawn[] = {
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

const int8_t sqIndexWPawn[] = {
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

const int8_t sqIndexBKnight[] = {
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

const int8_t sqIndexWKnight[] = {
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
const int8_t sqS2B[] = {
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

struct TableInfo {
  const int8_t* table;
  int begin;
  int end;
  int bonaOffset;
};

const TableInfo kkpTableInfo[] = {
  { sqIndexBPawn  , KKP_BPAWN,   KKP_BLANCE,   -9 },
  { sqIndexBPawn  , KKP_BLANCE,  KKP_BKNIGHT,  -9 },
  { sqIndexBKnight, KKP_BKNIGHT, KKP_BSILVER, -18 },
  { nullptr       , KKP_BSILVER, KKP_BGOLD,     0 },
  { nullptr       , KKP_BGOLD,   KKP_BBISHOP,   0 },
  { nullptr       , KKP_BBISHOP, KKP_BHORSE,    0 },
  { nullptr       , KKP_BHORSE,  KKP_BROOK,     0 },
  { nullptr       , KKP_BROOK,   KKP_BDRAGON,   0 },
  { nullptr       , KKP_BDRAGON, KKP_MAX,       0 },
};

int sqInv(const int8_t* table, int in) {
  SQUARE_EACH(sq) {
    if (static_cast<int>(table[sq.index()]) == in) {
      return sq.index();
    }
  }
  return -1;
}

} // namespace

#define SQ_INDEX_BPAWN(sq)   (static_cast<int>(sqIndexBPawn[(sq).index()]))
#define SQ_INDEX_WPAWN(sq)   (static_cast<int>(sqIndexWPawn[(sq).index()]))
#define SQ_INDEX_BKNIGHT(sq) (static_cast<int>(sqIndexBKnight[(sq).index()]))
#define SQ_INDEX_WKNIGHT(sq) (static_cast<int>(sqIndexWKnight[(sq).index()]))
#define SQ_INDEX_BNORMAL(sq) (sq.index())
#define SQ_INDEX_WNORMAL(sq) (sq.index())

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

  file.write((const char*)t_->kkp, sizeof(t_->kkp));

  file.close();

  return true;
}

/**
 * KKP のインデクスを左右反転します。
 */
int symmetrizeKkpIndex(int index) {
  if (index < KKP_BPAWN) {
    return index;
  }

  for (unsigned i = 0; i < sizeof(kkpTableInfo)/sizeof(kkpTableInfo[0]); i++) {
    const auto& ti = kkpTableInfo[i];
    if (ti.begin <= index && index < ti.end) {
      int sq = index - ti.begin;
      assert(sq >= 0);
      assert(sq < 81);
      if (ti.table != nullptr) {
        sq = sqInv(ti.table, sq);
        assert(sq >= 0);
        assert(sq < 81);
      }
      sq = Square(sq).sym().index();
      assert(sq >= 0);
      assert(sq < 81);
      if (ti.table != nullptr) {
        sq = ti.table[sq];
        assert(sq >= 0);
        assert(sq < 81);
      }
      int result = ti.begin + sq;
      assert(ti.begin <= result);
      assert(result < ti.end);
      return result;
    }
  }
  assert(false);
  return 0;
}

/**
 * 盤上の駒の種類から KKP のインデクスを取得します。
 */
int kkpBoardIndex(Piece piece, const Square& sq) {
  switch (piece.index()) {
    case Piece::BPawn:      case Piece::WPawn:      return KKP_BPAWN + SQ_INDEX_BPAWN(sq);
    case Piece::BLance:     case Piece::WLance:     return KKP_BLANCE + SQ_INDEX_BPAWN(sq);
    case Piece::BKnight:    case Piece::WKnight:    return KKP_BKNIGHT + SQ_INDEX_BKNIGHT(sq);
    case Piece::BSilver:    case Piece::WSilver:    return KKP_BSILVER + sq.index();
    case Piece::BGold:      case Piece::WGold:      return KKP_BGOLD + sq.index();
    case Piece::BBishop:    case Piece::WBishop:    return KKP_BBISHOP + sq.index();
    case Piece::BRook:      case Piece::WRook:      return KKP_BROOK + sq.index();
    case Piece::BTokin:     case Piece::WTokin:     return KKP_BGOLD + sq.index();
    case Piece::BProLance:  case Piece::WProLance:  return KKP_BGOLD + sq.index();
    case Piece::BProKnight: case Piece::WProKnight: return KKP_BGOLD + sq.index();
    case Piece::BProSilver: case Piece::WProSilver: return KKP_BGOLD + sq.index();
    case Piece::BHorse:     case Piece::WHorse:     return KKP_BHORSE + sq.index();
    case Piece::BDragon:    case Piece::WDragon:    return KKP_BDRAGON + sq.index();
  }

  assert(false);
  return 0;

}

/**
 * 持ち駒の種類から KKP のインデクスを取得します。
 */
int kkpHandIndex(Piece piece) {
  switch (piece.index()) {
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

template <class T>
template <class U, bool update>
U Feature<T>::extract(const Board& board, U delta) {
  U positional = 0;
  auto bking = board.getBKingSquare();
  auto wking = board.getWKingSquare();
  auto bkingR = bking.reverse();
  auto wkingR = wking.reverse();

#define ON_HAND(piece, pieceL, i) { \
  int count = board.getBlackHand(Piece::piece); \
  if (update) { \
    t_->kkp[bking.index()][wking.index()][KKP_H ## pieceL+count] += ValueType(delta); \
  } else { \
    positional += t_->kkp[bking.index()][wking.index()][KKP_H ## pieceL+count]; \
  } \
  count = board.getWhiteHand(Piece::piece); \
  if (update) { \
    t_->kkp[wkingR.index()][bkingR.index()][KKP_H ## pieceL+count] -= ValueType(delta); \
  } else { \
    positional -= t_->kkp[wkingR.index()][bkingR.index()][KKP_H ## pieceL+count]; \
  } \
}

  ON_HAND(Pawn, PAWN, 0);
  ON_HAND(Lance, LANCE, 2);
  ON_HAND(Knight, KNIGHT, 4);
  ON_HAND(Silver, SILVER, 6);
  ON_HAND(Gold, GOLD, 8);
  ON_HAND(Bishop, BISHOP, 10);
  ON_HAND(Rook, ROOK, 12);

#undef ON_HAND

#define ON_BOARD(blackBB, whiteBB, pieceL, SQ_INDEX_B, SQ_INDEX_W) { \
  auto bb = (blackBB); \
  BB_EACH_OPE(sq, bb, { \
    if (update) { \
      t_->kkp[bking.index()][wking.index()][KKP_B ## pieceL+SQ_INDEX_B(sq)] += ValueType(delta); \
    } else { \
      positional += t_->kkp[bking.index()][wking.index()][KKP_B ## pieceL+SQ_INDEX_B(sq)]; \
    } \
  }); \
  bb = (whiteBB); \
  BB_EACH_OPE(sq, bb, { \
    if (update) { \
      t_->kkp[wkingR.index()][bkingR.index()][KKP_B ## pieceL+SQ_INDEX_B(sq.reverse())] -= ValueType(delta); \
    } else { \
      positional -= t_->kkp[wkingR.index()][bkingR.index()][KKP_B ## pieceL+SQ_INDEX_B(sq.reverse())]; \
    } \
  }); \
}

  ON_BOARD(board.getBPawn(), board.getWPawn(), PAWN, SQ_INDEX_BPAWN, SQ_INDEX_WPAWN);
  ON_BOARD(board.getBLance(),board.getWLance(), LANCE, SQ_INDEX_BPAWN, SQ_INDEX_WPAWN);
  ON_BOARD(board.getBKnight(), board.getWKnight(), KNIGHT, SQ_INDEX_BKNIGHT, SQ_INDEX_WKNIGHT);
  ON_BOARD(board.getBSilver(), board.getWSilver(), SILVER, SQ_INDEX_BNORMAL, SQ_INDEX_WNORMAL);
  ON_BOARD(board.getBGold() | board.getBTokin() | board.getBProLance() | board.getBProKnight() | board.getBProSilver(),
           board.getWGold() | board.getWTokin() | board.getWProLance() | board.getWProKnight() | board.getWProSilver(), GOLD,
           SQ_INDEX_BNORMAL, SQ_INDEX_WNORMAL);
  ON_BOARD(board.getBBishop(), board.getWBishop(), BISHOP, SQ_INDEX_BNORMAL, SQ_INDEX_WNORMAL);
  ON_BOARD(board.getBHorse(), board.getWHorse(), HORSE, SQ_INDEX_BNORMAL, SQ_INDEX_WNORMAL);
  ON_BOARD(board.getBRook(), board.getWRook(), ROOK, SQ_INDEX_BNORMAL, SQ_INDEX_WNORMAL);
  ON_BOARD(board.getBDragon(), board.getWDragon(), DRAGON, SQ_INDEX_BNORMAL, SQ_INDEX_WNORMAL);

#undef ON_BOARD

  return positional;
}

template int32_t Feature<float>::extract<int32_t, false>(const Board& board, int32_t delta);
template float Feature<float>::extract<float, true>(const Board& board, float delta);

Evaluator::Evaluator(InitType initType /*= InitType::File*/) {
  switch (initType) {
  case InitType::File:
    init();
    readFile();
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
  memset(t_->kkp, 0, sizeof(t_->kkp));
}

void Evaluator::initRandom() {
  Random random;
  for (int i = 0; i < KKP_ALL; i++) {
    ((ValueType*)t_->kkp)[i] = (ValueType)(random.getInt32() % 21) - 10;
  }
}

/**
 * ファイルからパラメータを読み込みます。
 */
bool Evaluator::readFile() {
  return Feature<ValueType>::readFile(DEFAULT_FV_FILENAME);
}

/**
 * ファイルにパラメータを書き出します。
 */
bool Evaluator::writeFile() const {
  return Feature<ValueType>::writeFile(DEFAULT_FV_FILENAME);
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

  auto bking = board.getBKingSquare();
  auto wking = board.getWKingSquare();
  auto bkingR = bking.reverse();
  auto wkingR = wking.reverse();

  assert(piece != Piece::King);

  // 移動元
  if (isHand) {
    // 持ち駒を打った場合
    if (black) {
      int num = board.getBlackHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional += t_->kkp[bking.index()][wking.index()][kkpIndex];
      positional -= t_->kkp[bking.index()][wking.index()][kkpIndex+1];
    } else {
      int num = board.getWhiteHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpIndex];
      positional += t_->kkp[wkingR.index()][bkingR.index()][kkpIndex+1];
    }
  } else {
    // 盤上の駒を動かした場合
    auto from = move.from();
    if (black) {
      positional -= t_->kkp[bking.index()][wking.index()][kkpBoardIndex(piece, from)];
    } else {
      positional += t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(piece, from.reverse())];
    }
  }

  // 移動先
  if (isProm) {
    // 駒が成った場合
    if (black) {
      material += material::piecePromote(piece);
      auto promoted = piece.promote();
      positional += t_->kkp[bking.index()][wking.index()][kkpBoardIndex(promoted, to)];
    } else {
      material -= material::piecePromote(piece);
      auto promoted = piece.promote();
      positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(promoted, to.reverse())];
    }
  } else {
    // 成らなかった場合
    if (black) {
      positional += t_->kkp[bking.index()][wking.index()][kkpBoardIndex(piece, to)];
    } else {
      positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(piece, to.reverse())];
    }
  }

  // 駒を取った場合
  if (!captured.isEmpty()) {
    if (black) {
      material += material::pieceExchange(captured);
      positional += t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(captured, to.reverse())];
    } else {
      material -= material::pieceExchange(captured);
      positional -= t_->kkp[bking.index()][wking.index()][kkpBoardIndex(captured, to)];
    }
    auto hand = captured.hand();
    if (black) {
      int num = board.getBlackHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional += t_->kkp[bking.index()][wking.index()][kkpIndex];
      positional -= t_->kkp[bking.index()][wking.index()][kkpIndex-1];
    } else {
      int num = board.getWhiteHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpIndex];
      positional += t_->kkp[wkingR.index()][bkingR.index()][kkpIndex-1];
    }
  }

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

  Value material = 0;
  Value positional = 0;
  auto piece = move.piece();
  auto captured = board.getBoardPiece(move.to());

  assert(board.isBlack() == black);

  bool isHand = move.isHand();
  auto to = move.to();
  bool isProm = move.promote();

  auto bking = board.getBKingSquare();
  auto wking = board.getWKingSquare();
  auto bkingR = bking.reverse();
  auto wkingR = wking.reverse();

  // 移動元
  if (!isKing && isHand) {
    // 持ち駒を打った場合
    if (black) {
      int num = board.getBlackHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional += t_->kkp[bking.index()][wking.index()][kkpIndex];
      positional -= t_->kkp[bking.index()][wking.index()][kkpIndex+1];
    } else {
      int num = board.getWhiteHand(piece);
      int kkpIndex = kkpHandIndex(piece) + num;
      positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpIndex];
      positional += t_->kkp[wkingR.index()][bkingR.index()][kkpIndex+1];
    }
  } else {
    // 盤上の駒を動かした場合
    auto from = move.from();
    if (black) {
      if (!isKing) {
        positional -= t_->kkp[bking.index()][wking.index()][kkpBoardIndex(piece, from)];
      }
    } else {
      if (!isKing) {
        positional += t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(piece, from.reverse())];
      }
    }
  }

  // 移動先
  if (!isKing && isProm) {
    // 駒が成った場合
    if (black) {
      if (!positionalOnly) { material += material::piecePromote(piece); }
      auto promoted = piece.promote();
      positional += t_->kkp[bking.index()][wking.index()][kkpBoardIndex(promoted, to)];
    } else {
      if (!positionalOnly) { material -= material::piecePromote(piece); }
      auto promoted = piece.promote();
      positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(promoted, to.reverse())];
    }
  } else {
    // 成らなかった場合
    if (black) {
      if (!isKing) {
        positional += t_->kkp[bking.index()][wking.index()][kkpBoardIndex(piece, to)];
      }
    } else {
      if (!isKing) {
        positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(piece, to.reverse())];
      }
    }
  }

  // 駒を取った場合
  if (!captured.isEmpty()) {
    if (black) {
      if (!positionalOnly) { material += material::pieceExchange(captured); }
      positional += t_->kkp[wkingR.index()][bkingR.index()][kkpBoardIndex(captured, to.reverse())];
    } else {
      if (!positionalOnly) { material -= material::pieceExchange(captured); }
      positional -= t_->kkp[bking.index()][wking.index()][kkpBoardIndex(captured, to)];
    }
    auto hand = captured.hand();
    if (black) {
      int num = board.getBlackHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional += t_->kkp[bking.index()][wking.index()][kkpIndex];
      positional -= t_->kkp[bking.index()][wking.index()][kkpIndex-1];
    } else {
      int num = board.getWhiteHand(hand);
      int kkpIndex = kkpHandIndex(hand) + num;
      positional -= t_->kkp[wkingR.index()][bkingR.index()][kkpIndex];
      positional += t_->kkp[wkingR.index()][bkingR.index()][kkpIndex-1];
    }
  }

  auto valuePair = ValuePair(material, positional);
  value = black ? valuePair.value() : -valuePair.value();

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
