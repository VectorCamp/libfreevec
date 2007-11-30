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
void *memchr ( void const *str, int c, size_t len ) {
#else
void *vec_memchr ( void const *str, int c, size_t len ) {
#endif

  uint8_t *ptr = ( uint8_t * ) str;
  uint32_t al = ( uint32_t ) ( ptr ) % sizeof ( uint32_t );
  if ( al )
    MEMCHR_UNTIL_WORD_ALIGNED ( ptr, c, len, al );

  uint32_t lw, *ptr32 = ( uint32_t * ) ( ptr );
  uint32_t charmask = charmask32 ( c );

  if ( len >= ALTIVECWORD_SIZE ) {
    al = ( uint32_t ) ptr32 % ALTIVECWORD_SIZE;
    if ( al )
      MEMCHR_LOOP_UNTIL_ALTIVEC_ALIGNED ( ptr32, c, charmask, len, lw, al );
    ptr = ( uint8_t * ) ptr32;

    vec_dst ( ptr, DST_CTRL ( 2,2,16 ), DST_CHAN_SRC );
    union {
      vector uint8_t v;
      uint8_t c[16];
    } vc;
    vc.c[0] = c;
    vc.v = vec_splat ( vc.v, 0 );
    vector uint8_t v1;

    while ( len >= ALTIVECWORD_SIZE ) {
      MEMCHR_SINGLE_ALTIVEC_WORD ( v1, vc.v, ptr32, c, charmask );
      vec_dst ( ptr32, DST_CTRL ( 2,2,16 ), DST_CHAN_SRC );
    }
  }

  MEMCHR_LOOP_WORD ( ptr32, c, charmask, len, lw );

  ptr = ( uint8_t * ) ptr32;
  MEMCHR_REST_BYTES ( ptr, c, len );

  return 0;
}
#endif
