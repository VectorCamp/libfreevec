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

#include <memory.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#ifdef HAVE_ALTIVEC_H
#include <altivec.h>

#include "libfreevec.h"
#include "macros/memset.h"
#include "macros/common.h"

#ifdef VEC_GLIBC
void *memset ( void *s, int p, size_t len ) {
#else
void *vec_memset ( void *s, int p, size_t len ) {
#endif

  uint8_t* ptr = s;
  uint32_t p32 = charmask32(p);
    
  uint32_t al = ( uint32_t ) ( ptr ) % sizeof ( uint32_t );
  if ( al )
	  MEMSET_UNTIL_WORD_ALIGNED ( ptr, p, len, al );
  
  uint32_t *ptr32 = (uint32_t *)(ptr);

  if ( len >= ALTIVECWORD_SIZE ) {
	  FILL_VECTOR ( p128, p );
  
	  // ptr is now 32-bit aligned, memset until ptr is altivec aligned
	  al = ( uint32_t ) ptr32 % ALTIVECWORD_SIZE;
	  if ( al )
	    MEMSET_WORD_UNTIL_ALTIVEC_ALIGNED ( ptr32, p32, len );
  
	  // ptr is now 128-bit aligned
	  // Set 64-byte chunks at a time
	  if ( len >= QUAD_ALTIVECWORD ) {
	    MEMSET_LOOP_QUADWORD ( ptr32, p128, len );
	  }
  
	  // memset the remaining 16-byte chunks
	  while (len >= ALTIVECWORD_SIZE) {
	    MEMSET_ALTIVECWORD(ptr32, p128, len);
	  }
  }

  // memset the remaining words
  MEMSET_REST_WORDS ( ptr32, p32, len );
  ptr =  (uint8_t *)ptr32;

  // Handle the remaining bytes
  MEMSET_NIBBLE ( ptr, p, len, len );
  return s;
}
#endif
