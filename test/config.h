/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>
#include <stdint.h>
#include <memory.h>
#include <sqlite3.h>

#define TABLEPREFIX_bmove512 "bmove512"
#define TABLEPREFIX_memccpy  "memccpy"
#define TABLEPREFIX_memcpy  "memcpy"
#define TABLEPREFIX_mempcpy  "mempcpy"
#define TABLEPREFIX_memmove  "memmove"
#define TABLEPREFIX_memchr  "memchr"
#define TABLEPREFIX_memrchr  "memrchr"
#define TABLEPREFIX_memcmp  "memcmp"
#define TABLEPREFIX_memfrob  "memfrob"
#define TABLEPREFIX_memset  "memset"
#define TABLEPREFIX_strlen  "strlen"
#define TABLEPREFIX_strnlen  "strnlen"
#define TABLEPREFIX_strcmp  "strcmp"
#define TABLEPREFIX_strncmp  "strncmp"
#define TABLEPREFIX_strcpy  "strcpy"
#define TABLEPREFIX_strncpy "strncpy"
#define TABLEPREFIX_swab    "swab"
#define TABLEPREFIX_trigf   "trigf"

#define BENCHMARKSDB "benchmarks.db"

#define MAXTHREADS 4

#define MINSIZE     1
#define MAXSIZE     10000
#define LOOPS       100000

#define RANDOMSIZE  6291456

#define ALIGN(x)    ((intptr_t)(x) & 0xfffffffffffffff0)

#define sgn(a)           (((a) < 0) ? -1 : ((a) > 0) ? 1 : 0)

#define EOFCHAR     0x00
#define FILLCHAR    0x20
#define FINDCHAR    'b'
#define TESTCHAR    'Z'

struct bench_conf {
  char benchtitle[64], custom_title[64];
  int flag_aligned, flag_random, flag_scalar, flag_vector, flag_nodb, flag_custom;
  int nothreads;
  int curthread;
  int loops;
  int min_size, max_size, size;
  void *(*func)();
  char __attribute__((aligned(16))) *testdata;
  int testdatasize;
  sqlite3 *db;
};

void display_buf(char *buf, int len);

void *run_memcpy_bench(struct bench_conf *c);
void *run_memccpy_bench(struct bench_conf *c);
void *run_mempcpy_bench(struct bench_conf *c);
void *run_memmove_bench(struct bench_conf *c);
void *run_memchr_bench(struct bench_conf *c);
void *run_memrchr_bench(struct bench_conf *c);
void *run_memcmp_bench(struct bench_conf *c);
void *run_memset_bench(struct bench_conf *c);
void *run_memfrob_bench(struct bench_conf *c);
void *run_swab_bench(struct bench_conf *c);
void *run_strlen_bench(struct bench_conf *c);
void *run_strnlen_bench(struct bench_conf *c);
void *run_strcpy_bench(struct bench_conf *c);
void *run_strncpy_bench(struct bench_conf *c);
void *run_strcmp_bench(struct bench_conf *c);
void *run_strncmp_bench(struct bench_conf *c);
void *run_bmove512_bench(struct bench_conf *c);

void *run_memcpy_test(struct bench_conf *c);
void *run_mempcpy_test(struct bench_conf *c);
void *run_memccpy_test(struct bench_conf *c);
void *run_memmove_test(struct bench_conf *c);
void *run_memchr_test(struct bench_conf *c);
void *run_memrchr_test(struct bench_conf *c);
void *run_memcmp_test(struct bench_conf *c);
void *run_memset_test(struct bench_conf *c);
void *run_memfrob_test(struct bench_conf *c);
void *run_swab_test(struct bench_conf *c);
void *run_strlen_test(struct bench_conf *c);
void *run_strnlen_test(struct bench_conf *c);
void *run_strcpy_test(struct bench_conf *c);
void *run_strncpy_test(struct bench_conf *c);
void *run_strcmp_test(struct bench_conf *c);
void *run_strncmp_test(struct bench_conf *c);
void *run_bmove512_test(struct bench_conf *c);

void *run_trigf_test(struct bench_conf *c);

void *vec_memcpy(void *dstpp, const void *srcpp, size_t len);
void *vec_memcpy_aligned(void *dstpp, const void *srcpp, size_t len);
void *vec_mempcpy(void *dstpp, const void *srcpp, size_t len);
void *mempcpy(void *dstpp, const void *srcpp, size_t len);
void *vec_memcmp(void *src1pp, const void *src2pp, size_t len);
void *vec_memccpy(void *dstpp, const void *srcpp, int c, size_t len);
void *vec_memfrob(void *s, size_t len);
void *vec_memchr(void const *str, int c, size_t len);
void *vec_memrchr(void const *str, int c, size_t len);
void *vec_memmove(void *dstpp, const void *srcpp, size_t len);
void *vec_memset(void *s, int p, size_t len);
void *vec_swab(void *srcpp, const void *dstpp, size_t len);
void *vec_strnlen(const char *str, size_t maxlen);
void *vec_strlen(const int8_t *str);
void *vec_strcpy(int8_t *dstpp, const int8_t *srcpp);
void *vec_strncpy(void *dstpp, const void *srcpp, size_t n);
void *vec_strcmp(const uint8_t *src1pp, const uint8_t *src2pp);
void *vec_strncmp(const uint8_t *src1pp, const uint8_t *src2pp, size_t len);
void vec_bmove512(void *to, const void *from, uint32_t len);

// math functions
float vec_cosf(float x);
float vec_sinf(float x);
float vec_tanf(float x);
float vec_expf(float x);

double vec_cos(double x);
double vec_sin(double x);
double vec_tan(double x);
double vec_exp(double x);

#endif
