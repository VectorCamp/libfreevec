/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"


/***************************************************************************
 * These macros are used in memchr()-type of functions                     *
 * DO NOT CHANGE/EDIT THESE MACROS!!!                                      *
 *                                                                         *
 * memchr() macros, forward searching                                      *
 **************************************************************************/

#define MEMCHR_UNTIL_WORD_ALIGNED(ptr, c, len, al)  \
{                                                   \
  int l = MIN( len, sizeof(uint32_t) - al );        \
  switch (l) {                                      \
  case 3:                                           \
    if (*ptr == c) return ptr;                      \
    ptr++;                                          \
  case 2:                                           \
    if (*ptr == c) return ptr;                      \
    ptr++;                                          \
  case 1:                                           \
    if (*ptr == c) return ptr;                      \
    ptr++;                                          \
    len -= l;                                       \
  }                                                 \
}

#define MEMCHR_SINGLE_WORD_BYTE(ptr, c, len)  \
  switch (len) {                        \
  case 3:                               \
    if (*ptr == c) return ptr;          \
    ptr++;                              \
  case 2:                               \
    if (*ptr == c) return ptr;          \
    ptr++;                              \
  case 1:                               \
    if (*ptr == c) return ptr;          \
    ptr++;                              \
  case 0:                               \
    if (*ptr == c) return ptr;          \
  }

#define MEMCHR_SINGLE_WORD_MASK(ptr32, lw)  \
{                                           \
  if (lw) {                                 \
    uint32_t pos = find_leftfirst_nzb(lw);  \
    return ((uint8_t *)(ptr32)+ pos);       \
  }                                         \
}
// uint32_t lw = ((*ptr32 ^ mask) - lomagic) & himagic;

#define MEMCHR_SINGLE_WORD1(ptr32, c, mask)              \
{                                                       \
  uint32_t lw = *ptr32 ^ mask;                          \
  lw = ((lw + magic_bits32) ^ ~lw) & ~magic_bits32;     \
  MEMCHR_SINGLE_WORD_MASK(ptr32, lw);                   \
  ptr32++;                                              \
}

#define MEMCHR_SINGLE_WORD(ptr32, c, mask)              \
{                                                       \
  ptr = (uint8_t *)ptr32;                               \
  MEMCHR_SINGLE_WORD_BYTE(ptr, c, sizeof(uint32_t)-1);  \
  ptr32++;                                              \
}

#define MEMCHR_REST_WORDS(ptr32, c, mask, len)  \
{                                               \
  int l = len / sizeof(uint32_t);               \
  switch (l) {                                  \
  case 3:                                       \
    MEMCHR_SINGLE_WORD(ptr32, c, mask);         \
  case 2:                                       \
    MEMCHR_SINGLE_WORD(ptr32, c, mask);         \
  case 1:                                       \
    MEMCHR_SINGLE_WORD(ptr32, c, mask);         \
    len -= l*sizeof(uint32_t);                  \
  }                                             \
}

#define MEMCHR_LOOP_UNTIL_ALTIVEC_ALIGNED(ptr32, c, mask, len, al)  \
{                                                                   \
  int l = (ALTIVECWORD_SIZE - al) / sizeof(uint32_t);               \
  switch (l) {                                                      \
    case 3:                                                         \
      MEMCHR_SINGLE_WORD(ptr32, c, mask);                           \
    case 2:                                                         \
      MEMCHR_SINGLE_WORD(ptr32, c, mask);                           \
    case 1:                                                         \
      MEMCHR_SINGLE_WORD(ptr32, c, mask);                           \
      len -= l*sizeof(uint32_t);                                    \
  }                                                                 \
}

#define MEMCHR_SINGLE_ALTIVEC_WORD(vmask, ptr32, c, mask)  \
{                                                          \
  vector uint8_t vsrc, vm;                                 \
  vsrc = vec_ld(0, (uint8_t *)ptr32);                      \
  if (!vec_all_ne(vsrc, vmask)) {                          \
    uint32_t __attribute__ ((aligned(16))) lwa[4];         \
    vm = vec_cmpeq(vsrc, vmask);                           \
    vec_st(vm, 0, (uint8_t *) &lwa[0]);                    \
    MEMCHR_SINGLE_WORD_MASK(ptr32, lwa[0]);                \
    ptr32++;                                               \
    MEMCHR_SINGLE_WORD_MASK(ptr32, lwa[1]);                \
    ptr32++;                                               \
    MEMCHR_SINGLE_WORD_MASK(ptr32, lwa[2]);                \
    ptr32++;                                               \
    MEMCHR_SINGLE_WORD_MASK(ptr32, lwa[3]);                \
    ptr32++;                                               \
  }                                                        \
  ptr32 += 4;                                              \
  len -= ALTIVECWORD_SIZE;                                 \
}

