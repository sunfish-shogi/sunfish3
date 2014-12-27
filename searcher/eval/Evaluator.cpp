/* Evaluator.cpp
 *
 * Kubo Ryosuke
 */

#include "Evaluator.h"
#include "logger/Logger.h"
#include <fstream>
#include <cstdlib>

#define DEFAULT_FV_FILENAME									"evdata"
#define FVBIN_FILENAME											"fv.bin"

namespace {

	using namespace sunfish;

	inline int kpp_index(int x, int y) {
		return x*(x+1)/2+y;
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
		71, 70, 69, 68, 67, 66, 65, 64, -1,
		63, 62, 61, 60, 59, 58, 57, 56, -1,
		55, 54, 53, 52, 51, 50, 49, 48, -1,
		47, 46, 45, 44, 43, 42, 41, 40, -1,
		39, 38, 37, 36, 35, 34, 33, 32, -1,
		31, 30, 29, 28, 27, 26, 25, 24, -1,
		23, 22, 21, 20, 19, 18, 17, 16, -1,
		15, 14, 13, 12, 11, 10,  9,  8, -1,
		7,  6,  5,  4,  3,  2,  1,  0,  -1,
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
		62, 61, 60, 59, 58, 57, 56, -1, -1,
		55, 54, 53, 52, 51, 50, 49, -1, -1,
		48, 47, 46, 45, 44, 43, 42, -1, -1,
		41, 40, 39, 38, 37, 36, 35, -1, -1,
		34, 33, 32, 31, 30, 29, 28, -1, -1,
		27, 26, 25, 24, 23, 22, 21, -1, -1,
		20, 19, 18, 17, 16, 15, 14, -1, -1,
		13, 12, 11, 10,  9,  8, 7,  -1, -1,
		6,  5,  4,  3,  2,  1,  0,  -1, -1,
	};

	int posIndexW[] = {
		80, 79, 78, 77, 76, 75, 74, 73, 72,
		71, 70, 69, 68, 67, 66, 65, 64, 63,
		62, 61, 60, 59, 58, 57, 56, 55, 54,
		53, 52, 51, 50, 49, 48, 47, 46, 45,
		44, 43, 42, 41, 40, 39, 38, 37, 36,
		35, 34, 33, 32, 31, 30, 29, 28, 27,
		26, 25, 24, 23, 22, 21, 20, 19, 18,
		17, 16, 15, 14, 13, 12, 11, 10,  9,
		8,  7,  6,  5,  4,  3,  2,  1,  0,
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

}

namespace sunfish {

	Evaluator::Evaluator() : _t(nullptr) {
		_t = new Table();

		assert(_t != nullptr);

		memset(_t, 0, sizeof(*_t));

		_t->pawn = 100;
		_t->lance = 300;
		_t->knight = 400;
		_t->silver = 600;
		_t->gold = 700;
		_t->bishop = 800;
		_t->rook = 1000;
		_t->tokin = 800;
		_t->pro_lance = 700;
		_t->pro_knight = 700;
		_t->pro_silver = 600;
		_t->horse = 1200;
		_t->dragon = 1300;

		_t->pawnEx = _t->pawn * 2;
		_t->lanceEx = _t->lance * 2;
		_t->knightEx = _t->knight * 2;
		_t->silverEx = _t->silver * 2;
		_t->goldEx = _t->gold * 2;
		_t->bishopEx = _t->bishop * 2;
		_t->rookEx = _t->rook * 2;
		_t->tokinEx = _t->tokin + _t->pawn;
		_t->pro_lanceEx = _t->pro_lance + _t->lance;
		_t->pro_knightEx = _t->pro_knight + _t->knight;
		_t->pro_silverEx = _t->pro_silver + _t->silver;
		_t->horseEx = _t->horse + _t->bishop;
		_t->dragonEx = _t->dragon + _t->rook;

		if (!readFile()) {
			if (convertFromFvBin()) {
				writeFile();
			}
		}
	}

	Evaluator::~Evaluator() {
		if (_t != nullptr) {
			delete _t;
		}
	}

