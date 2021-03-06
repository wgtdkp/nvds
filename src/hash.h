#ifndef _NVDS_HASH_H_
#define _NVDS_HASH_H_

#include "MurmurHash2.h"

#include <string>

namespace nvds {

using KeyHash = uint64_t;
static const KeyHash kMaxKeyHash = static_cast<KeyHash>(-1);

struct KeyHashRange {
  KeyHash begin;
  KeyHash end;
};

static inline KeyHash Hash(const std::string& key) {
  return MurmurHash64B(key.c_str(), key.size(), 103);
}

static inline KeyHash Hash(const char* key, size_t key_len) {
  return MurmurHash64B(key, key_len, 103);
}

} // namespace nvds

#endif // _NVDS_HASH_H_
