/* SPDX-License-Identifier: GPL-3.0-only */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

static void print_err(limitless_status st) {
  printf("ERR:%d\n", (int)st);
}

static int parse_u64_arg(const char* s, limitless_u64* out) {
  char* end = NULL;
  unsigned long long v;
  if (!s || !out || *s == '\0') return 0;
  errno = 0;
  v = strtoull(s, &end, 10);
  if (errno != 0 || !end || *end != '\0') return 0;
  *out = (limitless_u64)v;
  return 1;
}

static int parse_base_arg(const char* s, int* out) {
  char* end = NULL;
  long v;
  if (!s || !out || *s == '\0') return 0;
  errno = 0;
  v = strtol(s, &end, 10);
  if (errno != 0 || !end || *end != '\0') return 0;
  if (v < INT_MIN || v > INT_MAX) return 0;
  *out = (int)v;
  return 1;
}

static int print_number_base(limitless_ctx* ctx, const limitless_number* n, int base) {
  limitless_size need = 0;
  char* out;
  limitless_status st = limitless_number_to_cstr(ctx, n, base, NULL, 0, &need);
  if (st != LIMITLESS_EBUF && st != LIMITLESS_OK) {
    print_err(st);
    return 1;
  }
  out = (char*)malloc((size_t)need + 1u);
  if (!out) {
    print_err(LIMITLESS_EOOM);
    return 1;
  }
  st = limitless_number_to_cstr(ctx, n, base, out, need + 1u, NULL);
  if (st != LIMITLESS_OK) {
    free(out);
    print_err(st);
    return 1;
  }
  printf("OK:%s\n", out);
  free(out);
  return 0;
}

static int print_number(limitless_ctx* ctx, const limitless_number* n) {
  return print_number_base(ctx, n, 10);
}

