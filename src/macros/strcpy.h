/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"
#include "macros/common.h"

#define MYSTRCPY_UNTIL_DEST_WORD_ALIGNED(dstpp, dst, src)   \
    while (((uint32_t)(dst) % sizeof(uint32_t))) {          \
        if ((*dst = *src) == '\0') return dstpp;        	\
        dst++; src++;										\
    }

#define MYSTRCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset)       		\
    if (srcoffset == 0) {                                       		\
		srct = *srcl;                                       			\
	} else {															\
		int diffoffset = sizeof(uint32_t) - srcoffset;					\
		srct = (*(srcl) << srcoffset*8) | (*(srcl+1) >> diffoffset*8);	\
    }                                                           \

#define MYSTRCPY_SINGLE_WORD_old(dstpp, dst, dstl, src, srcl, srct, srcoffset)	\
{																			\
	uint32_t pos, lw = (srct - lomagic) & himagic;							\
    if (lw) {																\
		src = (uint8_t *) srcl +srcoffset;									\
		dst = (uint8_t *) dstl;												\
		pos = find_leftfirst_in_word(lw);									\
		do {																\
			*dst++ = *src++;												\
		} while (pos--);													\
		return dstpp;														\
	}																		\
    *dstl++ = srct;                                                         \
    srcl++;																	\
}

#define MYSTRCPY_SINGLE_WORD(dstpp, dst, dstl, src, srcl, srct, srcoffset)		\
    {																			\
        if (( srct - lomagic) & himagic) {										\
            src = (uint8_t *) srcl +srcoffset;									\
            dst = (uint8_t *) dstl;												\
            if ((*dst = *src) == '\0' ) { VEC_DSS(); return dstpp; }			\
            dst++; src++;														\
            if ((*dst = *src) == '\0' ) { VEC_DSS(); return dstpp; }			\
            dst++; src++;														\
            if ((*dst = *src) == '\0' ) { VEC_DSS(); return dstpp; }			\
            dst++; src++;														\
            if ((*dst = *src) == '\0' ) { VEC_DSS(); return dstpp; }			\
            dst++; src++;														\
        }                                                                       \
    }                                                                           \
    *dstl++ = srct;                                                             \
    srcl++;

#define MYSTRCPY_QUADWORD(dstpp, dst, dstl, src, srcl, srcoffset)               \
	{																			\
		uint32_t srct = 0;                                                      \
		MYSTRCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                      \
		MYSTRCPY_SINGLE_WORD(dstpp, dst, dstl, src, srcl, srct, srcoffset);		\
		MYSTRCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                      \
		MYSTRCPY_SINGLE_WORD(dstpp, dst, dstl, src, srcl, srct, srcoffset);		\
		MYSTRCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                      \
		MYSTRCPY_SINGLE_WORD(dstpp, dst, dstl, src, srcl, srct, srcoffset);		\
		MYSTRCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                      \
		MYSTRCPY_SINGLE_WORD(dstpp, dst, dstl, src, srcl, srct, srcoffset);		\
	}
    
#define MYSTRCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dstpp, dst, dstl, src, srcl, srcoffset)      \
    while (((uint32_t)(dstl) % ALTIVECWORD_SIZE)) {                                         \
        uint32_t srct = 0;                                                                  \
        MYSTRCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                                  \
        MYSTRCPY_SINGLE_WORD(dstpp, dst, dstl, src, srcl, srct, srcoffset);                 \
    }

#define MYSTRCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED_new(dstpp, dst, src)	\
    while (((uint32_t)(dst) % ALTIVECWORD_SIZE)) {          		\
        if ((*dst = *src) == '\0') return dstpp;        			\
        dst++; src++;												\
    }

#define MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dstpp, dst, dstl, src, srcl, srcoffset, v0)   \
	vector uint8_t vsrc;																		\
    while (1) {                                                                                 \
        vsrc = (vector uint8_t) vec_ld(0, (uint8_t *)src);                   					\
        if (vec_any_eq(vsrc, v0)) {                                                         	\
            srcl = (uint32_t *)(src -srcoffset4);                                           	\
            MYSTRCPY_QUADWORD(dstpp, dst, dstl, src, srcl, srcoffset);                      	\
        }                                                                                   	\
        vec_st(vsrc, 0, (uint8_t *)dstl);                                                   	\
        dstl += 4; src += ALTIVECWORD_SIZE;                                                 	\
    }

#define MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dstpp, dst, dstl, src, srcl, srcoffset, v0) \
	vector uint8_t vsrc, MSQ, LSQ, vmask;                                               		\
    vmask = vec_lvsl(0, src);                                                           		\
	MSQ = vec_ld(0, src);                                                               	\
    while (1) {                                                                                 \
		printvec_char("MSQ", MSQ);\
        LSQ = vec_ld(15, src);                                                              	\
		printvec_char("LSQ", LSQ);\
        vsrc = vec_perm(MSQ, LSQ, vmask);                                                   	\
		printvec_char("vsrc", vsrc);\
        if (vec_any_eq(vsrc, v0)) {                                                         	\
            srcl = (uint32_t *)(src -srcoffset);                                           	\
			printf("in here\n");\
            MYSTRCPY_QUADWORD(dstpp, dst, dstl, src, srcl, srcoffset);                      	\
        }                                                                                   	\
		MSQ = LSQ;																				\
        vec_st(vsrc, 0, (uint8_t *)dstl);                                                   	\
        dstl += 4; src += ALTIVECWORD_SIZE;                                                     \
    }
    
/*------------------------------------*/

#define MYSTRCPY_UNTIL_SRC_WORD_ALIGNED_new(dstpp, dst, src)   	\
    while (((uint32_t)(src) % sizeof(uint32_t))) {          \
        if ((*dst = *src) == '\0') return dstpp;        	\
        dst++; src++;										\
    }

#define MYSTRCPY_UNTIL_SRC_IS_ALTIVEC_ALIGNED_new(dstpp, dst, src)	\
    while ((uint32_t)(src) & (ALTIVECWORD_SIZE-1)) {          		\
        if ((*dst = *src) == '\0') return dstpp;        			\
        dst++; src++;												\
    }

#define MYSTRCPY_REST_BYTES_new(dstpp, dst, src)					\
    while (1) {          											\
        if ((*dst = *src) == '\0') return dstpp;        			\
        dst++; src++;												\
    }

#define MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_new(dstpp, dst, dstl, src, srcl, srcoffset, v0)   	\
	vector uint8_t vsrc;																		\
    while (1) {                                                                                 \
        vsrc = (vector uint8_t) vec_ld(0, (uint8_t *)src);                   					\
        if (!vec_all_ne(vsrc, v0)) {                                                         	\
            MYSTRCPY_REST_BYTES_new(dstpp, dst, src);					                      	\
        }                                                                                   	\
		if (((uint32_t)(dst) % ALTIVECWORD_SIZE) == 0) {										\
        	vec_st(vsrc, 0, (uint8_t *)dst);                                                   	\
		} else {																				\
			vector uint8_t vmask = vec_lvsl(0, dst);                                                           \
			printvec_char("vmask", vmask);\
		}																						\
        dst += ALTIVECWORD_SIZE; src += ALTIVECWORD_SIZE;                                                 	\
    }
