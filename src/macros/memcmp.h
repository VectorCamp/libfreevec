/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define MEMCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, len)      \
{                                                            \
  uint8_t src1al = (uint32_t)(src2) % sizeof(uint32_t);      \
  int l = MIN( len, sizeof(uint32_t) - src1al );             \
  switch (l) {                                               \
  case 3:                                                    \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);   \
    src1++; src2++;                                          \
  case 2:                                                    \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);   \
    src1++; src2++;                                          \
  case 1:                                                    \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);   \
    src1++; src2++;                                          \
    len -= l;                                                \
  }                                                          \
}

#define MEMCMP_SRC_TO_SRC_UNALIGNED(srcl, srct, al)  \
{                                                    \
  if (al == 3) {                                     \
    srct = (*(srcl) << 24) | (*(srcl+1) >> 8);       \
  } else if (al == 2) {                              \
    srct = (*(srcl) << 16) | (*(srcl+1) >> 16);      \
  } else if (al == 1) {                              \
    srct = (*(srcl) << 8) | (*(srcl+1) >> 24);       \
  }                                                  \
}

#define MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l)  \
{                                                             \
  uint32_t lw = *src1l ^ *src2l;                              \
  if (lw) {                                                   \
    uint32_t pos = find_leftfirst_nzb(lw);                    \
    src2 = (uint8_t *) src2l;                                 \
    src1 = (uint8_t *) src1l;                                 \
    return CMP_LT_OR_GT(src1[pos], src2[pos])                 \
  }                                                           \
  src1l++; src2l++;                                           \
}

#define MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                       \
  uint32_t src2t;                                                       \
  if (src2al == 3) {                                                    \
    srct = (*(srcl) << 24) | (*(srcl+1) >> 8);                          \
  } else if (al == 2) {                                                 \
    srct = (*(srcl) << 16) | (*(srcl+1) >> 16);                         \
  } else if (al == 1) {                                                 \
    srct = (*(srcl) << 8) | (*(srcl+1) >> 24);                          \
  }                                                                     \
  uint32_t lw = *src1l ^ src2t;                                         \
  if (lw) {                                                             \
    uint32_t pos = find_leftfirst_nzb(lw);                              \
    src2 = (uint8_t *) src2l +src2al;                                   \
    src1 = (uint8_t *) src1l;                                           \
    return CMP_LT_OR_GT(src1[pos], src2[pos])                           \
  }                                                                     \
  src1l++; src2l++;                                                     \
}

#define MEMCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l)  \
{                                                          \
  MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
  MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
  MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
  MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
}

#define MEMCMP_QUADWORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
}

#define MEMCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, len, src2al)   \
	while (((uint32_t)(src1l) % ALTIVECWORD_SIZE) && (len >= sizeof(uint32_t))) {		\
		uint32_t src2t = 0;																\
		MEMCMP_SRC_TO_SRC_UNALIGNED(src2l, src2t, src2al);						\
		MEMCMP_SINGLE_WORD(src1, src1l, src2, src2l, src2t, src2al);				\
		len -= sizeof(uint32_t);														\
	}

#define MEMCMP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l)     \
{                                                                        \
  vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),  \
                  vsrc2 = (vector uint8_t) vec_ld(0, (uint8_t *)src2l);  \
  if (!vec_all_eq(vsrc1, vsrc2)) {                                       \
    MEMCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l);                   \
  }                                                                      \
}

#define MEMCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                               \
  vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),         \
                  vsrc2, MSQ, LSQ, vmask;                                       \
  vmask = vec_lvsl(0, src2);                                                    \
  MSQ = vec_ld(0, src2);                                                        \
  LSQ = vec_ld(15, src2);                                                       \
  vsrc2 = vec_perm(MSQ, LSQ, vmask);                                            \
  if (!vec_all_eq(vsrc1, vsrc2)) {                                              \
    src2l = (uint32_t *)(src2 -src2al);                                         \
    MEMCMP_QUADWORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                \
  }                                                                             \
  src2l = (uint32_t *)(src2 -src2al);                                           \
}

#define MEMCMP_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l)        \
{                                                                                \
  READ_PREFETCH_START(src1);                                                     \
  while (len >= ALTIVECWORD_SIZE) {                                              \
    MEMCMP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l);                \
    src1l += 4; src2 += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;               \
  }                                                                              \
}

#define MEMCMP_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                                    \
  READ_PREFETCH_START(src1);                                                         \
  READ_PREFETCH_START(src2);                                                         \
  while (len >= ALTIVECWORD_SIZE) {                                                  \
    MEMCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);          \
    src1l += 4; src2 += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                   \
  }                                                                                  \

#define MEMCMP_REST_WORDS(src1, src1l, src2, src2l, len, src2al)  \
{                                                                 \
  int l = len / sizeof(uint32_t);                                 \
  switch (l) {                                                    \
  case 3:                                                         \
    MEMCMP_SINGLE_WORD(ptr32, c, mask, lw);                       \
  case 2:                                                         \
    MEMCHR_SINGLE_WORD(ptr32, c, mask, lw);                       \
  case 1:                                                         \
    MEMCHR_SINGLE_WORD(ptr32, c, mask, lw);                       \
    len -= l*sizeof(uint32_t);                                    \
  }                                                               \
}

#define MEMCMP_NIBBLE(src1, src2, len)                       \
  switch (len) {                                             \
  case 3:                                                    \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);   \
    src1++; src2++;                                          \
  case 2:                                                    \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);   \
    src1++; src2++;                                          \
  case 1:                                                    \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);   \
    src1++; src2++;                                          \
  }                                                          \
