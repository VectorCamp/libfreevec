/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define STRLEN_UNTIL_WORD_ALIGNED(str, ptr)    \
{                                              \
  int l = (uint32_t)(ptr) % sizeof(uint32_t);  \
  switch (l) {                                 \
  case 1:                                      \
    if (*ptr == 0) return ptrdiff_t(ptr,str);  \
    ptr++;                                     \
  case 2:                                      \
    if (*ptr == 0) return ptrdiff_t(ptr,str);  \
    ptr++;                                     \
  case 3:                                      \
    if (*ptr == 0) return ptrdiff_t(ptr,str);  \
    ptr++;                                     \
  }                                            \
}

#define STRLEN_SINGLE_WORD_MASK(str, ptr32, lw)      \
{                                                    \
  if (lw) {                                          \
    uint32_t pos;                                    \
    FIND_LEFTFIRST_ZB_IN_WORD(pos, *ptr32, lw);      \
    return ptrdiff_t((uint8_t *)(ptr32)+ pos, str);  \
  }                                                  \
}

#define STRLEN_SINGLE_WORD(str, ptr32)      \
{                                           \
  uint32_t lw = HAS_ZERO_BYTE(*ptr32);      \
  STRLEN_SINGLE_WORD_MASK(str, ptr32, lw);  \
  ptr32++;                                  \
}

#define STRLEN_WORDS_UNTIL_ALTIVEC_ALIGNED(str, ptr32, al)  \
{                                                           \
  int l = al / sizeof(uint32_t);                            \
  switch (l) {                                              \
    case 1:                                                 \
      STRLEN_SINGLE_WORD(str, ptr32);                       \
    case 2:                                                 \
      STRLEN_SINGLE_WORD(str, ptr32);                       \
    case 3:                                                 \
      STRLEN_SINGLE_WORD(str, ptr32);                       \
  }                                                         \
}

#define STRLEN_SINGLE_ALTIVEC_WORD(v0, str, ptr32)      \
{                                                       \
  vector uint8_t vec = vec_ld(0, (uint8_t *)ptr32);     \
  if (!vec_all_ne(vec, v0)) {                           \
    STRLEN_SINGLE_WORD(str, ptr32);                     \
    STRLEN_SINGLE_WORD(str, ptr32);                     \
    STRLEN_SINGLE_WORD(str, ptr32);                     \
    STRLEN_SINGLE_WORD(str, ptr32);                     \
  }                                                     \
}

#define STRLEN_LOOP_ALTIVEC_WORD( str, ptr32 )   \
{                                                \
  vector uint8_t v0 = vec_splat_u8( 0 );         \
  do {                                           \
    READ_PREFETCH_START1( ptr32 );               \
    STRLEN_SINGLE_ALTIVEC_WORD(v0, str, ptr32);  \
    ptr32 += 4;                                  \
  } while (1);                                   \
}

/***************************************************************************
 *                                                                         *
 * strnlen() macros, strlen() with a length limit                          *
 **************************************************************************/

#define STRNLEN_UNTIL_WORD_ALIGNED(str, ptr, len)        \
{                                                        \
  int l = MIN(len, (uint32_t)(ptr) % sizeof(uint32_t));  \
  switch (l) {                                           \
  case 1:                                                \
    if (*ptr == 0) return ptrdiff_t(ptr,str);            \
    ptr++;                                               \
  case 2:                                                \
    if (*ptr == 0) return ptrdiff_t(ptr,str);            \
    ptr++;                                               \
  case 3:                                                \
    if (*ptr == 0) return ptrdiff_t(ptr,str);            \
    ptr++;                                               \
    len -= sizeof(uint32_t) - l;                         \
  }                                                      \
}

#define STRNLEN_LOOP_UNTIL_ALTIVEC_ALIGNED(str, ptr32, len)          \
{                                                                    \
  int l = ((uint32_t)(ptr32) % ALTIVECWORD_SIZE) / sizeof(uint32_t); \
  switch (l) {                                                       \
  case 1:                                                            \
    STRLEN_SINGLE_WORD(str, ptr32);                                  \
  case 2:                                                            \
    STRLEN_SINGLE_WORD(str, ptr32);                                  \
  case 3:                                                            \
    STRLEN_SINGLE_WORD(str, ptr32);                                  \
    len -= (sizeof(uint32_t)-l)*sizeof(uint32_t);                    \
  }                                                                  \
}

#define STRNLEN_SINGLE_ALTIVEC_WORD(str, ptr32, len)  \
{                                                     \
  vector uint8_t v0 = vec_splat_u8(0);                \
  STRLEN_SINGLE_ALTIVEC_WORD(v0, str, ptr32);         \
  ptr32 += 4; len -= ALTIVECWORD_SIZE;                \
}

#define STRNLEN_REST_WORDS(str, ptr32, len)  \
{                                            \
  int l = len / sizeof(uint32_t);            \
  switch (l) {                               \
    case 3:                                  \
      STRLEN_SINGLE_WORD(str, ptr32);        \
    case 2:                                  \
      STRLEN_SINGLE_WORD(str, ptr32);        \
    case 1:                                  \
      STRLEN_SINGLE_WORD(str, ptr32);        \
      len -= l*sizeof(uint32_t);             \
  }                                          \
}

#define STRNLEN_REST_BYTES(str, ptr, len)      \
{                                              \
  switch (len) {                               \
  case 3:                                      \
    if (*ptr == 0) return ptrdiff_t(ptr,str);  \
    ptr++;                                     \
  case 2:                                      \
    if (*ptr == 0) return ptrdiff_t(ptr,str);  \
    ptr++;                                     \
  case 1:                                      \
    if (*ptr == 0) return ptrdiff_t(ptr,str);  \
  }                                            \
}
