/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"
#include "macros/common.h"

#define STRCPY_NIBBLE(src, dst, len)  \
  switch(len) {                       \
  case 3:                             \
    *dst++ = *src++;                  \
  case 2:                             \
    *dst++ = *src++;                  \
  case 1:                             \
    *dst++ = *src;                    \
  case 0:                             \
    *dst = 0;                         \
  }

#define STRCPY_UNTIL_DST_WORD_ALIGNED(src, dst, dstal)       \
{                                                            \
  int l = sizeof(uint32_t) - dstal;                          \
  switch (l) {                                               \
  case 3:                                                    \
    if ((*dst++ = *src++) == 0) { *dst = 0; return dstpp; }  \
  case 2:                                                    \
    if ((*dst++ = *src++) == 0) { *dst = 0; return dstpp; }  \
  case 1:                                                    \
    if ((*dst++ = *src++) == 0) { *dst = 0; return dstpp; }  \
  }                                                          \
}

#define STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl)                            \
{                                                                                   \
  uint32_t lw = ~(((*srcl & magic_bits32) + magic_bits32) | *srcl | magic_bits32);  \
  if (lw) {                                                                         \
    uint32_t pos = find_leftfirst_nzb(lw);                                          \
    dst = (int8_t *) dstl;                                                          \
    src = (int8_t *) srcl;                                                          \
    STRCPY_NIBBLE(src, dst, pos);                                                   \
    return dstpp;                                                                   \
  }                                                                                 \
  *dstl++ = *srcl++;                                                                \
}

#define STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal)                 \
{                                                                                 \
  uint32_t srct = 0;                                                              \
  if (srcal == 0) {                                                               \
    srct = *srcl;                                                                 \
  } else if (srcal == 3) {                                                        \
    srct = (*(srcl) << 24) | (*(srcl+1) >> 8);                                    \
  } else if (srcal == 2) {                                                        \
    srct = (*(srcl) << 16) | (*(srcl+1) >> 16);                                   \
  } else if (srcal == 1) {                                                        \
    srct = (*(srcl) << 8) | (*(srcl+1) >> 24);                                    \
  }                                                                               \
  uint32_t lw = ~(((srct & magic_bits32) + magic_bits32) | srct | magic_bits32);  \
  if (lw) {                                                                       \
    uint32_t pos = find_leftfirst_nzb(lw);                                        \
    dst = (int8_t *)(dstl);                                                       \
    src = (int8_t *)(srcl) +srcal;                                                \
    STRCPY_NIBBLE(src, dst, pos);                                                 \
    return dstpp;                                                                 \
  }                                                                               \
  *dstl++ = srct; srcl++;                                                         \
}

#define STRCPY_UNTIL_DST_IS_ALTIVEC_ALIGNED(src, srcl, dst, dstl, srcal, dstal)  \
{                                                                                \
  uint32_t l = (ALTIVECWORD_SIZE - dstal)/sizeof(uint32_t);                      \
  if (srcal == 0) {                                                              \
    switch (l) {                                                                 \
    case 3:                                                                      \
      STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl);                          \
    case 2:                                                                      \
      STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl);                          \
    case 1:                                                                      \
      STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl);                          \
      src = (int8_t *)(srcl) +srcal;                                             \
    }                                                                            \
  } else {                                                                       \
    switch (l) {                                                                 \
    case 3:                                                                      \
      STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);                 \
    case 2:                                                                      \
      STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);                 \
    case 1:                                                                      \
      STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);                 \
      src = (int8_t *)(srcl) +srcal;                                             \
    }                                                                            \
  }                                                                              \
}

#define STRCPY_QUADWORD_ALIGNED(src, srcl, dst, dstl)  \
{                                                      \
  STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl);    \
  STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl);    \
  STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl);    \
  STRCPY_SINGLE_WORD_ALIGNED(src, srcl, dst, dstl);    \
}

#define STRCPY_QUADWORD_UNALIGNED(src, srcl, dst, dstl, srcal)  \
{                                                               \
  STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);    \
  STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);    \
  STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);    \
  STRCPY_SINGLE_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);    \
}

#define STRCPY_SINGLE_ALTIVEC_WORD_ALIGNED(src, srcl, dst, dstl)       \
{                                                                      \
  vector uint8_t  vsrc = (vector uint8_t) vec_ld(0, (uint8_t *)srcl),  \
                  v0 = vec_splat_u8(0);                                \
  if (!vec_all_ne(vsrc, v0)) {                                         \
    STRCPY_QUADWORD_ALIGNED(src, srcl, dst, dstl);                     \
  }                                                                    \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                     \
}

#define STRCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(src, srcl, dst, dstl, srcal)  \
{                                                                          \
  vector uint8_t vsrc, MSQ, LSQ, vmask, v0 = vec_splat_u8(0);              \
  vmask = vec_lvsl(0, src);                                                \
  MSQ = vec_ld(0, src);                                                    \
  LSQ = vec_ld(15, src);                                                   \
  vsrc = vec_perm(MSQ, LSQ, vmask);                                        \
  if (!vec_all_ne(vsrc, v0)) {                                             \
    srcl = (uint32_t *)(src -srcal);                                       \
    STRCPY_QUADWORD_UNALIGNED(src, srcl, dst, dstl, srcal);                \
  }                                                                        \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                        \
}

#define STRCPY_LOOP_ALTIVEC_WORD_ALIGNED(src, srcl, dst, dstl)             \
{                                                                          \
  READ_PREFETCH_START1(srcl);                                              \
  READ_PREFETCH_START2(dstl);                                              \
  while (1) {                                                              \
    vector uint8_t  vsrca = (vector uint8_t) vec_ld(0, (uint8_t *)srcl),   \
                    vsrcb = (vector uint8_t) vec_ld(16, (uint8_t *)srcl),  \
                    v0 = vec_splat_u8(0);                                  \
  if (!vec_all_ne(vsrca, v0)) {                                            \
    STRCPY_QUADWORD_ALIGNED(src, srcl, dst, dstl);                         \
  }                                                                        \
  vec_st(vsrca, 0, (uint8_t *)dstl);                                       \
  srcl += 4; dstl += 4;                                                    \
  if (!vec_all_ne(vsrcb, v0)) {                                            \
    STRCPY_QUADWORD_ALIGNED(src, srcl, dst, dstl);                         \
  }                                                                        \
  vec_st(vsrcb, 0, (uint8_t *)dstl);                                       \
  srcl += 4; dstl += 4;                                                    \
  READ_PREFETCH_START1(srcl);                                              \
  READ_PREFETCH_START2(dstl);                                              \
  }                                                                        \
}

#define STRCPY_LOOP_ALTIVEC_WORD_UNALIGNED(src, srcl, dst, dstl, srcal)  \
{                                                                        \
  READ_PREFETCH_START1(srcl);                                            \
  READ_PREFETCH_START2(dst);                                             \
  while (1) {                                                            \
    STRCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);   \
    dstl += 4; src += ALTIVECWORD_SIZE;                                  \
    STRCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);   \
    dstl += 4; src += ALTIVECWORD_SIZE;                                  \
    READ_PREFETCH_START1(srcl);                                          \
    READ_PREFETCH_START2(dst);                                           \
  }                                                                      \
  srcl = (uint32_t *)(src -srcal);                                       \
}
