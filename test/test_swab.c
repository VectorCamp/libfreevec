/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#define _XOPEN_SOURCE
#include <unistd.h>

int main(int argc, char *argv[]) {

  struct bench_conf conf = {
    TABLEPREFIX_swab,   // title
    "",                 // custom title
    1,                  // do aligned
    0,                  // no random
    0,                  // no scalar
    0,                  // do vector
    1,                  // no db
    0,                  // no custom title
    MAXTHREADS,         // fork up to MAXTHREADS processes
    0,                  // current fork
    LOOPS,              // no of loops
    MINSIZE,            // min size
    MAXSIZE,            // max size
    0,                  // size
    NULL,               // func()
    NULL,               // test buffer
    RANDOMSIZE,         // test buffer size
    NULL                // SQLite DB pointer
  };

  printf("test buffer size: %d\n", conf.testdatasize);
  int randomdata = open("random.dat", O_RDONLY);
  if (!randomdata) {
    printf("Error! Could not open file random.dat!\n");
    exit(10);
  }
  conf.testdata = mmap(0, conf.testdatasize, PROT_READ, MAP_PRIVATE, randomdata, 0);
  if (conf.testdata == MAP_FAILED) {
    printf("Error! Could not mmap random.dat!\n");
    exit(10);
  }

#ifdef HAVE_LIBFREEVEC
  conf.func = (void *)vec_swab;
  run_swab_test(&conf);
#endif

  munmap(conf.testdata, conf.testdatasize);
  close(randomdata);
  return EXIT_SUCCESS;
}
