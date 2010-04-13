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

static inline void swab_rest_bytes(uint8_t *dst, const uint8_t *src, size_t len) {
  switch (len) {
    case 4:
      dst[0] = src[1]; dst[1] = src[0];
      dst[2] = src[3]; dst[3] = src[2];
      break;
    case 2:
      dst[0] = src[1]; dst[1] = src[0];
      break;
  }
}

static inline int swab_until_dst_word_aligned_find_carry(uint8_t *dst, const uint8_t *src, size_t len, uint8_t *carry) {
  if (((word_t)dst) % sizeof(uint16_t)) {
    *dst = src[1];
    *carry = src[0];
    return 1;
  } else
    return 0;
}

#define SWAB_WORD_UNTIL_ALTIVEC_ALIGNED_HAS_CARRY(dst16, src, len)  \
  while (((uint32_t)(dst16) % 16) && len >= sizeof(uint16_t)) {     \
    *dst16++ = ((uint16_t)carry << 8) | ((uint16_t)src[2]);         \
    carry = src[1];                                                 \
    src += sizeof(uint16_t);                                        \
    len -= sizeof(uint16_t);                                        \
  }

#define SWAB_WORD_UNTIL_ALTIVEC_ALIGNED_NO_CARRY(dst16, src, len)              \
  while (((uint32_t)(dst16) % ALTIVECWORD_SIZE) && len >= sizeof(uint16_t)) {  \
    *dst16++ = ((uint16_t)src[0]) | ((uint16_t)src[1] << 8);                   \
    src += sizeof(uint16_t);                                                   \
    len -= sizeof(uint16_t);                                                   \
  }

#define SWAB_REST_WORDS_HAS_CARRY(dst16, src, len)           \
  while (len >= sizeof(uint16_t)) {                          \
    *dst16++ = ((uint16_t)carry << 8) | ((uint16_t)src[2]);  \
    carry = src[1];                                          \
    src += sizeof(uint16_t);                                 \
    len -= sizeof(uint16_t);                                 \
  }

#define SWAB_REST_WORDS_NO_CARRY(dst16, src, len)             \
  while (len >= sizeof(uint16_t)) {                           \
    *dst16++ = ((uint16_t)src[0]) | ((uint16_t)src[1] << 8);  \
    src += sizeof(uint16_t);                                  \
    len -= sizeof(uint16_t);                                  \
  }

