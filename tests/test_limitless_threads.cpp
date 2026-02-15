/* SPDX-License-Identifier: GPL-3.0-only */
#include <atomic>
#include <cassert>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#define LIMITLESS_CPP_NO_LEGACY_DEPRECATION 1
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

static void thread_worker(int id, std::atomic<int>* ready, std::atomic<int>* go) {
  limitless_ctx ctx;
  limitless_c_number a;
  limitless_c_number b;
  limitless_c_number c;
  int i;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless_cpp_set_default_ctx(&ctx);
  assert(limitless_cpp_get_default_ctx() == &ctx);
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &c) == LIMITLESS_OK);

  ready->fetch_add(1, std::memory_order_release);
  while (go->load(std::memory_order_acquire) == 0) {
    std::this_thread::yield();
  }

  for (i = 0; i < 600; ++i) {
    limitless_i64 outv;
    limitless_number x = 33424234;
    limitless_number y = (x + id + i) / 2.3f;
    std::string ys = y.str();

    assert(limitless_cpp_get_default_ctx() == &ctx);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
    assert(!ys.empty());

    assert(limitless_number_from_i64(&ctx, &a, (limitless_i64)(id * 1000 + i)) == LIMITLESS_OK);
    assert(limitless_number_from_i64(&ctx, &b, (limitless_i64)(i * 3 + 7)) == LIMITLESS_OK);
    assert(limitless_number_add(&ctx, &c, &a, &b) == LIMITLESS_OK);
    assert(limitless_number_to_i64(&ctx, &c, &outv) == LIMITLESS_OK);
    assert(outv == (limitless_i64)(id * 1000 + i + i * 3 + 7));
  }

  (void)limitless_number::parse("not-a-number", 10);
  assert(limitless_cpp_last_status() == LIMITLESS_EPARSE);

  {
    limitless_number ok = limitless_number::parse("7/3", 10);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
    assert(ok > 2);
  }

  assert(limitless_cpp_get_default_ctx() == &ctx);

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &c);
}

int main() {
  const int thread_count = 8;
  std::atomic<int> ready(0);
  std::atomic<int> go(0);
  std::vector<std::thread> threads;
  int i;

  limitless_ctx main_ctx;
  assert(limitless_ctx_init_default(&main_ctx) == LIMITLESS_OK);
  limitless_cpp_set_default_ctx(&main_ctx);

  for (i = 0; i < thread_count; ++i) {
    threads.push_back(std::thread(thread_worker, i, &ready, &go));
  }

  while (ready.load(std::memory_order_acquire) != thread_count) {
    std::this_thread::yield();
  }
  go.store(1, std::memory_order_release);

  for (i = 0; i < thread_count; ++i) {
    threads[(size_t)i].join();
  }

  assert(limitless_cpp_get_default_ctx() == &main_ctx);
  assert(limitless_cpp_last_status() == LIMITLESS_OK);

  {
    limitless_number x = limitless_number::parse("5", 10);
    limitless_number y = x + 2;
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
    assert(y.str() == "7");
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
  }

  std::puts("thread behavior tests ok");
  return 0;
}
