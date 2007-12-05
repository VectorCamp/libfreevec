/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define COPY_FWD_NIBBLE(dst, src, len)    \
  switch(len) {                           \
  case 3:                                 \
    *dst++ = *src++;                      \
  case 2:                                 \
    *dst++ = *src++;                      \
  case 1:                                 \
    *dst = *src;                          \
  }


#define COPY_FWD_UNTIL_DEST_IS_WORD_ALIGNED(dst, src, len)    \
{                                                             \
  int dstal = ((uint32_t)dst) % sizeof(uint32_t);             \
  switch (dstal) {                                            \
  case 3:                                                     \
    *dst++ = *src++;                                          \
  case 2:                                                     \
    *dst++ = *src++;                                          \
  case 1:                                                     \
    *dst++ = *src++;                                          \
  }                                                           \
  len -= dstal;                                               \
}

#define COPY_FWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, src, len, srcofst)       \
  while (len >= sizeof(uint32_t) && ((uint32_t)(dst) % ALTIVECWORD_SIZE)) {  \
    if (srcofst == 0) {                                                      \
      *dst++ = *src++;                                                       \
    } else if (srcofst == 3) {                                               \
      *dst++ = (*(src) << 24) | (*(src+1) >> 8);                             \
      src++;                                                                 \
    } else if (srcofst == 2) {                                               \
      *dst++ = (*(src) << 16) | (*(src+1) >> 16);                            \
      src++;                                                                 \
    } else if (srcofst == 1) {                                               \
      *dst++ = (*(src) << 8) | (*(src+1) >> 24);                             \
      src++;                                                                 \
    }                                                                        \
    len -= sizeof(uint32_t);                                                 \
  }

#define COPY_SINGLEQUADWORD_ALTIVEC_ALIGNED(dst, src, step)         \
    vec_st((vector uint8_t) vec_ld(step, (uint8_t *)src),           \
                step, (uint8_t *)dst);

#define COPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED(dst, src, step)  \
{                                                              \
  vector uint8_t MSQ, LSQ, mask;                               \
  mask = vec_lvsl(0, src);                                     \
  MSQ = vec_ld(step, src);                                     \
  LSQ = vec_ld(step+15, src);                                  \
  vec_st(vec_perm(MSQ, LSQ, mask), step, (uint8_t *)dst);      \
}

#define COPY_FWD_LOOP_QUADWORD_ALTIVEC_ALIGNED(dst, src, len)                 \
{                                                                             \
  uint32_t blocks = len >> LOG_ALTIVECQUAD;                                   \
  len -= blocks << LOG_ALTIVECQUAD;                                           \
  while (blocks--) {                                                          \
    vec_st((vector uint8_t) vec_ld(0, (uint8_t *)src), 0, (uint8_t *)dst);    \
    vec_st((vector uint8_t) vec_ld(16, (uint8_t *)src), 16, (uint8_t *)dst);  \
    vec_st((vector uint8_t) vec_ld(32, (uint8_t *)src), 32, (uint8_t *)dst);  \
    vec_st((vector uint8_t) vec_ld(48, (uint8_t *)src), 48, (uint8_t *)dst);  \
    dst += ALTIVECWORD_SIZE; src += QUAD_ALTIVECWORD;                         \
  }                                                                           \
}

#define COPY_FWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED(dst, src, len)  \
{                                                                \
  vector uint8_t mask, MSQ1, LSQ1, LSQ2, LSQ3, LSQ4;             \
  uint32_t blocks = len >> LOG_ALTIVECQUAD ;                     \
  len -= blocks << LOG_ALTIVECQUAD;                              \
  mask = vec_lvsl(0, src);                                       \
  while (blocks--) {                                             \
    MSQ1 = vec_ld(0, src);                                       \
    LSQ1 = vec_ld(15, src);                                      \
    LSQ2 = vec_ld(31, src);                                      \
    LSQ3 = vec_ld(47, src);                                      \
    LSQ4 = vec_ld(63, src);                                      \
    vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)dst);       \
    vec_st(vec_perm(LSQ1, LSQ2, mask), 16, (uint8_t *)dst);      \
    vec_st(vec_perm(LSQ2, LSQ3, mask), 32, (uint8_t *)dst);      \
    vec_st(vec_perm(LSQ3, LSQ4, mask), 48, (uint8_t *)dst);      \
    dst += ALTIVECWORD_SIZE; src += QUAD_ALTIVECWORD;            \
  }                                                              \
}

