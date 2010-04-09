/*********************************************************************************
 *   Copyright (C) 2008-2010 by Konstantinos Margaritis <markos@codex.gr>        *
 *   All rights reserved.                                                        *
 *                                                                               *
 * Redistribution and use in source and binary forms, with or without            *
 * modification, are permitted provided that the following conditions are met:   *
 *  1. Redistributions of source code must retain the above copyright            *
 *     notice, this list of conditions and the following disclaimer.             *
 *  2. Redistributions in binary form must reproduce the above copyright         *
 *     notice, this list of conditions and the following disclaimer in the       *
 *     documentation and/or other materials provided with the distribution.      *
 *  3. Neither the name of the Codex nor the                                     *
 *     names of its contributors may be used to endorse or promote products      *
 *     derived from this software without specific prior written permission.     *
 *                                                                               *
 * THIS SOFTWARE IS PROVIDED BY CODEX ''AS IS'' AND ANY                          *
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        *
 * DISCLAIMED. IN NO EVENT SHALL CODEX BE LIABLE FOR ANY                         *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND   *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                  *
 *********************************************************************************/

/* $Id$ */

#include <stdint.h>
#include <stddef.h>
#include "common.h"

#include "arch/neon.h"

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