#define MEMCHR_REST_BYTES(ptr, c, len)  \
  switch (len) {                        \
  case 3:                               \
    if (*ptr == c) return ptr;          \
    ptr++;                              \
  case 2:                               \
    if (*ptr == c) return ptr;          \
    ptr++;                              \
  case 1:                               \
    if (*ptr == c) return ptr;          \
    ptr++;                              \
  }

/***************************************************************************
 *                                                                         *
 * memrchr() macros, backward searching                                    *
 **************************************************************************/

#define MEMRCHR_BACKWARDS_UNTIL_WORD_ALIGNED(ptr, c, len, al) \
{                                                             \
  int l = MIN( len, al );                                     \
  switch (l) {                                                \
  case 3:                                                     \
    ptr--;                                                    \
    if (*ptr == c) return ptr;                                \
  case 2:                                                     \
    ptr--;                                                    \
    if (*ptr == c) return ptr;                                \
  case 1:                                                     \
    ptr--;                                                    \
    if (*ptr == c) return ptr;                                \
    len -= l;                                                 \
  }                                                           \
}

#define MEMRCHR_BACKWARDS_SINGLE_WORD_MASK(ptr32, lw)  \
{                                                      \
  if (lw) {                                            \
    uint32_t pos = find_rightfirst_nzb(&lw);            \
    return ((uint8_t *)(ptr32)+ pos);                  \
  }                                                    \
}

#define MEMRCHR_BACKWARDS_SINGLE_WORD(ptr32, c, mask)   \
{                                                       \
  ptr32--;                                              \
  uint32_t lw = ((*ptr32 ^ mask) - lomagic) & himagic;  \
  MEMRCHR_BACKWARDS_SINGLE_WORD_MASK(ptr32, lw);        \
}

#define MEMRCHR_BACKWARDS_REST_WORDS(ptr32, c, mask, len)  \
{                                                          \
  int l = len / sizeof(uint32_t);                          \
  switch (l) {                                             \
  case 3:                                                  \
    MEMRCHR_BACKWARDS_SINGLE_WORD(ptr32, c, mask);         \
  case 2:                                                  \
    MEMRCHR_BACKWARDS_SINGLE_WORD(ptr32, c, mask);         \
  case 1:                                                  \
    MEMRCHR_BACKWARDS_SINGLE_WORD(ptr32, c, mask);         \
    len -= l*sizeof(uint32_t);                             \
  }                                                        \
}

#define MEMRCHR_SINGLE_BACKWARDS_ALTIVEC_WORD( vmask, ptr32, c, mask)  \
{                                                                      \
  vector uint8_t vec;                                                  \
  vec = vec_ld(0, (uint8_t *)(ptr32) -16 );                            \
  if (!vec_all_ne(vec, vmask)) {                                       \
    uint32_t __attribute__ ((aligned(16))) lwa[4];                     \
    vec = vec_cmpeq(vec, vmask);                                       \
    vec_st(vec, 0, (uint8_t *) &lwa[0]);                               \
    ptr32--;                                                           \
    MEMRCHR_BACKWARDS_SINGLE_WORD_MASK(ptr32, lwa[3]);                 \
    ptr32--;                                                           \
    MEMRCHR_BACKWARDS_SINGLE_WORD_MASK(ptr32, lwa[2]);                 \
    ptr32--;                                                           \
    MEMRCHR_BACKWARDS_SINGLE_WORD_MASK(ptr32, lwa[1]);                 \
    ptr32--;                                                           \
    MEMRCHR_BACKWARDS_SINGLE_WORD_MASK(ptr32, lwa[0]);                 \
  }                                                                    \
  ptr32 -= 4;                                                          \
  len -= ALTIVECWORD_SIZE;                                             \
}

#define MEMRCHR_BACKWARDS_LOOP_UNTIL_ALTIVEC_ALIGNED(ptr32, c, mask, len, al)  \
{                                                                              \
  int l = al / sizeof(uint32_t);                                               \
  switch (l) {                                                                 \
    case 3:                                                                    \
      MEMRCHR_BACKWARDS_SINGLE_WORD(ptr32, c, mask);                           \
    case 2:                                                                    \
      MEMRCHR_BACKWARDS_SINGLE_WORD(ptr32, c, mask);                           \
    case 1:                                                                    \
      MEMRCHR_BACKWARDS_SINGLE_WORD(ptr32, c, mask);                           \
      len -= l*sizeof(uint32_t);                                               \
  }                                                                            \
}

#define MEMRCHR_BACKWARDS_REST_BYTES(ptr, c, len)    \
  switch (len) {                                     \
  case 3:                                            \
    --ptr;                                           \
    if (*ptr == c) return ptr;                       \
  case 2:                                            \
    --ptr;                                           \
    if (*ptr == c) return ptr;                       \
  case 1:                                            \
    --ptr;                                           \
    if (*ptr == c) return ptr;                       \
  }
