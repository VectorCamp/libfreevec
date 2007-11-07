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
        MYSWAB_NIBBLE(dst, src, len);
        return;
    }
    
    uint8_t carry = 0, has_carry = 0;

    // Now we know that we have at least 4 bytes, handle the word-alignment
    // of dstfirst again with a switch
    MYSWAB_FIND_IF_HAS_CARRY(dst, src, len, carry, has_carry);
    
    // Take the word-aligned long pointers of src and dest.
    uint16_t *dst16 = (uint16_t *)(dst);

    if (has_carry) {
        // While we're not 16-byte aligned, move in 4-byte long steps.
        MYSWAB_WORD_UNTIL_ALTIVEC_ALIGNED_HAS_CARRY(dst16, src, srcoffset2, len);
        
        // Now, dst is 16byte aligned. We can use Altivec if len >= 16 bytes
        if (len >= ALTIVECWORD_SIZE) {
            vec_dst(src, DST_CTRL(2,2,16), DST_CHAN_SRC);
            vec_dstst(dst, DST_CTRL(2,2,16), DST_CHAN_DEST);
            // Check for the alignment of src
            if ((uint32_t)(src) % 16 == 0) {
                vector uint8_t vdst_permute_mask = {  0, 1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15 };
                // Now, both buffers are 16-byte aligned, just copy everything directly
                MYSWAB_LOOP_ALTIVEC_ALIGNED_HAS_CARRY(vdst_permute_mask, dst16, src, len, carry);
            } else {
                vector uint8_t vdst_permute_mask = { 0, 2, 1, 4, 3, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13, 15 };
                // src is not 16-byte aligned so we have to a do little trick with Altivec.
                MYSWAB_LOOP_ALTIVEC_UNALIGNED_HAS_CARRY(vdst_permute_mask, dst16, src, len, carry);
            }
            // Stop prefetching.
            VEC_DSS();
        }
    
        // Copy the remaining bytes using word-copying
        // Handle alignment as appropriate
        MYSWAB_REST_WORDS_HAS_CARRY(dst16, src, srcoffset2, len);
        
        // And put the 'carry' byte in its place
        *((uint8_t *)dst16) = carry;
    } else {
        // While we're not 16-byte aligned, move in 4-byte long steps.
        MYSWAB_WORD_UNTIL_ALTIVEC_ALIGNED_NO_CARRY(dst16, src, srcoffset2, len);
        
        vec_dst(src, DST_CTRL(2,2,16), DST_CHAN_SRC);
        vec_dstst(dst, DST_CTRL(2,2,16), DST_CHAN_DEST);
        
        // Now, dst is 16byte aligned. We can use Altivec if len >= 16 bytes
        if (len >= ALTIVECWORD_SIZE) {
            vector uint8_t vdst_permute_mask = {  1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15,14 };
            
            // Check for the alignment of src
            if ((uint32_t)(src) % ALTIVECWORD_SIZE == 0) {
                // Now, both buffers are 16-byte aligned, just copy everything directly
                MYSWAB_LOOP_ALTIVEC_ALIGNED_NO_CARRY(vdst_permute_mask, dst16, src, len);
            } else {
                // src is not 16-byte aligned so we have to a do little trick with Altivec.
                MYSWAB_LOOP_ALTIVEC_UNALIGNED_NO_CARRY(vdst_permute_mask, dst16, src, len);
            }
            // Stop prefetching.
            VEC_DSS();
        }
    
        // Copy the remaining bytes using word-copying
        // Handle alignment as appropriate
        MYSWAB_REST_WORDS_NO_CARRY(dst16, src, srcoffset2, len);
    }
    // We don't have anything left to do.    
    return;
}
#endif
