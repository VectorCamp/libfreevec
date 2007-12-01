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
#include "macros/strcmp.h"

#ifdef VEC_GLIBC
int vec_strcmp(const uint8_t *src1pp, const uint8_t *src2pp) {
#else
int vec_strcmp(const uint8_t *src1pp, const uint8_t *src2pp) {
#endif

    const uint8_t *src2 = (uint8_t *)src2pp;
    const uint8_t *src1 = (uint8_t *)src1pp;

    vec_dst(src2, DST_CTRL(2,1,16), DST_CHAN_SRC);
    vec_dst(src1, DST_CTRL(2,1,16), DST_CHAN_DEST);

    MYSTRCMP_UNTIL_SRC1_WORD_ALIGNED(src1, src2);
    
    // Take the word-aligned long pointers of src2 and dest.
    uint8_t src2offset4 = ((uint32_t)(src2) & (sizeof(uint32_t)-1));
    uint32_t *src1l = (uint32_t *)(src1);
    const uint32_t *src2l = (uint32_t *)(src2 -src2offset4);
    
    MYSTRCMP_UNTIL_SRC1_IS_ALTIVEC_ALIGNED(src1, src1l, src2, src2l, src2offset4);

    // Now src1 is word aligned. If possible (ie if there are enough bytes left)
    // we want to align it to 16-byte boundaries as well.
    // For this we have to know the word-alignment of src2 also.

    src2 = (uint8_t *) src2l +src2offset4;
    
    if (((uint32_t)(src2) % ALTIVECWORD_SIZE) == 0) {
        // Now, both buffers are 16-byte aligned, just copy everything directly
        MYSTRCMP_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(src1, src1l, src2, src2l, src2offset4);
        src2l = (uint32_t *)(src2 -src2offset4);
    } else {
        // src2 is not 16-byte aligned so we have to a little trick with Altivec.
        MYSTRCMP_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(src1, src1l, src2, src2l, src2offset4);
        src2l = (uint32_t *)(src2 -src2offset4);
    }    
    
    return 0;
}
#endif
