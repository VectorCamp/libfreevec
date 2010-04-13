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

#include "arch/scalar32.h"

static inline int copy_fwd_until_dst_word_aligned(uint8_t *d, const uint8_t *s) {
    size_t dstal = ((size_t)d) % sizeof(word_t);

    switch (dstal) {
        case 1:
            *d++ = *s++;
        case 2:
            *d++ = *s++;
        case 3:
            *d = *s;
    }

    return dstal;
}

static inline void copy_fwd_rest_bytes(uint8_t *d, const uint8_t *s, size_t len) {
    switch (len) {
        case 3:
            *d++ = *s++;
        case 2:
            *d++ = *s++;
        case 1:
            *d++ = *s++;
    }
}

static inline void copy_fwd_rest_words_aligned(word_t *d, const word_t *s, size_t l) {
    while (l > 0) {
        *d++ = *s++;
        l--;
    }
}

static inline void copy_fwd_rest_words_unaligned(word_t *d, const word_t *s, int sl, int sr, size_t l) {
    while (l > 0) {
        *d++ = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr); s++;
        l--;
    }
}

static inline void copy_fwd_until_dst_simd_aligned(word_t *d, const word_t *s, 
                                                int srcoffset4, size_t dstal, int sl, int sr) {
    if (srcoffset4 == 0) {
        switch (dstal) {
            case 4:
                *d++ = *s++;
            case 8:
                *d++ = *s++;
            case 12:
                *d++ = *s++;
        }
    } else {
        switch (dstal) {
            case 4:
                *d = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr);
                d++; s++;
            case 8:
                *d = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr);
                d++; s++;
            case 12:
                *d = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr);
                d++; s++;
        }
    }
}

// Only define these if there is no SIMD_ENGINE defined
#ifndef LIBFREEVEC_SIMD_ENGINE
static inline void copy_fwd_rest_blocks_aligned(word_t *d, const uint8_t *src, size_t blocks) {
    word_t *s = (word_t *)src;
    // Unroll blocks of 4 words
    while (blocks > 0) {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        blocks--;
    }
}

static inline void copy_fwd_rest_blocks_unaligned(word_t *d, const uint8_t *src, int srcoffset, int sl, int sr, size_t blocks) {
    word_t *s = (word_t *)(src - srcoffset);
    // Unroll blocks of 4 words
    while (blocks > 0) {
        *d++ = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr); s++;
        *d++ = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr); s++;
        *d++ = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr); s++;
        *d++ = MERGE_SHIFTED_WORDS(*(s), *(s + 1), sl, sr); s++;
        blocks--;
    }
}
#endif // LIBFREEVEC_SIMD_ENGINE
