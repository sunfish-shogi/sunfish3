/* Evaluator.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_EVALUATOR__
#define __SUNFISH_EVALUATOR__

#include "EvaluateTable.h"
#include "Value.h"
#include "core/board/Board.h"
#include <memory>

namespace sunfish {

	class ValuePair {
	public:

		static CONSTEXPR int PositionalScale = 32;

	private:

		Value _material;
		Value _positional;

	public:

		ValuePair() : _material(0), _positional(0) {}
		ValuePair(const Value& material, const Value& positional) : _material(material), _positional(positional) {}

		Value material() const {
			return _material;
		}

		Value positional() const {
			return _positional;
		}

		Value value() const {
			return _material + _positional / PositionalScale;
		}

		ValuePair operator+(const ValuePair& right) const {
			return ValuePair(_material + right._material, _positional + right._positional);
		}

	};

	class Evaluator {
	public:

		enum class InitType : int {
			File,
			Zero,
			Random,
		};

		enum {
			KPP_HBPAWN   = 0,
			KPP_HWPAWN   = KPP_HBPAWN   + 19,
			KPP_HBLANCE  = KPP_HWPAWN   + 19,
			KPP_HWLANCE  = KPP_HBLANCE  + 5,
			KPP_HBKNIGHT = KPP_HWLANCE  + 5,
			KPP_HWKNIGHT = KPP_HBKNIGHT + 5,
			KPP_HBSILVER = KPP_HWKNIGHT + 5,
			KPP_HWSILVER = KPP_HBSILVER + 5,
			KPP_HBGOLD   = KPP_HWSILVER + 5,
			KPP_HWGOLD   = KPP_HBGOLD   + 5,
			KPP_HBBISHOP = KPP_HWGOLD   + 5,
			KPP_HWBISHOP = KPP_HBBISHOP + 3,
			KPP_HBROOK   = KPP_HWBISHOP + 3,
			KPP_HWROOK   = KPP_HBROOK   + 3,
			KPP_BBPAWN   = KPP_HWROOK   + 3,
			KPP_BWPAWN   = KPP_BBPAWN   + 81 - 9,
			KPP_BBLANCE  = KPP_BWPAWN   + 81 - 9,
			KPP_BWLANCE  = KPP_BBLANCE  + 81 - 9,
			KPP_BBKNIGHT = KPP_BWLANCE  + 81 - 9,
			KPP_BWKNIGHT = KPP_BBKNIGHT + 81 - 18,
			KPP_BBSILVER = KPP_BWKNIGHT + 81 - 18,
			KPP_BWSILVER = KPP_BBSILVER + 81,
			KPP_BBGOLD   = KPP_BWSILVER + 81,
			KPP_BWGOLD   = KPP_BBGOLD   + 81,
			KPP_BBBISHOP = KPP_BWGOLD   + 81,
			KPP_BWBISHOP = KPP_BBBISHOP + 81,
			KPP_BBHORSE  = KPP_BWBISHOP + 81,
			KPP_BWHORSE  = KPP_BBHORSE  + 81,
			KPP_BBROOK   = KPP_BWHORSE  + 81,
			KPP_BWROOK   = KPP_BBROOK   + 81,
			KPP_BBDRAGON = KPP_BWROOK   + 81,
			KPP_BWDRAGON = KPP_BBDRAGON + 81,
			KPP_MAX      = KPP_BWDRAGON + 81,
			KPP_SIZE     = KPP_MAX*(KPP_MAX+1)/2,
			KPP_ALL      = 81 * KPP_SIZE,

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
		};

		struct Table {
			int16_t pawn;
			int16_t lance;
			int16_t knight;
			int16_t silver;
			int16_t gold;
			int16_t bishop;
			int16_t rook;
			int16_t tokin;
			int16_t pro_lance;
			int16_t pro_knight;
			int16_t pro_silver;
			int16_t horse;
			int16_t dragon;

			int16_t pawnEx;
			int16_t lanceEx;
			int16_t knightEx;
			int16_t silverEx;
			int16_t goldEx;
			int16_t bishopEx;
			int16_t rookEx;
			int16_t tokinEx;
			int16_t pro_lanceEx;
			int16_t pro_knightEx;
			int16_t pro_silverEx;
			int16_t horseEx;
			int16_t dragonEx;

			int16_t kpp[81][KPP_SIZE];
			int16_t kkp[81][81][KKP_MAX];
		};

	private:

		Table* _t;

		EvaluateTable _hashTable;

		void alloc();
		void free();
		void initMaterial();
		void initPositional();
		void initPositionalRandom();

		std::shared_ptr<Table> readFvBin();

		void convertFromFvBin(Table* fvbin);

		int posInv(int table[], int in);

		int convertKppIndex4FvBin(int index);

		int convertKkpIndex4FvBin(int index);

		int kkpBoardIndex(Piece piece, const Position& pos) const;

		int kkpHandIndex(Piece piece) const;

		template <bool blackPiece>
		int kppBoardIndex(Piece piece, const Position& pos) const;

		template <bool blackPiece>
		int kppHandIndex(Piece piece) const;

		/**
		 * 局面の駒割りを算出します。
		 * @param board
		 */
		Value _evaluateMaterial(const Board& board) const;

		/**
		 * 局面の駒割りを除いた評価値を算出します。
		 * @param board
		 */
		Value _evaluate(const Board& board);

		/**
		 * 指定した指し手による評価値の変化値を算出します。
		 * @param board 着手後の局面を指定します。
		 * @param prevValuePair
		 * @param move
		 */
		template <bool black>
		ValuePair _evaluateDiff(const Board& board, const ValuePair& prevValuePair, const Move& move);

		template <bool black, bool isKing>
		Value _estimate(const Board& board, const Move& move);

	public:

		Evaluator(InitType initType = InitType::File);

		~Evaluator();

		/**
		 * ファイルからパラメータを読み込みます。
		 */
		bool readFile();

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
		 */
		bool writeFile() const;

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

		/**
		 * fv.bin があれば読み込んで並べ替えを行います。
		 */
		bool convertFromFvBin();

		/**
		 * 局面の評価値を算出します。
		 * @param board
		 */
		ValuePair evaluate(const Board& board) {
			return ValuePair(_evaluateMaterial(board), _evaluate(board));
		}

		/**
		 * 指定した指し手に基づき評価値の差分計算を行います。
		 * @param board 着手後の局面を指定します。
		 * @param prevValuePair
		 * @param move
		 */
		ValuePair evaluateDiff(const Board& board, const ValuePair& prevValuePair, const Move& move) {
			if (!board.isBlack()) {
				return _evaluateDiff<true>(board, prevValuePair, move);
			} else {
				return _evaluateDiff<false>(board, prevValuePair, move);
			}
		}

		Value estimate(const Board& board, const Move& move) {
			if (board.isBlack()) {
				return (move.piece() == Piece::King ?
								_estimate<true, true>(board, move):
								_estimate<true, false>(board, move));
			} else {
				return (move.piece() == Piece::King ?
								_estimate<false, true>(board, move):
								_estimate<false, false>(board, move));
			}
		}

		Value piece(const Piece& piece) const;
		Value pieceExchange(const Piece& piece) const;
		Value piecePromote(const Piece& piece) const;

		const Table& table() const {
			return *_t;
		}

	};
	
}

#endif // __SUNFISH_EVALUATOR__
