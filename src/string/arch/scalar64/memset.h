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

#include "arch/scalar64.h"

static inline void memset_fwd_until_dst_word_aligned(uint8_t *ptr, uint8_t c, size_t al) {
  switch(al) {
  case 1:
    *ptr++ = c;
  case 2:
    *ptr++ = c;
  case 3:
    *ptr++ = c;
  }
}

static inline void memset_fwd_until_simd_aligned(word_t *ptr_w, word_t w, size_t al) {
  switch (al) {
    case 4:
      *ptr_w++ = w;
    case 8:
      *ptr_w++ = w;
    case 12:
      *ptr_w++ = w;
  }
}

static inline void memset_rest_words(word_t *ptr_w, word_t w, size_t l) {
  while (l--) {
      *ptr_w++ = w;
  }
}

static inline int memset_rest_bytes(uint8_t *ptr, uint8_t c, size_t len) {
  switch(len) {
  case 3:
    *ptr++ = c;
  case 2:
    *ptr++ = c;
  case 1:
    *ptr++ = c;
  }
}
// Only define these if there is no SIMD_ENGINE defined
#ifndef LIBFREEVEC_SIMD_ENGINE
static inline void memset_set_blocks(word_t *ptr_w, word_t w, uint8_t c, size_t blocks) {
  while (blocks) { 
      *ptr_w++ = w;
      *ptr_w++ = w;
      *ptr_w++ = w;
      *ptr_w++ = w;
      blocks--;
  }
}
#endif
