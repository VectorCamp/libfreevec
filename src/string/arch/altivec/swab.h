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
#include <altivec.h>

#ifdef LINUX64
#include "scalar64/swab.h"
#else
#include "scalar32/swab.h"
#endif

typedef __vector uint8_t vuint8_t;

static vuint8_t vdst_permmask_has_carry_al   = { 0, 1, 0, 3, 2, 5, 4, 7, 6,  9,  8, 11, 10, 13, 12, 15 };
static vuint8_t vdst_permmask_has_carry_noal = { 0, 2, 1, 4, 3, 6, 5, 8, 7, 10,  9, 12, 11, 14, 13, 15 };
static vuint8_t vdst_permmask_no_carry_al    = { 1, 0, 3, 2, 5, 4, 7, 6, 9,  8, 11, 10, 13, 12, 15, 14 };
static vuint8_t vdst_permmask_no_carry_noal  = { 1, 0, 3, 2, 5, 4, 7, 6, 9,  8, 11, 10, 13, 12, 15, 14 };

static inline void swab_single_simdpacket_aligned_no_carry(uint8_t *d, const uint8_t *s) {
  vuint8_t vsrc, vdst;
  vsrc = vec_ld(0, (uint8_t *)s);
  vdst = vec_perm(vsrc, vsrc, vdst_permmask_no_carry_al);
  vec_st(vdst, 0, (uint8_t *)d);
}

static inline void swab_single_simdpacket_aligned_has_carry(uint8_t *d, const uint8_t *s) {
  vuint8_t vsrc, vdst;
  vsrc = vec_sld(vec_ld(0, (uint8_t *)s), vec_ld(16, (uint8_t *)s), 1);
  vdst = vec_perm(vsrc, vsrc, vdst_permmask_has_carry_al); 
  vec_st(vdst, 0, (uint8_t *)d); 
}

static inline void swab_single_simdpacket_unaligned(uint8_t *d, const uint8_t *s, vuint8_t perm_mask, vuint8_t align_mask) {
  vuint8_t vsrc, vdst, MSQ, LSQ;
  MSQ = vec_ld(0, s);
  LSQ = vec_ld(15, s);
  vsrc = vec_perm(MSQ, LSQ, align_mask);
  vdst = vec_perm(vsrc, vsrc, perm_mask);
  vec_st(vdst, 0, (uint8_t *)d);
}

static inline void swab_blocks_simd_aligned_has_carry(uint16_t *dst16, const uint8_t *src, size_t l, uint8_t *carry) {
  uint8_t *dst = (uint8_t *) dst16;
  while (l > 0) { 
    swab_single_simdpacket_aligned_has_carry(dst, src);
    *dst = *carry;
    *carry = src[15];
    dst += SIMD_PACKETSIZE; src += SIMD_PACKETSIZE; l--;
    READ_PREFETCH_START1(src);
    WRITE_PREFETCH_START2(dst);
  }
}

static inline void swab_blocks_simd_unaligned_has_carry(uint16_t *dst16, const uint8_t *src, size_t l, uint8_t *carry) {
  uint8_t *dst = (uint8_t *) dst16;
  vuint8_t align_mask = vec_lvsl(0, src);
  while (l > 0) {
    swab_single_simdpacket_unaligned(dst, src, vdst_permmask_has_carry_noal, align_mask);
    dst[0] = *carry; *carry = src[15]; dst[15] = src[16]; 
    dst += SIMD_PACKETSIZE; src += SIMD_PACKETSIZE; l--;
    READ_PREFETCH_START1(src);
    WRITE_PREFETCH_START2(dst);
  }
}

static inline void swab_blocks_simd_aligned_no_carry(uint16_t *dst16, const uint8_t *src, size_t l) {
  uint8_t *dst = (uint8_t *) dst16;
  while (l > 0) { 
    swab_single_simdpacket_aligned_no_carry(dst, src);
    dst += SIMD_PACKETSIZE; src += SIMD_PACKETSIZE; l--;
    READ_PREFETCH_START1(src);
    WRITE_PREFETCH_START2(dst);
  }
}

static inline void swab_blocks_simd_unaligned_no_carry(uint16_t *dst16, const uint8_t *src, size_t l) {
  uint8_t *dst = (uint8_t *) dst16;
  vuint8_t align_mask = vec_lvsl(0, src);
  while (l > 0) {
    swab_single_simdpacket_unaligned(dst, src, vdst_permmask_no_carry_noal, align_mask);
    dst += SIMD_PACKETSIZE; src += SIMD_PACKETSIZE; l--;
    READ_PREFETCH_START1(src);
    WRITE_PREFETCH_START2(dst);
  }
}
