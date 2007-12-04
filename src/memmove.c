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
#include <stdlib.h>

#ifdef HAVE_ALTIVEC_H
#include <altivec.h>

#include "libfreevec.h"
#include "macros/memcpy.h"
#include "macros/memmove.h"

#ifdef VEC_GLIBC
void *memmove ( void *dstpp, const void *srcpp, size_t len ) {
#else
void *vec_memmove ( void *dstpp, const void *srcpp, size_t len ) {
#endif

  // Copy forward, same as memcpy basically
  if ( abs ( ( uint32_t ) dstpp - ( uint32_t ) srcpp ) >= len ) {
    const uint8_t *src = srcpp;
    uint8_t *dst = dstpp;

    if ( len >= sizeof ( uint32_t ) ) {
      // Prefetch some stuff
      READ_PREFETCH_START ( src );
      WRITE_PREFETCH_START ( dst );

      // Copy until dst is word aligned
      COPY_FWD_UNTIL_DEST_IS_WORD_ALIGNED ( dst, src, len );

      // Now dst is word aligned. We'll continue by word copying, but
      // for this we have to know the word-alignment of src also.
      uint8_t srcoffset4 = ( ( uint32_t ) ( src ) & ( sizeof ( uint32_t )-1 ) );

      // Take the word-aligned long pointers of src and dest.
      uint32_t *dstl = ( uint32_t * ) ( dst );
      const uint32_t *srcl = ( uint32_t * ) ( src -srcoffset4 );

      // While we're not 16-byte aligned, move in 4-byte long steps.
      COPY_FWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED ( dstl, srcl, len, srcoffset4 );

      // Now, dst is 16byte aligned. We can use Altivec if len >= 32
      src = ( uint8_t * ) srcl +srcoffset4;

      if ( len >= QUAD_ALTIVECWORD ) {
        if ( ( ( uint32_t ) ( src ) % ALTIVECWORD_SIZE ) == 0 ) {
          COPY_FWD_LOOP_QUADWORD_ALTIVEC_ALIGNED ( dstl, src, len );
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        } else {
          COPY_FWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED ( dstl, src, len );
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        }
      }
      while ( len >= ALTIVECWORD_SIZE ) {
        if ( ( ( uint32_t ) ( src ) % ALTIVECWORD_SIZE ) == 0 ) {
          COPY_SINGLEQUADWORD_ALTIVEC_ALIGNED ( dstl, src, 0 );
          dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        } else {
          COPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED ( dstl, src, 0 );
          dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        }
      }
      // Copy the remaining bytes using word-copying
      // Handle alignment as appropriate
      COPY_FWD_REST_WORDS ( dstl, srcl, len, srcoffset4 );

      // For the end copy we have to use char * pointers.
      dst = ( uint8_t * ) dstl;
      src = ( uint8_t * ) srcl +srcoffset4;

      // Copy the remaining bytes
      COPY_FWD_NIBBLE ( dst, src, len );

      READ_PREFETCH_STOP;
      WRITE_PREFETCH_STOP;

      return dstpp;
    } else {
      COPY_FWD_NIBBLE ( dst, src, len );
      return dstpp;
    }
  // Copy backwards 
  } else {
    const uint8_t *src = srcpp+len;
    uint8_t *dst = dstpp+len;

    if ( len >= sizeof ( uint32_t ) ) {
      // Copy until dst is word aligned
      COPY_BWD_UNTIL_DEST_IS_WORD_ALIGNED ( dst, src, len );

      // Now dst is word aligned. We'll continue by word copying, but
      // for this we have to know the word-alignment of src also.
      uint8_t srcoffset4 = ( ( uint32_t ) ( src ) & ( sizeof ( uint32_t )-1 ) );

      // Take the word-aligned long pointers of src and dest.
      uint32_t *dstl = ( uint32_t * ) ( dst );
      const uint32_t *srcl = ( uint32_t * ) ( src -srcoffset4 );

      // While we're not 16-byte aligned, move in 4-byte long steps.
      COPY_BWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED ( dstl, srcl, len, srcoffset4 );

      // Now, dst is 16byte aligned. We can use Altivec if len >= 32
      src = ( uint8_t * ) srcl +srcoffset4;

      if ( len >= QUAD_ALTIVECWORD ) {
        if ( ( ( uint32_t ) ( src ) % ALTIVECWORD_SIZE ) == 0 ) {
          COPY_BWD_LOOP_QUADWORD_ALTIVEC_ALIGNED ( dstl, src, len );
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        } else {
          COPY_BWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED ( dstl, src, len );
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        }
      }

      if ( len >= ALTIVECWORD_SIZE ) {
        if ( ( ( uint32_t ) ( src ) % ALTIVECWORD_SIZE ) == 0 ) {
          dstl -= 4; src -= ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;
          COPY_SINGLEQUADWORD_ALTIVEC_ALIGNED ( dstl, src, 0 );
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        } else {
          dstl -= 4; src -= ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;
          COPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED ( dstl, src, 0 );
          srcl = ( uint32_t * ) ( src -srcoffset4 );
        }
      }
      // Copy the remaining bytes using word-copying
      // Handle alignment as appropriate
      COPY_BWD_REST_WORDS ( dstl, srcl, len, srcoffset4 );
      // For the end copy we have to use char * pointers.
      dst = ( uint8_t * ) dstl;
      src = ( uint8_t * ) srcl +srcoffset4;

      // Copy the remaining bytes
      COPY_BWD_NIBBLE ( dst, src, len );

      READ_PREFETCH_STOP;
      WRITE_PREFETCH_STOP;

      return dstpp;
    } else {
      COPY_BWD_NIBBLE ( dst, src, len );
      return dstpp;
    }
  }
}
#endif
