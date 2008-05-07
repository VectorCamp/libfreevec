/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define MEMSET_NIBBLE(ptr, p, len)  \
  switch(len) {                     \
  case 3:                           \
    *ptr++ = p;                     \
  case 2:                           \
    *ptr++ = p;                     \
  case 1:                           \
    *ptr++ = p;                     \
  }

#define MEMSET_UNTIL_WORD_ALIGNED( ptr, p, len, al )  \
{                                                      \
  int l = MIN( len, sizeof(uint32_t) - al );           \
  switch (l) {                                         \
  case 3:                                              \
    *ptr++ = p;                                        \
  case 2:                                              \
    *ptr++ = p;                                        \
  case 1:                                              \
    *ptr++ = p;                                        \
    len -= l;                                          \
  }                                                    \
}

#define MEMSET_WORD_UNTIL_ALTIVEC_ALIGNED( ptr32, p32, len )  \
{                                                              \
  int l = (ALTIVECWORD_SIZE - al) / sizeof(uint32_t);          \
  switch (l) {                                                 \
    case 3:                                                    \
      *ptr32++ = p32;                                          \
    case 2:                                                    \
      *ptr32++ = p32;                                          \
    case 1:                                                    \
      *ptr32++ = p32;                                          \
      len -= l*sizeof(uint32_t);                               \
  }                                                            \
}

#define MEMSET_ALTIVECWORD(ptr32, vc, len)  \
{                                           \
  vec_st(vc, 0, (uint8_t *) ptr32);         \
  ptr32 += 4; len -= ALTIVECWORD_SIZE;      \
}

#define MEMSET_LOOP_ALTIVECWORD(ptr32, vc, len)  \
{                                                \
  while (len >= ALTIVECWORD_SIZE) {              \
    MEMSET_ALTIVECWORD(ptr32, vc, len);          \
  }                                              \
}

#define MEMSET_LOOP_QUADWORD(ptr32, vc, len)  \
{                                             \
  WRITE_PREFETCH_START2(ptr32);               \
  uint32_t blocks = len >> LOG_ALTIVECQUAD;   \
  len -= blocks << LOG_ALTIVECQUAD;           \
  while (blocks--) {                          \
    vec_st(vc, 0, (uint8_t *)ptr32);          \
    vec_st(vc, 16, (uint8_t *)ptr32);         \
    vec_st(vc, 32, (uint8_t *)ptr32);         \
    vec_st(vc, 48, (uint8_t *)ptr32);         \
    ptr32 += 16;                              \
  }                                           \
}

#define MEMSET_REST_WORDS(ptr32, p32, len)  \
{                                           \
  int l = len / sizeof(uint32_t);           \
  len -= l*sizeof(uint32_t);                \
  while (l--) {                             \
      *ptr32++ = p32;                       \
  }                                         \
}

#define MEMSET_REST_WORDS2(ptr32, p32, len)  \
{                                           \
  int l = len / sizeof(uint32_t);           \
  switch (l) {                              \
    case 3:                                 \
      *ptr32++ = p32;                       \
    case 2:                                 \
      *ptr32++ = p32;                       \
    case 1:                                 \
      *ptr32++ = p32;                       \
      len -= l*sizeof(uint32_t);            \
  }                                         \
}
