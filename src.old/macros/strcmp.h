/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define STRCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, src1al)              \
{                                                                       \
  int l = sizeof(uint32_t) - src1al;                                    \
  switch (l) {                                                          \
  case 3:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  case 2:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  case 1:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  }                                                                     \
}

#define STRCMP_SINGLE_WORD_BYTE(src1, src1l, src2, src2l)             \
{                                                                     \
  if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
  src1++; src2++;                                                     \
  if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
  src1++; src2++;                                                     \
  if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
  src1++; src2++;                                                     \
  if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
  src1++; src2++;                                                     \
}

#define STRCMP_SINGLE_WORD_ALIGNED1(src1, src1l, src2, src2l)  \
{                                                             \
  src2 = (uint8_t *) src2l;                                   \
  src1 = (uint8_t *) src1l;                                   \
  STRCMP_SINGLE_WORD_BYTE(src1, src1l, src2, src2l);          \
  src1l++; src2l++;                                           \
}

#define STRCMP_SINGLE_WORD_UNALIGNED1(src1, src1l, src2, src2l, src2al)  \
{                                                                       \
  src2 = (uint8_t *)(src2l) +src2al;                                    \
  src1 = (uint8_t *)(src1l);                                            \
  STRCMP_SINGLE_WORD_BYTE(src1, src1l, src2, src2l);                    \
  src1l++; src2l++;                                                     \
}

#define STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l)  \
{                                                             \
  uint32_t lw = (*src1l ^ *src2l) | HAS_ZERO_BYTE(*src1l);    \
  if (lw) {                                                   \
    uint32_t pos;                                             \
    FIND_LEFTFIRST_IN_WORD(pos, lw);                          \
    src2 = (uint8_t *) src2l;                                 \
    src1 = (uint8_t *) src1l;                                 \
    return DIFF(src1[pos], src2[pos]);                        \
  }                                                           \
  src1l++; src2l++;                                           \
}

#define STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                       \
  uint32_t src2t = 0;                                                   \
  if (src2al == 0) {                                                    \
    src2t = *src2l;                                                     \
  } else {                                                              \
    src2t = (*(src2l) << sh_l) | (*(src2l+1) >> sh_r);                  \
  }                                                                     \
  uint32_t lw = (*src1l ^ src2t) | HAS_ZERO_BYTE(*src1l);               \
  if (lw) {                                                             \
    uint32_t pos;                                                       \
    FIND_LEFTFIRST_IN_WORD(pos, lw);                                    \
    src2 = (uint8_t *)(src2l) +src2al;                                  \
    src1 = (uint8_t *)(src1l);                                          \
    return DIFF(src1[pos], src2[pos]);                                  \
  }                                                                     \
  src1l++; src2l++;                                                     \
}

#define STRCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, src1al, src2al)  \
{                                                                                       \
  uint32_t l = (ALTIVECWORD_SIZE - src1al)/sizeof(uint32_t);                            \
  if (src2al == 0) {                                                                    \
    switch (l) {                                                                        \
    case 3:                                                                             \
      STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                             \
    case 2:                                                                             \
      STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                             \
    case 1:                                                                             \
      STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                             \
      src2 = (uint8_t *)(src2l);                                                        \
    }                                                                                   \
  } else {                                                                              \
    switch (l) {                                                                        \
    case 3:                                                                             \
      STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                   \
    case 2:                                                                             \
      STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                   \
    case 1:                                                                             \
      STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                   \
      src2 = (uint8_t *)(src2l) +src2al;                                                \
    }                                                                                   \
  }                                                                                     \
}

#define STRCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l)  \
{                                                          \
  STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
  STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
  STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
  STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);    \
}

#define STRCMP_QUADWORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                    \
  STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
  STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);    \
}

#define STRCMP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l)     \
{                                                                        \
  vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),  \
                  vsrc2 = (vector uint8_t) vec_ld(0, (uint8_t *)src2l);  \
                  v0 = vec_splat_u8(0);                                  \
  if (!vec_all_ne(vsrc1, v0) || !vec_all_eq(vsrc1, vsrc2)) {             \
    STRCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l);                   \
  }                                                                      \
}

#define STRCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                               \
  vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),         \
                  vsrc2, MSQ, LSQ, vmask;                                       \
  vmask = vec_lvsl(0, src2);                                                    \
  MSQ = vec_ld(0, src2);                                                        \
  LSQ = vec_ld(15, src2);                                                       \
  vsrc2 = vec_perm(MSQ, LSQ, vmask);                                            \
  if (!vec_all_ne(vsrc1, v0) || !vec_all_eq(vsrc1, vsrc2)) {                    \
    src2l = (uint32_t *)(src2 -src2al);                                         \
    STRCMP_QUADWORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                \
  }                                                                             \
}

