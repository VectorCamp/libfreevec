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

#define FUNCTION_NAME     MEMRCHR

#include "libfreevec.h"
#include "macros/memchr.h"
#include "macros/common.h"

#ifdef VEC_GLIBC
void *memrchr ( void const *str, int c_in, size_t len ) {
#else
void *vec_memrchr ( void const *str, int c_in, size_t len ) {
#endif

  uint8_t *ptr = ( uint8_t * ) ( str + len );
  uint8_t __attribute__ ((aligned(16))) c = c_in;
  uint32_t al = ( uint32_t ) ( ptr ) % sizeof ( uint32_t );
  if ( al )
    MEMRCHR_BACKWARDS_UNTIL_WORD_ALIGNED ( ptr, c, len, al );

  uint32_t *ptr32 = ( uint32_t * ) ( ptr );

  if ( len >= ALTIVECWORD_SIZE ) {
    al = ( uint32_t ) ptr32 % ALTIVECWORD_SIZE;
    if ( al )
      MEMRCHR_BACKWARDS_LOOP_UNTIL_ALTIVEC_ALIGNED ( ptr32, c, len, al );

    READ_PREFETCH_START1 ( ptr32 );
    FILL_VECTOR ( vc, c );

    while ( len >= ALTIVECWORD_SIZE ) {
      MEMRCHR_SINGLE_BACKWARDS_ALTIVEC_WORD ( vc, ptr32, c);
      READ_PREFETCH_START1 ( ptr32 );
    }
  }
  MEMRCHR_BACKWARDS_REST_WORDS ( ptr32, c, len );

  ptr = ( uint8_t * ) ptr32;
  MEMRCHR_BACKWARDS_REST_BYTES ( ptr, c, len );

  return 0;
}
#endif
