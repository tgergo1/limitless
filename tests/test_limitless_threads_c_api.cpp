/* SPDX-License-Identifier: GPL-3.0-only */
#include <atomic>
#include <cassert>
#include <cstdio>
#include <thread>
#include <vector>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

static void worker(int id, std::atomic<int>* failures) {
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  int i;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) {
    failures->fetch_add(1, std::memory_order_relaxed);
    return;
  }
  if (limitless_number_init(&ctx, &a) != LIMITLESS_OK ||
      limitless_number_init(&ctx, &b) != LIMITLESS_OK ||
      limitless_number_init(&ctx, &out) != LIMITLESS_OK) {
    failures->fetch_add(1, std::memory_order_relaxed);
    return;
  }

  for (i = 0; i < 3000; ++i) {
    limitless_i64 outv = 0;
    if (limitless_number_from_i64(&ctx, &a, (limitless_i64)(id * 100000 + i)) != LIMITLESS_OK) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }
    if (limitless_number_from_i64(&ctx, &b, (limitless_i64)(i * 7 + 11)) != LIMITLESS_OK) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }
    if (limitless_number_add(&ctx, &out, &a, &b) != LIMITLESS_OK) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }
    if (limitless_number_to_i64(&ctx, &out, &outv) != LIMITLESS_OK) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }
    if (outv != (limitless_i64)(id * 100000 + i + i * 7 + 11)) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }
  }

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

int main() {
  const int threads = 12;
  std::vector<std::thread> pool;
  std::atomic<int> failures(0);
  int i;

  for (i = 0; i < threads; ++i) {
    pool.push_back(std::thread(worker, i + 1, &failures));
  }
  for (i = 0; i < threads; ++i) {
    pool[(size_t)i].join();
  }

  assert(failures.load(std::memory_order_relaxed) == 0);
  std::puts("C API thread tests ok");
  return 0;
}
