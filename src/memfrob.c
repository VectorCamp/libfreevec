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
#include "macros/memfrob.h"

#ifdef VEC_GLIBC
void *vec_memfrob(void *s, size_t len) {
#else
void *vec_memfrob(void *s, size_t len) {
#endif

    uint8_t *ptr = (uint8_t *)s;
    
    if (len >= sizeof(uint32_t)) {
        // Frobnicate 'offset' bytes, so that ptr is 32-bit aligned.
        uint32_t offset = ((uint32_t)ptr) & (sizeof(uint32_t)-1);
        if (offset)
            offset = sizeof(uint32_t) -offset;
        MYFROBNICATE_NIBBLE(ptr, offset);
        len -= offset;
        
        uint32_t *ptr32 = (uint32_t *)(ptr);
        MYFROBNICATE_UNTIL_ALTIVEC_ALIGNED(ptr32, len);
        
        // build a vector full of fill chars 
        vector uint32_t frobnivector = { FROBNICATOR32, FROBNICATOR32, FROBNICATOR32, FROBNICATOR32 };
            
        MYFROBNICATE_LOOP_ALTIVEC_WORD(ptr32, len);
        
        MYFROBNICATE_REST_WORDS(ptr32, len);
        ptr = (uint8_t *)(ptr32);
        MYFROBNICATE_NIBBLE(ptr, len);
        
        return s;
    } else {
        MYFROBNICATE_NIBBLE(ptr, len);
        return s;
	}
}
#endif
