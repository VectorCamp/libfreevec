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
#include "macros/strlen.h"

#ifdef VEC_GLIBC
size_t strlen(const int8_t *str) {
#else
size_t vec_strlen(const int8_t *str) {
#endif

    int8_t *ptr = (int8_t *)str;
    
    MYSTRLEN_UNTIL_WORD_ALIGNED(str, ptr);
    
    uint32_t *ptr32 = (uint32_t *)(ptr);
    MYSTRLEN_LOOP_UNTIL_ALTIVEC_ALIGNED(str, ptr32);
        
    vec_dst(ptr, DST_CTRL(2,2,32), DST_CHAN_SRC);
        
    vector uint8_t v0 = vec_splat_u8(0);
    MYSTRLEN_LOOP_ALTIVEC_WORD(v0, str, ptr32);
    
    return ptrdiff_t(ptr,str);
}
#endif