#define COPY_FWD_REST_WORDS(dst, src, len, srcofst)  \
  while (len >= sizeof(uint32_t)) {                  \
    if (srcofst == 0) {                              \
      *dst++ = *src++;                               \
    } else if (srcofst == 3) {                       \
      *dst++ = (*(src) << 24) | (*(src+1) >> 8);     \
      src++;                                         \
    } else if (srcofst == 2) {                       \
      *dst++ = (*(src) << 16) | (*(src+1) >> 16);    \
      src++;                                         \
    } else if (srcofst == 1) {                       \
      *dst++ = (*(src) << 8) | (*(src+1) >> 24);     \
      src++;                                         \
    }                                                \
    len -= sizeof(uint32_t);                         \
  }

/***************************************************************************
 *                                                                         *
 * memccpy() macros, copy/searching                                        *
 **************************************************************************/

#define MEMCCPY_UNTIL_DEST_WORD_ALIGNED(dst, src, c, len, al) \
{                                                             \
  int l = MIN( len, sizeof(uint32_t) - al );                  \
  switch (l) {                                                \
  case 3:                                                     \
    if ((*dst = *src) == c) return dst;                       \
    dst++; src++;                                             \
  case 2:                                                     \
    if ((*dst = *src) == c) return dst;                       \
    dst++; src++;                                             \
  case 1:                                                     \
    if ((*dst = *src) == c) return dst;                       \
    dst++; src++;                                             \
    len -= l;                                                 \
  }                                                           \
}

#define MEMCCPY_CHECK_WORD(dst, src, c)       \
{                                             \
  if ((*dst++ = *src++) == c ) return dst;    \
  if ((*dst++ = *src++) == c ) return dst;    \
  if ((*dst++ = *src++) == c ) return dst;    \
  if ((*dst++ = *src++) == c ) return dst;    \
}


#define MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c) \
{                                                                      \
   uint32_t lw = ((*srcl ^ mask) - lomagic) & himagic;                 \
   if (lw) {                                                           \
     src = (uint8_t *) srcl;                                           \
     dst = (uint8_t *) dstl;                                           \
     MEMCCPY_CHECK_WORD(dst, src, c);                                  \
   }                                                                   \
   *dstl++ = *srcl++;                                                  \
}

#define MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c) \
{                                                                                   \
  uint32_t srct = 0;                                                                \
  if (srcoffset == 0) {                                                             \
    srct = *srcl;                                                                   \
  } else if (srcoffset == 3) {                                                      \
    srct = (*(srcl) << 24) | (*(srcl+1) >> 8);                                      \
  } else if (srcoffset == 2) {                                                      \
    srct = (*(srcl) << 16) | (*(srcl+1) >> 16);                                     \
  } else if (srcoffset == 1) {                                                      \
    srct = (*(srcl) << 8) | (*(srcl+1) >> 24);                                      \
  }                                                                                 \
  uint32_t lw = ( (srct ^ mask) - lomagic) & himagic;                               \
  if (lw) {                                                                         \
    src = (uint8_t *) srcl +srcoffset;                                              \
    dst = (uint8_t *) dstl;                                                         \
    MEMCCPY_CHECK_WORD(dst, src, c);                                                \
  }                                                                                 \
  *dstl++ = srct;                                                                   \
  srcl++;                                                                           \
}

#define MEMCCPY_QUADWORD_ALIGNED(dst, dstl, src, srcl, mask, c)   \
{                                                                 \
  MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c); \
  MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c); \
  MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c); \
  MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c); \
}

