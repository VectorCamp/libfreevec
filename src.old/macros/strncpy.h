/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define STRNCPY_UNTIL_DEST_WORD_ALIGNED(dst, src, len, al) \
{                                                          \
  int l = MIN( len, sizeof(uint32_t) - al );               \
  switch (l) {                                             \
  case 3:                                                  \
    if ((*dst++ = *src++) == 0) return dstpp;              \
  case 2:                                                  \
    if ((*dst++ = *src++) == 0) return dstpp;              \
  case 1:                                                  \
    if ((*dst++ = *src++) == 0) return dstpp;              \
    len -= l;                                              \
  }                                                        \
}

#define STRNCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, len, srcal, al)  \
{                                                                                    \
  int l = (ALTIVECWORD_SIZE - al);                                                   \
  l = MIN( len, l );                                                                 \
  if (srcal == 0) {                                                                  \
    switch (l) {                                                                     \
    case 12:                                                                         \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);                              \
    case 8:                                                                          \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);                              \
    case 4:                                                                          \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);                              \
      len -= l;                                                                      \
    }                                                                                \
  } else {                                                                           \
    switch (l) {                                                                     \
    case 12:                                                                         \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);                     \
    case 8:                                                                          \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);                     \
    case 4:                                                                          \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);                     \
      len -= l;                                                                      \
    }                                                                                \
  }                                                                                  \
}

#define STRNCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, vc)     \
{                                                                         \
  vector uint8_t vsrc = (vector uint8_t) vec_ld(0, (uint8_t *)srcl);      \
  if (!vec_all_ne(vsrc, vc)) {                                            \
    STRCPY_QUADWORD_ALIGNED(dst, dstl, src, srcl);                        \
  }                                                                       \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                       \
}

#define STRNCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal, vc)  \
{                                                                               \
  vector uint8_t vsrc, MSQ, LSQ, vmask;                                         \
  vmask = vec_lvsl(0, src);                                                     \
  MSQ = (vector uint8_t) vec_ld(0, src);                                        \
  LSQ = (vector uint8_t) vec_ld(15, src);                                       \
  vsrc = vec_perm(MSQ, LSQ, vmask);                                             \
  if (!vec_all_ne(vsrc, vc)) {                                                  \
    srcl = (uint32_t *)(src -srcal);                                            \
    STRCPY_QUADWORD_UNALIGNED(dst, dstl, src, srcl, srcal);                     \
  }                                                                             \
  vec_st(vsrc, 0, (uint8_t *)dstl);                                             \
}

#define STRNCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, len, vc)  \
  READ_PREFETCH_START1(srcl);                                                    \
  WRITE_PREFETCH_START2(dstl);                                                   \
  while (len >= ALTIVECWORD_SIZE) {                                              \
    STRNCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, vc);               \
    dstl += 4; srcl += 4; len -= ALTIVECWORD_SIZE;                               \
  }

#define STRNCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, len, srcal, vc)  \
  READ_PREFETCH_START1(srcl);                                                             \
  WRITE_PREFETCH_START2(dstl);                                                            \
  while (len >= ALTIVECWORD_SIZE) {                                                       \
    STRNCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal, vc)                \
    dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                          \
  }

#define STRNCPY_REST_WORDS(dst, dstl, src, srcl, len, srcal)      \
{                                                                 \
  int l = len / sizeof(uint32_t);                                 \
  if (srcal == 0) {                                               \
    switch (l) {                                                  \
    case 3:                                                       \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);           \
    case 2:                                                       \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);           \
    case 1:                                                       \
      STRCPY_SINGLE_WORD_ALIGNED(dst, dstl, src, srcl);           \
      len -= l*sizeof(uint32_t);                                  \
    }                                                             \
  } else {                                                        \
    switch (l) {                                                  \
    case 3:                                                       \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);  \
    case 2:                                                       \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);  \
    case 1:                                                       \
      STRCPY_SINGLE_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);  \
      len -= l*sizeof(uint32_t);                                  \
    }                                                             \
  }                                                               \
}

#define STRNCPY_NIBBLE(dst, src, len)          \
  switch (len) {                               \
  case 3:                                      \
    if ((*dst++ = *src++) == 0) return dstpp;  \
  case 2:                                      \
    if ((*dst++ = *src++) == 0) return dstpp;  \
  case 1:                                      \
    if ((*dst++ = *src++) == 0) return dstpp;  \
  }
