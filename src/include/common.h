/***************************************************************************
 *   Copyright (C) 2009 by Konstantinos Margaritis   *
 *   markos@codex.gr   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef LIBFREEVEC_COMMON_H
#define LIBFREEVEC_COMMON_H

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

#ifdef LIBFREEVEC_SSE
#include <emmintrin.h>
#include <xmmintrin.h>
#include "arch/sse.h"
#endif

#ifdef LIBFREEVEC_NEON
#include <arm_neon.h>
#include "arch/neon.h"
#endif

#ifdef LIBFREEVEC_ALTIVED
#include <altivec.h>
#include "arch/altivec.h"
#endif

#define himagic         0x80808080L
#define lomagic         0x01010101L
#define magic_bits32    0x07efefeff
#define magic_bits64    (((unsigned long int) 0x7efefefe << 32) | 0xfefefeff)

#define charmask8(c)    ((uint8_t)(c & 0xff))
#define charmask16(c)   (uint16_t)((charmask8(c)) | (charmask8(c) << 8))
#define charmask32(c)   (uint32_t)((charmask16(c)) | (charmask16(c) << 16))
#define charmask64(c)   (uint64_t)((charmask32(c)) | (charmask32(c) << 32))

#define QMAKESTR(x) #x
#define MAKESTR(x) QMAKESTR(x)
#define SMASH(x,y) x/y
#define MAKEINC(x) SMASH(x,MACROFILE)

#ifdef LINUX64
#define word_t  uint64_t
#define LIBFREEVEC_SCALAR_MACROS_INC MAKEINC(scalar64)
#define charmask(c) charmask64(c)
#else
#define word_t  uint32_t
#define LIBFREEVEC_SCALAR_MACROS_INC MAKEINC(scalar32)
#define charmask(c) charmask32(c)
#endif

#ifdef LIBFREEVEC_SIMD_ENGINE
#define LIBFREEVEC_SIMD_MACROS_INC MAKEINC(LIBFREEVEC_SIMD_ENGINE)
#else
#define LIBFREEVEC_SIMD_MACROS_INC LIBFREEVEC_SCALAR_MACROS_INC
#endif

#define ptrdiff_t(a, b)     ((word_t)(a)-(word_t)(b))

#define CMP_LT_OR_GT(a, b) ((a) - (b))

#define MIN(a,b)    ((a) <= (b) ? (a) : (b))

#define DIFF(a, b) ((a)-(b))



#if __BYTE_ORDER == __LITTLE_ENDIAN
#define MERGE_SHIFTED_WORDS(a, b, sl, sr)      ((a) >> sl) | ((b) << sr)
#else
#define MERGE_SHIFTED_WORDS(a, b, sl, sr)      ((a) << sl) | ((b) >> sr)
#endif

#ifdef DEBUG
#define debug printf
#else
#define debug
#endif

#endif // LIBFREEVEC_COMMON_H
