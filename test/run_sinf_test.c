/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                      *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                          *
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

#define MAX(x, y) (((x) > (y))? (x) : (y))

#include "config.h"

void *run_sinf_test(struct bench_conf *c) {

    struct tms tm0, tm1;
    double ticspersec, t0, t1, dt, dt0, mcalc;

    int l, status = 1, counter1 = 0, counter2 = 0;
    uint64_t cyc1, cyc2;
    float r1, r2, r3, diff1, diff2, maxdiff1 = 0.0, maxdiff2 = 0.0;
    float *x = malloc(c->loops * sizeof(float));

    if (x == NULL)
        exit(1);

    ticspersec = (double) sysconf(_SC_CLK_TCK);

    for (l = 0; l < c->loops; l++) {
        x[l] = M_PI_4 * ((float)rand() / (float)RAND_MAX);
    }

    // Calculate the minimum time spent on other instructions in the loop
    t0 = times(&tm0);

    for (l = 0; l < c->loops; l++)
        ;

    t1 = times(&tm1);

    dt0 = (t1 - t0) / ticspersec;

    t0 = times(&tm0);

    for (l = 0; l < c->loops; l++) {
        r1 = sinf(x[l]);
    }

    t1 = times(&tm1);

    dt = (t1 - t0) / ticspersec - dt0;
    mcalc = (double)(c->loops) / (dt);
    printf("Glibc      : %12.2f calculations of sinf()/sec\n", mcalc);

    t0 = times(&tm0);

    for (l = 0; l < c->loops; l++) {
        r2 = vec_sinf(x[l]);
    }

    t1 = times(&tm1);

    dt = (t1 - t0) / ticspersec - dt0;
    mcalc = (double)(c->loops) / (dt);
    printf("Libfreevec : %12.2f calculations of sinf()/sec\n", mcalc);

    counter1 = 0;
    counter2 = 0;
    maxdiff1 = 0.0;
    maxdiff2 = 0.0;

    for (l = 0; l < c->loops; l++) {
        r1 = sinf(x[l]);
        r2 = vec_sinf(x[l]);

        diff1 = fabs(r1 - r2);

        if (diff1 > 0.0000001) {
            maxdiff1 = MAX(diff1, maxdiff1);
            //printf("#1: l = %3d\tx = %02.7f, r1 = %02.7f, r2 = %02.7f, diff = %02.7f\n", l, x[l], r1, r2, diff1);
            status = 0;
            counter1++;
        }
    }

    printf("vec_sinf fail/tot = %d/%d, maxdiff = %2.7f\n", counter1, c->loops, maxdiff1);

    if (status)
        printf("\nAll tests have passed!\n");
    else
        printf("\nSome tests have failed!!!\n");

    free(x);

    return NULL;
}
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on; 
