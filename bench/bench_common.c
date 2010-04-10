/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#include "config.h"
#include "bench_common.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <getopt.h>
#include <unistd.h>
#include <sqlite3.h>

void show_conf_details(struct bench_conf *conf) {
  printf("Test: %s\n", conf->benchtitle);
  if (conf->flag_aligned)
    printf("Will do aligned tests\n");
  if (conf->flag_random)
    printf("Will do tests on random blocks of data to avoid cache hits\n");
  if (conf->flag_scalar)
    printf("Will do scalar (glibc) tests\n");
  if (conf->flag_vector)
    printf("Will do vector/SIMD (AltiVec/NEON/SSE) tests\n");
  if (conf->flag_custom)
    printf("Will do %s tests\n", conf->custom_title);
  if (conf->flag_nodb)
    printf("Will not write results to an SQLite database\n");
  printf("Number of threads\t: %d\n", conf->nothreads);
  printf("Number of loops \t: %d\n", conf->loops);
  printf("Minimum buffer size\t: %d\n", conf->min_size);
  printf("Maximum buffer size\t: %d\n", conf->max_size);
}

void run_threaded_test(void *(*bench_func)(), struct bench_conf *conf) {
  int maxthreads, thread_counter, size, rc;
  pthread_t tid[MAXTHREADS];
  struct bench_conf *c[MAXTHREADS];

  for (maxthreads = 1; maxthreads <= conf->nothreads; maxthreads <<= 1) {
    printf("title = %s, minsize = %d, maxsize = %d\n", conf->benchtitle, conf->min_size, conf->max_size);

    for (size = conf->min_size; size <= conf->max_size; size <<= 1) {
      for (thread_counter = 0; thread_counter < maxthreads; thread_counter++) {
        c[thread_counter] = malloc(sizeof(struct bench_conf));
        memcpy(c[thread_counter], conf, sizeof(struct bench_conf));
        c[thread_counter]->curthread = thread_counter + 1;
        c[thread_counter]->nothreads = maxthreads;
        c[thread_counter]->size = size;
      }
      for (thread_counter = 0; thread_counter < maxthreads; thread_counter++) {
        // rand() is reseeded in every thread, so that all threads use the same random numbers
        // for comparison reasons.
        srand(size << 5);
        rc = pthread_create(&(tid[thread_counter]), NULL, bench_func, c[thread_counter]);
        if (rc != 0) {
          printf("Error creating thread!\n");
          exit(EXIT_FAILURE);
        }
      }
      for (thread_counter = 0; thread_counter < maxthreads; thread_counter++) {
        rc = pthread_join(tid[thread_counter], NULL);
        free(c[thread_counter]);
      }
    }
  }
}

char *options_help[] = {
  "shows this message",
  "do aligned on a 16-byte boundary tests",
  "do tests on random blocks of data to avoid cache hits",
  "do scalar tests",
  "do vector tests",
  "will not write results to an SQLite database (disabled by default, will use DB)",
  "set the number of maximum threads to do",
  "set the number of loops to perform each time",
  "set minumum size of buffer",
  "set maximum size of buffer"
  "do custom tests",
};

void handle_cmdlineargs(struct bench_conf *conf, int argc, char *argv[]) {
  static struct option long_options[] = {
    /* These options set a flag. */
    {"help",   no_argument,  NULL,  'h'},
    {"aligned",   no_argument,  NULL,  'a'},
    {"random",   no_argument,  NULL,  'r'},
    {"scalar",   no_argument,  NULL,  's'},
    {"vector",   no_argument,  NULL,  'v'},
    {"nodb",   no_argument,  NULL,  'd'},
    /* These options don't set a flag.
     We distinguish them by their indices. */
    {"maxthreads",  required_argument, NULL,  't'},
    {"loops",   required_argument,  NULL,  'l'},
    {"minsize",   required_argument, NULL,  'm'},
    {"maxsize",   required_argument, NULL,  'M'},
    {"custom",   required_argument,  NULL,  'c'},
    {0, 0, 0, 0}
  };

  /* getopt_long stores the option index here. */
  int option_index = 0, opt, i;

  while (1) {
    opt = getopt_long(argc, argv, "harsvdt:l:m:M:c:", long_options, &option_index);

    /* Detect the end of the options. */
    if (opt == -1)
      break;

    switch (opt) {
      case 'a':
        conf->flag_aligned = 1;
        break;
      case 'r':
        conf->flag_random = 1;
        break;
      case 's':
        conf->flag_scalar = 1;
        break;
      case 'v':
        conf->flag_vector = 1;
        break;
      case 'd':
        conf->flag_nodb = 1;
        break;
      case 't':
        conf->nothreads = strtol(optarg, NULL, 10);
        break;
      case 'l':
        conf->loops = strtol(optarg, NULL, 10);
        break;
      case 'm':
        conf->min_size = strtol(optarg, NULL, 10);
        break;
      case 'M':
        conf->max_size = strtol(optarg, NULL, 10);
        break;
      case 'c':
        conf->flag_custom = 1;
        strcpy(conf->custom_title, optarg);
        break;
      case '?':
      case 'h':
        i = 0;
        while (long_options[i].name) {
          printf("--%s  \t: %s\n", long_options[i].name, options_help[i]);
          i++;
        }
        break;
      case 0:
        printf("option %s", long_options[option_index].name);
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0) break;
        if (optarg)
          printf(" with arg %s", optarg);
        printf("\n");
        break;
    }
  }
}

void db_repeat_until_done(struct bench_conf *c, char *cmd) {
  int rc;
  char *zErrMsg = 0;

  rc = sqlite3_open(BENCHMARKSDB, &c->db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(c->db));
    sqlite3_close(c->db);
    exit(EXIT_FAILURE);
  }

  do {
    rc = sqlite3_exec(c->db, cmd, NULL, 0, &zErrMsg);
    if (rc == SQLITE_BUSY) {
      fprintf(stderr, "Database busy, retrying...\n");
      sleep(2*c->curthread);
    } else if (rc == SQLITE_OK) {
      break;
    } else {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      exit(EXIT_FAILURE);
    }
  } while (rc == SQLITE_BUSY);

  sqlite3_close(c->db);
}
