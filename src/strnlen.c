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
#include "macros/strlen.h"

#ifdef VEC_GLIBC
size_t strnlen(const char *str, size_t maxlen) {
#else
size_t vec_strnlen(const char *str, size_t maxlen) {
#endif

  uint8_t *ptr = (uint8_t *)str;
  uint32_t *ptr32, len = maxlen;

  STRNLEN_UNTIL_WORD_ALIGNED(str, ptr, len);
  ptr32 = (uint32_t *)(ptr);

  if (len >= ALTIVECWORD_SIZE) {
    STRNLEN_LOOP_UNTIL_ALTIVEC_ALIGNED(str, ptr32, len);

    READ_PREFETCH_START(ptr32);

    while (len >= ALTIVECWORD_SIZE) {
      STRNLEN_SINGLE_ALTIVEC_WORD(str, ptr32, len);
      READ_PREFETCH_START(ptr32);
    }
    READ_PREFETCH_STOP;
  }

  STRNLEN_REST_WORDS(str, ptr32, len);

  ptr = (uint8_t *) ptr32;
  STRNLEN_REST_BYTES(str, ptr, len);

  return maxlen;
}
#endif
