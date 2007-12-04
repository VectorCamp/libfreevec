/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define MEMCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, len)     \
	while (((uint32_t)(src1) % sizeof(uint32_t)) && len--) {	\
		if (*src1 == *src2)	{                                   \
			src1++; src2++;                                       \
		} else {                                                \
			return CMP_LT_OR_GT(*src1, *src2);                    \
		}															                          \
	}

#define MEMCMP_SRC_TO_SRC_UNALIGNED(srcl, srct, srcoffset)  \
{                                                           \
	if (srcoffset == 3) {                                     \
		srct = (*(srcl) << 24) | (*(srcl+1) >> 8);              \
	} else if (srcoffset == 2) {                              \
		srct = (*(srcl) << 16) | (*(srcl+1) >> 16);             \
	} else if (srcoffset == 1) {                              \
		srct = (*(srcl) << 8) | (*(srcl+1) >> 24);              \
	}                                                         \
}

#define MEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset)                   \
{                                                                                         \
  if (*src1l != src2t) {                                                                  \
    src2 = (uint8_t *) src2l +src2offset;                                               \
    src1 = (uint8_t *) src1l;                                                           \
    if ((src1[0] != src2[0])) { READ_PREFETCH_STOP; return CMP_LT_OR_GT(src1[0], src2[0]); }     \
    if ((src1[1] != src2[1])) { READ_PREFETCH_STOP; return CMP_LT_OR_GT(src1[1], src2[1]); }     \
    if ((src1[2] != src2[2])) { READ_PREFETCH_STOP; return CMP_LT_OR_GT(src1[2], src2[2]); }     \
    if ((src1[3] != src2[3])) { READ_PREFETCH_STOP; return CMP_LT_OR_GT(src1[3], src2[3]); }     \
  }                                                                                       \
  src1l++; src2l++;                                                                       \

/*
#define MYMEMCMP_QUADWORD_ALIGNED(src1, src2, mask)												\
{	vector uint8_t permMask = {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };		\
    int32_t __attribute__ ((aligned(16))) result[4];											\
    vector bool char SingularMask = vec_and(permMask, (vector uint8_t) mask);					\
	printvec_long("mask", mask);\
	printvec_long("SingularMask", SingularMask);\
    vector int32_t vresult = vec_sum4s((vector int8_t)SingularMask, (vector int32_t)v0);		\
	printvec_long("vresult", vresult);\
    vresult = vec_sums(vresult, (vector int32_t)v0);											\
	printvec_long("vresult", vresult);\
    vec_ste(vresult, 12, result);																\
    int pos = result[3];																		\
	printf("pos = %d\n", pos);\
	return CMP_LT_OR_GT(src1[pos], src2[pos]);													\
}
*/

#define MEMCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l, src2offset)  \
{																		                                     \
  uint32_t src2t = 0;                                                    \
	src2t = *src2l;                                   					           \
  MEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
	src2t = *src2l;                                   					\
  MEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
  src2t = *src2l;                                   					\
  MEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
  src2t = *src2l;                                   					\
  MEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
}

#define MYMEMCMP_QUADWORD_UNALIGNED(src1, src1l, src2, src2l, src2offset)   \
    {																		\
		uint32_t src2t = 0;                                                 \
		MYMEMCMP_SRC_TO_SRC_UNALIGNED(src2l, src2t, src2offset);            \
		MYMEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
		MYMEMCMP_SRC_TO_SRC_UNALIGNED(src2l, src2t, src2offset);            \
		MYMEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
		MYMEMCMP_SRC_TO_SRC_UNALIGNED(src2l, src2t, src2offset);            \
		MYMEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
		MYMEMCMP_SRC_TO_SRC_UNALIGNED(src2l, src2t, src2offset);            \
		MYMEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);  \
	}
    
#define MYMEMCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, len, src2offset)   \
	while (((uint32_t)(src1l) % ALTIVECWORD_SIZE) && (len >= sizeof(uint32_t))) {		\
		uint32_t src2t = 0;																\
		MYMEMCMP_SRC_TO_SRC_UNALIGNED(src2l, src2t, src2offset);						\
		MYMEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);				\
		len -= sizeof(uint32_t);														\
	}

