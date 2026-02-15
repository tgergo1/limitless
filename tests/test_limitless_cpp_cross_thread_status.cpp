/* SPDX-License-Identifier: GPL-3.0-only */
#include <atomic>
#include <cassert>
#include <string>
#include <thread>
#include <vector>

#define LIMITLESS_CPP_NO_LEGACY_DEPRECATION 1
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

static void status_worker(int id, std::atomic<int>* failures) {
  limitless_ctx ctx;
  int i;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) {
    failures->fetch_add(1, std::memory_order_relaxed);
    return;
  }

  limitless::limitless_cpp_set_default_ctx(&ctx);
  if (limitless::limitless_cpp_get_default_ctx() != &ctx) {
    failures->fetch_add(1, std::memory_order_relaxed);
    return;
  }

  for (i = 0; i < 1200; ++i) {
    limitless::number ok = limitless::number::parse("7/3", 10);
    if (limitless::limitless_cpp_last_status() != LIMITLESS_OK) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }

    (void)ok;
    (void)limitless::number::parse("invalid$$$", 10);
    if (limitless::limitless_cpp_last_status() != LIMITLESS_EPARSE) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }

    ok = limitless::number::parse("5", 10);
    if (limitless::limitless_cpp_last_status() != LIMITLESS_OK) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }

    if ((ok + id).str().empty()) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }
    if (limitless::limitless_cpp_last_status() != LIMITLESS_OK) {
      failures->fetch_add(1, std::memory_order_relaxed);
      break;
    }
  }

  limitless::limitless_cpp_set_default_ctx(NULL);
}

int main() {
  const int threads = 10;
  std::vector<std::thread> pool;
  std::atomic<int> failures(0);
  int i;

  for (i = 0; i < threads; ++i) {
    pool.push_back(std::thread(status_worker, i + 1, &failures));
  }
  for (i = 0; i < threads; ++i) {
    pool[(size_t)i].join();
  }

  assert(failures.load(std::memory_order_relaxed) == 0);
  return 0;
}
