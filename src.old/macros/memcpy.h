/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define MEMCPY_FWD_NIBBLE(d, s, len)  \
  switch(len) {                       \
  case 3:                             \
    *d++ = *s++;                      \
  case 2:                             \
    *d++ = *s++;                      \
  case 1:                             \
    *d++ = *s++;                      \
  }

#define MEMCPY_FWD_UNTIL_DEST_IS_WORD_ALIGNED(d, s, len)  \
{                                                         \
  int dstal = ((uint32_t)d) % sizeof(uint32_t);           \
  switch (dstal) {                                        \
  case 1:                                                 \
    *d++ = *s++;                                          \
  case 2:                                                 \
    *d++ = *s++;                                          \
  case 3:                                                 \
    *d++ = *s++;                                          \
    len -= sizeof(uint32_t) -dstal;                       \
  }                                                       \
}

#define MEMCPY_FWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(d, s, len, srcofst, sh_l, sh_r)  \
  while (len >= sizeof(uint32_t) && ((uint32_t)(d) % ALTIVECWORD_SIZE)) {         \
    if (srcofst == 0) {                                                           \
      *d++ = *s++;                                                                \
    } else {                                                                      \
      *d++ = (*(s) << sh_l) | (*(s+1) >> sh_r);                                   \
      s++;                                                                        \
    }                                                                             \
    len -= sizeof(uint32_t);                                                      \
  }

#define MEMCPY_SINGLEQUADWORD_ALTIVEC_ALIGNED(d, s, step)  \
{                                                          \
  vec_st((vector uint8_t) vec_ld(step, (uint8_t *)s),      \
         step, (uint8_t *)d);                              \
}

#define MEMCPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED(d, s, step)  \
{                                                            \
  vector uint8_t MSQ, LSQ, mask;                             \
  mask = vec_lvsl(0, s);                                     \
  MSQ = vec_ld(step, s);                                     \
  LSQ = vec_ld(step+15, s);                                  \
  vec_st(vec_perm(MSQ, LSQ, mask), step, (uint8_t *)d);      \
}

#define MEMCPY_FWD_LOOP_QUADWORD_ALTIVEC_ALIGNED(d, s, len)               \
{                                                                         \
  uint32_t blocks = len >> LOG_ALTIVECQUAD;                               \
  len -= blocks << LOG_ALTIVECQUAD;                                       \
  while (blocks--) {                                                      \
    vec_st((vector uint8_t) vec_ld(0, (uint8_t *)s), 0, (uint8_t *)d);    \
    vec_st((vector uint8_t) vec_ld(16, (uint8_t *)s), 16, (uint8_t *)d);  \
    vec_st((vector uint8_t) vec_ld(32, (uint8_t *)s), 32, (uint8_t *)d);  \
    vec_st((vector uint8_t) vec_ld(48, (uint8_t *)s), 48, (uint8_t *)d);  \
    d += ALTIVECWORD_SIZE; s += QUAD_ALTIVECWORD;                         \
  }                                                                       \
}

#define MEMCPY_FWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED(d, s, len)  \
{                                                              \
  vector uint8_t mask, MSQ1, LSQ1, LSQ2, LSQ3, LSQ4;           \
  uint32_t blocks = len >> LOG_ALTIVECQUAD ;                   \
  len -= blocks << LOG_ALTIVECQUAD;                            \
  mask = vec_lvsl(0, s);                                       \
  while (blocks--) {                                           \
    MSQ1 = vec_ld(0, s);                                       \
    LSQ1 = vec_ld(15, s);                                      \
    LSQ2 = vec_ld(31, s);                                      \
    LSQ3 = vec_ld(47, s);                                      \
    LSQ4 = vec_ld(63, s);                                      \
    vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)d);       \
    vec_st(vec_perm(LSQ1, LSQ2, mask), 16, (uint8_t *)d);      \
    vec_st(vec_perm(LSQ2, LSQ3, mask), 32, (uint8_t *)d);      \
    vec_st(vec_perm(LSQ3, LSQ4, mask), 48, (uint8_t *)d);      \
    d += ALTIVECWORD_SIZE; s += QUAD_ALTIVECWORD;              \
  }                                                            \
}

#define MEMCPY_FWD_REST_WORDS_ALIGNED(d, s, len)  \
{                                                 \
  while (len >= sizeof(uint32_t)) {               \
    *d++ = *s++;                                  \
    len -= sizeof(uint32_t);                      \
  }                                               \
}

#define MEMCPY_FWD_REST_WORDS_UNALIGNED(d, s, len, sh_l, sh_r)  \
{                                                               \
  while (len >= sizeof(uint32_t)) {                             \
    *d++ = (*(s) << sh_l) | (*(s+1) >> sh_r);                   \
    s++;                                                        \
    len -= sizeof(uint32_t);                                    \
  }                                                             \
}
