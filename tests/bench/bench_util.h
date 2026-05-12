/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef LIMITLESS_TESTS_BENCH_BENCH_UTIL_H
#define LIMITLESS_TESTS_BENCH_BENCH_UTIL_H

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

typedef struct limitless_bench_report {
  const char* benchmark;
  const char* operation;
  uint64_t iterations;
  uint64_t operations_per_iteration;
  char started_at_utc[32];
  char completed_at_utc[32];
  uint64_t started_at_unix_ns;
  uint64_t completed_at_unix_ns;
  uint64_t duration_ns;
} limitless_bench_report;

static int limitless_bench_timespec_to_ns(time_t seconds, long nanoseconds, uint64_t* out) {
  const uint64_t ns_per_second = UINT64_C(1000000000);
  const uint64_t max_seconds = UINT64_MAX / ns_per_second;
  uint64_t seconds_u64;

  if (out == NULL || seconds < 0 || nanoseconds < 0L || nanoseconds >= (long)ns_per_second) return 0;
  seconds_u64 = (uint64_t)seconds;
  if (seconds_u64 > max_seconds) return 0;
  *out = (seconds_u64 * ns_per_second) + (uint64_t)nanoseconds;
  return 1;
}

static int limitless_bench_capture_utc(char* out, size_t out_size, uint64_t* unix_ns_out) {
#ifdef _WIN32
  FILETIME ft;
  SYSTEMTIME st;
  ULARGE_INTEGER ticks;
  const uint64_t ns_per_100ns_tick = UINT64_C(100);
  /* 1601-01-01 to 1970-01-01 spans 116444736000000000 FILETIME ticks (100 ns units). */
  const uint64_t windows_to_unix_ns = UINT64_C(116444736000000000) * UINT64_C(100);

  if (out == NULL || out_size < 32U || unix_ns_out == NULL) return 0;
  GetSystemTimePreciseAsFileTime(&ft);
  FileTimeToSystemTime(&ft, &st);
  ticks.LowPart = ft.dwLowDateTime;
  ticks.HighPart = ft.dwHighDateTime;
  *unix_ns_out = ticks.QuadPart * ns_per_100ns_tick - windows_to_unix_ns;
  return snprintf(out,
                  out_size,
                  "%04u-%02u-%02uT%02u:%02u:%02u.%03uZ",
                  (unsigned)st.wYear,
                  (unsigned)st.wMonth,
                  (unsigned)st.wDay,
                  (unsigned)st.wHour,
                  (unsigned)st.wMinute,
                  (unsigned)st.wSecond,
                  (unsigned)st.wMilliseconds) > 0;
#else
  struct timespec ts;
  struct tm tm_utc;
  int written;
  uint64_t unix_ns;

  if (out == NULL || out_size < 32U || unix_ns_out == NULL) return 0;
  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) return 0;
  if (gmtime_r(&ts.tv_sec, &tm_utc) == NULL) return 0;
  if (!limitless_bench_timespec_to_ns(ts.tv_sec, ts.tv_nsec, &unix_ns)) return 0;
  *unix_ns_out = unix_ns;
  written = snprintf(out,
                     out_size,
                     "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                     tm_utc.tm_year + 1900,
                     tm_utc.tm_mon + 1,
                     tm_utc.tm_mday,
                     tm_utc.tm_hour,
                     tm_utc.tm_min,
                     tm_utc.tm_sec,
                     (long)(ts.tv_nsec / 1000000L));
  return written > 0 && (size_t)written < out_size;
#endif
}

