/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#include <sys/times.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sqlite3.h>
#include <pthread.h>

#include "config.h"
#include "bench_common.h"

void *run_mempcpy_bench(struct bench_conf *c) {
  int j, k, l, index1, index2, arraysize, r;
  struct tms tm0, tm1;
  double ticspersec, t0, t1, dt, dt0, bw;

  char cmd[255] = "";

  k = l = 0;
  dt = bw = 0.0;
  ticspersec = (double) sysconf(_SC_CLK_TCK);

  char __attribute__((aligned(16))) *testdata2 = NULL;
  char *test1 = NULL, *test2 = NULL;

  testdata2 = malloc(c->testdatasize);
  if (testdata2 == 0) {
    printf("Error! Could not malloc/realloc %d bytes!\n", c->testdatasize);
    exit(10);
  }
  memset(testdata2, 'b', c->testdatasize);

  arraysize = c->testdatasize / c->size;
  printf("process = %d/%d, size = %d, arraysize = %d\n", c->curthread, c->nothreads, c->size, arraysize);

  // Calculate the minimum time spent on other instructions in the loop
  t0 = times(&tm0);
  index1 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
  index2 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
  test1 = (char *) ALIGN(&c->testdata[index1 *c->size] + 16) + k;
  test2 = (char *) ALIGN(&testdata2[index2 *c->size] + 16) + l;
  t1 = times(&tm1);
  dt0 = (t1 - t0) / ticspersec;

  if (c->flag_aligned) {
    k = l = 0;
  } else {
    r = rand();
    k = 1 + (r & 3);
    l = r & 15;
  }

  if (c->flag_random) {
    t0 = times(&tm0);
    for (j = 0; j < c->loops; j++) {
      index1 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
      index2 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
      test1 = (char *) ALIGN(&c->testdata[index1 *c->size] + 16) + k;
      test2 = (char *) ALIGN(&testdata2[index2 *c->size] + 16) + l;
      c->func(test2, test1, c->size);
    }
    t1 = times(&tm1);
    dt = (t1 - t0) / ticspersec - dt0 * (c->loops);
    bw = (double)(c->loops) * (double)(c->size) / (1048576.0 * dt);
    printf("%d/%d\t\t%8d bytes \t\t%6.2f sec \t%6.2f MB/s \t%d/%d\n", c->nothreads, c->curthread, c->size, dt, bw, k, l);
    if (c->flag_nodb == 0) {
      sprintf(cmd, "INSERT INTO %s VALUES(%d, %d, %d, %d, %f, %f, %d, %d, %d)",
              c->benchtitle, c->nothreads, c->curthread, c->size, c->loops, dt, bw, k, l, c->flag_random);
      db_repeat_until_done(c, cmd);
    }
  } else {
    index1 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
    index2 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
    test1 = (char *) ALIGN(&c->testdata[index1 *c->size] + 16) + k;
    test2 = (char *) ALIGN(&testdata2[index2 *c->size] + 16) + l;

    t0 = times(&tm0);
    for (j = 0; j < c->loops; j++) {
      c->func(test2, test1, c->size);
    }
    t1 = times(&tm1);
    dt = (t1 - t0) / ticspersec;
    bw = (double)(c->loops) * (double)(c->size) / (1048576.0 * dt);
    printf("%d/%d\t\t%8d bytes \t\t%6.2f sec \t%6.2f MB/s \t%d/%d\n", c->nothreads, c->curthread, c->size, dt, bw, k, l);
    if (c->flag_nodb == 0) {
      sprintf(cmd, "INSERT INTO %s VALUES(%d, %d, %d, %d, %f, %f, %d, %d, %d)",
              c->benchtitle, c->nothreads, c->curthread, c->size, c->loops, dt, bw, k, l, c->flag_random);
      db_repeat_until_done(c, cmd);
    }
  }

  free(testdata2);
  return NULL;
}
