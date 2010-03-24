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
#include "macros/strcpy.h"

#ifdef VEC_GLIBC
int8_t *strcpy(int8_t *dstpp, const int8_t *srcpp) {
#else
int8_t *vec_strcpy(int8_t *dstpp, const int8_t *srcpp) {
#endif

  int8_t *src = (int8_t *)srcpp;
  int8_t *dst = dstpp;

  uint32_t al = (uint32_t)(dst) % sizeof(uint32_t);
  if (al)
    STRCPY_UNTIL_DST_WORD_ALIGNED(dst, src, al);

  // Take the word-aligned long pointers of src and dest.
  uint8_t srcal = (uint32_t)(src) % sizeof(uint32_t);
  uint32_t sh_l, sh_r;
  sh_l = srcal * CHAR_BIT; sh_r = CHAR_BIT*sizeof(uint32_t) - sh_l;

  const uint32_t *srcl = (uint32_t *)(src -srcal);
  uint32_t *dstl = (uint32_t *)(dst);
  al = (uint32_t) dstl % ALTIVECWORD_SIZE;
  if (al) {
    STRCPY_UNTIL_DST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, srcal, al);
  }

  // Now dst is word aligned. If possible (ie if there are enough bytes left)
  // we want to align it to 16-byte boundaries as well.
  // For this we have to know the word-alignment of src also.
  src = (int8_t *) srcl +srcal;

  // Prefetch some stuff
  READ_PREFETCH_START1(srcl);
  WRITE_PREFETCH_START2(dstl);

  // Check for the alignment of src
  if (((uint32_t)(src) % ALTIVECWORD_SIZE) == 0) {
    // Now, both buffers are 16-byte aligned, just copy everything directly
    STRCPY_LOOP_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl);
  } else {
    // src is not 16-byte aligned so we have to a little trick with Altivec.
    STRCPY_LOOP_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcal);
  }
}
#endif
