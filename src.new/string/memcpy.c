/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#include "libfreevec.h"

#define MACROFILE memcpy.h

#include "common.h"

#ifdef SIMD_ENGINE
#define SIMD_MACROS_INC MAKEINC(SIMD_ENGINE)
#else
#ifdef LINUX64
#define SIMD_MACROS_INC MAKEINC(scalar64)
#else
#define SIMD_MACROS_INC MAKEINC(scalar32)
#endif
#endif

#define SIMD_MACROS_MEMCPY_H MAKESTR(SIMD_MACROS_INC)
#include SIMD_MACROS_MEMCPY_H

#ifdef TEST_LIBC
void *memcpy(void *dstpp, const void *srcpp, size_t len) {
#else
void *vec_memcpy(void *dstpp, const void *srcpp, size_t len) {
#endif

    const uint8_t *src = srcpp;
    uint8_t *dst = dstpp;

    if (len >= WORDSIZE) {
        // Prefetch some stuff
        READ_PREFETCH_START1(src);
        WRITE_PREFETCH_START2(dst);

        debug("\nsrc = %016x, dst = %016x, len = %d\n", src, dst, len);
        // Copy until dst is word aligned
        int al = copy_fwd_until_dst_word_aligned(dst, src);
        if (al) {
            src += WORDSIZE-al;
            dst += WORDSIZE-al;
            len -= WORDSIZE-al;
        }
        debug("src = %016x, dst = %016x, len = %d, al = %d\n", src, dst, len, al);

        // Now dst is word aligned. We'll continue by word copying, but
        // for this we have to know the word-alignment of src also.
        word_t srcoffset = ((word_t)(src) % WORDSIZE), sh_l, sh_r;
        sh_l = srcoffset * CHAR_BIT;
        sh_r = CHAR_BIT * WORDSIZE - sh_l;
        
        debug("srcoffset = %d, sh_l = %d, sh_r = %d\n", srcoffset, sh_l, sh_r);

        // Take the word-aligned long pointers of src and dest.
        word_t *dstl = (word_t *)(dst);
        const word_t *srcl = (word_t *)(src - srcoffset);

#ifdef SIMD_ENGINE
        // While we're not 16-byte aligned, move in 4-byte long steps.
        MEMCPY_FWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dstl, srcl, len, srcoffset4, sh_l, sh_r);
        src = (uint8_t *) srcl + srcoffset4;

        // Now, dst is 16byte aligned. We can use Altivec if len >= 16

        if (((uint32_t)(src) % ALTIVECWORD_SIZE) == 0) {
            copy_fwd_rest_blocks_aligned(dstl, src, len);
        } else {
            copy_fwd_rest_blocks_unaligned(dstl, src, len);
        }

        srcl = (uint32_t *)(src - srcoffset4);

        // Stop the prefetching
        PREFETCH_STOP1;
        PREFETCH_STOP2;
#endif

        // Copy the remaining bytes using word-copying
        // Handle alignment as appropriate
        int l = len / WORDSIZE;
        len -= l * WORDSIZE;
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

#ifdef TEST_LIBC
void *memcpy_aligned(void *dstpp, const void *srcpp, size_t len) {
#else
void *vec_memcpy_aligned(void *dstpp, const void *srcpp, size_t len) {
#endif

    const uint8_t *src = srcpp;
    uint8_t *dst = dstpp;

    if (len >= WORDSIZE) {
        // Prefetch some stuff
        READ_PREFETCH_START1(src);
        WRITE_PREFETCH_START2(dst);

        // Take the word-aligned long pointers of src and dest.
        word_t *dstl = (word_t *)(dst);
        const word_t *srcl = (word_t *)(src);

#ifdef SIMD_ENGINE
        // Now, dst is 16byte aligned. We can use SIMD if len >= 16
        copy_fwd_rest_blocks_aligned(dstl, srcl, len);
#endif

        // Copy the remaining bytes using word-copying
        // Handle alignment as appropriate
        int l = len / WORDSIZE;
        printf("srcl = %016x, dstl = %016x, len = %d, l = %d\n", srcl, dstl, len, l);
        copy_fwd_rest_words_aligned(dstl, srcl, l);
        srcl += l;
        dstl += l;
        len -= l * WORDSIZE;
        // For the end copy we have to use char * pointers.
        src = (uint8_t *) srcl;
        dst = (uint8_t *) dstl;
    }

    // Stop the prefetching
    PREFETCH_STOP1;
    PREFETCH_STOP2;

    printf("src = %016x, dst = %016x, len = %d\n", src, dst, len);

    // Copy the remaining bytes
    copy_fwd_rest_bytes(dst, src, len);

    return dstpp;
}

