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
#include "macros/strcpy.h"

#ifdef VEC_GLIBC
int8_t *strcpy(int8_t *dstpp, const int8_t *srcpp)
{
#else
int8_t *vec_strcpy(int8_t *dstpp, const int8_t *srcpp)
{
#endif

  const int8_t *src = srcpp;
  int8_t *dst = dstpp;
  
  uint32_t srcal = (uint32_t)(src) % ALTIVECWORD_SIZE;
  uint32_t dstal = (uint32_t)(dst) % ALTIVECWORD_SIZE;
  if ((srcal | dstal) == 0) {
    const uint32_t *srcl = (uint32_t *)(src);
    uint32_t *dstl = (uint32_t *)(dst);
    
    STRCPY_LOOP_ALTIVEC_WORD_ALIGNED(src, srcl, dst, dstl);
  } else {
    srcal = (uint32_t)(src) % sizeof(uint32_t);
    dstal = (uint32_t)(dst) % sizeof(uint32_t);
    if (srcal)
      STRCPY_UNTIL_SRC_WORD_ALIGNED(src, dst, srcal);

    // Take the word-aligned long pointers of dst and dest.
    uint8_t dstal = ((uint32_t)(dst) % sizeof(uint32_t));

    const uint32_t *srcl = (uint32_t *)(src);
    uint32_t *dstl = (uint32_t *)(dst -dstal);

    srcal = (uint32_t)src % ALTIVECWORD_SIZE;
    if (srcal)
      STRCPY_UNTIL_SRC_IS_ALTIVEC_ALIGNED(src, srcl, dst, dstl, srcal, dstal);

    // Now src is word aligned. If possible (ie if there are enough bytes left)
    // we want to align it to 16-byte boundaries as well.
    // For this we have to know the word-alignment of dst also.
    if (((uint32_t)(dst) % ALTIVECWORD_SIZE) == 0) {
      // Now, both buffers are 16-byte aligned, just copy everything directly
      STRCPY_LOOP_ALTIVEC_WORD_ALIGNED(src, srcl, dst, dstl);    
    } else {
      // dst is not 16-byte aligned so we have to a little trick with Altivec.
      STRCPY_LOOP_ALTIVEC_WORD_UNALIGNED(src, srcl, dst, dstl, srcal);
    }
  }
}
#endif
