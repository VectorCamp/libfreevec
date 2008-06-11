/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define MEMCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, len, src1al) \
{                                                               \
  int l = MIN(len, sizeof(uint32_t) - src1al );                 \
  switch (l) {                                                  \
  case 3:                                                       \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);      \
    src1++; src2++;                                             \
  case 2:                                                       \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);      \
    src1++; src2++;                                             \
  case 1:                                                       \
    if (*src1 != *src2) return CMP_LT_OR_GT(*src1, *src2);      \
    src1++; src2++;                                             \
    len -= l;                                                   \
  }                                                             \
}

#define MEMCMP_SINGLE_WORD_ALIGNED_MASK(src1, src1l, src2, src2l, lw)  \
{                                                                      \
  if (lw) {                                                            \
    uint32_t pos;                                                      \
    FIND_LEFTFIRST_IN_WORD(pos, lw);                                   \
    src2 = (uint8_t *) src2l;                                          \
    src1 = (uint8_t *) src1l;                                          \
    return CMP_LT_OR_GT(src1[pos], src2[pos]);                         \
  }                                                                    \
}

#define MEMCMP_SINGLE_WORD_UNALIGNED_MASK(src1, src1l, src2, src2l, lw)  \
{                                                                        \
  if (lw) {                                                              \
    uint32_t pos;                                                        \
    FIND_LEFTFIRST_IN_WORD(pos, lw);                                     \
    src2 = (uint8_t *)(src2l) +src2al;                                   \
    src1 = (uint8_t *)(src1l);                                           \
    return CMP_LT_OR_GT(src1[pos], src2[pos]);                           \
  }                                                                      \
}

#define MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l)      \
{                                                                 \
  uint32_t lw = *src1l ^ *src2l;                                  \
  MEMCMP_SINGLE_WORD_ALIGNED_MASK(src1, src1l, src2, src2l, lw);  \
  src1l++; src2l++;                                               \
}

#define MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                       \
  uint32_t src2t = 0;                                                   \
  if (src2al == 0) {                                                    \
    src2t = *src2l;                                                     \
  } else {                                                              \
    src2t = (*(src2l) << sh_l) | (*(src2l+1) >> sh_r);                  \
  }                                                                     \
  uint32_t lw = *src1l ^ src2t;                                         \
  MEMCMP_SINGLE_WORD_UNALIGNED_MASK(src1, src1l, src2, src2l, lw);      \
  src1l++; src2l++;                                                     \
}

#define MEMCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, len, src1al, src2al)  \
{                                                                                            \
  uint32_t l = ALTIVECWORD_SIZE - src1al;                                                    \
  l = MIN(len, l);                                                                           \
  l /= sizeof(uint32_t);                                                                     \
  if (src2al == 0) {                                                                         \
    switch (l) {                                                                             \
    case 3:                                                                                  \
      MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                                  \
    case 2:                                                                                  \
      MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                                  \
    case 1:                                                                                  \
      MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                                  \
      len -= l*sizeof(uint32_t);                                                             \
      src2 = (uint8_t *)(src2l);                                                             \
    }                                                                                        \
  } else {                                                                                   \
    switch (l) {                                                                             \
    case 3:                                                                                  \
      MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                        \
    case 2:                                                                                  \
      MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                        \
    case 1:                                                                                  \
      MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                        \
      len -= l*sizeof(uint32_t);                                                             \
      src2 = (uint8_t *)(src2l) +src2al;                                                     \
    }                                                                                        \
  }                                                                                          \
}

#define MEMCMP_QUADWORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
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
}

#define MEMCMP_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l)    \
{                                                                            \
  READ_PREFETCH_START1(src1l);                                               \
  READ_PREFETCH_START2(src2l);                                               \
  while (len >= ALTIVECWORD_SIZE) {                                          \
    vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),    \
                    vsrc2 = (vector uint8_t) vec_ld(0, (uint8_t *)src2l);    \
    if (!vec_all_eq(vsrc1, vsrc2)) {                                         \
      uint32_t __attribute__ ((aligned(16))) lwa[4];                         \
      vsrc1 = (vector uint8_t) vec_cmpeq(vsrc1, vsrc2);                      \
      vsrc1 = vec_nor(vsrc1, vsrc1);                                         \
      vec_st(vsrc1, 0, (uint8_t *) &lwa[0]);                                 \
      MEMCMP_SINGLE_WORD_ALIGNED_MASK(src1, src1l, src2, src2l, lwa[0]);     \
      src1l++; src2l++;                                                      \
      MEMCMP_SINGLE_WORD_ALIGNED_MASK(src1, src1l, src2, src2l, lwa[1]);     \
      src1l++; src2l++;                                                      \
      MEMCMP_SINGLE_WORD_ALIGNED_MASK(src1, src1l, src2, src2l, lwa[2]);     \
      src1l++; src2l++;                                                      \
      MEMCMP_SINGLE_WORD_ALIGNED_MASK(src1, src1l, src2, src2l, lwa[3]);     \
    }                                                                        \
    src1l += 4; src2l += 4; len -= ALTIVECWORD_SIZE;                         \
  }                                                                          \
}

#define MEMCMP_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                                    \
  READ_PREFETCH_START1(src1l);                                                       \
  READ_PREFETCH_START2(src2);                                                        \
  while (len >= ALTIVECWORD_SIZE) {                                                  \
    MEMCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);          \
    src1l += 4; src2 += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                   \
  }                                                                                  \
  src2l = (uint32_t *)(src2 -src2al);                                                \
}

#define MEMCMP_REST_WORDS(src1, src1l, src2, src2l, len, src2al)       \
{                                                                      \
  int l = len / sizeof(uint32_t);                                      \
  if (src2al == 0) {                                                   \
    while (l--) {                                                      \
      MEMCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);            \
      len -= sizeof(uint32_t);                                         \
    }                                                                  \
  } else {                                                             \
    while (l--) {                                                      \
      MEMCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);  \
      len -= sizeof(uint32_t);                                         \
    }                                                                  \
  }                                                                    \
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
  }