#define MYMEMCMP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l, src2offset)  \
    {                                                                               \
        vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),       \
                        vsrc2 = (vector uint8_t) vec_ld(0, (uint8_t *)src2);        \
        if (!vec_all_eq(vsrc1, vsrc2)) {                                  			\
			src2l = (uint32_t *)(src2 -src2offset);                                 \
            MYMEMCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l, src2offset);        \
        }                                                                           \
    }

#define MYMEMCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2offset)    \
    {                                                                                   \
        vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),           \
                        vsrc2, MSQ, LSQ, vmask;                                         \
        vmask = vec_lvsl(0, src2);                                                      \
        MSQ = vec_ld(0, src2);                                                          \
        LSQ = vec_ld(15, src2);                                                         \
        vsrc2 = vec_perm(MSQ, LSQ, vmask);                                              \
        if (!vec_all_eq(vsrc1, vsrc2)) {                                                \
            src2l = (uint32_t *)(src2 -src2offset);                                     \
            MYMEMCMP_QUADWORD_UNALIGNED(src1, src1l, src2, src2l, src2offset);          \
        }                                                                               \
    }

#define MYMEMCMP_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l, src2offset) \
    vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                      \
    vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                     \
    while (len >= ALTIVECWORD_SIZE) {                                                   \
        MYMEMCMP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l, src2offset)      \
        src1l += 4; src2 += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                  \
    }

#define MYMEMCMP_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2offset)   \
    vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                          \
    vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                         \
    while (len >= ALTIVECWORD_SIZE) {                                                       \
        MYMEMCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2offset)        \
        src1l += 4; src2 += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                      \
    }

