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
#include "macros/swab.h"

#ifdef VEC_GLIBC
void swab(void *srcpp, const void *dstpp, size_t len) {
#else
void vec_swab(void *srcpp, const void *dstpp, size_t len) {
#endif

  const uint8_t *src = (uint8_t *)srcpp;
  uint8_t *dst = (uint8_t *)dstpp;

  // In odd lenghts, the last byte is useless
  len &= ~((size_t) 1);

  if (len <= sizeof(uint32_t)) {
    SWAB_NIBBLE(dst, src, len);
    return;
  }

  // depending on the alignment we might have to use a carry byte or not
  uint8_t carry = 0, has_carry = 0;

  // Now we know that we have at least 4 bytes, handle the word-alignment
  // of dst again with a switch
  SWAB_FIND_IF_HAS_CARRY(dst, src, len, carry, has_carry);

  // Take the shortword-aligned long pointers of src and dst.
  uint16_t *dst16 = (uint16_t *)(dst);

  if (has_carry) {
    // While we're not 16-byte aligned, move in 4-byte long steps.
    SWAB_WORD_UNTIL_ALTIVEC_ALIGNED_HAS_CARRY(dst16, src, len);

    // Now, dst is 16byte aligned. We can use Altivec if len >= 16 bytes
    if (len >= ALTIVECWORD_SIZE) {
      // Prefetch some stuff
      READ_PREFETCH_START1 ( src );
      WRITE_PREFETCH_START2 ( dst );

      // Check for the alignment of src
      if ((uint32_t)(src) % ALTIVECWORD_SIZE == 0) {
        // Now, both buffers are 16-byte aligned, just copy everything directly
        SWAB_LOOP_ALTIVEC_ALIGNED_HAS_CARRY(dst16, src, len, carry);
      } else {
        // src is not 16-byte aligned so we have to a do little trick with Altivec.
        SWAB_LOOP_ALTIVEC_UNALIGNED_HAS_CARRY(dst16, src, len, carry);
      }
      // Stop prefetching.
      PREFETCH_STOP1;
    }

    // Copy the remaining bytes using word-copying
    // Handle alignment as appropriate
    SWAB_REST_WORDS_HAS_CARRY(dst16, src, len);

    // And put the 'carry' byte in its place
    *((uint8_t *)dst16) = carry;
  } else {
    // While we're not 16-byte aligned, move in 4-byte long steps.
    SWAB_WORD_UNTIL_ALTIVEC_ALIGNED_NO_CARRY(dst16, src, len);

    // Prefetch some stuff
    READ_PREFETCH_START1 ( src );
    WRITE_PREFETCH_START2 ( dst );

    // Now, dst is 16byte aligned. We can use Altivec if len >= 16 bytes
    if (len >= ALTIVECWORD_SIZE) {
      // Check for the alignment of src
      if ((uint32_t)(src) % ALTIVECWORD_SIZE == 0) {
        // Now, both buffers are 16-byte aligned, just copy everything directly
        SWAB_LOOP_ALTIVEC_ALIGNED_NO_CARRY(dst16, src, len);
      } else {
        // src is not 16-byte aligned so we have to a do little trick with Altivec.
        SWAB_LOOP_ALTIVEC_UNALIGNED_NO_CARRY(dst16, src, len);
      }
      // Stop prefetching.
      PREFETCH_STOP1;
      PREFETCH_STOP2;
    }

    // Copy the remaining bytes using word-copying
    // Handle alignment as appropriate
    SWAB_REST_WORDS_NO_CARRY(dst16, src, len);
  }
  // We don't have anything left to do.
  return;
}
#endif
