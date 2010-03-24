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

#ifdef SIMD_ENGINE
	#define SIMD_MACROS_INC MAKEINC(SIMD_ENGINE)
#else
	#define SIMD_MACROS_INC MAKEINC(scalar)
#endif

#define SIMD_MACROS_TRIG_H MAKESTR(SIMD_MACROS_INC)
#include SIMD_MACROS_TRIG_H

#ifdef TEST_LIBC
float sinf(float x) {
#else
float vec_sinf(float x) {
#endif

	// First perform range reduction to x in the range [-2*pi..2*pi] and get the absolute,
	// so end range is [0.. 2*pi]
	float x_2pi = reduce_2pi(x);

	float nom, denom, x2, x3, x4, x5;
	x2 = x_2pi * x_2pi;
	x4 = x2 * x2;
	x5 = x_2pi * x4;

	// In the range [0..pi/4] we found that this Pade approximant gives exact results (3.5 * 10^-7)
	nom   = x_2pi * 166320.0 - x3 * 22260.0 + x5 * 551.0;
  	denom =         166320.0 + x2 * 5460.0  + x4 * 75.0;

	float sinx = nom/denom;

	return sinx;
}
