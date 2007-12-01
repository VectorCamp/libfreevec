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
#include "macros/strlen.h"

#ifdef VEC_GLIBC
size_t strnlen(const char *str, size_t maxlen) {
#else
size_t vec_strnlen(const char *str, size_t maxlen) {
#endif

    uint8_t *ptr = (uint8_t *)str;
    uint32_t *ptr32, len = maxlen;
    
    MYSTRNLEN_UNTIL_WORD_ALIGNED(ptr, len, maxlen);
    
    ptr32 = (uint32_t *)(ptr);
   
    if (len >= ALTIVECWORD_SIZE) {
        MYSTRNLEN_LOOP_UNTIL_ALTIVEC_ALIGNED(ptr32, len, maxlen);
        ptr = (uint8_t *) ptr32;
        
        vec_dst(ptr, DST_CTRL(2,2,16), DST_CHAN_SRC);
        
        vector uint8_t v0 = vec_splat_u8(0);
        vector uint8_t v1;

        while (len >= ALTIVECWORD_SIZE) {
            MYSTRNLEN_SINGLE_ALTIVEC_WORD(v1, v0, ptr32, len, maxlen);
            vec_dst(ptr32, DST_CTRL(2,2,16), DST_CHAN_SRC);
        }
        ptr = (uint8_t *) ptr32;
        vec_dss(DST_CHAN_SRC);
        
        MYSTRNLEN_LOOP_WORD(ptr32, len, maxlen);
        ptr = (uint8_t *) ptr32;
        
        MYSTRNLEN_REST_BYTES(ptr, len, maxlen);
    } else {
        MYSTRNLEN_LOOP_WORD(ptr32, len, maxlen);
        ptr = (uint8_t *) ptr32;
        MYSTRNLEN_REST_BYTES(ptr, len, maxlen);
	}
    
    return maxlen;
}
#endif
