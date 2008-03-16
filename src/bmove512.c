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

#ifdef VEC_GLIBC
void bmove512 ( void *to, const void *from, uint32_t len ) {
#else
void vec_bmove512 ( void *to, const void *from, uint32_t len ) {
#endif

  // dstl and srcl are assumed to be 32-bit aligned and length to be a multiple of 512.
  // Now dstl is word aligned but we want to align it to 16-byte boundaries as well.
  const uint8_t *src = from;
  uint8_t *dst = to;

  // Prefetch some stuff
  READ_PREFETCH_START1 ( src );
  WRITE_PREFETCH_START2 ( dst );

  uint32_t *dstl = ( uint32_t * ) ( dst );
  const uint32_t *srcl = ( uint32_t * ) ( src );

  // While we're not 16-byte aligned, move in 4-byte long steps.
  MEMCPY_FWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED ( dstl, srcl, len, 0, 0, 0);
  src = ( uint8_t * ) srcl;

  if ( ( ( uint32_t ) ( src ) % ALTIVECWORD_SIZE ) == 0 ) {
    MEMCPY_FWD_LOOP_QUADWORD_ALTIVEC_ALIGNED ( dstl, src, len );
  } else {
    MEMCPY_FWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED ( dstl, src, len );
  }

  while ( len >= ALTIVECWORD_SIZE ) {
    if ( ( ( uint32_t ) ( src ) % ALTIVECWORD_SIZE ) == 0 ) {
      MEMCPY_SINGLEQUADWORD_ALTIVEC_ALIGNED ( dstl, src, 0 );
      dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;
    } else {
      MEMCPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED ( dstl, src, 0 );
      dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;
    }
  }

  // Copy the remaining bytes using word-copying
  // Handle alignment as appropriate
  srcl = ( uint32_t * ) ( src );
  MEMCPY_FWD_REST_WORDS_ALIGNED ( dstl, srcl, len );

  PREFETCH_STOP1;
  PREFETCH_STOP2;

  return;
}
#endif
