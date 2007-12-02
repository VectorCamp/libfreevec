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
#include "macros/memcpy.h"
#include "macros/common.h"

#ifdef VEC_GLIBC
void *memccpy(void *dstpp, const void *srcpp, int c, size_t len) {
#else
void *vec_memccpy(void *dstpp, const void *srcpp, int c, size_t len) {
#endif

  const uint8_t *src = (uint8_t *) srcpp;
  uint8_t *dst = (uint8_t *) dstpp;

  if (len >= sizeof(uint32_t)) {
    uint32_t charmask = charmask32(c);

    uint32_t al = (uint32_t)(dst) % sizeof(uint32_t);
    MEMCCPY_UNTIL_DEST_WORD_ALIGNED(dst, src, c, len, al);

    // Prefetch some stuff
    READ_PREFETCH_START(src);
    WRITE_PREFETCH_START(dst);

    // Take the word-aligned long pointers of src and dest.
    uint8_t srcoffset4 = (uint32_t)(src) % sizeof(uint32_t);
    const uint32_t *srcl = (uint32_t *)(src -srcoffset4);
    uint32_t *dstl = (uint32_t *)(dst);
    al = (uint32_t) dstl % ALTIVECWORD_SIZE;
    MEMCCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, len, srcoffset4, charmask, c, al);

    // Now dst is word aligned. If possible (ie if there are enough bytes left)
    // we want to align it to 16-byte boundaries as well.
    // For this we have to know the word-alignment of src also.

    src = (uint8_t *) srcl +srcoffset4;

    FILL_VECTOR(vc, c);

    // Check for the alignment of src
    if (((uint32_t)(src) % ALTIVECWORD_SIZE) == 0) {
      // Now, both buffers are 16-byte aligned, just copy everything directly
      MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, len, charmask, vc, c);
      srcl = (uint32_t *)(src);
    } else {
      // src is not 16-byte aligned so we have to a little trick with Altivec.
      MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, len, srcoffset4, charmask, vc, c);
      srcl = (uint32_t *)(src -srcoffset4);
    }
    MEMCCPY_REST_WORDS(dst, dstl, src, srcl, len, srcoffset4, charmask, c);

    dst = (uint8_t *) dstl;
    src = (uint8_t *) srcl +srcoffset4;
    MEMCCPY_FWD_NIBBLE(dst, src, c, len);

    READ_PREFETCH_STOP;
    WRITE_PREFETCH_STOP;

    return 0;
  } else {
    MEMCCPY_FWD_NIBBLE(dst, src, c, len);
    return 0;
  }
}
#endif
