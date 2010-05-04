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

#define MACROFILE swab.h

#include "common.h"

#define LIBFREEVEC_SIMD_MACROS_SWAB_H MAKESTR(LIBFREEVEC_SIMD_MACROS_INC)
#include LIBFREEVEC_SIMD_MACROS_SWAB_H

#ifdef LIBFREEVEC_BUILD_AS_LIBC
void swab(void *srcpp, const void *dstpp, size_t len) {
#else
void vec_swab(void *srcpp, const void *dstpp, size_t len) {
#endif

  const uint8_t *src = (uint8_t *)srcpp;
  uint8_t *dst = (uint8_t *)dstpp;

  // In odd lenghts, the last byte is useless
  len &= ~((size_t) 1);
  int l;

  if (len >= sizeof(word_t)) {
    // depending on the alignment we might have to use a carry byte or not
    uint8_t carry = 0, has_carry = 0;

    // Now we know that we have at least 4 bytes, handle the word-alignment
    // of dst again with a switch
    has_carry = swab_until_dst_word_aligned_find_carry(dst, src, len, &carry);
    if (has_carry) {
      dst++; src++; len--;
    }

    // Take the shortword-aligned long pointers of src and dst.
    uint16_t *dst16 = (uint16_t *)(dst);

    if (has_carry) {
      // While we're not 16-byte aligned, move in 4-byte long steps.
      l = swab_word_until_simd_aligned_has_carry(dst16, src, len/sizeof(uint16_t), &carry);
      dst16 += l;
      len -= l * sizeof(uint16_t);
      src += l * sizeof(uint16_t);

      // Now, dst is 16byte aligned. We can use SIMD if len >= 16 bytes
      if (len > SIMD_PACKETSIZE) {
        // Prefetch some stuff
        READ_PREFETCH_START1 ( src );
        WRITE_PREFETCH_START2 ( dst );

        l = len / SIMD_PACKETSIZE;
        len -= l * SIMD_PACKETSIZE;
        // Check for the alignment of src
        if ((word_t)(src) % SIMD_PACKETSIZE == 0) {
          // Now, both buffers are 16-byte aligned, just copy everything directly
          swab_blocks_simd_aligned_has_carry(dst16, src, l, &carry);
        } else {
          // src is not 16-byte aligned so we have to a do little trick with SIMD
          swab_blocks_simd_unaligned_has_carry(dst16, src, l, &carry);
        }
        dst16 += l * SIMD_PACKETSIZE/sizeof(uint16_t);
        src += l * SIMD_PACKETSIZE;
        // Stop prefetching.
        PREFETCH_STOP1;
        PREFETCH_STOP2;
      }
      // Copy the remaining bytes using word-copying
      // Handle alignment as appropriate
      l = len / sizeof(uint16_t);
      swab_rest_words_has_carry(dst16, src, l, &carry);
      len -= l * sizeof(uint16_t);
      dst16 += l;
      src += l * sizeof(uint16_t);

      // And put the 'carry' byte in its place
      *((uint8_t *)dst16) = carry;
      dst = (uint8_t *) dst16; 
    } else {
      // While we're not 16-byte aligned, move in 4-byte long steps.
      l = swab_word_until_simd_aligned_no_carry(dst16, src, len/sizeof(uint16_t));
      dst16 += l;
      len -= l * sizeof(uint16_t);
      src += l * sizeof(uint16_t);

      // Prefetch some stuff
      READ_PREFETCH_START1 ( src );
      WRITE_PREFETCH_START2 ( dst );

      // Now, dst is 16byte aligned. We can use SIMD if len >= 16 bytes
      if (len > SIMD_PACKETSIZE) {
        l = len / SIMD_PACKETSIZE;
        len -= l * SIMD_PACKETSIZE;
        // Check for the alignment of src
        if ((word_t)(src) % SIMD_PACKETSIZE == 0) {
          // Now, both buffers are 16-byte aligned, just copy everything directly
          swab_blocks_simd_aligned_no_carry(dst16, src, l);
        } else {
          // src is not 16-byte aligned so we have to a do little trick with SIMD
          swab_blocks_simd_unaligned_no_carry(dst16, src, l);
        }
        dst16 += l * SIMD_PACKETSIZE/sizeof(uint16_t);
        src += l * SIMD_PACKETSIZE;
        // Stop prefetching.
        PREFETCH_STOP1;
        PREFETCH_STOP2;
      }

      // Copy the remaining bytes using word-copying
      // Handle alignment as appropriate
      l = len / sizeof(uint16_t);
      len -= l * sizeof(uint16_t);
      swab_rest_words_no_carry(dst16, src, l);
      dst16 += l;
      src += l * sizeof(uint16_t);
      dst = (uint8_t *) dst16; 
    }
  }
 
  // We don't have anything left to do.
  swab_rest_bytes(dst, src, len);
  return;
}
