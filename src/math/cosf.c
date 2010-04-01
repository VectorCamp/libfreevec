/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#define MACROFILE trig.h

#include "common.h"
#include "math_common.h"

#ifdef LIBFREEVEC_SIMD_ENGINE
#define LIBFREEVEC_SIMD_MACROS_INC MAKEINC(LIBFREEVEC_SIMD_ENGINE)
#else
#define LIBFREEVEC_SIMD_MACROS_INC MAKEINC(scalar)
#endif

#define LIBFREEVEC_SIMD_MACROS_TRIG_H MAKESTR(LIBFREEVEC_SIMD_MACROS_INC)
#include LIBFREEVEC_SIMD_MACROS_TRIG_H
#ifdef SIMD_ENGINE
	#define SIMD_MACROS_INC MAKEINC(SIMD_ENGINE)
#else
	#define SIMD_MACROS_INC MAKEINC(scalar)
#endif

#ifdef LIBFREEVEC_BUILD_AS_LIBC
float cosf(float x) {
#else
float vec_cosf(float x) {
#endif

	// First perform range reduction to x in the range [-2*pi..2*pi] and get the absolute,
	// so end range is [0.. 2*pi]
	float x_2pi = reduce_2pi(x);

	float nom, denom, x2, x4;
	x2 = x_2pi * x_2pi;
	x4 = x2 * x2;

	// In the range [0..pi/4] we found that this Pade approximant gives exact results (3.5 * 10^-7)
	nom   = 1.0 - x2 * 0.4563492063492063492  + x4 * 0.02070101020105820106;
	denom = 1.0 + x2 * 0.04365079365079365079 + x4 * 0.0008597883597883597885;

	float cosx = nom/denom;

	return cosx;
}
