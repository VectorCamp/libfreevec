/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define COPY_BWD_NIBBLE(dst, src, len)	\
switch(len) {                           \
  case 3:                               \
    *--dst = *--src;                    \
  case 2:                               \
    *--dst = *--src;                    \
  case 1:                               \
    *--dst = *--src;                    \
  case 0:                               \
    *--dst = *--src;                    \
  }

#define COPY_BWD_UNTIL_DEST_IS_WORD_ALIGNED(dst, src, len)  \
{                                                           \
  int dstal = ((uint32_t)dst) % sizeof(uint32_t);           \
  switch (dstal) {                                          \
  case 3:                                                   \
    *--dst = *--src;                                        \
  case 2:                                                   \
    *--dst = *--src;                                        \
  case 1:                                                   \
    *--dst = *--src;                                        \
  }                                                         \
  len -= dstal;                                             \
}

#define COPY_BWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, src, len, srcal)         \
  while (len >= sizeof(uint32_t) && ((uint32_t)(dst) % ALTIVECWORD_SIZE)) {  \
    if (srcal == 0) {                                                        \
      *--dst = *--src;                                                       \
    } else if (srcal == 3) {                                                 \
      *--dst = (*(src-1) << 24) | (*(src) >> 8);                             \
      --src;                                                                 \
    } else if (srcal == 2) {                                                 \
      *--dst = (*(src-1) << 16) | (*(src) >> 16);                            \
      --src;                                                                 \
    } else if (srcal == 1) {                                                 \
      *--dst = (*(src-1) << 8) | (*(src) >> 24);                             \
      --src;                                                                 \
    }                                                                        \
    len -= sizeof(uint32_t);                                                 \
  }

#define COPY_BWD_LOOP_QUADWORD_ALTIVEC_ALIGNED(dst, src, len)                 \
{                                                                             \
  uint32_t blocks = len >> LOG_ALTIVECQUAD ;                                  \
  len -= blocks << LOG_ALTIVECQUAD;                                           \
  while (blocks--) {                                                          \
    dst -= ALTIVECWORD_SIZE; src -= QUAD_ALTIVECWORD;                         \
    READ_PREFETCH_START1 ( src );                                              \
    WRITE_PREFETCH_START2 ( dst );                                             \
    vec_st((vector uint8_t) vec_ld(48, (uint8_t *)src), 48, (uint8_t *)dst);  \
    vec_st((vector uint8_t) vec_ld(32, (uint8_t *)src), 32, (uint8_t *)dst);  \
    vec_st((vector uint8_t) vec_ld(16, (uint8_t *)src), 16, (uint8_t *)dst);  \
    vec_st((vector uint8_t) vec_ld(0, (uint8_t *)src), 0, (uint8_t *)dst);    \
  }                                                                           \
}

#define COPY_BWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED(dst, src, len)  \
{                                                                \
  vector uint8_t mask, MSQ1, LSQ1, LSQ2, LSQ3, LSQ4;             \
  uint32_t blocks = len >> LOG_ALTIVECQUAD ;                     \
  len -= blocks << LOG_ALTIVECQUAD;                              \
  mask = vec_lvsl(0, src);                                       \
  while (blocks--) {                                             \
    dst -= ALTIVECWORD_SIZE; src -= QUAD_ALTIVECWORD;            \
    READ_PREFETCH_START1 ( src );                                 \
    WRITE_PREFETCH_START2 ( dst );                                \
    LSQ4 = vec_ld(63, src);                                      \
    LSQ3 = vec_ld(47, src);                                      \
    LSQ2 = vec_ld(31, src);                                      \
    LSQ1 = vec_ld(15, src);                                      \
    MSQ1 = vec_ld(0, src);                                       \
    vec_st(vec_perm(LSQ3, LSQ4, mask), 48, (uint8_t *)dst);      \
    vec_st(vec_perm(LSQ2, LSQ3, mask), 32, (uint8_t *)dst);      \
    vec_st(vec_perm(LSQ1, LSQ2, mask), 16, (uint8_t *)dst);      \
    vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)dst);       \
  }                                                              \
}

#define COPY_BWD_REST_WORDS(dst, src, len, srcal)  \
  while (len >= sizeof(uint32_t)) {                  \
    if (srcal == 0) {                              \
      *--dst = *--src;                               \
    } else if (srcal == 3) {                       \
      *--dst = (*(src-1) << 24) | (*(src) >> 8);     \
      --src;                                         \
    } else if (srcal == 2) {                       \
      *--dst = (*(src-1) << 16) | (*(src) >> 16);    \
      --src;                                         \
    } else if (srcal == 1) {                       \
      *--dst = (*(src-1) << 8) | (*(src) >> 24);     \
      --src;                                         \
    }                                                \
    len -= sizeof(uint32_t);                         \
  }
