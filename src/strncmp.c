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
#include <limits.h>

#ifdef HAVE_ALTIVEC_H
#include <altivec.h>

#include "libfreevec.h"
#include "macros/common.h"
#include "macros/strcmp.h"

#ifdef VEC_GLIBC
int strncmp(const uint8_t *src1pp, const uint8_t *src2pp, size_t len) {
#else
int vec_strncmp(const uint8_t *src1pp, const uint8_t *src2pp, size_t len) {
#endif

  uint8_t *src2 = (uint8_t *)src2pp;
  uint8_t *src1 = (uint8_t *)src1pp;

  if (len >= sizeof(uint32_t)) {
    uint32_t src1al = (uint32_t)(src1) % ALTIVECWORD_SIZE;
    uint32_t src2al = (uint32_t)(src2) % ALTIVECWORD_SIZE;
    uint32_t sh_l, sh_r;
    sh_l = src2al * CHAR_BIT; sh_r = CHAR_BIT*sizeof(uint32_t) - sh_l;
    if ((src1al | src2al) == 0) {
      uint32_t *src1l = (uint32_t *)(src1);
      const uint32_t *src2l = (uint32_t *)(src2);

      // Both buffers aligned to 16-byte boundaries, proceed with AltiVec
      STRNCMP_LOOP_ALTIVEC_WORDS_ALIGNED(src1, src1l, src2, src2l);
      PREFETCH_STOP1;
      PREFETCH_STOP2;

      STRNCMP_REST_WORDS(src1, src1l, src2, src2l, len, src2al);
      src1 = (uint8_t *) src1l;
      src2 = (uint8_t *) src2l;

      STRNCMP_NIBBLE(src1, src2, len);
      return 0;
    } else {
      src1al = (uint32_t)(src1) % sizeof(uint32_t);
      if (src1al)
        STRNCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, len, src1al);

      src2al = (uint32_t)(src2) % sizeof(uint32_t);
      sh_l = src2al * CHAR_BIT; sh_r = CHAR_BIT*sizeof(uint32_t) - sh_l;

      uint32_t *src1l = (uint32_t *)(src1);
      const uint32_t *src2l = (uint32_t *)(src2 -src2al);

      // Now src1 is word aligned. If possible (ie if there are enough bytes left)
      // we want to align it to 16-byte boundaries as well.
      // For this we have to know the word-alignment of src2 also.
      src1al = (uint32_t)src1 % ALTIVECWORD_SIZE;
      if (src1al)
        STRNCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, len, src1al, src2al);

      if (((uint32_t)(src2) % ALTIVECWORD_SIZE) == 0) {
        STRNCMP_LOOP_ALTIVEC_WORDS_ALIGNED(src1, src1l, src2, src2l);
      } else {
        STRNCMP_LOOP_ALTIVEC_WORDS_UNALIGNED(src1, src1l, src2, src2l, src2al);
      }
      PREFETCH_STOP1;
      PREFETCH_STOP2;

      STRNCMP_REST_WORDS(src1, src1l, src2, src2l, len, src2al);
      src1 = (uint8_t *) src1l;
      src2 = (uint8_t *) src2l +src2al;

      STRNCMP_NIBBLE(src1, src2, len);
      return 0;
    }
  } else {
    STRNCMP_NIBBLE(src1, src2, len);
    return 0;
  }
}
#endif
