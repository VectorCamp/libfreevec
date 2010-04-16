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
#include <arm_neon.h>

#ifdef LINUX64
#include "scalar64/swab.h"
#else
#include "scalar32/swab.h"
#endif

#define SWAB_SINGLE_ALTIVEC_ALIGNED_NO_CARRY(vsrc, vdst, src, dst, vpermute_mask)  \
{                                                                                  \
  vsrc = vec_ld(0, (uint8_t *)src);                                                \
  vdst = vec_perm(vsrc, vsrc, vpermute_mask);                                      \
  vec_st(vdst, 0, (uint8_t *)dst);                                                 \
}

#define SWAB_SINGLE_ALTIVEC_ALIGNED_HAS_CARRY(vsrc, vdst, src, dst, vpermute_mask)  \
{                                                                                   \
  vsrc = vec_sld(vec_ld(0, (uint8_t *)src), vec_ld(16, (uint8_t *)src), 1);         \
  vdst = vec_perm(vsrc, vsrc, vdst_permute_mask);                                   \
  vec_st(vdst, 0, (uint8_t *)dst);                                                  \
}

#define SWAB_SINGLE_ALTIVEC_UNALIGNED(MSQ, LSQ, vsrc, vdst, src, dst, vpermute_mask)  \
{                                                                                     \
  MSQ = vec_ld(0, src);                                                               \
  LSQ = vec_ld(15, src);                                                              \
  vsrc = vec_perm(MSQ, LSQ, mask);                                                    \
  vdst = vec_perm(vsrc, vsrc, vdst_permute_mask);                                     \
  vec_st(vdst, 0, (unsigned char *)dst16);                                            \
}

#define SWAB_LOOP_ALTIVEC_ALIGNED_HAS_CARRY(dst16, src, len, carry)                        \
{                                                                                          \
  vector uint8_t vdst_permute_mask = {  0, 1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15 };  \
  vector uint8_t vsrc, vdst;                                                               \
  while (len >= ALTIVECWORD_SIZE) {                                                        \
    SWAB_SINGLE_ALTIVEC_ALIGNED_HAS_CARRY(vsrc, vdst, src, dst16, vdst_permute_mask);      \
    *((uint8_t *)dst16) = carry;                                                           \
    carry = src[15];                                                                       \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                          \
    READ_PREFETCH_START1(src);                                                              \
    WRITE_PREFETCH_START2(dst);                                                             \
  }                                                                                        \
}

#define SWAB_LOOP_ALTIVEC_UNALIGNED_HAS_CARRY(dst16, src, len, carry)                           \
{                                                                                               \
  vector uint8_t vdst_permute_mask = { 0, 2, 1, 4, 3, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13, 15 };  \
  vector uint8_t vsrc, vdst, MSQ, LSQ, mask;                                                    \
  mask = vec_lvsl(0, src);                                                                      \
  while (len >= ALTIVECWORD_SIZE) {                                                             \
    SWAB_SINGLE_ALTIVEC_UNALIGNED(MSQ, LSQ, vsrc, vdst, src, dst16, vdst_permute_mask);         \
    dst = (uint8_t *)dst16;                                                                     \
    dst[0] = carry; carry = src[15]; dst[15] = src[16];                                         \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                               \
    READ_PREFETCH_START1(src);                                                                   \
    WRITE_PREFETCH_START2(dst);                                                                  \
  }                                                                                             \
}

#define SWAB_LOOP_ALTIVEC_ALIGNED_NO_CARRY(dst16, src, len)                                \
{                                                                                          \
  vector uint8_t vdst_permute_mask = {  1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15,14 };  \
  vector uint8_t vsrc, vdst;                                                               \
  while (len >= ALTIVECWORD_SIZE) {                                                        \
    SWAB_SINGLE_ALTIVEC_ALIGNED_NO_CARRY(vsrc, vdst, src, dst16, vdst_permute_mask);       \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                          \
    READ_PREFETCH_START1(src);                                                              \
    WRITE_PREFETCH_START2(dst);                                                             \
  }                                                                                        \
}

#define SWAB_LOOP_ALTIVEC_UNALIGNED_NO_CARRY(dst16, src, len)                              \
{                                                                                          \
  vector uint8_t vdst_permute_mask = {  1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15,14 };  \
  vector uint8_t vsrc, vdst, MSQ, LSQ, mask;                                               \
  mask = vec_lvsl(0, src);                                                                 \
  while (len >= ALTIVECWORD_SIZE) {                                                        \
    SWAB_SINGLE_ALTIVEC_UNALIGNED(MSQ, LSQ, vsrc, vdst, src, dst16, vdst_permute_mask);    \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                          \
    READ_PREFETCH_START1(src);                                                              \
    WRITE_PREFETCH_START2(dst);                                                             \
  }                                                                                        \
}
