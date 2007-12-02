/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"
#include "macros/common.h"

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

#define STRLEN_SINGLE_WORD(str, ptr32)                                                \
{                                                                                     \
  uint32_t lw = ~(((*ptr32 & magic_bits32) + magic_bits32) | *ptr32 | magic_bits32);  \
  if (lw) {                                                                           \
    uint32_t pos = find_leftfirst_nzb(lw);                                            \
    return ptrdiff_t((uint8_t *)(ptr32)+ pos, str);                                   \
  }                                                                                   \
  ptr32++;                                                                            \
}

#define STRLEN_UNTIL_ALTIVEC_ALIGNED(str, ptr)                      \
{                                                                   \
  uint32_t al = ALTIVECWORD_SIZE -(uint32_t)ptr % ALTIVECWORD_SIZE; \
  while (al--) {                                                    \
    if (*ptr == 0) return ptrdiff_t(ptr,str);                       \
    ptr++;                                                          \
  }                                                                 \
}

#define STRLEN_WORDS_UNTIL_ALTIVEC_ALIGNED(str, ptr32)  \
{                                                       \
  uint32_t al = (uint32_t)ptr32 % ALTIVECWORD_SIZE;     \
  int l = al / sizeof(uint32_t);                        \
  switch (l) {                                          \
    case 1:                                             \
      STRLEN_SINGLE_WORD(str, ptr32);                   \
    case 2:                                             \
      STRLEN_SINGLE_WORD(str, ptr32);                   \
    case 3:                                             \
      STRLEN_SINGLE_WORD(str, ptr32);                   \
  }                                                     \
}

#define STRLEN_SINGLE_ALTIVEC_WORD(v0, str, ptr32)   \
{                                                    \
  vector uint8_t vec = vec_ld(0, (uint8_t *)ptr32);  \
  if (!vec_all_ne(vec, v0)) {                        \
    STRLEN_SINGLE_WORD(str, ptr32);                  \
    STRLEN_SINGLE_WORD(str, ptr32);                  \
    STRLEN_SINGLE_WORD(str, ptr32);                  \
    STRLEN_SINGLE_WORD(str, ptr32);                  \
  }                                                  \
}

#define STRLEN_LOOP_ALTIVEC_WORD( str, ptr32 )   \
{                                                \
  vector uint8_t v0 = vec_splat_u8( 0 );         \
  do {                                           \
    STRLEN_SINGLE_ALTIVEC_WORD(v0, str, ptr32);  \
    ptr32 += 4;                                  \
    READ_PREFETCH_START( ptr32 );                \
  } while (1);                                   \
}

/***************************************************************************
 *                                                                         *
 * strnlen() macros, strlen() with a length limit                          *
 **************************************************************************/

#define STRNLEN_UNTIL_WORD_ALIGNED(ptr, len, maxlen)            \
    while (((uint32_t)(ptr) & (sizeof(uint32_t)-1)) && len--) { \
        if (*ptr == '\0') return ptrdiff_t(ptr,str);            \
        ptr++;             \
    }

#define STRNLEN_SINGLE_WORD(ptr32, len, maxlen)    \
    if (( *ptr32 - lomagic) & himagic) {      \
        uint8_t *cptr = (uint8_t *) ptr32;      \
        if (cptr[0] == '\0' ) return ptrdiff_t(ptr32,str);  \
        if (cptr[1] == '\0' ) return ptrdiff_t(ptr32,str) +1; \
        if (cptr[2] == '\0' ) return ptrdiff_t(ptr32,str) +2; \
        if (cptr[3] == '\0' ) return ptrdiff_t(ptr32,str) +3; \
    }                                               \
    ptr32++;

#define STRNLEN_LOOP_WORD(ptr32, len, maxlen)      \
    while (len >= sizeof(uint32_t)) {                \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);   \
        len -= sizeof(uint32_t);                     \
    }

#define STRNLEN_LOOP_UNTIL_ALTIVEC_ALIGNED(ptr32, len, maxlen)    \
 {                \
  int ptr32al = (uint32_t)(ptr32) % ALTIVECWORD_SIZE;   \
        if (ptr32al == 4) {           \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            len -= 3*sizeof(uint32_t);                              \
        } else if (ptr32al == 8) {                            \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            len -= 2*sizeof(uint32_t);                              \
        } else if (ptr32al == 12) {                            \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            len -= sizeof(uint32_t);                                \
  }               \
    }

#define STRNLEN_SINGLE_ALTIVEC_WORD(vec, vmask, ptr32, len, maxlen)   \
    vec = vec_ld(0, (uint8_t *)ptr32);                                  \
    if (vec_any_eq(vec, vmask)) {                                       \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
    }                                                                   \
    ptr32 += 4;                                                         \
    len -= 16;

#define STRNLEN_REST_BYTES(ptr, len, maxlen)   \
 if (len == 3) {                             \
  if (*ptr == '\0') return ptrdiff_t(ptr,str); \
  ptr++;           \
  if (*ptr == '\0') return ptrdiff_t(ptr,str); \
  ptr++;           \
  if (*ptr == '\0') return ptrdiff_t(ptr,str); \
 } else if (len == 2) {        \
  if (*ptr == '\0') return ptrdiff_t(ptr,str); \
  ptr++;           \
  if (*ptr == '\0') return ptrdiff_t(ptr,str); \
 } else if (len == 1) {        \
  if (*ptr == '\0') return ptrdiff_t(ptr,str); \
    }
