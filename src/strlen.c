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
#include "macros/common.h"
#include "macros/strlen.h"


#ifdef VEC_GLIBC
size_t strlen(const int8_t *str) {
#else
size_t vec_strlen(const int8_t *str) {
#endif
  int8_t *ptr = (int8_t *) str;
  STRLEN_UNTIL_WORD_ALIGNED(str, ptr);

  uint32_t *ptr32 = (uint32_t *)(ptr);
  READ_PREFETCH_START1(ptr32);

  uint32_t al = (uint32_t) ptr32 % ALTIVECWORD_SIZE;
  if (al)
    STRLEN_WORDS_UNTIL_ALTIVEC_ALIGNED(str, ptr32, al);

  STRLEN_LOOP_ALTIVEC_WORD(str, ptr32);

  return ptrdiff_t(ptr,str);
}
#endif
