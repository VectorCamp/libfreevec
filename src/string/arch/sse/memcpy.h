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

#include <sys/types.h>
#include <stdint.h>
#include <emmintrin.h>
#include <xmmintrin.h>

#ifdef LINUX64
#include "scalar64/memcpy.h"
#else
#include "scalar32/memcpy.h"
#endif

static inline void copy_fwd_rest_blocks_aligned(word_t *d, const uint8_t *s, size_t blocks) {
    __m128i v1, v2, v3, v4;
    // Unroll blocks of 4 words
    while (blocks > 4) {
        v1 = _mm_load_si128((__m128i *)s);
        v2 = _mm_load_si128((__m128i *)(s+16));
        v3 = _mm_load_si128((__m128i *)(s+32));
        v4 = _mm_load_si128((__m128i *)(s+48));
        _mm_store_si128((__m128i *)(d), v1);
        _mm_store_si128((__m128i *)(d + WORDS_IN_PACKET), v2);
        _mm_store_si128((__m128i *)(d + 2*WORDS_IN_PACKET), v3);
        _mm_store_si128((__m128i *)(d + 3*WORDS_IN_PACKET), v4);
        d += 16; s += 4 * SIMD_PACKETSIZE;
        blocks -= 4;
    }

    while (blocks > 0) {
        v1 = _mm_load_si128((__m128i *)s);
        _mm_store_si128((__m128i *)(d), v1);
        d += 4; s += SIMD_PACKETSIZE;
        blocks--;
    }
}

static inline void copy_fwd_rest_blocks_unaligned(word_t *d, const uint8_t *s, int srcoffset, int sl, int sr, size_t blocks) {
   /* __vector uint8_t mask, MSQ1, LSQ1, LSQ2, LSQ3, LSQ4;
    mask = vec_lvsl(0, s);*/
    
    // Unroll blocks of 4 words
    while (blocks > 4) {
/*        MSQ1 = vec_ld(0, s);
        LSQ1 = vec_ld(15, s);
        LSQ2 = vec_ld(31, s);
        LSQ3 = vec_ld(47, s);
        LSQ4 = vec_ld(63, s);
        vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)d);
        vec_st(vec_perm(LSQ1, LSQ2, mask), 16, (uint8_t *)d);
        vec_st(vec_perm(LSQ2, LSQ3, mask), 32, (uint8_t *)d);
        vec_st(vec_perm(LSQ3, LSQ4, mask), 48, (uint8_t *)d);*/
        d += 16; s += 4 * SIMD_PACKETSIZE;
        blocks -= 4;
    }

    while (blocks > 0) {
/*        MSQ1 = vec_ld(0, s);
        LSQ1 = vec_ld(15, s);
        vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)d);*/
        d += 4; s += SIMD_PACKETSIZE;
        blocks--;
    }
}
