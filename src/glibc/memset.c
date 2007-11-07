/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under a BSD-type license                     *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <memory.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#ifdef HAVE_ALTIVEC_H 
#include <altivec.h>

#include "libfreevec.h"
#include "macros/memset.h"

#ifdef VEC_GLIBC
void *memset(void *s, int p, size_t len) {
#else
void *vec_memset(void *s, int p, size_t len) {
#endif

    uint8_t* ptr = s;
    
    // Handle sizes < 4 bytes
    if (len < sizeof(uint32_t)) {
        MYSET_NIBBLE(ptr, p, len, len);
        return s;
    } else if (len < ALTIVECWORD_SIZE) {
        // Set 'offset' bytes, so that ptr is 32-bit aligned.
        uint32_t offset = ((uint32_t)ptr) % sizeof(uint32_t);
        MYSET_NIBBLE(ptr, p, len, offset);
        
        // ptr is now 32-bit aligned, proceed with seting words.
        int loops = len >> 2; //  divide by sizeof(uint32_t)
        if (loops) {
            MYSET_WORDS(ptr, p, len, loops);
        }
        
        // Handle the remaining bytes
        MYSET_NIBBLE(ptr, p, len, len);
        return s;
    } else if (len < 2*ALTIVECWORD_SIZE) {
        MYFILL_VECTOR(p128, p);
        
        // Set 'offset' bytes, so that ptr is 32-bit aligned.
        int offset = ((uint32_t)ptr) % sizeof(uint32_t);
        if (offset) {
            offset = sizeof(uint32_t) -offset;
            MYSET_NIBBLE(ptr, p, len, offset);
        }
        // ptr is now 32-bit aligned, set words until ptr is 128-bit aligned
        offset = ((uint32_t)(ptr) & 15);
        int loops;
        if (offset) {
            loops = (16-offset) >> 2; //  divide by sizeof(uint32_t)
            MYSET_WORDS(ptr, p, len, loops);
        }
        offset = ((uint32_t)(ptr) & 15);
        
        // ptr is now 128-bit aligned, proceed with seting words.
        if (len >= ALTIVECWORD_SIZE) {
            MYSET_ALTIVECWORD(ptr, p, len);
        }
        
        // We have no vectors left, but we know that ptr is 32-bit aligned, 
        // again proceed with seting words.
        loops = len >> 2; //  divide by sizeof(uint32_t)
        if (loops) {
            MYSET_WORDS(ptr, p, len, loops);
        }
        
        // Handle the remaining bytes
        MYSET_NIBBLE(ptr, p, len, len);
        
        return s;
    } else {
        MYFILL_VECTOR(p128, p);
        
        // Set 'offset' bytes, so that ptr is 32-bit aligned.
        int offset = ((uint32_t)ptr) % sizeof(uint32_t);
        if (offset) {
            offset = sizeof(uint32_t) -offset;
            MYSET_NIBBLE(ptr, p, len, offset);
        }

        // ptr is now 32-bit aligned, set words until ptr is 128-bit aligned
        offset = ((uint32_t)(ptr) & 15);
        int loops;

        if (offset) {
            loops = (16-offset) >> 2; //  divide by sizeof(uint32_t)
            MYSET_WORDS(ptr, p, len, loops);
        }
        offset = ((uint32_t)(ptr) & 15);
        
        // ptr is now 128-bit aligned, proceed with seting words.
        loops = len >> 5; //  divide by 32 (2*size of Altivec registers)
        if (loops) {
            MYSET_LOOP_ALTIVECWORD(ptr, p, len, loops);
        }
        
        // ptr is now 32-bit aligned, proceed with seting words.
        loops = len >> 2; //  divide by sizeof(uint32_t)
        if (loops) {
            MYSET_WORDS(ptr, p, len, loops);
        }
        // Handle the remaining bytes
        MYSET_NIBBLE(ptr, p, len, len);
        
        return s;
    }
}
#endif
