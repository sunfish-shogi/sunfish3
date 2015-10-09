/* FV.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_FV__
#define SUNFISH_FV__

#include "core/def.h"
#include "core/board/Board.h"
#include "searcher/eval/Evaluator.h"

namespace sunfish {

struct FVM {
  float pawn;
  float lance;
  float knight;
  float silver;
  float gold;
  float bishop;
  float rook;
  float tokin;
  float pro_lance;
  float pro_knight;
  float pro_silver;
  float horse;
  float dragon;

  void init() {
    pawn = 0.0f;
    lance = 0.0f;
    knight = 0.0f;
    silver = 0.0f;
    gold = 0.0f;
    bishop = 0.0f;
    rook = 0.0f;
    tokin = 0.0f;
    pro_lance = 0.0f;
    pro_knight = 0.0f;
    pro_silver = 0.0f;
    horse = 0.0f;
    dragon = 0.0f;
  }

  float* ref(const Piece& piece) {
    switch (piece.index()) {
      case Piece::BPawn     : case Piece::WPawn     : return &pawn;
      case Piece::BLance    : case Piece::WLance    : return &lance;
      case Piece::BKnight   : case Piece::WKnight   : return &knight;
      case Piece::BSilver   : case Piece::WSilver   : return &silver;
      case Piece::BGold     : case Piece::WGold     : return &gold;
      case Piece::BBishop   : case Piece::WBishop   : return &bishop;
      case Piece::BRook     : case Piece::WRook     : return &rook;
      case Piece::BTokin    : case Piece::WTokin    : return &tokin;
      case Piece::BProLance : case Piece::WProLance : return &pro_lance;
      case Piece::BProKnight: case Piece::WProKnight: return &pro_knight;
      case Piece::BProSilver: case Piece::WProSilver: return &pro_silver;
      case Piece::BHorse    : case Piece::WHorse    : return &horse;
      case Piece::BDragon   : case Piece::WDragon   : return &dragon;
    }
    return nullptr;
  }

  void extract(const Board& board, float g) {
    SQUARE_EACH(sq) {
      Piece piece = board.getBoardPiece(sq);
      float* p = ref(piece);
      if (p != nullptr) {
        if (piece.isBlack()) {
          *p += g;
        } else {
          *p -= g;
        }
      }
    }

    HAND_EACH(piece) {
      int num = board.getBlackHand(piece);
      *ref(piece) += g * num;
    }

    HAND_EACH(piece) {
      int num = board.getWhiteHand(piece);
      *ref(piece) -= g * num;
    }
  }
};

class FV : public Feature<float> {
public:
  void init() {
    memset(t_, 0, sizeof(*t_));
  }
};

template<class T>
class FeatureX {
public:

  using ValueType = T;

  struct Table {
    ValueType kpb[81][KKP_MAX];
    ValueType kpw[81][KKP_MAX];
  };

  Table* t_;

  static CONSTEXPR size_t size() {
    return sizeof(Table) / sizeof(ValueType);
  }

protected:

  FeatureX() : t_(nullptr) {
    t_ = new Table();
  }

  FeatureX(const FeatureX&) = delete;
  FeatureX(FeatureX&&) = delete;

  ~FeatureX() {
    delete t_;
  }

};

class EvaluatorX : public FeatureX<int16_t> {
public:
  void init() {
    memset(t_, 0, sizeof(*t_));
  }
};

class FVX : public FeatureX<float> {
public:
  void init() {
    memset(t_, 0, sizeof(*t_));
  }
};

} // namespace sunfish

#endif // SUNFISH_FV__
