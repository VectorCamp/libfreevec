/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#include <sys/times.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sys/mman.h>

#include "config.h"

void *run_memcpy_test(struct bench_conf *c) {

    int k, l, i, index1, arraysize, size, status = 1;
    double r;

    unsigned char __attribute__((aligned(16))) *test1 = NULL, *test2 = NULL;
    unsigned char *test2a = NULL, *result1, *result2;

    printf("title = %s, minsize = %d, maxsize = %d\n", c->benchtitle, c->min_size, c->max_size);
    
    printf("\n\n**** Same alignment test ****\n\n");

    for (i = 0; i < c->loops; i++) {
        l = k = rand() & 15;
        r = (double)(rand()) / (double)(RAND_MAX);
        r *= (double)(c->max_size);
        size = c->min_size + (int) r;
        size /= 4;
        size++;

        test2 = realloc(test2, size + 8 + l);
        memset(test2, FILLCHAR, size);

        printf("size = %8d, src al. = %x, dst al. = %x \t......", size, k, l);
        arraysize = c->testdatasize / size;
        index1 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
        test1 = (unsigned char *) ALIGN(&c->testdata[index1 *size] + 16) + k;
        test2a = (unsigned char *) ALIGN(test2 + 15) + l;

        result1 = memcpy(test2a, test1, size);
        memset(test2, FILLCHAR, size + l);
        result2 = c->func(test2a, test1, size);

        if (memcmp(test1, test2a, size) == 0 && result1 == result2) {
            printf("PASS\n");
        } else {
            printf("FAIL\n");
            status = 0;
            printf("result1 = %08lx, result2 = %08lx\n", (size_t)result1, (size_t)result2);
            printf("original:\n");
            display_buf((char *)test1, size);
            printf("copy:\n");
            display_buf((char *)test2a, size);
        }
    }

    printf("\n\n**** Random alignment test ****\n\n");

    for (i = 0; i < c->loops; i++) {
        l = rand() & 15;
        k = rand() & 15;
        r = (double)(rand()) / (double)(RAND_MAX);
        r *= (double)(c->max_size);
        size = c->min_size + (int) r;
        size /= 4;
        size++;

        test2 = realloc(test2, size + 8 + l);
        memset(test2, FILLCHAR, size + l);

        printf("size = %8d, src al. = %x, dst al. = %x \t......", size, k, l);
        arraysize = c->testdatasize / size;
        index1 = (arraysize - 2) * (rand() / (RAND_MAX + 1.0));
        test1 = (unsigned char *) ALIGN(&c->testdata[index1 *size] + 16) + k;
        test2a = (unsigned char *) ALIGN(test2 + 15) + l;

        result1 = memcpy(test2a, test1, size);
        memset(test2, FILLCHAR, size + l);
        result2 = c->func(test2a, test1, size);

        if (memcmp(test1, test2a, size) == 0 && result1 == result2) {
            printf("PASS\n");
        } else {
            printf("FAIL\n");
            status = 0;
            printf("result1 = %08lx, result2 = %08lx\n", (size_t)result1, (size_t)result2);
            printf("original:\n");
            display_buf((char *)test1, size);
            printf("copy:\n");
            display_buf((char *)test2a, size);
        }
    }

    if (status)
        printf("\nAll tests have passed!\n");
    else
        printf("\nSome tests have failed!!!\n");

    free(test2);

    return NULL;
}
