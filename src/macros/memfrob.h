/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define FROBNICATOR     42
#define FROBNICATOR16   (((FROBNICATOR) << 8) | (FROBNICATOR))
#define FROBNICATOR32   (((FROBNICATOR16) << 16) | (FROBNICATOR16))

#define MEMFROB_NIBBLE(ptr, len)  \
  switch(len) {                              \
  case 3:                                    \
    *ptr++ ^= FROBNICATOR;                   \
  case 2:                                    \
    *ptr++ ^= FROBNICATOR;                   \
  case 1:                                    \
    *ptr++ ^= FROBNICATOR;                   \
  }

#define MEMFROB_UNTIL_WORD_ALIGNED( ptr, len, al )  \
{                                                      \
  int l = MIN( len, sizeof(uint32_t) - al );           \
  switch (l) {                                         \
  case 3:                                              \
    *ptr++ ^= FROBNICATOR;                             \
  case 2:                                              \
    *ptr++ ^= FROBNICATOR;                             \
  case 1:                                              \
    *ptr++ ^= FROBNICATOR;                             \
    len -= l;                                          \
  }                                                    \
}

#define MEMFROB_WORD_UNTIL_ALTIVEC_ALIGNED( ptr32, len )  \
{                                                              \
  int l = (ALTIVECWORD_SIZE - al) / sizeof(uint32_t);          \
  switch (l) {                                                 \
    case 3:                                                    \
      *ptr32++ ^= FROBNICATOR32;                               \
    case 2:                                                    \
      *ptr32++ ^= FROBNICATOR32;                               \
    case 1:                                                    \
      *ptr32++ ^= FROBNICATOR32;                               \
      len -= l*sizeof(uint32_t);                               \
  }                                                            \
}

#define MEMFROB_ALTIVECWORD(ptr32, vf, len)          \
{                                                    \
  vector uint32_t v = vec_ld(0, (uint32_t *)ptr32);  \
  vec_st(vec_xor(v, vf), 0, (uint32_t *)ptr32);      \
  ptr32 += 4; len -= ALTIVECWORD_SIZE;               \
}

#define MEMFROB_LOOP_ALTIVECWORD(ptr32, vf, len)  \
{                                                 \
  while (len >= ALTIVECWORD_SIZE) {               \
    MEMFROB_ALTIVECWORD(ptr32, vf, len);          \
  }                                               \
}

#define MEMFROB_LOOP_QUADWORD(ptr32, vf, len)        \
{                                                    \
  vector uint32_t v1, v2, v3, v4;                    \
  uint32_t blocks = len >> LOG_ALTIVECQUAD;          \
  len -= blocks << LOG_ALTIVECQUAD;                  \
  while (blocks--) {                                 \
    v1 = vec_ld(0, (uint32_t *)ptr32);               \
    v2 = vec_ld(16, (uint32_t *)ptr32);              \
    v3 = vec_ld(32, (uint32_t *)ptr32);              \
    v4 = vec_ld(48, (uint32_t *)ptr32);              \
    vec_st(vec_xor(v1, vf), 0, (uint32_t *)ptr32);   \
    vec_st(vec_xor(v2, vf), 16, (uint32_t *)ptr32);  \
    vec_st(vec_xor(v3, vf), 32, (uint32_t *)ptr32);  \
    vec_st(vec_xor(v4, vf), 48, (uint32_t *)ptr32);  \
    ptr32 += 16;                                     \
  }                                                  \
}

#define MEMFROB_REST_WORDS(ptr32, len)  \
{                                            \
  int l = len / sizeof(uint32_t);            \
  switch (l) {                               \
    case 3:                                  \
      *ptr32++ ^= FROBNICATOR32;             \
    case 2:                                  \
      *ptr32++ ^= FROBNICATOR32;             \
    case 1:                                  \
      *ptr32++ ^= FROBNICATOR32;             \
      len -= l*sizeof(uint32_t);             \
  }                                          \
}