int main(int argc, char** argv) {
  const char* op;
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  limitless_status st;

  if (argc < 3) {
    fprintf(stderr,
            "usage: %s <op> <a> [b] [c]\n"
            "  unary: neg|abs|parse|to_i64|to_u64\n"
            "  unary+arg: parse_base <a> <base>, fmt <a> <base>, pow <a> <exp>\n"
            "  binary: add|sub|mul|div|gcd|cmp <a> <b>\n"
            "  ternary: modexp <a> <exp> <mod>\n",
            argv[0]);
    return 2;
  }

  op = argv[1];
  st = limitless_ctx_init_default(&ctx);
  if (st != LIMITLESS_OK) {
    print_err(st);
    return 1;
  }
  if (limitless_number_init(&ctx, &a) != LIMITLESS_OK ||
      limitless_number_init(&ctx, &b) != LIMITLESS_OK ||
      limitless_number_init(&ctx, &out) != LIMITLESS_OK) {
    print_err(LIMITLESS_EOOM);
    return 1;
  }

  if (strcmp(op, "neg") == 0 || strcmp(op, "abs") == 0 || strcmp(op, "parse") == 0 ||
      strcmp(op, "to_i64") == 0 || strcmp(op, "to_u64") == 0) {
    st = limitless_number_from_str(&ctx, &a, argv[2]);
    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }

    if (strcmp(op, "neg") == 0) st = limitless_number_neg(&ctx, &out, &a);
    else if (strcmp(op, "abs") == 0) st = limitless_number_abs(&ctx, &out, &a);
    else if (strcmp(op, "parse") == 0) st = limitless_number_copy(&ctx, &out, &a);
    else if (strcmp(op, "to_i64") == 0) {
      limitless_i64 v = 0;
      st = limitless_number_to_i64(&ctx, &a, &v);
      if (st != LIMITLESS_OK) {
        print_err(st);
        goto done;
      }
      printf("OK:%lld\n", (long long)v);
      goto done;
    } else {
      limitless_u64 v = 0;
      st = limitless_number_to_u64(&ctx, &a, &v);
      if (st != LIMITLESS_OK) {
        print_err(st);
        goto done;
      }
      printf("OK:%llu\n", (unsigned long long)v);
      goto done;
    }

    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }
    (void)print_number(&ctx, &out);
    goto done;
  }

  if (strcmp(op, "parse_base") == 0 || strcmp(op, "fmt") == 0 || strcmp(op, "pow") == 0) {
    int base = 0;
    limitless_u64 exp = 0;

    if (argc < 4) {
      fprintf(stderr, "%s requires two operands\n", op);
      return 2;
    }

    if (strcmp(op, "pow") == 0) {
      if (!parse_u64_arg(argv[3], &exp)) {
        fprintf(stderr, "invalid exponent: %s\n", argv[3]);
        return 2;
      }
      st = limitless_number_from_str(&ctx, &a, argv[2]);
      if (st != LIMITLESS_OK) {
        print_err(st);
        goto done;
      }
      st = limitless_number_pow_u64(&ctx, &out, &a, exp);
      if (st != LIMITLESS_OK) {
        print_err(st);
        goto done;
      }
      (void)print_number(&ctx, &out);
      goto done;
    }

    if (!parse_base_arg(argv[3], &base)) {
      fprintf(stderr, "invalid base: %s\n", argv[3]);
      return 2;
    }

    if (strcmp(op, "parse_base") == 0) {
      st = limitless_number_from_cstr(&ctx, &a, argv[2], base);
      if (st != LIMITLESS_OK) {
        print_err(st);
        goto done;
      }
      st = limitless_number_copy(&ctx, &out, &a);
      if (st != LIMITLESS_OK) {
        print_err(st);
        goto done;
      }
      (void)print_number(&ctx, &out);
      goto done;
    }

    st = limitless_number_from_str(&ctx, &a, argv[2]);
    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }
    (void)print_number_base(&ctx, &a, base);
    goto done;
  }

  if (strcmp(op, "modexp") == 0) {
    limitless_u64 exp = 0;
    if (argc < 5) {
      fprintf(stderr, "modexp requires three operands\n");
      return 2;
    }
    if (!parse_u64_arg(argv[3], &exp)) {
      fprintf(stderr, "invalid exponent: %s\n", argv[3]);
      return 2;
    }

    st = limitless_number_from_str(&ctx, &a, argv[2]);
    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }
    st = limitless_number_from_str(&ctx, &b, argv[4]);
    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }

    st = limitless_number_modexp_u64(&ctx, &out, &a, exp, &b);
    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }
    (void)print_number(&ctx, &out);
    goto done;
  }

  if (argc < 4) {
    fprintf(stderr, "binary op requires two operands\n");
    return 2;
  }

  st = limitless_number_from_str(&ctx, &a, argv[2]);
  if (st != LIMITLESS_OK) {
    print_err(st);
    goto done;
  }
  st = limitless_number_from_str(&ctx, &b, argv[3]);
  if (st != LIMITLESS_OK) {
    print_err(st);
    goto done;
  }

  if (strcmp(op, "add") == 0) st = limitless_number_add(&ctx, &out, &a, &b);
  else if (strcmp(op, "sub") == 0) st = limitless_number_sub(&ctx, &out, &a, &b);
  else if (strcmp(op, "mul") == 0) st = limitless_number_mul(&ctx, &out, &a, &b);
  else if (strcmp(op, "div") == 0) st = limitless_number_div(&ctx, &out, &a, &b);
  else if (strcmp(op, "gcd") == 0) st = limitless_number_gcd(&ctx, &out, &a, &b);
  else if (strcmp(op, "cmp") == 0) {
    limitless_status cst = LIMITLESS_OK;
    int cmp = limitless_number_cmp(&ctx, &a, &b, &cst);
    if (cst != LIMITLESS_OK) {
      print_err(cst);
      goto done;
    }
    printf("OK:%d\n", cmp);
    goto done;
  } else {
    fprintf(stderr, "unknown op: %s\n", op);
    return 2;
  }

  if (st != LIMITLESS_OK) {
    print_err(st);
    goto done;
  }
  (void)print_number(&ctx, &out);

done:
  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  return 0;
}
