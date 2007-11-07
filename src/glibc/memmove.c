/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis						 *
 *   markos@debian.gr													  *
 *																		 *
 *   This code is distributed under the LGPL license					   *
 *   See http://www.gnu.org/copyleft/lesser.html						   *
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
void *vec_memmove(void *dstpp, const void *srcpp, size_t len) {
#else
void *vec_memmove(void *dstpp, const void *srcpp, size_t len) {
#endif

	if (abs((uint32_t)dstpp - (uint32_t)srcpp) >= len) {
		const uint8_t *src = srcpp;
		uint8_t *dst = dstpp;

		if (len >= sizeof(uint32_t)) {
			// Prefetch some stuff
			vec_dst(src, DST_CTRL(2,1,16), DST_CHAN_SRC);
			vec_dstst(dst, DST_CTRL(2,1,16), DST_CHAN_DEST);

			// Copy until dst is word aligned
			MYCOPY_FWD_UNTIL_DEST_IS_WORD_ALIGNED(dst, src, len);

			// Now dst is word aligned. We'll continue by word copying, but
			// for this we have to know the word-alignment of src also.
			uint8_t srcoffset4 = ((uint32_t)(src) & (sizeof(uint32_t)-1));

			// Take the word-aligned long pointers of src and dest.
			uint32_t *dstl = (uint32_t *)(dst);
			const uint32_t *srcl = (uint32_t *)(src -srcoffset4);

			// While we're not 16-byte aligned, move in 4-byte long steps.
			MYCOPY_FWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dstl, srcl, len, srcoffset4);

			// Now, dst is 16byte aligned. We can use Altivec if len >= 32
			src = (uint8_t *) srcl +srcoffset4;

			if (len >= ALTIVEC_BIGLOOP) {
				if (((uint32_t)(src) & 15) == 0) {
					int blocks = (len >> 6);
					MYCOPY_FWD_LOOP_QUADWORD_ALTIVEC_ALIGNED(dstl, src, blocks);
					srcl = (uint32_t *)(src -srcoffset4);
				} else {
					int blocks = (len >> 6);
					MYCOPY_FWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED(dstl, src, blocks);
					srcl = (uint32_t *)(src -srcoffset4);
				}
			}
			while (len >= ALTIVECWORD_SIZE) {
				if (((uint32_t)(src) & 15) == 0) {
					MYCOPY_SINGLEQUADWORD_ALTIVEC_ALIGNED(dstl, src, 0);
					dstl += 4;
					src += ALTIVECWORD_SIZE;
					len -= ALTIVECWORD_SIZE;
					srcl = (uint32_t *)(src -srcoffset4);
				} else {
					vector uint8_t MSQ, LSQ, mask;
					MYCOPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED(dstl, src, 0);
					dstl += 4;
					src += ALTIVECWORD_SIZE;
					len -= ALTIVECWORD_SIZE;
					srcl = (uint32_t *)(src -srcoffset4);
				}
			}
			// Copy the remaining bytes using word-copying
			// Handle alignment as appropriate
			MYCOPY_FWD_REST_WORDS(dstl, srcl, len, srcoffset4);

			// For the end copy we have to use char * pointers.
			dst = (uint8_t *) dstl;
			src = (uint8_t *) srcl +srcoffset4;

			// Copy the remaining bytes
			MYNIBBLE_COPY_FWD(dst, src, len);

			vec_dss(DST_CHAN_SRC);
			vec_dss(DST_CHAN_DEST);

			return dstpp;
		} else {
			MYNIBBLE_COPY_FWD(dst, src, len);
			return dstpp;
		}
	} else {
		const uint8_t *src = srcpp+len;
		uint8_t *dst = dstpp+len;

		if (len >= sizeof(uint32_t)) {
			// Prefetch some stuff
			vec_dst(src, DST_CTRL(2,1,16), DST_CHAN_SRC);
			vec_dstst(dst, DST_CTRL(2,1,16), DST_CHAN_DEST);
			
			// Copy until dst is word aligned
			MYCOPY_BWD_UNTIL_DEST_IS_WORD_ALIGNED(dst, src, len);
			
			// Now dst is word aligned. We'll continue by word copying, but
			// for this we have to know the word-alignment of src also.
			uint8_t srcoffset4 = ((uint32_t)(src) & (sizeof(uint32_t)-1));

			// Take the word-aligned long pointers of src and dest.
			uint32_t *dstl = (uint32_t *)(dst);
			const uint32_t *srcl = (uint32_t *)(src -srcoffset4);

			// While we're not 16-byte aligned, move in 4-byte long steps.
			MYCOPY_BWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dstl, srcl, len, srcoffset4);

			// Now, dst is 16byte aligned. We can use Altivec if len >= 32
			src = (uint8_t *) srcl +srcoffset4;

			if (len >= 4*ALTIVECWORD_SIZE) {
				if (((uint32_t)(src) & 15) == 0) {
					int blocks = (len >> 6);
					MYCOPY_BWD_LOOP_QUADWORD_ALTIVEC_ALIGNED(dstl, src, blocks);
					srcl = (uint32_t *)(src -srcoffset4);
				} else {
					int blocks = (len >> 6);
					MYCOPY_BWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED(dstl, src, blocks);
					srcl = (uint32_t *)(src -srcoffset4);
				}
			}
			if (len >= 2*ALTIVECWORD_SIZE) {
				if (((uint32_t)(src) & 15) == 0) {
					dstl -= 8;
					src -= 32;
					len -= 32;
					MYCOPY_SINGLEQUADWORD_ALTIVEC_ALIGNED(dstl, src, 16);
					MYCOPY_SINGLEQUADWORD_ALTIVEC_ALIGNED(dstl, src, 0);
					srcl = (uint32_t *)(src -srcoffset4);
				} else {
					vector uint8_t MSQ, LSQ, mask;
					dstl -= 8;
					src -= 32;
					len -= 32;
					MYCOPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED(dstl, src, 16);
					MYCOPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED(dstl, src, 0);
					srcl = (uint32_t *)(src -srcoffset4);
				}
			}
			if (len >= ALTIVECWORD_SIZE) {
				if (((uint32_t)(src) & 15) == 0) {
					dstl -= 4;
					src -= 16;
					len -= 16;
					MYCOPY_SINGLEQUADWORD_ALTIVEC_ALIGNED(dstl, src, 0);
					srcl = (uint32_t *)(src -srcoffset4);
				} else {
					vector uint8_t MSQ, LSQ, mask;
					dstl -= 4;
					src -= 16;
					len -= 16;
					MYCOPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED(dstl, src, 0);
					srcl = (uint32_t *)(src -srcoffset4);
				}
			}
			// Copy the remaining bytes using word-copying
			// Handle alignment as appropriate
			MYCOPY_BWD_REST_WORDS(dstl, srcl, len, srcoffset4);
			// For the end copy we have to use char * pointers.
			dst = (uint8_t *) dstl;
			src = (uint8_t *) srcl +srcoffset4;

			// Copy the remaining bytes
			MYNIBBLE_COPY_BWD(dst, src, len);

			vec_dss(DST_CHAN_SRC);
			vec_dss(DST_CHAN_DEST);

			return dstpp;
		} else {
			MYNIBBLE_COPY_BWD(dst, src, len);
			return dstpp;
		}
	}
}
#endif
