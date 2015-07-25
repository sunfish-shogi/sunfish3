/* HashTable.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_HASHTABLE__
#define SUNFISH_HASHTABLE__

#include "core/def.h"
#include <cstdint>

namespace sunfish {

/**
 * base class for hash table
 */
template <class E> class HashTable {
private:

  E* table_;
  uint32_t size_;
  uint32_t mask_;

protected:

  E& getEntity(uint64_t hash) {
    return table_[hash&mask_];
  }

  E& getEntity(uint32_t index) {
    return table_[index];
  }

  const E& getEntity(uint64_t hash) const {
    return table_[hash&mask_];
  }

  const E& getEntity(uint32_t index) const {
    return table_[index];
  }

public:

  static CONSTEXPR uint32_t DefaultBits = 18;

  HashTable(uint32_t bits = DefaultBits) : table_(nullptr), size_(0) {
    init(bits);
  }
  HashTable(const HashTable&) = delete;
  HashTable(HashTable&&) = delete;

  ~HashTable() {
    delete [] table_;
  }

  void init(uint32_t bits = 0) {
    uint32_t newSize = 1 << bits;
    if (bits != 0 && size_ != newSize) {
      size_ = newSize;
      mask_ = size_ - 1;
      if (table_ != nullptr) {
        delete[] table_;
      }
      table_ = new E[size_];
    } else {
      for (uint32_t i = 0; i < size_; i++) {
        table_[i].init(i);
      }
    }
  }

  uint32_t getSize() const {
    return size_;
  }
};

} // namespace sunfish

#endif // SUNFISH_HASHTABLE__
