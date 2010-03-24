/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
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
#include "macros/common.h"
#include "macros/memchr.h"

#ifdef VEC_GLIBC
void *memchr(void const *str, int c_in, size_t len) {
#else
void *vec_memchr(void const *str, int c_in, size_t len) {
#endif
  if (len) {

    uint8_t *ptr = (uint8_t *) str;
    uint8_t __attribute__ ((aligned(16))) c = c_in;
    uint32_t al = (uint32_t)(ptr) % sizeof(uint32_t);

    if (al)
      MEMCHR_UNTIL_WORD_ALIGNED(ptr, c, len, al);

    uint32_t *ptr32 = (uint32_t *)(ptr);

    if (len >= ALTIVECWORD_SIZE) {
      al = (uint32_t) ptr32 % ALTIVECWORD_SIZE;
      if (al)
        MEMCHR_LOOP_UNTIL_ALTIVEC_ALIGNED(ptr32, c, len, al);

      READ_PREFETCH_START1(ptr32);
      FILL_VECTOR(vc, c);

      while (len >= ALTIVECWORD_SIZE) {
        MEMCHR_SINGLE_ALTIVEC_WORD(vc, ptr32, c);
      }
    }
    MEMCHR_REST_WORDS(ptr32, c, len);

    ptr = (uint8_t *) ptr32;
    MEMCHR_REST_BYTES(ptr, c, len);
  }

  return 0;
}
#endif
