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

static inline int memset_fwd_until_dst_word_aligned(uint8_t *ptr, int p, size_t len) {
  switch(len) {
  case 3:
    *ptr++ = p;
  case 2:
    *ptr++ = p;
  case 1:
    *ptr++ = p;
  }
}

#define MEMSET_UNTIL_WORD_ALIGNED( ptr, p, len, al )  \
{                                                      \
  int l = MIN( len, sizeof(uint32_t) - al );           \
  switch (l) {                                         \
  case 3:                                              \
    *ptr++ = p;                                        \
  case 2:                                              \
    *ptr++ = p;                                        \
  case 1:                                              \
    *ptr++ = p;                                        \
    len -= l;                                          \
  }                                                    \
}

#define MEMSET_WORD_UNTIL_ALTIVEC_ALIGNED( ptr32, p32, len )  \
{                                                              \
  int l = (ALTIVECWORD_SIZE - al) / sizeof(uint32_t);          \
  switch (l) {                                                 \
    case 3:                                                    \
      *ptr32++ = p32;                                          \
    case 2:                                                    \
      *ptr32++ = p32;                                          \
    case 1:                                                    \
      *ptr32++ = p32;                                          \
      len -= l*sizeof(uint32_t);                               \
  }                                                            \
}

#define MEMSET_ALTIVECWORD(ptr32, vc, len)  \
{                                           \
  vec_st(vc, 0, (uint8_t *) ptr32);         \
  ptr32 += 4; len -= ALTIVECWORD_SIZE;      \
}

#define MEMSET_LOOP_ALTIVECWORD(ptr32, vc, len)  \
{                                                \
  while (len >= ALTIVECWORD_SIZE) {              \
    MEMSET_ALTIVECWORD(ptr32, vc, len);          \
  }                                              \
}

#define MEMSET_LOOP_QUADWORD(ptr32, vc, len)  \
{                                             \
  WRITE_PREFETCH_START2(ptr32);               \
  uint32_t blocks = len >> LOG_ALTIVECQUAD;   \
  len -= blocks << LOG_ALTIVECQUAD;           \
  while (blocks--) {                          \
    vec_st(vc, 0, (uint8_t *)ptr32);          \
    vec_st(vc, 16, (uint8_t *)ptr32);         \
    vec_st(vc, 32, (uint8_t *)ptr32);         \
    vec_st(vc, 48, (uint8_t *)ptr32);         \
    ptr32 += 16;                              \
  }                                           \
}

#define MEMSET_REST_WORDS(ptr32, p32, len)  \
{                                           \
  int l = len / sizeof(uint32_t);           \
  len -= l*sizeof(uint32_t);                \
  while (l--) {                             \
      *ptr32++ = p32;                       \
  }                                         \
}

#define MEMSET_REST_WORDS2(ptr32, p32, len)  \
{                                           \
  int l = len / sizeof(uint32_t);           \
  switch (l) {                              \
    case 3:                                 \
      *ptr32++ = p32;                       \
    case 2:                                 \
      *ptr32++ = p32;                       \
    case 1:                                 \
      *ptr32++ = p32;                       \
      len -= l*sizeof(uint32_t);            \
  }                                         \
}