/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#include <stdio.h>
#include <stdint.h>

void display_buf(char *buf, int len) {
    long i;
    printf("%016lx: ", (size_t) buf);
    for (i = 0; i < len; i++) {
        if (i && i % 16 == 0)
            printf("\n%08lx: ", (size_t)(buf+i));
        printf("%02x ", *(buf+i));
    }
    printf("\n");
}


