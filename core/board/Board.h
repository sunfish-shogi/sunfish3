/* Board.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_BOARD__
#define __SUNFISH_BOARD__

#include "Bitboard.h"
#include "Hand.h"
#include "Zobrist.h"
#include "../move/Move.h"

namespace sunfish {

	class Board {
	private:

		Bitboard _bbBOccupy;
		Bitboard _bbBPawn;
		Bitboard _bbBLance;
		Bitboard _bbBKnight;
		Bitboard _bbBSilver;
		Bitboard _bbBGold;
		Bitboard _bbBBishop;
		Bitboard _bbBRook;
		Bitboard _bbBKing;
		Bitboard _bbBTokin;
		Bitboard _bbBProLance;
		Bitboard _bbBProKnight;
		Bitboard _bbBProSilver;
		Bitboard _bbBHorse;
		Bitboard _bbBDragon;

		Bitboard _bbWOccupy;
		Bitboard _bbWPawn;
		Bitboard _bbWLance;
		Bitboard _bbWKnight;
		Bitboard _bbWSilver;
		Bitboard _bbWGold;
		Bitboard _bbWBishop;
		Bitboard _bbWRook;
		Bitboard _bbWKing;
		Bitboard _bbWTokin;
		Bitboard _bbWProLance;
		Bitboard _bbWProKnight;
		Bitboard _bbWProSilver;
		Bitboard _bbWHorse;
		Bitboard _bbWDragon;

		Piece _board[Position::N];
		Position _posBKing;
		Position _posWKing;

		Hand _blackHand;
		Hand _whiteHand;
		bool _black;

		uint64_t _boardHash;
		uint64_t _handHash;

		Bitboard& getBB(const Piece& piece) {
			return *(const_cast<Bitboard*>(&_getBB(piece)));
		}
		const Bitboard& getBB(const Piece& piece) const {
			return _getBB(piece);
		}
		const Bitboard& _getBB(const Piece& piece) const;
		template<bool black> bool _isPin(const Position& pos, const Bitboard& occ) const;
		template<bool black> bool _isChecking(const Position& king, const Bitboard& occ) const;
		template<bool black> bool _isChecking() const {
			Bitboard occ = _bbBOccupy | _bbWOccupy;
			return _isChecking<black>(black ? _posBKing : _posWKing, occ);
		}
		template<bool black, DirectionEx dir> bool _isDirectCheck(const Move& move) const;
		template<bool black, Direction dir> bool _isDiscoveredCheck(const Position& king, const Position& from) const;
		template<bool black> bool _isCheck(const Move& move) const;
		template<bool black> bool _isPawnDropMate() const;
		template<bool black> bool _isValidMove(const Piece& piece, const Position& to) const;
		template<bool black> bool _isValidMove(const Piece& piece, const Position& from, const Position& to) const;
		template<bool black> bool _isValidMoveStrict(const Move& move) const;
		template<bool black> bool _makeMove(Move& move);
		template<bool black> bool _unmakeMove(const Move& move);

	public:

		enum class Handicap {
			Even,
			TwoPieces
		};

		Board();

		void init();
		void init(Handicap handicap);
		void refreshHash();

		/** 局面のハッシュ値を返します。 */
		uint64_t getHash() const {
			return getBoardHash() ^ getHandHash() ^ getTurnHash();
		}
		/** 盤上のみのハッシュ値を返します。 */
		uint64_t getBoardHash() const {
			return _boardHash;
		}
		/** 持ち駒のみのハッシュ値を返します。 */
		uint64_t getHandHash() const {
			return _handHash;
		}
		/** 手番のみのハッシュ値を返します。 */
		uint64_t getTurnHash() const {
			return _black ? Zobrist::black() : 0ull;
		}
		/** 手番を除く局面のハッシュ値を返します。 */
		uint64_t getNoTurnHash() const {
			return getBoardHash() ^ getHandHash();
		}

		/** 盤面の駒を取得します。 */
		Piece getBoardPiece(const Position& pos) const {
			return _board[pos];
		}
		/** 先手の持ち駒を取得します。 */
		const Hand& getBlackHand() const {
			return _blackHand;
		}
		/** 先手の持ち駒を取得します。 */
		Hand& getBlackHand() {
			return _blackHand;
		}
		/** 後手の持ち駒を取得します。 */
		const Hand& getWhiteHand() const {
			return _blackHand;
		}
		/** 後手の持ち駒を取得します。 */
		Hand& getWhiteHand() {
			return _blackHand;
		}
		/** 先手の持ち駒を取得します。 */
		int getBlackHand(const Piece& piece) const {
			return _blackHand.get(piece);
		}
		/** 先手の持ち駒を取得します。 */
		int getBlackHandUnsafe(const Piece& piece) const {
			return _blackHand.getUnsafe(piece);
		}
		/** 後手の持ち駒を取得します。 */
		int getWhiteHand(const Piece& piece) const {
			return _whiteHand.get(piece);
		}
		/** 後手の持ち駒を取得します。 */
		int getWhiteHandUnsafe(const Piece& piece) const {
			return _whiteHand.getUnsafe(piece);
		}
		/** 先手番の場合に true を返します。 */
		bool isBlack() const {
			return _black;
		}
		/** 後手番の場合に true を返します。 */
		bool isWhite() const {
			return !_black;
		}
		/** 先手の玉の位置を取得します。 */
		const Position& getBKingPosition() const {
			return _posBKing;
		}
		/** 後手の玉の位置を取得します。 */
		const Position& getWKingPosition() const {
			return _posWKing;
		}

		/** 盤面の駒をセットします。 */
		void setBoardPiece(const Position& pos, const Piece& piece);
		/** 先手の持ち駒をセットします。 */
		void setBlackHand(const Piece& piece, int count) {
			_blackHand.set(piece, count);
		}
		/** 後手の持ち駒をセットします。 */
		void setWhiteHand(const Piece& piece, int count) {
			_whiteHand.set(piece, count);
		}
		/** 先手の駒を増やします。 */
		void incBlackHand(const Piece& piece) {
			_blackHand.inc(piece);
		}
		/** 先手の駒を減らします。 */
		void decBlackHand(const Piece& piece) {
			_blackHand.dec(piece);
		}
		/** 後手の駒を増やします。 */
		void incWhiteHand(const Piece& piece) {
			_whiteHand.inc(piece);
		}
		/** 後手の駒を減らします。 */
		void decWhiteHand(const Piece& piece) {
			_whiteHand.dec(piece);
		}
		/** 先手番にします。 */
		void setBlack() {
			_black = true;
		}
		/** 後手番にします。 */
		void setWhite() {
			_black = false;
		}

		/** 王手がかかっているか判定します。 */
		bool isChecking() const {
			if (_black) {
				return _isChecking<true>();
			} else {
				return _isChecking<false>();
			}
		}

		/** 王手か判定します。 */
		bool isCheck(const Move& move) const {
			if (_black) {
				return _isCheck<true>(move);
			} else {
				return _isCheck<false>(move);
			}
		}

		/**
		 * MoveGenerator によって生成された手が合法手であるかチェックします。
		 * 厳密なチェックは行いません。
		 * @param move
		 */
		bool isValidMove(const Move& move) const {
			const Piece& piece = move.piece();
			const Position& to = move.to();
			if (move.isHand()) {
				return _black ? _isValidMove<true>(piece, to) : _isValidMove<false>(piece, to);
			} else {
				const Position& from = move.from();
				return _black ? _isValidMove<true>(piece, from, to) : _isValidMove<false>(piece, from, to);
			}
		}

		/**
		 * 任意の手が合法手であるか厳密なチェックをします。
		 * MoveGenerator で生成した手に対しては isValidMove を使用してください。
		 * @param move
		 */
		bool isValidMoveStrict(const Move& move) const {
			if (_black) {
				return _isValidMoveStrict<true>(move);
			} else {
				return _isValidMoveStrict<false>(move);
			}
		}

		/**
		 * 指定した手で局面を進めます。
		 * 厳密な合法手チェックを行いません。
		 * @param move
		 */
		bool makeMove(Move& move) {
			if (_black) {
				return _makeMove<true>(move);
			} else {
				return _makeMove<false>(move);
			}
		}

		/**
		 * 指定した手で局面を進めます。
		 * 厳密な合法手チェックをします。
		 * 低速な処理のため探索中で使用すべきではありません。
		 * @param move
		 */
		bool makeMoveStrict(Move& move) {
			if (_black) {
				return _isValidMoveStrict<true>(move) && _makeMove<true>(move);
			} else {
				return _isValidMoveStrict<false>(move) && _makeMove<false>(move);
			}
		}

		/**
		 * 局面を1手戻します。
		 */
		bool unmakeMove(const Move& move) {
			if (_black) {
				return _unmakeMove<false>(move);
			} else {
				return _unmakeMove<true>(move);
			}
		}

		/**
		 * 指定した手で局面を進めます。
		 * move に unmakeMove のための情報を書き込みません。
		 * @param move
		 */
		bool makeMoveIrr(const Move& move) {
			Move mtemp = move;
			return makeMove(mtemp);
		}

		/**
		 * パスをして相手に手番を渡します。
		 * 王手放置のチェックは行いません。
		 */
		void makeNullMove();

		/**
		 * パスした手を元に戻します。
		 */
		void unmakeNullMove();

		const Bitboard& getBOccupy() const { return _bbBOccupy; }
		const Bitboard& getBPawn() const { return _bbBPawn; }
		const Bitboard& getBLance() const { return _bbBLance; }
		const Bitboard& getBKnight() const { return _bbBKnight; }
		const Bitboard& getBSilver() const { return _bbBSilver; }
		const Bitboard& getBGold() const { return _bbBGold; }
		const Bitboard& getBBishop() const { return _bbBBishop; }
		const Bitboard& getBRook() const { return _bbBRook; }
		const Bitboard& getBKing() const { return _bbBKing; }
		const Bitboard& getBTokin() const { return _bbBTokin; }
		const Bitboard& getBProLance() const { return _bbBProLance; }
		const Bitboard& getBProKnight() const { return _bbBProKnight; }
		const Bitboard& getBProSilver() const { return _bbBProSilver; }
		const Bitboard& getBHorse() const { return _bbBHorse; }
		const Bitboard& getBDragon() const { return _bbBDragon; }
		const Bitboard& getWOccupy() const { return _bbWOccupy; }
		const Bitboard& getWPawn() const { return _bbWPawn; }
		const Bitboard& getWLance() const { return _bbWLance; }
		const Bitboard& getWKnight() const { return _bbWKnight; }
		const Bitboard& getWSilver() const { return _bbWSilver; }
		const Bitboard& getWGold() const { return _bbWGold; }
		const Bitboard& getWBishop() const { return _bbWBishop; }
		const Bitboard& getWRook() const { return _bbWRook; }
		const Bitboard& getWKing() const { return _bbWKing; }
		const Bitboard& getWTokin() const { return _bbWTokin; }
		const Bitboard& getWProLance() const { return _bbWProLance; }
		const Bitboard& getWProKnight() const { return _bbWProKnight; }
		const Bitboard& getWProSilver() const { return _bbWProSilver; }
		const Bitboard& getWHorse() const { return _bbWHorse; }
		const Bitboard& getWDragon() const { return _bbWDragon; }

		/**
		 * データが壊れていないか検査します。
		 */
		bool validate() const;

		/**
		 * ダンプを出力します。
		 */
		std::string dump() const;

		/**
		 * 盤面を表す文字列を生成します。
		 */
		std::string toString(bool showNumbers = true) const;

		/**
		 * 盤面を表すCSA形式の文字列を生成します。
		 */
		std::string toStringCsa() const;

	};

}

#endif //__SUNFISH_BOARD__
