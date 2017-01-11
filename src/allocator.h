#ifndef _NVDS_ALLOCATOR_H_
#define _NVDS_ALLOCATOR_H_

#include "common.h"

namespace nvds {

class Allocator {
 public:
  static const uint32_t kMaxBlockSize = 1024 * 1024;
  static const uint32_t kSize = 64 * 1024 * 1024;
  Allocator(uintptr_t base) : base_(base) {
    flm_ = OffsetToPtr<FreeListManager>(0);
  }
  ~Allocator() {}
  DISALLOW_COPY_AND_ASSIGN(Allocator);

  uintptr_t Alloc(uint32_t size);
  void Free(uintptr_t ptr);
  void Format();

  uintptr_t base() const { return base_; }

 private:
  static const uint32_t kNumFreeList = 2 * kMaxBlockSize / 16;

  template<typename T>
  T* OffsetToPtr(uint32_t offset) const {
    return reinterpret_cast<T*>(base_ + offset);
  }

  template<typename T>
  void Write(uint32_t offset, T val) {
    auto ptr = OffsetToPtr<T>(offset);
    *ptr = val;
    // TODO(wgtdkp): Collect
  }

  template<typename T>
  void Write(T* ptr, T val) {
    *ptr = val;
    // TODO(wgtdkp): Collect
  }

  template<typename T>
  T Read(uint32_t offset) {
    return *OffsetToPtr<T>(offset);
  }

  template<typename T>
  class PMemObj {
   public:
    const PMemObj<T>& operator=(const T& other) {
      static_cast<T>(*this) = other;
      // TODO(wgtdkp): Collect
    }
  };

  PACKED(
  struct BlockHeader {
    // Denoting if previous block is free.
    uint32_t free: 1;
    // Block size, include block header.
    uint32_t size: 31;
    // Pointer to previous block in the same free list.
    // Relative to base_.
    uint32_t prev;
    // Pointer to next block in the same free list.
    // Relative to base_.
    uint32_t next;

    static const uint32_t kFreeMask = static_cast<uint32_t>(1) << 31;
    BlockHeader() {}
    ~BlockHeader() {}
  });

  PACKED(
  struct BlockFooter {
    // Denoting if current block is free.
    uint32_t free: 1;
    // Block size, include block header.
    uint32_t size: 31;

    BlockFooter() {}
    ~BlockFooter() {}
  });

  PACKED(
  struct FreeListManager {
    uint32_t free_lists[kNumFreeList];
  });

  uint32_t GetFreeListByBlockOffset(uint32_t blk) {
    auto blk_size = OffsetToPtr<const BlockHeader>(blk)->size;
    return GetFreeListByBlockSize(blk_size);
  }

  uint32_t GetFreeListByBlockSize(uint32_t blk_size) {
    return (blk_size / 16 - 1) * sizeof(uint32_t);
  }

  void SetTheFreeTag(uint32_t blk, uint32_t blk_size) {
    auto next_blk = blk + blk_size;
    Write(next_blk, Read<uint32_t>(next_blk) | BlockHeader::kFreeMask);
  }
  void ResetTheFreeTag(uint32_t blk, uint32_t blk_size) {
    auto next_blk = blk + blk_size;
    Write(next_blk, Read<uint32_t>(next_blk) & ~BlockHeader::kFreeMask);
  }
  uint32_t ReadTheFreeTag(uint32_t blk, uint32_t blk_size) {
    return Read<uint32_t>(blk + blk_size) & BlockHeader::kFreeMask;
  }
  uint32_t ReadThePrevFreeTag(uint32_t blk) {
    return Read<uint32_t>(blk) & BlockHeader::kFreeMask;
  }
  uint32_t AllocBlock(uint32_t size);
  void FreeBlock(uint32_t blk);
  
  // Remove the block from the free list, and it keeps free.
  void RemoveBlock(uint32_t blk, uint32_t blk_size) {
    assert(ReadTheFreeTag(blk, blk_size) != 0);
    auto prev_blk = Read<uint32_t>(blk + offsetof(BlockHeader, prev));
    auto next_blk = Read<uint32_t>(blk + offsetof(BlockHeader, next));
    if (prev_blk != 0) {
      Write(prev_blk + offsetof(BlockHeader, next),
            Read<uint32_t>(blk + offsetof(BlockHeader, next)));
    } else {
      auto free_list = GetFreeListByBlockSize(blk_size);
      Write(free_list, Read<uint32_t>(blk + offsetof(BlockHeader, next)));
    }
    if (next_blk != 0) {
      Write(next_blk + offsetof(BlockHeader, prev),
            Read<uint32_t>(blk + offsetof(BlockHeader, prev)));
    }
  }
  uint32_t SplitBlock(uint32_t blk, uint32_t blk_size, uint32_t needed_size);

 private:
  uintptr_t base_;
  FreeListManager* flm_;
};

} // namespace nvds

#endif // _NVDS_ALLOCATOR_H_
