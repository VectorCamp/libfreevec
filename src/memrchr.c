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
#include "macros/memchr.h"
#include "macros/common.h"

#ifdef VEC_GLIBC
void *memrchr(void const *str, int c, size_t len) {
#else
void *vec_memrchr(void const *str, int c, size_t len) {
#endif
    
    uint8_t *ptr = (uint8_t *)(str + len);
    uint32_t *ptr32, lw;
    uint32_t charmask = charmask32(c);

    MYMEMRCHR_BACKWARDS_UNTIL_WORD_ALIGNED(ptr, c, len);
    
    ptr32 = (uint32_t *)(ptr);
    
    if (len >= ALTIVECWORD_SIZE) {
        MYMEMRCHR_BACKWARDS_LOOP_UNTIL_ALTIVEC_ALIGNED(ptr32, c, charmask, len);
        vec_dst(ptr-32, DST_CTRL(2,2,16), DST_CHAN_SRC);
        union {
            vector uint8_t v; 
            uint8_t c[16]; 
        } vc;
        vc.c[0] = c;
        vc.v = vec_splat(vc.v, 0);
        vector uint8_t v1;

        while (len >= ALTIVECWORD_SIZE) {
            MYMEMRCHR_SINGLE_BACKWARDS_ALTIVEC_WORD(v1, vc.v, ptr32, c, charmask);
            vec_dst(ptr32-32, DST_CTRL(2,2,16), DST_CHAN_SRC);
        }
        ptr = (uint8_t *) ptr32;
        
        MYMEMRCHR_BACKWARDS_LOOP_WORD(ptr32, c, charmask, len);
        ptr = (uint8_t *) ptr32;
        MYMEMRCHR_BACKWARDS_REST_BYTES(ptr, c, len);
    } else {
        MYMEMRCHR_BACKWARDS_LOOP_WORD(ptr32, c, charmask, len);
        ptr = (uint8_t *) ptr32;
        MYMEMRCHR_BACKWARDS_REST_BYTES(ptr, c, len);
	}
           
    return 0;
}
#endif
