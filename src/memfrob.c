/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#ifdef HAVE_ALTIVEC_H
#include <altivec.h>

#include "libfreevec.h"
#include "macros/memfrob.h"

#ifdef VEC_GLIBC
void *memfrob(void *s, size_t len) {
#else
void *vec_memfrob(void *s, size_t len) {
#endif

  uint8_t* ptr = s;

  uint32_t al = (uint32_t)(ptr) % sizeof(uint32_t);
  if (al)
    MEMFROB_UNTIL_WORD_ALIGNED(ptr, len, al);

  uint32_t *ptr32 = (uint32_t *)(ptr);

  if (len >= ALTIVECWORD_SIZE) {
    // ptr is now 32-bit aligned, memset until ptr is altivec aligned
    al = (uint32_t) ptr32 % ALTIVECWORD_SIZE;
    if (al)
      MEMFROB_WORD_UNTIL_ALTIVEC_ALIGNED(ptr32, len);

    // setup the XOR vector
    vector uint32_t frobnivector = { FROBNICATOR32, FROBNICATOR32, FROBNICATOR32, FROBNICATOR32 };

    // ptr is now 128-bit aligned
    // Set 64-byte chunks at a time
    if (len >= QUAD_ALTIVECWORD) {
      MEMFROB_LOOP_QUADWORD(ptr32, frobnivector, len);
    }

    // memset the remaining 16-byte chunks
    while (len >= ALTIVECWORD_SIZE) {
      MEMFROB_ALTIVECWORD(ptr32, frobnivector, len);
    }
  }

  // memset the remaining words
  MEMFROB_REST_WORDS(ptr32, len);
  ptr = (uint8_t *)ptr32;

  // Handle the remaining bytes
  MEMFROB_NIBBLE(ptr, len);
  return s;
}
#endif
