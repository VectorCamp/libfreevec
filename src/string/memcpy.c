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

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>

#define MACROFILE memcpy.h

#include "common.h"

#define LIBFREEVEC_SIMD_MACROS_MEMCPY_H MAKESTR(LIBFREEVEC_SIMD_MACROS_INC)
#include LIBFREEVEC_SIMD_MACROS_MEMCPY_H

#ifdef LIBFREEVEC_BUILD_AS_LIBC
void *memcpy(void *dstpp, const void *srcpp, size_t len) {
#else
void *vec_memcpy(void *dstpp, const void *srcpp, size_t len) {
#endif

    const uint8_t *src = srcpp;
    uint8_t *dst = dstpp;

    if (len >= sizeof(word_t)) {
        // Prefetch some stuff
        READ_PREFETCH_START1(src);
        WRITE_PREFETCH_START2(dst);

        debug("\n1. src = %016x, dst = %016x, len = %d\n", src, dst, len);
        // Copy until dst is word aligned
        int al = copy_fwd_until_dst_word_aligned(dst, src), l;

        if (al) {
            src += sizeof(word_t) - al;
            dst += sizeof(word_t) - al;
            len -= sizeof(word_t) - al;
        }

        debug("src = %016x, dst = %016x, len = %d, al = %d\n", src, dst, len, al);

        // Now dst is word aligned. We'll continue by word copying, but
        // for this we have to know the word-alignment of src also.
        int srcoffset = ((word_t)(src) % sizeof(word_t)), sh_l, sh_r;
        sh_l = srcoffset * CHAR_BIT;
        sh_r = CHAR_BIT * sizeof(word_t) - sh_l;

        debug("srcoffset = %d, sh_l = %d, sh_r = %d\n", srcoffset, sh_l, sh_r);

        // Take the word-aligned long pointers of src and dest.
        word_t *dstl = (word_t *)(dst);
        const word_t *srcl = (word_t *)(src - srcoffset);

//#ifdef LIBFREEVEC_SIMD_ENGINE
        debug("srcl = %016x, dstl = %016x, len = %d\n", srcl, dstl, len);
       
        if (len >= SIMD_PACKETSIZE) { 
        // While we're not 16-byte aligned, move in 4-byte long steps.
        
        al = (word_t)dstl % SIMD_PACKETSIZE;
        debug("srcl = %016x, dstl = %016x, len = %d, al = %d\n", srcl, dstl, len, al);
        if (al) {
            copy_fwd_until_dst_simd_aligned(dstl, srcl, srcoffset, al, sh_l, sh_r);
            srcl += (SIMD_PACKETSIZE - al)/WORDS_IN_PACKET;
            src = (uint8_t *) srcl + srcoffset;
            dstl += (SIMD_PACKETSIZE - al)/WORDS_IN_PACKET;
            len -= SIMD_PACKETSIZE - al;
        }
        
        debug("srcl = %016x, dstl = %016x, len = %d\n", srcl, dstl, len);
        debug("src = %016x, al = %d\n", src, al);
        // Now, dst is 16byte aligned. We can use SIMD if len >= 16
        l = len / SIMD_PACKETSIZE;
        len -= l * SIMD_PACKETSIZE;
        debug("srcl = %016x, dstl = %016x, len = %d, l = %d, step = %d\n", srcl, dstl, len, l, l*WORDS_IN_PACKET);
        if (((word_t)(src) % SIMD_PACKETSIZE) == 0) {
            copy_fwd_rest_blocks_aligned(dstl, src, l);
        } else {
            copy_fwd_rest_blocks_unaligned(dstl, src, srcoffset, sh_l, sh_r, l);
        }
        src += l*SIMD_PACKETSIZE;
        dstl += l * WORDS_IN_PACKET;
        srcl = (word_t *)(src - srcoffset);
        }
        // Stop the prefetching
        PREFETCH_STOP1;
        PREFETCH_STOP2;
//#endif

        // Copy the remaining bytes using word-copying
        // Handle alignment as appropriate
        l = len / sizeof(word_t);
        len -= l * sizeof(word_t);
        debug("srcl = %016x, dstl = %016x, len = %d, l = %d\n", srcl, dstl, len, l);

        if (srcoffset == 0) {
            copy_fwd_rest_words_aligned(dstl, srcl, l);
            srcl += l;
            src = (uint8_t *) srcl;
        } else {
            copy_fwd_rest_words_unaligned(dstl, srcl, sh_l, sh_r, l);
            srcl += l;
            src = (uint8_t *) srcl + srcoffset;
        }

        dstl += l;

        // For the end copy we have to use char * pointers.
        dst = (uint8_t *) dstl;
    }

    debug("src = %016x, dst = %016x, len = %d\n", src, dst, len);

    // Copy the remaining bytes
    copy_fwd_rest_bytes(dst, src, len);

    return dstpp;
}

#ifdef LIBFREEVEC_BUILD_AS_LIBC
void *memcpy_aligned(void *dstpp, const void *srcpp, size_t len) {
#else
void *vec_memcpy_aligned(void *dstpp, const void *srcpp, size_t len) {
#endif

    const uint8_t *src = srcpp;
    uint8_t *dst = dstpp;

    if (len >= sizeof(word_t)) {
        // Prefetch some stuff
        READ_PREFETCH_START1(src);
        WRITE_PREFETCH_START2(dst);

        // Take the word-aligned long pointers of src and dest.
        word_t *dstl = (word_t *)(dst);
        const word_t *srcl = (word_t *)(src);
        int l;

#ifdef LIBFREEVEC_SIMD_ENGINE
        if (len >= SIMD_PACKETSIZE) { 
        l = len / SIMD_PACKETSIZE;
        len -= l * SIMD_PACKETSIZE;
        // Now, dst is 16byte aligned. We can use SIMD if len >= 16
        copy_fwd_rest_blocks_aligned(dstl, src, l);
        }
#endif

        // Copy the remaining bytes using word-copying
        // Handle alignment as appropriate
        l = len / sizeof(word_t);
        debug("srcl = %016x, dstl = %016x, len = %d, l = %d\n", srcl, dstl, len, l);
        copy_fwd_rest_words_aligned(dstl, srcl, l);
        srcl += l;
        dstl += l;
        len -= l * sizeof(word_t);
        // For the end copy we have to use char * pointers.
        src = (uint8_t *) srcl;
        dst = (uint8_t *) dstl;
    }

    // Stop the prefetching
    PREFETCH_STOP1;

    PREFETCH_STOP2;

    debug("src = %016x, dst = %016x, len = %d\n", src, dst, len);

    // Copy the remaining bytes
    copy_fwd_rest_bytes(dst, src, len);

    return dstpp;
}