static int limitless_bench_monotonic_ns(uint64_t* ns_out) {
#ifdef _WIN32
  LARGE_INTEGER counter;
  LARGE_INTEGER frequency;
  uint64_t whole;
  uint64_t remainder;

  if (ns_out == NULL) return 0;
  if (QueryPerformanceCounter(&counter) == 0) return 0;
  if (QueryPerformanceFrequency(&frequency) == 0 || frequency.QuadPart <= 0) return 0;
  whole = ((uint64_t)counter.QuadPart / (uint64_t)frequency.QuadPart) * UINT64_C(1000000000);
  remainder = (((uint64_t)counter.QuadPart % (uint64_t)frequency.QuadPart) * UINT64_C(1000000000)) /
              (uint64_t)frequency.QuadPart;
  *ns_out = whole + remainder;
  return 1;
#else
  struct timespec ts;
  uint64_t monotonic_ns;

  if (ns_out == NULL) return 0;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
  if (!limitless_bench_timespec_to_ns(ts.tv_sec, ts.tv_nsec, &monotonic_ns)) return 0;
  *ns_out = monotonic_ns;
  return 1;
#endif
}

static int limitless_bench_begin(limitless_bench_report* report,
                                 const char* benchmark,
                                 const char* operation,
                                 uint64_t iterations,
                                 uint64_t operations_per_iteration) {
  if (report == NULL || benchmark == NULL || operation == NULL || iterations == 0U ||
      operations_per_iteration == 0U) {
    return 0;
  }
  memset(report, 0, sizeof(*report));
  report->benchmark = benchmark;
  report->operation = operation;
  report->iterations = iterations;
  report->operations_per_iteration = operations_per_iteration;
  return limitless_bench_capture_utc(report->started_at_utc,
                                     sizeof(report->started_at_utc),
                                     &report->started_at_unix_ns) &&
         limitless_bench_monotonic_ns(&report->duration_ns);
}

static int limitless_bench_end(limitless_bench_report* report) {
  uint64_t end_ns = 0U;
  uint64_t start_ns = 0U;

  if (report == NULL) return 0;
  start_ns = report->duration_ns;
  if (!limitless_bench_monotonic_ns(&end_ns)) return 0;
  if (!limitless_bench_capture_utc(report->completed_at_utc,
                                   sizeof(report->completed_at_utc),
                                   &report->completed_at_unix_ns)) {
    return 0;
  }
  report->duration_ns = end_ns - start_ns;
  return 1;
}

static void limitless_bench_print_json(const limitless_bench_report* report) {
  uint64_t total_operations;
  double duration_us;
  double duration_ms;
  double avg_iteration_ns;
  double avg_operation_ns;

  if (report == NULL) return;
  total_operations = report->iterations * report->operations_per_iteration;
  duration_us = (double)report->duration_ns / 1000.0;
  duration_ms = (double)report->duration_ns / 1000000.0;
  avg_iteration_ns = (double)report->duration_ns / (double)report->iterations;
  avg_operation_ns = (double)report->duration_ns / (double)total_operations;

  printf("{\n");
  printf("  \"benchmark\": \"%s\",\n", report->benchmark);
  printf("  \"operation\": \"%s\",\n", report->operation);
  printf("  \"timestamp_clock\": \"utc\",\n");
  printf("  \"duration_clock\": \"monotonic\",\n");
  printf("  \"iterations\": %" PRIu64 ",\n", report->iterations);
  printf("  \"operations_per_iteration\": %" PRIu64 ",\n", report->operations_per_iteration);
  printf("  \"total_operations\": %" PRIu64 ",\n", total_operations);
  printf("  \"started_at_utc\": \"%s\",\n", report->started_at_utc);
  printf("  \"completed_at_utc\": \"%s\",\n", report->completed_at_utc);
  printf("  \"started_at_unix_ns\": %" PRIu64 ",\n", report->started_at_unix_ns);
  printf("  \"completed_at_unix_ns\": %" PRIu64 ",\n", report->completed_at_unix_ns);
  printf("  \"duration_ns\": %" PRIu64 ",\n", report->duration_ns);
  printf("  \"duration_us\": %.3f,\n", duration_us);
  printf("  \"duration_ms\": %.6f,\n", duration_ms);
  printf("  \"avg_iteration_ns\": %.3f,\n", avg_iteration_ns);
  printf("  \"avg_operation_ns\": %.3f\n", avg_operation_ns);
  printf("}\n");
}

#endif
