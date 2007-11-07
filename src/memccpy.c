/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
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
void *memccpy(void *dstpp, const void *srcpp, int c, size_t len) {
#else
void *vec_memccpy(void *dstpp, const void *srcpp, int c, size_t len) {
#endif

    const uint8_t *src = (uint8_t *)srcpp;
    uint8_t *dst = (uint8_t *)dstpp;
    
    if (len >= sizeof(uint32_t)) {
        uint32_t charmask = charmask32(c);
        uint8_t srcoffset4 = ((uint32_t)(src) & (sizeof(uint32_t)-1));
        
        vec_dst(src, DST_CTRL(2,1,16), DST_CHAN_SRC);
        vec_dstst(dst, DST_CTRL(2,1,16), DST_CHAN_DEST);
    
        MYMEMCCPY_UNTIL_DEST_WORD_ALIGNED(dst, src, c, len);

        // Take the word-aligned long pointers of src and dest.
        uint32_t *dstl = (uint32_t *)(dst);
        const uint32_t *srcl = (uint32_t *)(src -srcoffset4);
        
        MYMEMCCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, len, srcoffset4, charmask, c);
		
        // Now dst is word aligned. If possible (ie if there are enough bytes left)
        // we want to align it to 16-byte boundaries as well.
        // For this we have to know the word-alignment of src also.

        src = (uint8_t *) srcl +srcoffset4;

        MYFILL_VECTOR(vc, c);

        // Check for the alignment of src
        if (((uint32_t)(src) % ALTIVECWORD_SIZE) == 0) {
            // Now, both buffers are 16-byte aligned, just copy everything directly
            MYMEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, srcoffset4, charmask, vc, c);
            srcl = (uint32_t *)(src -srcoffset4);
        } else {
            // src is not 16-byte aligned so we have to a little trick with Altivec.
            MYMEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcoffset4, charmask, vc, c);
            srcl = (uint32_t *)(src -srcoffset4);
        }

        MYMEMCCPY_REST_WORDS(dst, dstl, src, srcl, len, srcoffset4, charmask, c);
        dst = (uint8_t *) dstl;
        src = (uint8_t *) srcl +srcoffset4;

        MYNIBBLE_MEMCCPY_FWD(dst, src, c, len);

        vec_dss(DST_CHAN_SRC);
        vec_dss(DST_CHAN_DEST);
        
        return 0;
    } else {
	    MYNIBBLE_MEMCCPY_FWD(dst, src, c, len);
        return 0;
    }

}
#endif
