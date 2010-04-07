/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

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

#include "config.h"

int main(int argc, char *argv[]) {

    struct bench_conf conf = {
        TABLEPREFIX_trigf,     // title
        "",                    // custom title
        1,                     // do aligned
        0,                     // no random
        0,                     // no scalar
        0,                     // do vector
        1,                     // no db
        0,                     // no custom title
        MAXTHREADS,            // fork up to MAXTHREADS processes
        0,                     // current fork
        100*LOOPS,                 // no of loops
        MINSIZE,               // min size
        MAXSIZE,               // max size
        0,                     // size
        NULL,                  // func()
        NULL,                  // test buffer
        RANDOMSIZE,            // test buffer size
        NULL                   // SQLite DB pointer
    };

    conf.func = (void *)vec_sin;
    run_sin_test(&conf);

    return EXIT_SUCCESS;
}
