/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

static limitless_ctx make_ctx(void) {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  return ctx;
}

static void expect_str(limitless_ctx* ctx, const limitless_number* n, const char* expected) {
  char buf[256];
  assert(limitless_number_to_str(ctx, n, buf, (limitless_size)sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, expected) == 0);
}

static void set_marker(limitless_ctx* ctx, limitless_number* n) {
  assert(limitless_number_from_str(ctx, n, "123/77") == LIMITLESS_OK);
}

static void expect_marker(limitless_ctx* ctx, const limitless_number* n) {
  expect_str(ctx, n, "123/77");
}

static void test_valid_parse_cases(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_cstr(&ctx, &n, "   +42", 0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "42");

  assert(limitless_number_from_cstr(&ctx, &n, "-0", 0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "0");

  assert(limitless_number_from_cstr(&ctx, &n, "0/3", 0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "0");

  assert(limitless_number_from_cstr(&ctx, &n, "-0/302864205528967743915270145433880631669", 0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "0");

  assert(limitless_number_from_cstr(&ctx, &n, "0x10", 0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "16");

  assert(limitless_number_from_cstr(&ctx, &n, "010", 0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "8");

  assert(limitless_number_from_cstr(&ctx, &n, "0b1011", 0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "11");

  assert(limitless_number_from_cstr(&ctx, &n, "-8/-12", 10) == LIMITLESS_OK);
  expect_str(&ctx, &n, "2/3");

  assert(limitless_number_from_cstr(&ctx, &n, "6/3", 10) == LIMITLESS_OK);
  expect_str(&ctx, &n, "2");

  assert(limitless_number_from_cstr(&ctx, &n, "ff", 16) == LIMITLESS_OK);
  expect_str(&ctx, &n, "255");

  assert(limitless_number_from_cstr(&ctx, &n, "z", 36) == LIMITLESS_OK);
  expect_str(&ctx, &n, "35");

  limitless_number_clear(&ctx, &n);
}

static void test_invalid_parse_cases(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  const char* bad[] = {
      "", "+", "-", "/", "1/", "/1", "1//2", "1/2/3", "0x", "0b", "0b102", "0xg", "--1", "++1", "1 2"};
  limitless_size i;
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  for (i = 0; i < (limitless_size)(sizeof(bad) / sizeof(bad[0])); ++i) {
    set_marker(&ctx, &n);
    assert(limitless_number_from_cstr(&ctx, &n, bad[i], 0) == LIMITLESS_EPARSE);
    expect_marker(&ctx, &n);
  }

  set_marker(&ctx, &n);
  assert(limitless_number_from_cstr(&ctx, &n, "12/0", 10) == LIMITLESS_EDIVZERO);
  expect_marker(&ctx, &n);

  set_marker(&ctx, &n);
  assert(limitless_number_from_cstr(&ctx, &n, "10", 1) == LIMITLESS_EPARSE);
  expect_marker(&ctx, &n);

  set_marker(&ctx, &n);
  assert(limitless_number_from_cstr(&ctx, &n, "10", 37) == LIMITLESS_EPARSE);
  expect_marker(&ctx, &n);

  set_marker(&ctx, &n);
  assert(limitless_number_from_cstr(&ctx, &n, "10", -2) == LIMITLESS_EPARSE);
  expect_marker(&ctx, &n);

  limitless_number_clear(&ctx, &n);
}

int main(void) {
  test_valid_parse_cases();
  test_invalid_parse_cases();
  printf("parse edge tests ok\n");
  return 0;
}
