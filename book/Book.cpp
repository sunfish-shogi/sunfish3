/* Book.cpp
 *
 * Kubo Ryosuke
 */

#include "Book.h"
#include <fstream>

#define DEFAULT_BOOK_FILENAME									"book.bin"

namespace sunfish {

	bool BookElement::add(const Move& move) {
		for (auto& bookMove : _moves) {
			if (bookMove.move == move) {
				bookMove.count++;
				_count++;
				return true;
			}
		}

		_moves.push_back({move, (uint32_t)1});
		_count++;
		return true;
	}

	BookResult BookElement::selectRandom(uint64_t hash, Random& random) const {
		assert(_count != 0);
		if (_count == 0) {
			return BookResult{ Move::empty(), 0, 0 };
		}

		uint32_t r = random.getInt32(_count);
		uint32_t c = 0;

		for (auto& bookMove : _moves) {
			c += bookMove.count;
			if (c > r) {
				return BookResult{ bookMove.move, bookMove.count, _count };
			}
		}

		return BookResult{ Move::empty(), 0, 0 };
	}

	bool Book::add(uint64_t hash, const Move& move) {
		auto ite = _map.find(hash);

		if (ite == _map.end()) {
			ite = _map.emplace_hint(_map.end(),
															std::piecewise_construct,
															std::forward_as_tuple(hash),
															std::forward_as_tuple());
		}

		return ite->second.add(move);
	}

	const BookElement* Book::find(uint64_t hash) const {
		auto ite = _map.find(hash);

		if (ite != _map.end()) {
			return &ite->second;
		}

		return nullptr;
	}

	BookResult Book::selectRandom(uint64_t hash) {
		auto ite = _map.find(hash);

		if (ite != _map.end()) {
			return ite->second.selectRandom(hash, random);
		}

		return BookResult{ Move::empty(), 0, 0 };
	}

	void BookElement::read(std::istream& is) {
		uint32_t size;
		// read: size(uint32_t)
		is.read((char*)&size, sizeof(size));

		uint32_t totalCount = 0;
		_moves.clear();
		for ( ; size != 0; size--) {
			uint32_t move;
			uint32_t count;
			// read: move(uint32_t)
			is.read((char*)&move, sizeof(move));
			// read: count(uint32_t)
			is.read((char*)&count, sizeof(count));
			_moves.push_back({ Move::deserialize(move), count });
			totalCount += count;
		}
		_count = totalCount;
	}

	void BookElement::write(std::ostream& os) const {
		uint32_t size = (uint32_t)_moves.size();
		// write: size(uint32_t)
		os.write((const char*)&size, sizeof(size));

		for (const auto& bookMove : _moves) {
			uint32_t move = Move::serialize(bookMove.move);
			uint32_t count = bookMove.count;
			// write: move(uint32_t)
			os.write((const char*)&move, sizeof(move));
			// write: count(uint32_t)
			os.write((const char*)&count, sizeof(count));
		}
	}

	bool Book::readFile() {
		return readFile(DEFAULT_BOOK_FILENAME);
	}

	bool Book::readFile(const char* filename) {
		std::ifstream file(filename, std::ios::binary | std::ios::in);

		if (!file) {
			return false;
		}

		_map.clear();
		while (true) {
			uint64_t hash;
			// read: hash(uint64_t)
			file.read((char*)&hash, sizeof(hash));
			if (file.eof()) {
				break;
			}
			auto ite = _map.emplace_hint(_map.end(),
																	 std::piecewise_construct,
																	 std::forward_as_tuple(hash),
																	 std::forward_as_tuple());
			// read: moves
			ite->second.read(file);
		}

		file.close();

		return true;
	}

	bool Book::writeFile() const {
		return writeFile(DEFAULT_BOOK_FILENAME);
	}

	bool Book::writeFile(const char* filename) const {
		std::ofstream file(filename, std::ios::binary | std::ios::out);

		if (!file) {
			return false;
		}

		for (const auto& pair : _map) {
			uint64_t hash = pair.first;
			const auto& element = pair.second;

			// write: hash(uint64_t)
			file.write((const char*)&hash, sizeof(hash));

			// write: moves
			element.write(file);
		}

		file.close();

		return true;
	}

}
