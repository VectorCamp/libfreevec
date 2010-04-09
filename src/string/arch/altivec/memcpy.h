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
#include "arch/altivec.h"

static inline void copy_fwd_rest_blocks_aligned(word_t *d, const word_t *s, size_t blocks) {
    vector unsigned char v1, v2, v3, v4;
    // Unroll blocks of 4 words
    while (blocks % 4 > 0) {
        v1 = vec_ld(0, (uint8_t *)s);
        v2 = vec_ld(16, (uint8_t *)s);
        v3 = vec_ld(32, (uint8_t *)s);
        v4 = vec_ld(48, (uint8_t *)s);
        vec_st(v1, 0, (uint8_t *)d);
        vec_st(v2, 0, (uint8_t *)d);
        vec_st(v3, 0, (uint8_t *)d);
        vec_st(v4, 0, (uint8_t *)d);
        d += 16; s + 16;
        blocks -= 4;
    }

    while (blocks > 0) {
        v1 = vec_ld(0, (uint8_t *)s);
        vec_st(v1, 0, (uint8_t *)d);
        d += 4; s + 4;
        blocks--;
    }
}

static inline void copy_fwd_rest_blocks_unaligned(word_t *d, const word_t *s, int sl, int sr, size_t blocks) {
    vector unsigned char mask, MSQ1, LSQ1, LSQ2, LSQ3, LSQ4;
    mask = vec_lvsl(0, s);
    
    // Unroll blocks of 4 words
    while (blocks % 4 > 0) {
        MSQ1 = vec_ld(0, (uint8_t *)s);
        LSQ1 = vec_ld(15, (uint8_t *)s);
        LSQ2 = vec_ld(31, (uint8_t *)s);
        LSQ3 = vec_ld(47, (uint8_t *)s);
        LSQ4 = vec_ld(63, (uint8_t *)s);
        vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)d);
        vec_st(vec_perm(LSQ1, LSQ2, mask), 16, (uint8_t *)d);
        vec_st(vec_perm(LSQ2, LSQ3, mask), 32, (uint8_t *)d);
        vec_st(vec_perm(LSQ3, LSQ4, mask), 48, (uint8_t *)d);
        d += 16; s += 16;
        blocks -= 4;
    }

    while (blocks > 0) {
        MSQ1 = vec_ld(0, s);
        LSQ1 = vec_ld(15, s);
        vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)d);
        d += 4; s + 4;
        blocks--;
    }
}