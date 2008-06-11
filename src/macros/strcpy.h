/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define STRCPY_NIBBLE(d, s, l)  \
{                               \
  switch(l) {                   \
  case 3:                       \
    *d++ = *s++;                \
  case 2:                       \
    *d++ = *s++;                \
  case 1:                       \
    *d++ = *s++;                \
  case 0:                       \
    *d = *s;                    \
  }                             \
}

#define STRCPY_UNTIL_DST_WORD_ALIGNED(dst, src, dstal)  \
{                                                       \
  int l = sizeof(uint32_t) - dstal;                     \
  switch (l) {                                          \
  case 3:                                               \
    if ((*dst++ = *src++) == 0) { return dstpp; }       \
  case 2:                                               \
    if ((*dst++ = *src++) == 0) { return dstpp; }       \
  case 1:                                               \
    if ((*dst++ = *src++) == 0) { return dstpp; }       \
  }                                                     \
}

#define STRCPY_SINGLE_WORD_FWD_ALIGNED_MASK(dst, dstl, src, srcl, lw)  \
{                                                                      \
  if (lw) {                                                            \
    src = (int8_t *) srcl;                                             \
    dst = (int8_t *) dstl;                                             \
    uint32_t pos;                                                      \
    FIND_LEFTFIRST_IN_WORD(pos, lw);                                   \
    STRCPY_NIBBLE(dst, src, pos);                                      \
    return dstpp;                                                      \
  }                                                                    \
}

#define STRCPY_SINGLE_WORD_FWD_UNALIGNED_MASK(dst, dstl, src, srcl, srcal, lw)  \
{                                                                               \
  if (lw) {                                                                     \
    src = (int8_t *) srcl +srcal;                                               \
    dst = (int8_t *) dstl;                                                      \
    uint32_t pos;                                                               \
    FIND_LEFTFIRST_IN_WORD(pos, lw);                                            \
    STRCPY_NIBBLE(dst, src, pos);                                               \
    return dstpp;                                                               \
  }                                                                             \
}

#define STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl)          \
{                                                                 \
  uint32_t lw = HAS_ZERO_BYTE(*srcl);                             \
  STRCPY_SINGLE_WORD_FWD_ALIGNED_MASK(dst, dstl, src, srcl, lw);  \
  *dstl++ = *srcl++;                                              \
}

#define STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal)          \
{                                                                          \
  uint32_t srct = *srcl, srct2 = *(srcl+1);                                \
  if (srcal) {                                                             \
    srct = (srct << sh_l) | (srct2 >> sh_r);                               \
  }                                                                        \
  uint32_t lw = HAS_ZERO_BYTE(srct);                                       \
  STRCPY_SINGLE_WORD_FWD_UNALIGNED_MASK(dst, dstl, src, srcl, srcal, lw);  \
  *dstl++ = srct; srcl++;                                                  \
}

#define STRCPY_UNTIL_DST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, srcal, dstal)  \
{                                                                                \
  uint32_t l = (ALTIVECWORD_SIZE - dstal)/sizeof(uint32_t);                      \
  if (srcal == 0) {                                                              \
    switch (l) {                                                                 \
    case 3:                                                                      \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);                          \
    case 2:                                                                      \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);                          \
    case 1:                                                                      \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);                          \
      src = (int8_t *)(srcl);                                                    \
    }                                                                            \
  } else {                                                                       \
    switch (l) {                                                                 \
    case 3:                                                                      \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);                 \
    case 2:                                                                      \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);                 \
    case 1:                                                                      \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);                 \
      src = (int8_t *)(srcl) +srcal;                                             \
    }                                                                            \
  }                                                                              \
}

#define STRCPY_QUADWORD_ALIGNED(dst, dstl, src, srcl)  \
{                                                      \
  STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);    \
  STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);    \
  STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);    \
  STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);    \
}

#define STRCPY_QUADWORD_UNALIGNED(dst, dstl, src, srcl, srcal)  \
{                                                               \
  STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);    \
  STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);    \
  STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);    \
  STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);    \
}

#define STRCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal)  \
{                                                                          \
  vector uint8_t vsrc, MSQ, LSQ, vmask, v0 = vec_splat_u8(0);              \
  vmask = vec_lvsl(0, src);                                                \
  MSQ = (vector uint8_t) vec_ld(0, src);                                   \
  LSQ = (vector uint8_t) vec_ld(15, src);                                  \
  vsrc = vec_perm(MSQ, LSQ, vmask);                                        \
  if (!vec_all_ne(vsrc, v0)) {                                             \
    srcl = (uint32_t *)(src -srcal);                                       \
    STRCPY_QUADWORD_UNALIGNED(dst, dstl, src, srcl, srcal);                \
  }                                                                        \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                        \
}

#define STRCPY_LOOP_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl)            \
{                                                                         \
  do {                                                                    \
    vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)srcl),  \
                    v0 = vec_splat_u8(0);                                 \
    if (!vec_all_ne(vsrc1, v0)) {                                         \
      STRCPY_QUADWORD_ALIGNED(dst, dstl, src, srcl);                      \
    }                                                                     \
    vec_st(vsrc1, 0, (uint8_t *)dstl);                                    \
    srcl += 4; dstl += 4;                                                 \
  } while (1);                                                            \
}

#define STRCPY_LOOP_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal)  \
{                                                                        \
  while (1) {                                                            \
    STRCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);   \
    dstl += 4; src += ALTIVECWORD_SIZE;                                  \
  }                                                                      \
  srcl = (uint32_t *)(src -srcal);                                       \
}
