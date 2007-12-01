/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define FROBNICATOR     42
#define FROBNICATOR16   (((FROBNICATOR) << 8) | (FROBNICATOR))
#define FROBNICATOR32   (((FROBNICATOR16) << 16) | (FROBNICATOR16))

#define MYFROBNICATE_NIBBLE(ptr, len)       \
    if (len == 3) {                         \
		*ptr++ ^= FROBNICATOR;          	\
		*ptr++ ^= FROBNICATOR;          	\
		*ptr++ ^= FROBNICATOR;          	\
	} else if (len == 2) {					\
		*ptr++ ^= FROBNICATOR;          	\
		*ptr++ ^= FROBNICATOR;          	\
	} else if (len == 1) {					\
		*ptr++ ^= FROBNICATOR;          	\
    }

#define MYFROBNICATE_UNTIL_ALTIVEC_ALIGNED(ptr32, len)                              \
    while (len >= sizeof(uint32_t) && ((uint32_t)(ptr32) % ALTIVECWORD_SIZE)) {     \
        *ptr32++ ^= FROBNICATOR32;                                                  \
        len -= sizeof(uint32_t);                                                    \
    }
    
#define MYFROBNICATE_SINGLE_ALTIVEC_WORD(ptr32, len)                \
    {                                                               \
        vector uint32_t v = vec_ld(0, (uint32_t *)ptr32);           \
        vec_st(vec_xor(v, frobnivector), 0, (uint32_t *)ptr32);     \
        ptr32 += 4; len -= ALTIVECWORD_SIZE;                        \
    }

#define MYFROBNICATE_LOOP_ALTIVEC_WORD(ptr32, len)                  \
    while (len >= ALTIVECWORD_SIZE) {                               \
        MYFROBNICATE_SINGLE_ALTIVEC_WORD(ptr32, len);               \
    }
    
#define MYFROBNICATE_REST_WORDS(ptr32, len) \
    while (len >= sizeof(uint32_t)) {       \
        *ptr32++ ^= FROBNICATOR32;          \
        len -= sizeof(uint32_t);            \
    }