#define MYMEMCMP_LOOP_QUAD_ALTIVEC_WORD_ALIGNEDorig(src1, src1l, src2, src2l, src2offset)    \
    vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                      \
    vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                     \
    while (len >= 4*ALTIVECWORD_SIZE) {                                                 \
        vector uint8_t  vsrc1a, vsrc1b, vsrc1c, vsrc1d,                                 \
                        vsrc2a, vsrc2b, vsrc2c, vsrc2d;                                 \
        int res1, res2, res3, res4;                                                     \
        vsrc1a = (vector uint8_t) vec_ld(0, (uint8_t *)src1l);                          \
        vsrc2a = (vector uint8_t) vec_ld(0, (uint8_t *)src2);                           \
        res1 = vec_any_ne(vsrc1a, vsrc2a);                                              \
        vsrc1b = (vector uint8_t) vec_ld(16, (uint8_t *)src1l);                         \
        vsrc2b = (vector uint8_t) vec_ld(16, (uint8_t *)src2);                          \
        res2 = vec_any_ne(vsrc1b, vsrc2b);                                              \
        vsrc1c = (vector uint8_t) vec_ld(32, (uint8_t *)src1l);                         \
        vsrc2c = (vector uint8_t) vec_ld(32, (uint8_t *)src2);                          \
        res3 = vec_any_ne(vsrc1c, vsrc2c);                                              \
        vsrc1d = (vector uint8_t) vec_ld(48, (uint8_t *)src1l);                         \
        vsrc2d = (vector uint8_t) vec_ld(48, (uint8_t *)src2);                          \
        res4 = vec_any_ne(vsrc1d, vsrc2d);                                              \
        if (res1|res2|res3|res4) {                                                      \
            if (res1) {                                                                 \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
            if (res2) {                                                                 \
                src1l += 4; src2 += ALTIVECWORD_SIZE;                                   \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
            if (res3) {                                                                 \
                src1l += 8; src2 += 2*ALTIVECWORD_SIZE;                                 \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
            if (res4) {                                                                 \
                src1l += 12; src2 += 3*ALTIVECWORD_SIZE;                                \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
        }                                                                               \
        src1l += 16; src2 += 4*ALTIVECWORD_SIZE;                                        \
        len -= 4* ALTIVECWORD_SIZE;                                                     \
        vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                  \
        vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                 \
    }

#define MYMEMCMP_LOOP_QUAD_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l, src2offset)	\
    vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                      \
    vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                     \
    while (len >= 4*ALTIVECWORD_SIZE) {                                                 \
        vector uint8_t  vsrc1a, vsrc1b, vsrc1c, vsrc1d,                                 \
                        vsrc2a, vsrc2b, vsrc2c, vsrc2d;                                 \
		vector bool char mask1, mask2, mask3, mask4;									\
		vector int32_t sum;																\
		vector uint8_t v0 = vec_splat_u8(0);											\
        vsrc1a = (vector uint8_t) vec_ld(0, (uint8_t *)src1l);                          \
        vsrc2a = (vector uint8_t) vec_ld(0, (uint8_t *)src2);                           \
        vsrc1b = (vector uint8_t) vec_ld(16, (uint8_t *)src1l);                         \
        vsrc2b = (vector uint8_t) vec_ld(16, (uint8_t *)src2);                          \
        vsrc1c = (vector uint8_t) vec_ld(32, (uint8_t *)src1l);                         \
        vsrc2c = (vector uint8_t) vec_ld(32, (uint8_t *)src2);                          \
        vsrc1d = (vector uint8_t) vec_ld(48, (uint8_t *)src1l);                         \
        vsrc2d = (vector uint8_t) vec_ld(48, (uint8_t *)src2);                          \
		mask1 = vec_cmpeq(vsrc1a, vsrc2a);                           					\
        mask2 = vec_cmpeq(vsrc1b, vsrc2b);                             					\
        mask3 = vec_cmpeq(vsrc1c, vsrc2c);                             					\
        mask4 = vec_cmpeq(vsrc1d, vsrc2d);                             					\
		mask1 = vec_nor(mask1, mask1);													\
		mask2 = vec_nor(mask2, mask2);													\
		mask3 = vec_nor(mask3, mask3);													\
		mask4 = vec_nor(mask4, mask4);													\
		printvec_long("mask1", mask1);\
		printvec_long("mask2", mask2);\
		printvec_long("mask3", mask3);\
		printvec_long("mask4", mask4);\
		sum = vec_sums((vector int32_t)mask1, (vector int32_t)mask2);					\
		printvec_long("sum", sum);\
		sum = vec_sums(sum, (vector int32_t)mask3);										\
		printvec_long("sum", sum);\
		sum = vec_sums(sum, (vector int32_t)mask4);										\
		printvec_long("sum", sum);\
        if (!vec_all_eq((vector uint8_t) sum, v0)) {                                  	\
			src2l = (uint32_t *)(src2 -src2offset);                                 	\
			if (!vec_all_eq(mask1, v0))													\
				MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
			src1l += 4; src2 += ALTIVECWORD_SIZE;                                   	\
			src2l = (uint32_t *)(src2 -src2offset);                                 	\
			if (!vec_all_eq(mask2, v0))													\
				MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
			src1l += 8; src2 += 2*ALTIVECWORD_SIZE;                                 	\
			src2l = (uint32_t *)(src2 -src2offset);                                 	\
			if (!vec_all_eq(mask3, v0))													\
				MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);               	\
			src1l += 12; src2 += 3*ALTIVECWORD_SIZE;                                	\
			src2l = (uint32_t *)(src2 -src2offset);                                 	\
			if (!vec_all_eq(mask4, v0))													\
				MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
        }                                                                               \
        src1l += 16; src2 += 4*ALTIVECWORD_SIZE;                                        \
        len -= 4* ALTIVECWORD_SIZE;                                                     \
        vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                  \
        vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                 \
    }


#define MYMEMCMP_LOOP_QUAD_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2offset)  \
    vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                      \
    vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                     \
    while (len >= 4*ALTIVECWORD_SIZE) {                                                 \
        vector uint8_t  vsrc1a, vsrc1b, vsrc1c, vsrc1d,                                 \
                        vsrc2a, vsrc2b, vsrc2c, vsrc2d, vmask, LSQ;                     \
        int res1, res2, res3, res4;                                                     \
        vsrc1a = (vector uint8_t) vec_ld(0, (uint8_t *)src1l);                          \
        vsrc1b = (vector uint8_t) vec_ld(16, (uint8_t *)src1l);                         \
        vsrc1c = (vector uint8_t) vec_ld(32, (uint8_t *)src1l);                         \
        vsrc1d = (vector uint8_t) vec_ld(48, (uint8_t *)src1l);                         \
        vmask = vec_lvsl(0, src2);                                                      \
        vsrc2a = (vector uint8_t) vec_ld(0, (uint8_t *)src2);                           \
        vsrc2b = (vector uint8_t) vec_ld(15, (uint8_t *)src2);                          \
        vsrc2c = (vector uint8_t) vec_ld(31, (uint8_t *)src2);                          \
        vsrc2d = (vector uint8_t) vec_ld(47, (uint8_t *)src2);                          \
        LSQ = (vector uint8_t) vec_ld(63, (uint8_t *)src2);                             \
        vsrc2a = vec_perm(vsrc2a, vsrc2b, vmask);                                       \
        res1 = vec_any_ne(vsrc1a, vsrc2a);                                              \
        vsrc2b = vec_perm(vsrc2b, vsrc2c, vmask);                                       \
        res2 = vec_any_ne(vsrc1b, vsrc2b);                                              \
        vsrc2c = vec_perm(vsrc2c, vsrc2d, vmask);                                       \
        res3 = vec_any_ne(vsrc1c, vsrc2c);                                              \
        vsrc2d = vec_perm(vsrc2d, LSQ, vmask);                                          \
        res4 = vec_any_ne(vsrc1d, vsrc2d);                                              \
        if (res1|res2|res3|res4) {                                                      \
            if (res1) {                                                                 \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
            if (res2) {                                                                 \
                src1l += 4; src2 += ALTIVECWORD_SIZE;                                   \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
            if (res3) {                                                                 \
                src1l += 8; src2 += 2*ALTIVECWORD_SIZE;                                 \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
            if (res4) {                                                                 \
                src1l += 12; src2 += 3*ALTIVECWORD_SIZE;                                \
                src2l = (uint32_t *)(src2 -src2offset);                                 \
                MYMEMCMP_QUADWORD(src1, src1l, src2, src2l, src2offset);                \
            }                                                                           \
        }                                                                               \
        src1l += 16; src2 += 4*ALTIVECWORD_SIZE;                                        \
        len -= 4* ALTIVECWORD_SIZE;                                                     \
        vec_dst(src2, DST_CTRL(2,2,32), DST_CHAN_SRC);                                  \
        vec_dst(src1, DST_CTRL(2,2,32), DST_CHAN_DEST);                                 \
    }
    
#define MYMEMCMP_REST_WORDS(src1, src1l, src2, src2l, len, src2offset)              \
    while (len >= sizeof(uint32_t)) {                                               \
        uint32_t src2t = 0;                                                         \
        MYMEMCMP_SRC_TO_SRC_UNALIGNED(src2l, src2t, src2offset);                    \
        MYMEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2offset);          \
        len -= sizeof(uint32_t);                                                    \
    }

#define MEMCMP_NIBBLE(src1, src2, len)                                        \
	if (len == 3) {																\
		if ((src1[0] != src2[0])) return CMP_LT_OR_GT(src1[0], src2[0]);		\
		if ((src1[1] != src2[1])) return CMP_LT_OR_GT(src1[1], src2[1]);		\
		if ((src1[2] != src2[2])) return CMP_LT_OR_GT(src1[2], src2[2]);		\
	} else if (len == 2) {														\
		if ((src1[0] != src2[0])) return CMP_LT_OR_GT(src1[0], src2[0]);    	\
		if ((src1[1] != src2[1])) return CMP_LT_OR_GT(src1[1], src2[1]);    	\
	} else if (len == 1) {														\
        if ((src1[0] != src2[0])) return CMP_LT_OR_GT(src1[0], src2[0]);		\
    }
