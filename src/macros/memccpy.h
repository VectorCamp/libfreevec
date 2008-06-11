/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#define MEMCPY_FWD_NIBBLE(dst, src, len)  \
  switch(len) {                           \
  case 3:                                 \
    *dst++ = *src++;                      \
  case 2:                                 \
    *dst++ = *src++;                      \
  case 1:                                 \
    *dst++ = *src++;                      \
  }

#define MEMCCPY_UNTIL_DEST_WORD_ALIGNED(dst, src, c, len, al) \
{                                                             \
  int l = MIN( len, sizeof(uint32_t) - al );                  \
  switch (l) {                                                \
  case 3:                                                     \
    if ((*dst++ = *src++) == c) return dst;                   \
  case 2:                                                     \
    if ((*dst++ = *src++) == c) return dst;                   \
  case 1:                                                     \
    if ((*dst++ = *src++) == c) return dst;                   \
    len -= l;                                                 \
  }                                                           \
}

#define MEMCCPY_SINGLE_WORD_FWD_ALIGNED_MASK(dst, dstl, src, srcl, lw)  \
{                                                                       \
  if (lw) {                                                             \
    src = (uint8_t *) srcl;                                             \
    dst = (uint8_t *) dstl;                                             \
    uint32_t pos;                                                       \
    FIND_LEFTFIRST_IN_WORD(pos, lw);                                    \
    MEMCPY_FWD_NIBBLE(dst, src, pos);                                   \
    return dst+1;                                                       \
  }                                                                     \
}

#define MEMCCPY_SINGLE_WORD_FWD_BYTE(dst, src, c)  \
{                                                  \
  if ((*dst++ = *src++) == c) return dst;          \
  if ((*dst++ = *src++) == c) return dst;          \
  if ((*dst++ = *src++) == c) return dst;          \
  if ((*dst++ = *src++) == c) return dst;          \
}

#define MEMCCPY_SINGLE_WORD_FWD_ALIGNED1(dst, dstl, src, srcl, c)  \
{                                                                 \
  int32_t lw = *srcl ^ mask;                                      \
  lw = ((lw + magic_bits32) ^ ~lw) & ~magic_bits32;               \
  if (lw) {                                                       \
    src = (uint8_t *) srcl;                                       \
    dst = (uint8_t *) dstl;                                       \
    MEMCCPY_SINGLE_WORD_FWD_BYTE(dst, src, c);                    \
  }                                                               \
  *dstl++ = *srcl++;                                              \
}


#define MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, c)  \
{                                                                 \
  src = (uint8_t *) srcl;                                         \
  dst = (uint8_t *) dstl;                                         \
  MEMCCPY_SINGLE_WORD_FWD_BYTE(dst, src, c);                      \
  dstl++; srcl++;                                                 \
}

#define MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c) \
{                                                                         \
  src = (uint8_t *) srcl +srcal;                                          \
  dst = (uint8_t *) dstl;                                                 \
  MEMCCPY_SINGLE_WORD_FWD_BYTE(dst, src, c);                              \
  dstl++; srcl++;                                                         \
}

#define MEMCCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, len, srcal, c, al)  \
{                                                                                       \
  int l = (ALTIVECWORD_SIZE - al) / sizeof(uint32_t);                                   \
  l = MIN( len, l );                                                                    \
  if (srcal == 0) {                                                                     \
    switch (l) {                                                                        \
    case 3:                                                                             \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, c);                         \
    case 2:                                                                             \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, c);                         \
    case 1:                                                                             \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, c);                         \
      len -= l*sizeof(uint32_t);                                                        \
    }                                                                                   \
  } else {                                                                              \
    switch (l) {                                                                        \
    case 3:                                                                             \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);                \
    case 2:                                                                             \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);                \
    case 1:                                                                             \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);                \
      len -= l*sizeof(uint32_t);                                                        \
    }                                                                                   \
  }                                                                                     \
}

#define MEMCCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, vc, c)  \
{                                                                         \
  vector uint8_t vsrc = (vector uint8_t) vec_ld(0, (uint8_t *)src);       \
  if (!vec_all_ne(vsrc, vc)) {                                            \
    srcl = (uint32_t *)(src);                                             \
    uint32_t __attribute__ ((aligned(16))) lwa[4];                        \
    vsrc = (vector uint8_t) vec_cmpeq(vsrc, vc);                          \
    vec_st(vsrc, 0, (uint8_t *) &lwa[0]);                                 \
    MEMCCPY_SINGLE_WORD_FWD_ALIGNED_MASK(dst, dstl, src, srcl, lwa[0]);   \
    *dstl++ = *srcl++;                                                    \
    MEMCCPY_SINGLE_WORD_FWD_ALIGNED_MASK(dst, dstl, src, srcl, lwa[1]);   \
    *dstl++ = *srcl++;                                                    \
    MEMCCPY_SINGLE_WORD_FWD_ALIGNED_MASK(dst, dstl, src, srcl, lwa[2]);   \
    *dstl++ = *srcl++;                                                    \
    MEMCCPY_SINGLE_WORD_FWD_ALIGNED_MASK(dst, dstl, src, srcl, lwa[3]);   \
  }                                                                       \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                       \
}

#define MEMCCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal, vc, c)  \
{                                                                                  \
  vector uint8_t vsrc, MSQ, LSQ, vmask;                                            \
  vmask = vec_lvsl(0, src);                                                        \
  MSQ = vec_ld(0, src);                                                            \
  LSQ = vec_ld(15, src);                                                           \
  vsrc = vec_perm(MSQ, LSQ, vmask);                                                \
  if (!vec_all_ne(vsrc, vc)) {                                                     \
    srcl = (uint32_t *)(src -srcal);                                               \
    MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);             \
    MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);             \
    MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);             \
    MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);             \
  }                                                                                \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                                \
}

#define MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, len, vc, c)  \
  READ_PREFETCH_START1(srcl);                                                       \
  WRITE_PREFETCH_START2(dstl);                                                      \
  while (len >= ALTIVECWORD_SIZE) {                                                 \
    MEMCCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, vc, c)                \
    dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                    \
  }

#define MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, len, srcal, vc, c)  \
  READ_PREFETCH_START1(srcl);                                                       \
  WRITE_PREFETCH_START2(dstl);                                                      \
  while (len >= ALTIVECWORD_SIZE) {                                                          \
    MEMCCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal, vc, c)                \
    dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                             \
  }

#define MEMCCPY_REST_WORDS(dst, dstl, src, srcl, len, srcal, c)           \
{                                                                         \
  int l = len / sizeof(uint32_t);                                         \
  if (srcal == 0) {                                                       \
    switch (l) {                                                          \
    case 3:                                                               \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, c);           \
    case 2:                                                               \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, c);           \
    case 1:                                                               \
      MEMCCPY_SINGLE_WORD_FWD_ALIGNED(dst, dstl, src, srcl, c);           \
      len -= l*sizeof(uint32_t);                                          \
    }                                                                     \
  } else {                                                                \
    switch (l) {                                                          \
    case 3:                                                               \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);  \
    case 2:                                                               \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);  \
    case 1:                                                               \
      MEMCCPY_SINGLE_WORD_FWD_UNALIGNED(dst, dstl, src, srcl, srcal, c);  \
      len -= l*sizeof(uint32_t);                                          \
    }                                                                     \
  }                                                                       \
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
