/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define MYSET_NIBBLE(ptr, p, lenvar, len)           \
    if (len == 3) {                                 \
		*ptr++ = p; *ptr++ = p; *ptr++ = p;     	\
		lenvar -= len;                          	\
	} else if (len == 2) {							\
		*ptr++ = p; *ptr++ = p;                 	\
		lenvar -= len;                          	\
	} else if (len == 1) {							\
		*ptr++ = p;                             	\
		lenvar--;                               	\
    }

#define MYSET_WORDS(ptr, p, lenvar, loops)          \
    uint32_t p32 = (p << 8) | p;                    \
    p32 |= p32 << 16;                               \
    lenvar -= loops << 2;                           \
    uint32_t *ptr32 = (uint32_t *)(ptr);            \
    while (loops--)                                 \
        *ptr32++ = p32;                             \
    ptr =  (uint8_t *)ptr32;

#define MYFILL_VECTOR(vecname, p)                   \
    union {                                         \
        vector uint8_t v;                           \
        uint8_t c[16];                              \
    } p_env;                                        \
    p_env.c[0] = p;                                 \
    vector uint8_t vecname = vec_splat(p_env.v, 0);
    
#define MYSET_ALTIVECWORD(ptr, p, len)              \
    vec_st(p128, 0, ptr);                           \
    ptr += 16; len -= 16;

#define MYSET_LOOP_ALTIVECWORD(ptr, p, len, loops)          \
    while (loops--) {                                       \
        vec_st(p128, 0, ptr);                               \
        vec_st(p128, 16, ptr);                              \
        ptr += 32; len -= 32;                               \
    } 
