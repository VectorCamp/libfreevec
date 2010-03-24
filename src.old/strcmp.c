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
int strcmp(const uint8_t *src1pp, const uint8_t *src2pp) {
#else
int vec_strcmp(const uint8_t *src1pp, const uint8_t *src2pp) {
#endif

  const uint8_t *src2 = (uint8_t *)src2pp;
  const uint8_t *src1 = (uint8_t *)src1pp;

  uint32_t src1al = (uint32_t)(src1) % ALTIVECWORD_SIZE;
  uint32_t src2al = (uint32_t)(src2) % ALTIVECWORD_SIZE;
  if ((src1al | src2al) == 0) {
    uint32_t *src1l = (uint32_t *)(src1);
    const uint32_t *src2l = (uint32_t *)(src2);

    STRCMP_LOOP_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l);
  } else {
    src1al = (uint32_t)(src1) % sizeof(uint32_t);
    src2al = (uint32_t)(src2) % sizeof(uint32_t);
    if (src1al)
      STRCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, src1al);

    // Take the word-aligned long pointers of src2 and dest.
    src2al = (uint32_t)(src2) % sizeof(uint32_t);
    uint32_t sh_l, sh_r;
    sh_l = src2al * CHAR_BIT; sh_r = CHAR_BIT*sizeof(uint32_t) - sh_l;

    uint32_t *src1l = (uint32_t *)(src1);
    const uint32_t *src2l = (uint32_t *)(src2 -src2al);

    src1al = (uint32_t)src1 % ALTIVECWORD_SIZE;
    if (src1al)
      STRCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, src1al, src2al);

    // Now src1 is word aligned. If possible (ie if there are enough bytes left)
    // we want to align it to 16-byte boundaries as well.
    // For this we have to know the word-alignment of src2 also.
    if (((uint32_t)(src2) % ALTIVECWORD_SIZE) == 0) {
      // Now, both buffers are 16-byte aligned, just copy everything directly
      STRCMP_LOOP_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l);    
    } else {
      // src2 is not 16-byte aligned so we have to a little trick with Altivec.
      STRCMP_LOOP_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2al);
    }
  }

  return 0;
}
#endif
