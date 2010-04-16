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

#define MACROFILE memset.h

#include "common.h"

#define LIBFREEVEC_SIMD_MACROS_MEMSET_H MAKESTR(LIBFREEVEC_SIMD_MACROS_INC)
#include LIBFREEVEC_SIMD_MACROS_MEMSET_H

#ifdef LIBFREEVEC_BUILD_AS_LIBC
void *memset(void *s, int p, size_t len) {
#else
void *vec_memset(void *s, int p, size_t len) {
#endif

  uint8_t* ptr = s;
  uint8_t __attribute__ ((aligned(16))) P = p;
  if (len >= sizeof(word_t)) {
    word_t pw = charmask(P);

    debug("\n1. ptr = %016x, pw = %016x, len = %d\n", ptr, pw, len);
    size_t al = ((size_t)ptr) % sizeof(word_t);
    debug("al = %d\n", al);
    if (al) {
      memset_fwd_until_dst_word_aligned(ptr, P, al);
      ptr += sizeof(word_t) - al;
      len -= sizeof(word_t) - al;
    }

    int l;
    word_t *ptr_w = (word_t *)(ptr);
    debug("ptr_w = %016x, ptr = %016x, pw = %016x, len = %d\n", ptr_w, ptr, pw, len);
    if (len >= SIMD_PACKETSIZE) {
      // ptr is now word (32/64bit) aligned, memset until ptr is SIMD aligned
      al = (word_t) ptr_w % SIMD_PACKETSIZE;
      debug("al = %d\n", al);
      if (al) {
        memset_fwd_until_simd_aligned(ptr_w, pw, al);
        ptr_w += (SIMD_PACKETSIZE - al)/WORDS_IN_PACKET;
        len -= SIMD_PACKETSIZE - al;
      }
      // ptr is now 128-bit aligned
      // perform set using SIMD
     
      l = len / SIMD_PACKETSIZE;
      debug("ptr_w = %016x, ptr = %016x, pw = %016x, len = %d, l = %d\n", ptr_w, ptr, pw, len, l);
      len -= l * SIMD_PACKETSIZE; 
      memset_set_blocks(ptr_w, pw, P, l);
      ptr_w += l * WORDS_IN_PACKET;
      debug("ptr_w = %016x, ptr = %016x, pw = %016x, len = %d\n", ptr_w, ptr, pw, len);
    }
    // memset the remaining words
    l = len / sizeof(word_t);
    len -= l * sizeof(word_t); 
    memset_rest_words(ptr_w, pw, l);
    ptr_w += l;
    ptr = (uint8_t *)ptr_w;
    debug("ptr_w = %016x, ptr = %016x, pw = %016x, len = %d\n", ptr_w, ptr, pw, len);
  }
  // Handle the remaining bytes
  memset_rest_bytes(ptr, P, len);
  return s;
}
