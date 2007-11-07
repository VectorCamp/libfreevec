/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under a BSD-type license                     *
 ***************************************************************************/

#include "libfreevec.h"

#define MYNIBBLE_COPY_BWD(dst, src, len)	\
    if (len == 3) {							\
		*--dst = *--src;					\
		*--dst = *--src;					\
		*--dst = *--src;					\
	} else if (len == 2) {					\
		*--dst = *--src;					\
		*--dst = *--src;					\
	} else if (len == 1) {					\
		*--dst = *--src;            		\
    }
    
#define MYCOPY_BWD_UNTIL_DEST_IS_WORD_ALIGNED(dst, src, len)    \
	{															\
		int dstal = ((uint32_t)dst) % sizeof(uint32_t);			\
		if (dstal == 3) {										\
            *--dst = *--src;                                    \
            *--dst = *--src;                                    \
            *--dst = *--src;                                    \
            len -= 3;                                           \
		} else if (dstal == 2) {								\
            *--dst = *--src;                                    \
            *--dst = *--src;                                    \
            len -= 2;                                           \
		} else if (dstal == 1) {								\
            *--dst = *--src;                                    \
            len--;                                              \
		}														\
    }
    
#define MYCOPY_BWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, src, len, srcofst)    \
    while (len >= sizeof(uint32_t) && ((uint32_t)(dst) & 15)) {             \
        if (srcofst == 0) {                                                 \
			*--dst = *--src;                                            	\
		} else if (srcofst == 3) {											\
			*--dst = (*(src-1) << 24) | (*(src) >> 8);                  	\
			--src;                                                      	\
		} else if (srcofst == 2) {											\
			*--dst = (*(src-1) << 16) | (*(src) >> 16);                 	\
			--src;                                                      	\
		} else if (srcofst == 1) {											\
			*--dst = (*(src-1) << 8) | (*(src) >> 24);                  	\
			--src;                                                      	\
        }                                                                   \
        len -= sizeof(uint32_t);                                            \
    }
    
#define MYCOPY_BWD_LOOP_QUADWORD_ALTIVEC_ALIGNED(dst, src, blocks)                  \
    len -= blocks << 6;                                                             \
    while (blocks--) {                                                              \
        dst -= 16; src -= 64;                                                       \
        vec_dst(src, DST_CTRL(2,2,16), DST_CHAN_SRC);                               \
        vec_dstst(dst, DST_CTRL(2,2,16), DST_CHAN_DEST);                            \
        vec_st((vector uint8_t) vec_ld(48, (uint8_t *)src), 48, (uint8_t *)dst);    \
        vec_st((vector uint8_t) vec_ld(32, (uint8_t *)src), 32, (uint8_t *)dst);    \
        vec_st((vector uint8_t) vec_ld(16, (uint8_t *)src), 16, (uint8_t *)dst);    \
        vec_st((vector uint8_t) vec_ld(0, (uint8_t *)src), 0, (uint8_t *)dst);      \
    }
    
#define MYCOPY_BWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED(dst, src, blocks)    \
    vector uint8_t mask, MSQ1, LSQ1, LSQ2, LSQ3, LSQ4;                  \
    mask = vec_lvsl(0, src);                                            \
    len -= blocks << 6;                                                 \
    while (blocks--) {                                                  \
        dst -= 16; src -= 64;                                           \
        vec_dst(src, DST_CTRL(2,2,16), DST_CHAN_SRC);                   \
        vec_dstst(dst, DST_CTRL(2,2,16), DST_CHAN_DEST);                \
        MSQ1 = vec_ld(0, src);                                          \
        LSQ1 = vec_ld(15, src);                                         \
        LSQ2 = vec_ld(31, src);                                         \
        LSQ3 = vec_ld(47, src);                                         \
        LSQ4 = vec_ld(63, src);                                         \
        vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)dst);          \
        vec_st(vec_perm(LSQ1, LSQ2, mask), 16, (uint8_t *)dst);         \
        vec_st(vec_perm(LSQ2, LSQ3, mask), 32, (uint8_t *)dst);         \
        vec_st(vec_perm(LSQ3, LSQ4, mask), 48, (uint8_t *)dst);         \
    }
    
#define MYCOPY_BWD_REST_WORDS(dst, src, len, srcofst)           \
    while (len >= sizeof(uint32_t)) {                           \
        if (srcofst == 0) {                                     \
			*--dst = *--src;                                	\
		} else if (srcofst == 3) {								\
			*--dst = (*(src-1) << 24) | (*(src) >> 8);      	\
			--src;                                          	\
		} else if (srcofst == 2) {								\
			*--dst = (*(src-1) << 16) | (*(src) >> 16);     	\
			--src;                                          	\
		} else if (srcofst == 1) {								\
			*--dst = (*(src-1) << 8) | (*(src) >> 24);      	\
			--src;                                          	\
        }                                                       \
        len -= sizeof(uint32_t);                                \
    }
