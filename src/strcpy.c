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

/*  uint8_t *src = (uint8_t *)srcpp;
  uint8_t *dst = (uint8_t *)dstpp;

  const uint8_t *src = ( uint8_t * ) srcpp;
  uint8_t *dst = ( uint8_t * ) dstpp;

  uint32_t charmask = charmask32 ( c );

    uint32_t al = ( uint32_t ) ( dst ) % sizeof ( uint32_t );
    MEMCCPY_UNTIL_DEST_WORD_ALIGNED ( dst, src, c, len, al );

    // Prefetch some stuff
    READ_PREFETCH_START1 ( src );
    WRITE_PREFETCH_START2 ( dst );

    // Take the word-aligned long pointers of src and dest.
    uint8_t srcoffset4 = ( uint32_t ) ( src ) % sizeof ( uint32_t );
    const uint32_t *srcl = ( uint32_t * ) ( src -srcoffset4 );
    uint32_t *dstl = ( uint32_t * ) ( dst );
    al = ( uint32_t ) dstl % ALTIVECWORD_SIZE;
    MEMCCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED ( dst, dstl, src, srcl, len, srcoffset4, charmask, c, al );

    // Now dst is word aligned. If possible (ie if there are enough bytes left)
    // we want to align it to 16-byte boundaries as well.
    // For this we have to know the word-alignment of src also.

    src = ( uint8_t * ) srcl +srcoffset4;

    FILL_VECTOR ( vc, c );

    // Check for the alignment of src
    if ( ( ( uint32_t ) ( src ) % ALTIVECWORD_SIZE ) == 0 ) {
      // Now, both buffers are 16-byte aligned, just copy everything directly
      MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED ( dst, dstl, src, srcl, len, charmask, vc, c );
      srcl = ( uint32_t * ) ( src );
    } else {
      // src is not 16-byte aligned so we have to a little trick with Altivec.
      MEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED ( dst, dstl, src, srcl, len, srcoffset4, charmask, vc, c );
      srcl = ( uint32_t * ) ( src -srcoffset4 );
    }
    MEMCCPY_REST_WORDS ( dst, dstl, src, srcl, len, srcoffset4, charmask, c );

    dst = ( uint8_t * ) dstl;
    src = ( uint8_t * ) srcl +srcoffset4;
    MEMCCPY_FWD_NIBBLE ( dst, src, c, len );

    PREFETCH_STOP1;
    PREFETCH_STOP2;

    return 0;
  } else {
    MEMCCPY_FWD_NIBBLE ( dst, src, c, len );
    return 0;
  }

  MYSTRCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED_new(dstpp, dst, src);

  // Take the word-aligned long pointers of src and dest.
  uint8_t srcoffset4 = ((uint32_t)(src) & (sizeof(uint32_t)-1));
  uint32_t *dstl = (uint32_t *)(dst);
  const uint32_t *srcl = (uint32_t *)(src -srcoffset4);

  vector uint8_t v0 = vec_splat_u8(0);
  // Check for the alignment of src
  if (((uint32_t)(src) % ALTIVECWORD_SIZE) == 0)
    {
      // Now, both buffers are 16-byte aligned, just copy everything directly
      MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dstpp, dst, dstl, src, srcl, srcoffset4, v0);
    }
  else
    {
      // src is not 16-byte aligned so we have to a little trick with Altivec.
      MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dstpp, dst, dstl, src, srcl, srcoffset4, v0);
    }*/
  return NULL;
}
#endif
