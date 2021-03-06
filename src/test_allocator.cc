#include "allocator.h"
#include "measurement.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;
using namespace nvds;

TEST (AllocatorTest, Alloc) {
  auto base = malloc(Allocator::kSize);
  assert(base != nullptr);

  Allocator a(base);
  Allocator::ModificationList modifications;
  modifications.reserve(20);
  a.set_modifications(&modifications);

  std::vector<uint32_t> collector;

  Measurement alloc_measurement;
  alloc_measurement.begin();
  for (size_t i = 0; i < 1000 * 1000; ++i) {
    modifications.clear();
    auto ptr = a.Alloc(40);
    collector.push_back(ptr);
  }
  alloc_measurement.end();
  alloc_measurement.Print();
  free(base);
}

/*
TEST (AllocatorTest, Init) {
  auto mem = malloc(Allocator::kSize);

  uintptr_t base = reinterpret_cast<uintptr_t>(mem);
  Allocator a(base);
  free(mem);
}

TEST (AllocatorTest, Format) {
  auto mem = malloc(Allocator::kSize);

  uintptr_t base = reinterpret_cast<uintptr_t>(mem);
  Allocator a(base);

  free(mem);
}

TEST (AllocatorTest, Alloc) {
  auto mem = malloc(Allocator::kSize);

  uintptr_t base = reinterpret_cast<uintptr_t>(mem);
  Allocator a(base);

  uintptr_t blk;
  vector<uintptr_t> blks;
  while ((blk = a.Alloc(12)) != 0) {
    blks.push_back(blk);
  }
  ASSERT_EQ(4194285, blks.size());
  cout << "average cnt_writes: " << a.cnt_writes() * 1.0 / blks.size() << endl;
  free(mem);
}

TEST (AllocatorTest, AllocFree) {
  auto mem = malloc(Allocator::kSize);

  uintptr_t base = reinterpret_cast<uintptr_t>(mem);
  Allocator a(base);

  vector<uintptr_t> blks;
  uintptr_t blk;
  while ((blk = a.Alloc(12)) != 0) {
    blks.push_back(blk);
  }
  ASSERT_EQ(4194285, blks.size());

  while (blks.size() > 0) {
    auto blk = blks.back();
    blks.pop_back();
    a.Free(blk);
  }

  while ((blk = a.Alloc(12)) != 0) {
    blks.push_back(blk);
  }
  ASSERT_EQ(4194285, blks.size());

  free(mem);  
}

TEST (AllocatorTest, Random) {
  auto mem = malloc(Allocator::kSize);

  uintptr_t base = reinterpret_cast<uintptr_t>(mem);
  Allocator a(base);

  vector<uintptr_t> blks;
  uintptr_t blk;
  auto g = default_random_engine();
  size_t cnt = 0;
  clock_t begin = clock();
  for (size_t i = 0, j = 0, k = 0; i < 64 * 1024 * 1024; ++i) {
    int op = g() % 2;
    if (op == 0) {
      blk = a.Alloc(g() % (Allocator::kMaxBlockSize - sizeof(uint32_t)));
      if (blk == 0)
        break;
      blks.push_back(blk);
      ++k;
      ++cnt;
    } else if (j < blks.size()) {
      a.Free(blks[j++]);
      ++cnt;
    }
  }
  auto total_time = (clock() - begin) * 1.0 / CLOCKS_PER_SEC;
  cout << "cnt: " << cnt << endl;
  cout << "total time: " << total_time << endl;
  cout << "average time: " << total_time / cnt << endl;
  cout << "average cnt_writes: " << a.cnt_writes() * 1.0 / cnt << endl;

  free(mem);
}

TEST (AllocatorTest, UsageRate) {
  auto mem = malloc(Allocator::kSize);

  uintptr_t base = reinterpret_cast<uintptr_t>(mem);
  Allocator a(base);

  vector<uintptr_t> blks;
  vector<uint32_t> blk_sizes;
  uintptr_t blk;
  auto g = default_random_engine();
  uint32_t total_size = 0;
  size_t j = 0;
  while (true) {
    int op = g() % 3;
    if (op != 0) {
      uint32_t size = g() % (Allocator::kMaxBlockSize - sizeof(uint32_t));
      blk = a.Alloc(size);
      if (blk == 0)
        break;
      blks.push_back(blk);
      blk_sizes.push_back(size);
      total_size += size;
    } else if (j < blks.size()) {
      total_size -= blk_sizes[j];
      a.Free(blks[j++]);
    }
  }

  cout << "usage rate: " << total_size * 1.0 / Allocator::kSize << endl;

  free(mem);
}
*/

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
