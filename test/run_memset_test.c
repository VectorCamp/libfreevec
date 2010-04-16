/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

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

void *run_memset_test(struct bench_conf *c) {

  int i, k, size, status = 1;
  uint8_t *test1 = NULL, *test2 = NULL, *test1a, *test2a, fillchar;
  double r;

  printf("title = %s, minsize = %d, maxsize = %d\n", c->benchtitle, c->min_size, c->max_size);
  for (i = 0; i < c->loops; i++) {
    k = rand() & 15;
    r = (double)(rand()) / (double)(RAND_MAX);
    r *= (double)(c->max_size);
    size = c->min_size + (int) r;
    size /= 4;
    size++;

    fillchar = rand();
    printf("test1 = %016x, test2 = %016x, size = %d -> %d\n", test1, test2, size, size + 16 +k);
    test1 = realloc(test1, size + 16 +k);
    test2 = realloc(test2, size + 16 +k);

    test1a = (uint8_t *) ALIGN(test1+15) + k;
    test2a = (uint8_t *) ALIGN(test2+15) + k;

    printf("size = %8d, al. = %x\t......", size, k);
    memset(test1a, fillchar, size);
    c->func(test2a, fillchar, size);
    if (memcmp(test1a, test2a, size) == 0)
      printf("PASS\n");
    else {
      printf("FAIL\n");
      status = 0;
      printf("original:\n");
      display_buf((char *)test1a, size);
      printf("vector:\n");
      display_buf((char *)test2a, size);
    }
  }

  if (status)
    printf("\nAll tests have passed!\n");
  else
    printf("\nSome tests have failed!!!\n");

  free(test1);
  free(test2);
  return NULL;
}