#define STRCMP_LOOP_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l)         \
{                                                                          \
  READ_PREFETCH_START1(src1l);                                             \
  READ_PREFETCH_START2(src2l);                                             \
  vector uint8_t v0 = vec_splat_u8(0);                                     \
  do {                                                                     \
    vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),  \
                    vsrc2 = (vector uint8_t) vec_ld(0, (uint8_t *)src2l);  \
    if (!vec_all_ne(vsrc1, v0) || !vec_all_eq(vsrc1, vsrc2)) {             \
      STRCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l);                   \
    }                                                                      \
    src1l += 4; src2l += 4;                                                \
  } while (1);                                                             \
}

#define STRCMP_LOOP_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                             \
  READ_PREFETCH_START1(src1l);                                                \
  READ_PREFETCH_START2(src2);                                                 \
  vector uint8_t v0 = vec_splat_u8(0);                                        \
  do {                                                                        \
    STRCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);   \
    src1l += 4; src2 += ALTIVECWORD_SIZE;                                     \
  } while (1);                                                                \
  src2l = (uint32_t *)(src2 -src2al);                                         \
}

/***************************************************************************
 *                                                                         *
 * strncmp() macros, compare strings                                       *
 **************************************************************************/

#define STRNCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, len, src1al)        \
{                                                                       \
  int l = MIN(len, sizeof(uint32_t) - src1al );                         \
  switch (l) {                                                          \
  case 3:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  case 2:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  case 1:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  }                                                                     \
  len -= l;                                                             \
}

#define STRNCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, len, src1al, src2al)  \
{                                                                                             \
  uint32_t l = ALTIVECWORD_SIZE - src1al;                                                     \
  l = MIN(len, l);                                                                            \
  l /= sizeof(uint32_t);                                                                      \
  if (src2al == 0) {                                                                          \
    switch (l) {                                                                              \
    case 3:                                                                                   \
      STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                                   \
    case 2:                                                                                   \
      STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                                   \
    case 1:                                                                                   \
      STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);                                   \
      len -= l*sizeof(uint32_t);                                                              \
      src2 = (uint8_t *)(src2l);                                                              \
    }                                                                                         \
  } else {                                                                                    \
    switch (l) {                                                                              \
    case 3:                                                                                   \
      STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                         \
    case 2:                                                                                   \
      STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                         \
    case 1:                                                                                   \
      STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);                         \
      len -= l*sizeof(uint32_t);                                                              \
      src2 = (uint8_t *)(src2l) +src2al;                                                      \
    }                                                                                         \
  }                                                                                           \
}

#define STRNCMP_LOOP_ALTIVEC_WORDS_ALIGNED(src1, src1l, src2, src2l)       \
{                                                                          \
  READ_PREFETCH_START1(src1l);                                             \
  READ_PREFETCH_START2(src2l);                                             \
  vector uint8_t v0 = vec_splat_u8(0);                                     \
  while (len >= ALTIVECWORD_SIZE) {                                        \
    vector uint8_t  vsrc1 = (vector uint8_t) vec_ld(0, (uint8_t *)src1l),  \
                    vsrc2 = (vector uint8_t) vec_ld(0, (uint8_t *)src2l);  \
    if (!vec_all_ne(vsrc1, v0) || !vec_all_eq(vsrc1, vsrc2)) {             \
      STRCMP_QUADWORD_ALIGNED(src1, src1l, src2, src2l);                   \
  }                                                                        \
  src1l += 4; src2l += 4; len -= ALTIVECWORD_SIZE;                         \
  }                                                                        \
}

#define STRNCMP_LOOP_ALTIVEC_WORDS_UNALIGNED(src1, src1l, src2, src2l, src2al)  \
{                                                                               \
  READ_PREFETCH_START1(src1l);                                                  \
  READ_PREFETCH_START2(src2);                                                   \
  vector uint8_t v0 = vec_splat_u8(0);                                          \
  while (len >= ALTIVECWORD_SIZE) {                                             \
    STRCMP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);     \
    src1l += 4; src2 += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;              \
  }                                                                             \
  src2l = (uint32_t *)(src2 -src2al);                                           \
}

#define STRNCMP_REST_WORDS(src1, src1l, src2, src2l, len, src2al)      \
{                                                                      \
  int l = len / sizeof(uint32_t);                                      \
  if (src2al == 0) {                                                   \
    while (l--) {                                                      \
      STRCMP_SINGLE_WORD_ALIGNED(src1, src1l, src2, src2l);            \
      len -= sizeof(uint32_t);                                         \
    }                                                                  \
  } else {                                                             \
    while (l--) {                                                      \
      STRCMP_SINGLE_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);  \
      len -= sizeof(uint32_t);                                         \
    }                                                                  \
  }                                                                    \
}

#define STRNCMP_NIBBLE(src1, src2, len)                                 \
{                                                                       \
  switch (len) {                                                        \
  case 3:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  case 2:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  case 1:                                                               \
    if ((*src1 - *src2) != 0 || *src1 == 0) return DIFF(*src1, *src2);  \
    src1++; src2++;                                                     \
  }                                                                     \
}

