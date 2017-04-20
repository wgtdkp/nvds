#include "tablet.h"

namespace nvds {

int32_t Tablet::Put(KeyHash hash, uint16_t key_len, const char* key,
                    uint16_t val_len, const char* val) {
  auto idx = hash % kHashTableSize;
  auto& head = nvm_tablet_->hash_table[idx];
  auto next = head == nullptr ? NVMPtr<NVMObject>(nullptr) : head->next;
  auto size = sizeof(NVMObject) + key_len + val_len;
  auto obj = allocator_.Alloc<NVMObject>(size);
  *obj = {next, hash, key_len, val_len};
  // FIXME(wgtdkp): shouldn't use memcpy, it is wrong to simulate NVM speed/latency.
  memcpy(obj->data, key, key_len);
  memcpy(obj->data + key_len, val, val_len);
  head = obj;
  return 0;
}

int32_t Tablet::Get(char* val, KeyHash hash,
                    uint16_t key_len, const char* key) {
  auto idx = hash % kHashTableSize;
  auto p = nvm_tablet_->hash_table[idx];
  if (p == nullptr)
    return -1;
  p = p->next;
  while (p != nullptr) {
    // TODO(wgtdkp): use a different hash to speedup searching
    if (p->key_len == key_len && memcmp(p->data, key, key_len) == 0) {
      // FIXME(wgtdkp): use nvmcpy instead.
      memcpy(val, p->data + key_len, p->val_len);
      return p->val_len;
    }
    p = p->next;
  }
  return -1;
}

void Tablet::Del(KeyHash hash, uint16_t key_len, const char* key) {
  auto idx = hash % kHashTableSize;
  auto p = nvm_tablet_->hash_table[idx];
  if (p == nullptr)
    return;
  auto q = p->next;
  while (q != nullptr) {
    if (p->key_len == key_len && memcmp(p->data, key, key_len) == 0) {
      p->next = q->next;
      allocator_.Free(q);
      return;
    }
  }
}

} // namespace nvds
