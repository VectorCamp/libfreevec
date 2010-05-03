/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#define _XOPEN_SOURCE

#include <sys/times.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sys/mman.h>

#include "config.h"

void *run_swab_test(struct bench_conf *c) {
  int i, k, l, index1, arraysize, size, status = 1;
  k = l = 0;

  char *test1 = NULL, *test2 = NULL, *test3 = NULL, *test2a, *test3a;
  double r;

  printf("title = %s, minsize = %d, maxsize = %d\n", c->benchtitle, c->min_size, c->max_size);
  printf("\n\n**** Same alignment test ****\n\n");
  for (i = 0; i < c->loops; i++) {
    l = k = rand() & 15;
    r = (double)(rand()) / (double)(RAND_MAX);
    r *= (double)(c->max_size);
    size = c->min_size + (int32_t) r;
    size++;

    test2 = realloc(test2, size + 16 +l);
    test3 = realloc(test3, size + 16 +l);

    arraysize = c->testdatasize / size;
    index1 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
    test1 = (char *) ALIGN(&c->testdata[index1 *size] + 16) + k;
    test2a = (char *) ALIGN(test2+15) + l;
    test3a = (char *) ALIGN(test3+15) + l;

    memcpy(test2a, test1, size);
    memcpy(test3a, test1, size);
    printf("size = %8d, arraysize = %5d, src al. = %x, dst al. = %x \t......", size, arraysize, k, l);

    swab(test1, test2a, size);
    c->func(test1, test3a, size);
    if (memcmp(test2a, test3a, size) == 0)
      printf("PASS\n");
    else {
      printf("FAIL\n");
      status = 0;
      printf("original:\n");
      display_buf((char *)test1, size);
      printf("glibc swab():\n");
      display_buf((char *)test2a, size);
      printf("libfreevec swab():\n");
      display_buf((char *)test3a, size);
    }
  }

  printf("\n\n**** Random alignment test ****\n\n");
  for (i = 0; i < c->loops/4; i++) {
    l = rand() & 15;
    k = rand() & 15;
    r = (double)(rand()) / (double)(RAND_MAX);
    r *= (double)(c->max_size);
    size = c->min_size + (int32_t) r;

    test2 = realloc(test2, size + 16 +l);
    test3 = realloc(test3, size + 16 +l);

    arraysize = c->testdatasize / size;
    index1 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
    test1 = (char *) ALIGN(&c->testdata[index1 *size] + 16) + k;
    test2a = (char *) ALIGN(test2+15) + l;
    test3a = (char *) ALIGN(test3+15) + l;

    memcpy(test2a, test1, size);
    memcpy(test3a, test1, size);
    printf("size = %8d, arraysize = %5d, src al. = %x, dst al. = %x \t......", size, arraysize, k, l);

    swab(test1, test2a, size);
    c->func(test1, test3a, size);
    if (memcmp(test2a, test3a, size) == 0)
      printf("PASS\n");
    else {
      printf("FAIL\n");
      status = 0;
      printf("original:\n");
      display_buf((char *)test1, size);
      printf("glibc swab():\n");
      display_buf((char *)test2a, size);
      printf("glibc swab():\n");
      display_buf((char *)test3a, size);
    }
  }

  if (status)
    printf("\nAll tests have passed!\n");
  else
    printf("\nSome tests have failed!!!\n");

  return NULL;
}
