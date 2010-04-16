/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#define _XOPEN_SOURCE
#include "config.h"
#include "bench_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>

#define BENCHFUNC run_swab_bench
#define SCALARFUNC swab
#define VECTORFUNC vec_swab

void prepare_dbtable(struct bench_conf *conf) {
  int rc;
  char *zErrMsg = 0;
  char cmd[255];

  rc = sqlite3_open(BENCHMARKSDB, &conf->db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(conf->db));
    sqlite3_close(conf->db);
    exit(EXIT_FAILURE);
  }

  strcpy(cmd, "CREATE TABLE IF NOT EXISTS ");
  strcat(cmd, conf->benchtitle);
  strcat(cmd, " (nothreads INTEGER, curthread INTEGER, size INTEGER, loops INTEGER, dt DOUBLE, bw DOUBLE, al_src INTEGER, al_dst INTEGER, random INTEGER)");
  rc = sqlite3_exec(conf->db, cmd, NULL, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    exit(EXIT_FAILURE);
  }

  sqlite3_close(conf->db);
}

int main(int argc, char *argv[]) {

  static struct bench_conf conf = {
    TABLEPREFIX_swab,   // title
    "",                 // custom title
    0,                  // do aligned
    0,                  // do random
    0,                  // do scalar
    0,                  // do vector
    0,                  // no db (false, will use db by default)
    0,                  // no custom title
    MAXTHREADS,         // thread up to 32 threades
    0,                  // current thread
    LOOPS,              // no of loops
    MINSIZE,            // min len
    MAXSIZE,            // max len
    0,                  // size
    NULL,               // func()
    NULL,               // test buffer
    RANDOMSIZE,         // test buffer size
    NULL                // SQLite DB pointer
  };
  char t[128] = "";
  strcpy(t, conf.benchtitle);

  handle_cmdlineargs(&conf, argc, argv);

  show_conf_details(&conf);

  int randomdata = open("random.dat", O_RDONLY);
  if (!randomdata) {
    printf("Error! Could not open file random.dat!\n");
    exit(EXIT_FAILURE);
  }

  conf.testdata = mmap(0, conf.testdatasize, PROT_READ, MAP_PRIVATE, randomdata, 0);
  if (conf.testdata == MAP_FAILED) {
    printf("Error! Could not mmap random.dat!\n");
    exit(EXIT_FAILURE);
  }

  if (conf.flag_scalar) {
    strcat(conf.benchtitle, "_scalar");

    if (conf.flag_nodb == 0) {
      prepare_dbtable(&conf);
    }
    printf("\nScalar swab()\n\n");
    conf.func = (void *)SCALARFUNC;

    run_threaded_test(BENCHFUNC, &conf);
  }

#ifdef HAVE_LIBFREEVEC
  if (conf.flag_vector) {
    strcpy(conf.benchtitle, t);
    strcat(conf.benchtitle, "_vector");

    if (conf.flag_nodb == 0) {
      prepare_dbtable(&conf);
    }
    printf("\nAltiVec swab()\n\n");

    conf.func = (void *)VECTORFUNC;
    run_threaded_test(BENCHFUNC, &conf);
  }
#endif

  if (conf.flag_custom) {
    strcpy(conf.benchtitle, t);
    strcat(conf.benchtitle, "_");
    strcat(conf.benchtitle, conf.custom_title);

    if (conf.flag_nodb == 0) {
      prepare_dbtable(&conf);
    }
    printf("\n%s swab()\n\n", conf.custom_title);

    conf.func = (void *)SCALARFUNC;
    run_threaded_test(BENCHFUNC, &conf);
  }

  munmap(conf.testdata, conf.testdatasize);
  close(randomdata);

  return EXIT_SUCCESS;
}