#define MEMCCPY_QUADWORD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c)   \
{                                                                              \
  MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c); \
  MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c); \
  MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c); \
  MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c); \
}

#define MEMCCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, len, srcoffset, mask, c, al) \
{                                                                                                \
  int l = (ALTIVECWORD_SIZE - al) / sizeof(uint32_t);                                            \
  if (srcoffset == 0) {                                                                          \
    switch (l) {                                                                                 \
    case 3:                                                                                      \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c);                            \
    case 2:                                                                                      \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c);                            \
    case 1:                                                                                      \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c);                            \
      len -= l*sizeof(uint32_t);                                                                 \
    }                                                                                            \
  } else {                                                                                       \
    switch (l) {                                                                                 \
    case 3:                                                                                      \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c);               \
    case 2:                                                                                      \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c);               \
    case 1:                                                                                      \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c);               \
      len -= l*sizeof(uint32_t);                                                                 \
    }                                                                                            \
  }                                                                                              \
}

#define MEMCCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, mask, vc, c) \
{                                                                              \
  vector uint8_t vsrc = (vector uint8_t) vec_ld(0, (uint8_t *)src);            \
  if (vec_any_eq(vsrc, vc)) {                                                  \
    srcl = (uint32_t *)(src);                                                  \
    MEMCCPY_QUADWORD_ALIGNED(dst, dstl, src, srcl, mask, c);                   \
  }                                                                            \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                            \
}

#define MEMCCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c) \
{                                                                                           \
  vector uint8_t vsrc, MSQ, LSQ, vmask;                                                     \
  vmask = vec_lvsl(0, src);                                                                 \
  MSQ = vec_ld(0, src);                                                                     \
  LSQ = vec_ld(15, src);                                                                    \
  vsrc = vec_perm(MSQ, LSQ, vmask);                                                         \
  if (vec_any_eq(vsrc, vc)) {                                                               \
    srcl = (uint32_t *)(src -srcoffset4);                                                   \
    MEMCCPY_QUADWORD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c);                   \
  }                                                                                         \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                                         \
}

#define MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, len, mask, vc, c) \
  while (len >= ALTIVECWORD_SIZE) {                                                      \
    MEMCCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, mask, vc, c)               \
    dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                         \
    READ_PREFETCH_START1(src);                                                            \
    WRITE_PREFETCH_START2(dst);                                                           \
  }

#define MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, len, srcoffset, mask, vc, c) \
  while (len >= ALTIVECWORD_SIZE) {                                                                   \
    MEMCCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c)               \
    dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                                      \
    READ_PREFETCH_START1(src);                                                                         \
    WRITE_PREFETCH_START2(dst);                                                                        \
  }

#define MEMCCPY_REST_WORDS(dst, dstl, src, srcl, len, srcoffset, mask, c)          \
{                                                                                  \
  int l = len / sizeof(uint32_t);                                                  \
  if (srcoffset == 0) {                                                            \
    switch (l) {                                                                   \
    case 3:                                                                        \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c);              \
    case 2:                                                                        \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c);              \
    case 1:                                                                        \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, mask, c);              \
      len -= l*sizeof(uint32_t);                                                   \
    }                                                                              \
  } else {                                                                         \
    switch (l) {                                                                   \
    case 3:                                                                        \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c); \
    case 2:                                                                        \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c); \
    case 1:                                                                        \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, c); \
      len -= l*sizeof(uint32_t);                                                   \
    }                                                                              \
  }                                                                                \
}

#define MEMCCPY_FWD_NIBBLE(dst, src, c, len) \
  switch (len) {                             \
  case 3:                                    \
    if ((*dst++ = *src++) == c) return dst;  \
  case 2:                                    \
    if ((*dst++ = *src++) == c) return dst;  \
  case 1:                                    \
    if ((*dst++ = *src++) == c) return dst;  \
  }