	/**
	 * ファイルからパラメータを読み込みます。
	 */
	bool Evaluator::readFile() {
		return readFile(DEFAULT_FV_FILENAME);
	}

	/**
	 * ファイルからパラメータを読み込みます。
	 * @param filename
	 */
	bool Evaluator::readFile(const char* filename) {
		std::ifstream file(filename, std::ios::binary | std::ios::in);

		if (!file) {
			return false;
		}

		file.read((char*)_t->kpp, sizeof(Table::kpp));
		file.read((char*)_t->kkp, sizeof(Table::kkp));


		file.close();

		return true;
	}

	/**
	 * ファイルにパラメータを書き出します。
	 */
	bool Evaluator::writeFile() const {
		return writeFile(DEFAULT_FV_FILENAME);
	}

	/**
	 * ファイルにパラメータを書き出します。
	 * @param filename
	 */
	bool Evaluator::writeFile(const char* filename) const {
		std::ofstream file(filename, std::ios::binary | std::ios::out);

		if (!file) {
			return false;
		}

		file.write((const char*)_t->kpp, sizeof(Table::kpp));
		file.write((const char*)_t->kkp, sizeof(Table::kkp));

		file.close();

		return true;
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

		file.read((char*)fvbin->kpp, sizeof(Table::kpp));
		file.read((char*)fvbin->kkp, sizeof(Table::kkp));

		file.close();

		return fvbin_ptr;

	}

