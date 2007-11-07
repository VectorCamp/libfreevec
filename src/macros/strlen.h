/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"
#include "macros/common.h"

#define MYSTRLEN_UNTIL_WORD_ALIGNED(str, ptr)               \
	{														\
		int ptral = (uint32_t)(ptr) % (sizeof(uint32_t));	\
    	if (ptral == 3) {         							\
            if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
            ptr++;											\
        } else if (ptral == 2) {							\
            if (*ptr == '\0') return ptrdiff_t(ptr,str);  	\
            ptr++;											\
            if (*ptr == '\0') return ptrdiff_t(ptr,str);  	\
            ptr++;											\
        } else if (ptral == 1) {							\
            if (*ptr == '\0') return ptrdiff_t(ptr,str); 	\
            ptr++;											\
            if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
            ptr++;											\
            if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
            ptr++;											\
		}													\
    }

#define MYSTRLEN_SINGLE_WORD_old(str, ptr32)        							\
{																				\
	uint32_t pos, lw = ((*ptr32 ^ 0) - lomagic) & ~(*ptr32) & himagic;			\
    if (lw) {																	\
		pos = find_leftfirst_in_word(lw);										\
		return ptrdiff_t((uint8_t *)(ptr32)+ pos, str);							\
	}																			\
    ptr32++;																	\
}

#define MYSTRLEN_SINGLE_WORD(str, ptr32)                     \
    if (( *ptr32 - lomagic) & himagic) {                     \
        uint8_t *cptr = (uint8_t *) ptr32;                   \
        if (cptr[0] == '\0' ) return ptrdiff_t(ptr32,str);     \
        if (cptr[1] == '\0' ) return ptrdiff_t(ptr32,str) +1;  \
        if (cptr[2] == '\0' ) return ptrdiff_t(ptr32,str) +2;  \
        if (cptr[3] == '\0' ) return ptrdiff_t(ptr32,str) +3;  \
    }                                                        \
    ptr32++;

#define MYSTRLEN_LOOP_UNTIL_ALTIVEC_ALIGNED(str, ptr32)     \
	{														\
		int ptr32al = (uint32_t)(ptr32) % ALTIVECWORD_SIZE;	\
    	if (ptr32al == 4) {									\
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
        } else if (ptr32al == 8) {                          \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
        } else if (ptr32al == 12) {							\
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
		}													\
    }

#define MYSTRLEN_SINGLE_ALTIVEC_WORD(vmask, str, ptr32)     \
    {                                                       \
        vector uint8_t vec = vec_ld(0, (uint8_t *)ptr32);   \
        if (vec_any_eq(vec, vmask)) {                       \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
            MYSTRLEN_SINGLE_WORD(str, ptr32);               \
        }                                                   \
    }

#define MYSTRLEN_LOOP_ALTIVEC_WORD(vmask, str, ptr32)       \
    {                                                       \
        vector uint8_t v1;                                  \
        int res1;                                           \
        while (1) {                                         \
            v1 = vec_ld(0, (uint8_t *)ptr32);               \
            res1 = vec_any_eq(v1, vmask);                   \
            if (res1) {                                     \
                MYSTRLEN_SINGLE_WORD(str, ptr32);           \
                MYSTRLEN_SINGLE_WORD(str, ptr32);           \
                MYSTRLEN_SINGLE_WORD(str, ptr32);           \
                MYSTRLEN_SINGLE_WORD(str, ptr32);           \
            }                                               \
            ptr32 += 4;                                     \
            vec_dst(ptr32, DST_CTRL(2,2,16), DST_CHAN_SRC); \
        }                                                   \
    }

#define MYSTRNLEN_UNTIL_WORD_ALIGNED(ptr, len, maxlen)          \
    while (((uint32_t)(ptr) & (sizeof(uint32_t)-1)) && len--) { \
        if (*ptr == '\0') return ptrdiff_t(ptr,str);           	\
       	ptr++;													\
    }

#define MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen)				\
    if (( *ptr32 - lomagic) & himagic) {						\
        uint8_t *cptr = (uint8_t *) ptr32;						\
        if (cptr[0] == '\0' ) return ptrdiff_t(ptr32,str);		\
        if (cptr[1] == '\0' ) return ptrdiff_t(ptr32,str) +1;	\
        if (cptr[2] == '\0' ) return ptrdiff_t(ptr32,str) +2;	\
        if (cptr[3] == '\0' ) return ptrdiff_t(ptr32,str) +3;	\
    }                                               \
    ptr32++;

#define MYSTRNLEN_LOOP_WORD(ptr32, len, maxlen)      \
    while (len >= sizeof(uint32_t)) {                \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);   \
        len -= sizeof(uint32_t);                     \
    }

#define MYSTRNLEN_LOOP_UNTIL_ALTIVEC_ALIGNED(ptr32, len, maxlen)    \
	{																\
		int ptr32al = (uint32_t)(ptr32) % ALTIVECWORD_SIZE;			\
        if (ptr32al == 4) {											\
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            len -= 3*sizeof(uint32_t);                              \
        } else if (ptr32al == 8) {                          		\
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            len -= 2*sizeof(uint32_t);                              \
        } else if (ptr32al == 12) {                          		\
            MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);              \
            len -= sizeof(uint32_t);                                \
		}															\
    }

#define MYSTRNLEN_SINGLE_ALTIVEC_WORD(vec, vmask, ptr32, len, maxlen)   \
    vec = vec_ld(0, (uint8_t *)ptr32);                                  \
    if (vec_any_eq(vec, vmask)) {                                       \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
        MYSTRNLEN_SINGLE_WORD(ptr32, len, maxlen);                      \
    }                                                                   \
    ptr32 += 4;                                                         \
    len -= 16;
        
#define MYSTRNLEN_REST_BYTES(ptr, len, maxlen)			\
	if (len == 3) {         							\
		if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
		ptr++;											\
		if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
		ptr++;											\
		if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
	} else if (len == 2) {								\
		if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
		ptr++;											\
		if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
	} else if (len == 1) {								\
		if (*ptr == '\0') return ptrdiff_t(ptr,str);	\
    }
