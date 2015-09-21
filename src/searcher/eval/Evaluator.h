/* Evaluator.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_EVALUATOR__
#define SUNFISH_EVALUATOR__

#include "EvaluateTable.h"
#include "Value.h"
#include "core/board/Board.h"
#include "Material.h"
#include <memory>

namespace sunfish {

class ValuePair {
public:

  static CONSTEXPR_CONST int PositionalScale = 32;

private:

  Value material_;
  Value positional_;

public:

  ValuePair() : material_(0), positional_(0) {}
  ValuePair(const Value& material, const Value& positional) : material_(material), positional_(positional) {}

  Value material() const {
    return material_;
  }

  Value positional() const {
    return positional_;
  }

  Value value() const {
    return material_ + positional_ / PositionalScale;
  }

  ValuePair operator+(const ValuePair& right) const {
    return ValuePair(material_ + right.material_, positional_ + right.positional_);
  }

};

enum {
  KKP_HPAWN   = 0,
  KKP_HLANCE  = KKP_HPAWN   + 19,
  KKP_HKNIGHT = KKP_HLANCE  + 5,
  KKP_HSILVER = KKP_HKNIGHT + 5,
  KKP_HGOLD   = KKP_HSILVER + 5,
  KKP_HBISHOP = KKP_HGOLD   + 5,
  KKP_HROOK   = KKP_HBISHOP + 3,
  KKP_BPAWN   = KKP_HROOK + 3,
  KKP_BLANCE  = KKP_BPAWN   + 81 - 9,
  KKP_BKNIGHT = KKP_BLANCE  + 81 - 9,
  KKP_BSILVER = KKP_BKNIGHT + 81 - 18,
  KKP_BGOLD   = KKP_BSILVER + 81,
  KKP_BBISHOP = KKP_BGOLD   + 81,
  KKP_BHORSE  = KKP_BBISHOP + 81,
  KKP_BROOK   = KKP_BHORSE  + 81,
  KKP_BDRAGON = KKP_BROOK   + 81,
  KKP_MAX     = KKP_BDRAGON + 81,
  KKP_ALL     = 81 * 81 * KKP_MAX,

  EVAL_ALL    = KKP_ALL,
};

/**
 * KKP のインデクスを左右反転します。
 */
int symmetrizeKkpIndex(int index);

/**
 * 盤上の駒の種類から KKP のインデクスを取得します。
 */
int kkpBoardIndex(Piece piece, const Square& sq);

/**
 * 持ち駒の種類から KKP のインデクスを取得します。
 */
int kkpHandIndex(Piece piece);

template <class T>
class Feature {
public:

  using ValueType = T;

  struct Table {
    ValueType kkp[81][81][KKP_MAX];
  };

  Table* t_;

  bool allocated;

protected:

  Feature() : t_(nullptr) {
    t_ = new Table();
    allocated = true;
    assert(t_ != nullptr);
  }

  Feature(Feature& ref) : t_(nullptr) {
    t_ = ref.t_;
    allocated = false;
    assert(t_ != nullptr);
  }

  Feature(const Feature&) = delete;
  Feature(Feature&&) = delete;

  ~Feature() {
    if (allocated && t_ != nullptr) {
      delete t_;
      t_ = nullptr;
    }
  }

public:

  /**
   * ファイルからパラメータを読み込みます。
   * @param filename
   */
  bool readFile(const char* filename);

  /**
   * ファイルからパラメータを読み込みます。
   * @param filename
   */
  bool readFile(const std::string& filename) {
    return readFile(filename.c_str());
  }

  /**
   * ファイルにパラメータを書き出します。
   * @param filename
   */
  bool writeFile(const char* filename) const;

  /**
   * ファイルにパラメータを書き出します。
   * @param filename
   */
  bool writeFile(const std::string& filename) const {
    return writeFile(filename.c_str());
  }

  template <class U, bool update>
  U extract(const Board& board, U delta);

};

class Evaluator : public Feature<int16_t> {
public:

  enum class InitType : int {
    File,
    Zero,
    Random,
  };

private:

  EvaluateTable<18> evaluateCache_;

  /**
   * 局面の駒割りを算出します。
   * @param board
   */
  Value evaluateMaterial_(const Board& board) const;

  /**
   * 局面の駒割りを除いた評価値を算出します。
   * @param board
   */
  Value evaluate_(const Board& board);

  /**
   * 指定した指し手による評価値の変化値を算出します。
   * @param board 着手後の局面を指定します。
   * @param prevValuePair
   * @param move
   */
  template <bool black>
  ValuePair evaluateDiff_(const Board& board, const ValuePair& prevValuePair, const Move& move);

  template <bool black, bool isKing, bool positionalOnly>
  Value estimate_(const Board& board, const Move& move);

public:

  Evaluator(InitType initType = InitType::File);

  Evaluator(Evaluator& ref);

  void init();
  void initRandom();

  /**
   * ファイルからパラメータを読み込みます。
   */
  bool readFile();

  /**
   * ファイルからパラメータを読み込みます。
   * @param filename
   */
  bool readFile(const char* filename) {
    return Feature<int16_t>::readFile(filename);
  }

  /**
   * ファイルからパラメータを読み込みます。
   * @param filename
   */
  bool readFile(const std::string& filename) {
    return Feature<int16_t>::readFile(filename);
  }

  /**
   * ファイルにパラメータを書き出します。
   */
  bool writeFile() const;

  /**
   * ファイルにパラメータを書き出します。
   * @param filename
   */
  bool writeFile(const char* filename) {
    return Feature<int16_t>::writeFile(filename);
  }

  /**
   * ファイルにパラメータを書き出します。
   * @param filename
   */
  bool writeFile(const std::string& filename) const {
    return Feature<int16_t>::writeFile(filename);
  }

  /**
   * 局面の評価値を算出します。
   * @param board
   */
  ValuePair evaluate(const Board& board) {
    return ValuePair(evaluateMaterial_(board), evaluate_(board));
  }

  /**
   * 指定した指し手に基づき評価値の差分計算を行います。
   * @param board 着手後の局面を指定します。
   * @param prevValuePair
   * @param move
   */
  ValuePair evaluateDiff(const Board& board, const ValuePair& prevValuePair, const Move& move) {
    if (!board.isBlack()) {
      return evaluateDiff_<true>(board, prevValuePair, move);
    } else {
      return evaluateDiff_<false>(board, prevValuePair, move);
    }
  }

  template <bool positionalOnly = false>
  Value estimate(const Board& board, const Move& move) {
    if (board.isBlack()) {
      return (move.piece() == Piece::King ?
              estimate_<true, true, positionalOnly>(board, move):
              estimate_<true, false, positionalOnly>(board, move));
    } else {
      return (move.piece() == Piece::King ?
              estimate_<false, true, positionalOnly>(board, move):
              estimate_<false, false, positionalOnly>(board, move));
    }
  }

  const Table& table() const {
    return *t_;
  }

  void clearCache() {
    evaluateCache_.init();
  }

  void prefetch(uint64_t hash) const {
    evaluateCache_.prefetch(hash);
  }

};

} // namespace sunfish

#endif // SUNFISH_EVALUATOR__