	void Evaluator::convertFromFvBin(Table* fvbin) {

#define CONV_ERROR_CHECK						0

#if CONV_ERROR_CHECK
		for (int i = 0; i < KPP_SIZE; i++) {
			_t->kpp[0][i] = (int16_t)0x7fff;
		}
		for (int i = 0; i < KKP_MAX; i++) {
			_t->kkp[0][0][i] = (int16_t)0x7fff;
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
  				_t->kpp[king][index] = fvbin->kpp[bona][bonaIndex];
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
					_t->kkp[bking][wking][index] = fvbin->kkp[bbona][wbona][bonaIndex];
				}
			}
		}

#if CONV_ERROR_CHECK
		for (int i = 0; i < KPP_SIZE; i++) {
			if (_t->kpp[0][i] == (int16_t)0x7fff) {
				std::cout << "error kpp " << i << std::endl;
			}
		}
		for (int i = 0; i < KKP_MAX; i++) {
			if (_t->kkp[0][0][i] == (int16_t)0x7fff) {
				std::cout << "error kkp " << i << std::endl;
			}
		}
#endif

	}

	int Evaluator::posInv(int table[], int in) {
		POSITION_EACH(pos) {
			if (table[pos] == in) {
				return pos;
			}
		}
		return 0;
	}

	/**
	 * KPP のインデクスを Bonanza の並びに変換します。
	 */
	int Evaluator::convertKppIndex4FvBin(int index) {
		struct BoardInfo {
			int begin;
			int end;
			int offset;
			int* table;
		};

		static const BoardInfo biList[] = {
			{ KPP_BBPAWN,   KPP_BWPAWN,   -9,  posIndexBPawn },
			{ KPP_BWPAWN,   KPP_BBLANCE,  -9,  posIndexBPawn },
			{ KPP_BBLANCE,  KPP_BWLANCE,  -9,  posIndexBPawn },
			{ KPP_BWLANCE,  KPP_BBKNIGHT, -9,  posIndexBPawn },
			{ KPP_BBKNIGHT, KPP_BWKNIGHT, -18, posIndexBKnight },
			{ KPP_BWKNIGHT, KPP_BBSILVER, -18, posIndexBKnight },
			{ KPP_BBSILVER, KPP_BWSILVER, 0,   nullptr },
			{ KPP_BWSILVER, KPP_BBGOLD,   0,   nullptr },
			{ KPP_BBGOLD,   KPP_BWGOLD,   0,   nullptr },
			{ KPP_BWGOLD,   KPP_BBBISHOP, 0,   nullptr },
			{ KPP_BBBISHOP, KPP_BWBISHOP, 0,   nullptr },
			{ KPP_BWBISHOP, KPP_BBROOK,   0,   nullptr },
			{ KPP_BBROOK,   KPP_BWROOK,   0,   nullptr },
			{ KPP_BWROOK,   KPP_BBHORSE,  0,   nullptr },
			{ KPP_BBHORSE,  KPP_BWHORSE,  0,   nullptr },
			{ KPP_BWHORSE,  KPP_BBDRAGON, 0,   nullptr },
			{ KPP_BBDRAGON, KPP_BWDRAGON, 0,   nullptr },
			{ KPP_BWDRAGON, KPP_MAX,      0,   nullptr },
		};

		if (index < KPP_BBPAWN) {
			return index;
		}

		for (int i = 0; i < sizeof(biList)/sizeof(biList[0]); i++) {
			const auto& bi = biList[i];
			if (bi.begin <= index && index < bi.end) {
  			int pos = index - bi.begin;
				assert(pos >= 0);
				assert(pos < 81);
				if (bi.table != nullptr) {
    			pos = posInv(bi.table, pos);
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
	int Evaluator::convertKkpIndex4FvBin(int index) {
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
			{ KKP_BBISHOP, KKP_BROOK,     0,   nullptr },
			{ KKP_BROOK,   KKP_BHORSE,    0,   nullptr },
			{ KKP_BHORSE,  KKP_BDRAGON,   0,   nullptr },
			{ KKP_BDRAGON, KKP_MAX,       0,   nullptr },
		};

		if (index < KKP_BPAWN) {
			return index;
		}

		for (int i = 0; i < sizeof(biList)/sizeof(biList[0]); i++) {
			const auto& bi = biList[i];
			if (bi.begin <= index && index < bi.end) {
  			int pos = index - bi.begin;
				assert(pos >= 0);
				assert(pos < 81);
				if (bi.table != nullptr) {
    			pos = posInv(bi.table, pos);
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
	 * 局面の評価値を算出します。
	 * @param board
	 */
	ValuePair Evaluator::evaluate(const Board& board) const {
		Value base = Value::Zero;
		Value positional = Value::Zero;
		auto bking = board.getBKingPosition();
		auto wking = board.getWKingPosition();
		auto bkingR = bking.reverse();
		auto wkingR = wking.reverse();
		Bitboard bb;

		base += _t->pawn * board.getBPawn().count();
		base += _t->lance * board.getBLance().count();
		base += _t->knight * board.getBKnight().count();
		base += _t->silver * board.getBSilver().count();
		base += _t->gold * board.getBGold().count();
		base += _t->bishop * board.getBBishop().count();
		base += _t->rook * board.getBRook().count();
		base += _t->tokin * board.getBTokin().count();
		base += _t->pro_lance * board.getBProLance().count();
		base += _t->pro_knight * board.getBProKnight().count();
		base += _t->pro_silver * board.getBProSilver().count();
		base += _t->horse * board.getBHorse().count();
		base += _t->dragon * board.getBDragon().count();

		base -= _t->pawn * board.getWPawn().count();
		base -= _t->lance * board.getWLance().count();
		base -= _t->knight * board.getWKnight().count();
		base -= _t->silver * board.getWSilver().count();
		base -= _t->gold * board.getWGold().count();
		base -= _t->bishop * board.getWBishop().count();
		base -= _t->rook * board.getWRook().count();
		base -= _t->tokin * board.getWTokin().count();
		base -= _t->pro_lance * board.getWProLance().count();
		base -= _t->pro_knight * board.getWProKnight().count();
		base -= _t->pro_silver * board.getWProSilver().count();
		base -= _t->horse * board.getWHorse().count();
		base -= _t->dragon * board.getWDragon().count();

		base += _t->pawn * board.getBlackHand(Piece::Pawn);
		base += _t->lance * board.getBlackHand(Piece::Lance);
		base += _t->knight * board.getBlackHand(Piece::Knight);
		base += _t->silver * board.getBlackHand(Piece::Silver);
		base += _t->gold * board.getBlackHand(Piece::Gold);
		base += _t->bishop * board.getBlackHand(Piece::Bishop);
		base += _t->rook * board.getBlackHand(Piece::Rook);

		base -= _t->pawn * board.getWhiteHand(Piece::Pawn);
		base -= _t->lance * board.getWhiteHand(Piece::Lance);
		base -= _t->knight * board.getWhiteHand(Piece::Knight);
		base -= _t->silver * board.getWhiteHand(Piece::Silver);
		base -= _t->gold * board.getWhiteHand(Piece::Gold);
		base -= _t->bishop * board.getWhiteHand(Piece::Bishop);
		base -= _t->rook * board.getWhiteHand(Piece::Rook);

		int count;
		int num = 14;
		int bList[52]; // 52 = 40(総駒数) - 2(玉) + 14(駒台)
		int wList[52];
		int nTemp;
		int wTemp[34]; // 34 = 18(と金) + 16(成香, 成桂, 成銀, 金)

#define ON_HAND(piece, pieceL, index) \
		count = board.getBlackHand(Piece::piece); \
		for (int i = 0; i <= count; i++) { \
			positional += _t->kkp[bking][wking][KKP_H ## pieceL+i]; \
		} \
		bList[index] = KPP_HB ## pieceL + count; \
		wList[index] = KPP_HW ## pieceL + count; \
		count = board.getWhiteHand(Piece::piece); \
		for (int i = 0; i <= count; i++) { \
			positional -= _t->kkp[wkingR][bkingR][KKP_H ## pieceL+i]; \
		} \
		bList[index+1] = KPP_HW ## pieceL + count; \
		wList[index+1] = KPP_HB ## pieceL + count;

		ON_HAND(Pawn, PAWN, 0);
		ON_HAND(Lance, LANCE, 2);
		ON_HAND(Knight, KNIGHT, 4);
		ON_HAND(Silver, SILVER, 6);
		ON_HAND(Gold, GOLD, 8);
		ON_HAND(Bishop, BISHOP, 10);
		ON_HAND(Rook, ROOK, 12);

#undef ON_HAND

#define ON_BOARD(blackBB, whiteBB, pieceL) \
		nTemp = 0; \
		bb = (blackBB); \
		BB_EACH_OPE(pos, bb, \
			positional += _t->kkp[bking][wking][KKP_B ## pieceL+POS_INDEX_B(pos)]; \
			bList[num++] = KPP_BB ## pieceL + POS_INDEX_B(pos); \
			wTemp[nTemp++] = KPP_BW ## pieceL + POS_INDEX_W(pos); \
		); \
		bb = (whiteBB); \
		BB_EACH_OPE(pos, bb, \
			positional -= _t->kkp[wkingR][bkingR][KKP_B ## pieceL+POS_INDEX_W(pos)]; \
			bList[num++] = KPP_BW ## pieceL + POS_INDEX_B(pos); \
			wTemp[nTemp++] = KPP_BB ## pieceL + POS_INDEX_W(pos); \
		); \
		for (int i = 0; i < nTemp; i++) { wList[num-i-1] = wTemp[i]; }

#define POS_INDEX_B(pos)								posIndexBPawn[pos]
#define POS_INDEX_W(pos)								posIndexWPawn[pos]
		ON_BOARD(board.getBPawn(), board.getWPawn(), PAWN);
		ON_BOARD(board.getBLance(),board.getWLance(), LANCE);
#undef POS_INDEX_B
#undef POS_INDEX_W
#define POS_INDEX_B(pos)								posIndexBKnight[pos]
#define POS_INDEX_W(pos)								posIndexWKnight[pos]
		ON_BOARD(board.getBKnight(), board.getWKnight(), KNIGHT);
		ON_BOARD(board.getBSilver(), board.getWSilver(), SILVER);
#undef POS_INDEX_B
#undef POS_INDEX_W
#define POS_INDEX_B(pos)								pos
#define POS_INDEX_W(pos)								posIndexW[pos]
		ON_BOARD(board.getBGold() | board.getBProLance() | board.getBProKnight() | board.getBProSilver(),
						 board.getWGold() | board.getWProLance() | board.getWProKnight() | board.getWProSilver(), GOLD);
		ON_BOARD(board.getBBishop(), board.getWBishop(), BISHOP);
		ON_BOARD(board.getBRook(), board.getWRook(), ROOK);
		ON_BOARD(board.getBTokin(), board.getWTokin(), GOLD);
		ON_BOARD(board.getBHorse(), board.getWHorse(), HORSE);
		ON_BOARD(board.getBDragon(), board.getWDragon(), DRAGON);
#undef POS_INDEX_B
#undef POS_INDEX_W

#undef ON_BOARD

		/*
		for (int i = 0; i < num; i++) {
			int bx = bList[i];
			int wx = wList[i];
			for (int j = 0; j <= i; j++) {
				int by = bList[j];
				int wy = wList[j];
				positional += _t->kpp[bking][kpp_index(bx, by)];
				positional -= _t->kpp[wking][kpp_index(wx, wy)];
			}
		}
		 */

		return ValuePair(base, positional);
	}

	/**
	 * 指定した指し手による評価値の変化値を算出します。
	 * @param board 着手後の局面を指定します。
	 * @param move
	 */
	template <bool black>
	ValuePair Evaluator::_evaluateDiff(const Board& board, const Move& move) const {

		Value base = 0;
		Value positional = 0;
		auto from = move.from();
		auto to = move.to();
		bool promote = move.promote();
		auto piece = move.piece();
		auto captured = move.captured();

		auto bking = black ? board.getBKingPosition() : board.getWKingPosition().reverse();
		auto wking = black ? board.getWKingPosition() : board.getBKingPosition().reverse();
		auto bkingR = bking.reverse();
		auto wkingR = wking.reverse();

		assert(piece != Piece::King);

		positional -= _t->kkp[bking][wking][kkpBoardIndex(piece)+from];

		if (promote) {
			base += piecePromote(piece);
			positional += _t->kkp[bking][wking][kkpBoardIndex(piece.promote())+to];
		} else {
			positional += _t->kkp[bking][wking][kkpBoardIndex(piece)+to];
		}

		if (!captured.isEmpty()) {
			base += pieceExchange(captured);
			positional -= _t->kkp[wkingR][bkingR][kkpBoardIndex(captured)+to];
			auto hand = captured.kindOnly().unpromote();
			if (black) {
				int num = board.getBlackHand(hand);
				positional += _t->kkp[bking][wking][kkpHandIndex(hand)+num];
				positional -= _t->kkp[bking][wking][kkpHandIndex(hand)+num-1];
			} else {
				int num = board.getWhiteHand(hand);
				positional -= _t->kkp[wkingR][bkingR][kkpHandIndex(hand)+num];
				positional += _t->kkp[wkingR][bkingR][kkpHandIndex(hand)+num-1];
			}
		}

		if (black) {
			return ValuePair(base, positional);
		} else {
			return ValuePair(-base, -positional);
		}

	}
	template ValuePair Evaluator::_evaluateDiff<true>(const Board&, const Move&) const;
	template ValuePair Evaluator::_evaluateDiff<false>(const Board&, const Move&) const;

	/**
	 * 駒種から KPP のインデクスを取得します。
	 */
	int Evaluator::kkpBoardIndex(Piece piece) const {
		switch (piece) {
			case Piece::BPawn:      case Piece::WPawn:      return KKP_BPAWN;
			case Piece::BLance:     case Piece::WLance:     return KKP_BLANCE;
			case Piece::BKnight:    case Piece::WKnight:    return KKP_BKNIGHT;
			case Piece::BSilver:    case Piece::WSilver:    return KKP_BSILVER;
			case Piece::BGold:      case Piece::WGold:      return KKP_BGOLD;
			case Piece::BBishop:    case Piece::WBishop:    return KKP_BBISHOP;
			case Piece::BRook:      case Piece::WRook:      return KKP_BROOK;
			case Piece::BTokin:     case Piece::WTokin:     return KKP_BGOLD;
			case Piece::BProLance:  case Piece::WProLance:  return KKP_BGOLD;
			case Piece::BProKnight: case Piece::WProKnight: return KKP_BGOLD;
			case Piece::BProSilver: case Piece::WProSilver: return KKP_BGOLD;
			case Piece::BHorse:     case Piece::WHorse:     return KKP_BHORSE;
			case Piece::BDragon:    case Piece::WDragon:    return KKP_BDRAGON;
		}

		assert(false);
		return 0;

	}

	/**
	 * 駒種から KPP のインデクスを取得します。
	 */
	int Evaluator::kkpHandIndex(Piece piece) const {
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
	 * 駒割を取得します。
	 */
	Value Evaluator::piece(const Piece& piece) const {
		switch(piece) {
			case Piece::BPawn: case Piece::WPawn:
				return _t->pawn;
			case Piece::BLance: case Piece::WLance:
				return _t->lance;
			case Piece::BKnight: case Piece::WKnight:
				return _t->knight;
			case Piece::BSilver: case Piece::WSilver:
				return _t->silver;
			case Piece::BGold: case Piece::WGold:
				return _t->gold;
			case Piece::BBishop: case Piece::WBishop:
				return _t->bishop;
			case Piece::BRook: case Piece::WRook:
				return _t->rook;
			case Piece::BTokin: case Piece::WTokin:
				return _t->tokin;
			case Piece::BProLance: case Piece::WProLance:
				return _t->pro_lance;
			case Piece::BProKnight: case Piece::WProKnight:
				return _t->pro_knight;
			case Piece::BProSilver: case Piece::WProSilver:
				return _t->pro_silver;
			case Piece::BHorse: case Piece::WHorse:
				return _t->horse;
			case Piece::BDragon: case Piece::WDragon:
				return _t->dragon;
			default:
				return 0;
		}
	}

	/**
	 * 駒を取った時の変化値を取得します。
	 */
	Value Evaluator::pieceExchange(const Piece& piece) const {
		switch(piece) {
			case Piece::BPawn: case Piece::WPawn:
				return _t->pawnEx;
			case Piece::BLance: case Piece::WLance:
				return _t->lanceEx;
			case Piece::BKnight: case Piece::WKnight:
				return _t->knightEx;
			case Piece::BSilver: case Piece::WSilver:
				return _t->silverEx;
			case Piece::BGold: case Piece::WGold:
				return _t->goldEx;
			case Piece::BBishop: case Piece::WBishop:
				return _t->bishopEx;
			case Piece::BRook: case Piece::WRook:
				return _t->rookEx;
			case Piece::BTokin: case Piece::WTokin:
				return _t->tokinEx;
			case Piece::BProLance: case Piece::WProLance:
				return _t->pro_lanceEx;
			case Piece::BProKnight: case Piece::WProKnight:
				return _t->pro_knightEx;
			case Piece::BProSilver: case Piece::WProSilver:
				return _t->pro_silverEx;
			case Piece::BHorse: case Piece::WHorse:
				return _t->horseEx;
			case Piece::BDragon: case Piece::WDragon:
				return _t->dragonEx;
			default:
				return 0;
		}
	}

	/**
	 * 駒が成った時の変化値を取得します。
	 */
	Value Evaluator::piecePromote(const Piece& piece) const {
		switch(piece) {
			case Piece::BPawn: case Piece::WPawn:
				return _t->tokin - _t->pawn;
			case Piece::BLance: case Piece::WLance:
				return _t->pro_lance - _t->lance;
			case Piece::BKnight: case Piece::WKnight:
				return _t->pro_knight - _t->knight;
			case Piece::BSilver: case Piece::WSilver:
				return _t->pro_silver - _t->silver;
			case Piece::BGold: case Piece::WGold:
				return 0;
			case Piece::BBishop: case Piece::WBishop:
				return _t->horse - _t->bishop;
			case Piece::BRook: case Piece::WRook:
				return _t->dragon - _t->rook;
			case Piece::BTokin: case Piece::WTokin:
			case Piece::BProLance: case Piece::WProLance:
			case Piece::BProKnight: case Piece::WProKnight:
			case Piece::BProSilver: case Piece::WProSilver:
			case Piece::BHorse: case Piece::WHorse:
			case Piece::BDragon: case Piece::WDragon:
			default:
				return 0;
		}
	}

}
