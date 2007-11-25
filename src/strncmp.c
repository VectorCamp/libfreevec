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
#include "macros/strcmp.h"

#ifdef VEC_GLIBC
int vec_strncmp(const uint8_t *src1pp, const uint8_t *src2pp, size_t len) {
#else
int vec_strncmp(const uint8_t *src1pp, const uint8_t *src2pp, size_t len) {
#endif

    uint8_t *src2 = (uint8_t *)src2pp;
    uint8_t *src1 = (uint8_t *)src1pp;
    
    if (len >= sizeof(uint32_t)) {
        vec_dst(src2, DST_CTRL(2,1,16), DST_CHAN_SRC);
        vec_dst(src1, DST_CTRL(2,1,16), DST_CHAN_DEST);
    
        // MYSTRNCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2, len);

        //MYSTRNCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, len, src2offset4);

		MYSTRNCMP_UNTIL_SRC1_ALTIVEC_ALIGNED_new(src1, src2, len);

		// Take the word-aligned long pointers of src2 and dest.
        uint8_t src2offset4 = ((uint32_t)(src2) & (sizeof(uint32_t)-1));
        uint32_t *src1l = (uint32_t *)(src1);
        const uint32_t *src2l = (uint32_t *)(src2 -src2offset4);

        // Now src1 is word aligned. If possible (ie if there are enough bytes left)
        // we want to align it to 16-byte boundaries as well.
        // For this we have to know the word-alignment of src2 also.

        src2 = (uint8_t *) src2l +src2offset4;

        if (len >= ALTIVECWORD_SIZE) {
            // Check for the alignment of src2
            if (((uint32_t)(src2) % ALTIVECWORD_SIZE) == 0) {
                // Now, both buffers are 16-byte aligned, just copy everything directly
                MYSTRNCMP_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l, src2offset4);
                src2l = (uint32_t *)(src2 -src2offset4);
            } else {
                // src2 is not 16-byte aligned so we have to a little trick with Altivec.
                MYSTRNCMP_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2offset4);
                src2l = (uint32_t *)(src2 -src2offset4);
            }
        }
        
        MYSTRNCMP_REST_WORDS(src1, src1l, src2, src2l, len, src2offset4);
        src1 = (uint8_t *) src1l;
        src2 = (uint8_t *) src2l +src2offset4;
        
        READ_PREFETCH_STOP;
        MYNIBBLE_STRNCMP(src1, src2, len);
        
        return 0;
    } else {
        MYNIBBLE_STRNCMP(src1, src2, len);
        return 0;
	}
}
#endif
