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
float tanf(float x) {
#else
float vec_tanf(float x) {
#endif

	// First perform range reduction to x in the range [-2*pi..2*pi] and get the absolute,
	// so end range is [0.. 2*pi]
	float x_pi_2 = reduce_2pi(x);

	float nom, denom, x2, x3, x4;
	x2 = x_pi_2 * x_pi_2;
	x3 = x * x2;
	x4 = x2 * x2;

	// In the range [0..pi/4] we found that this Pade approximant gives exact results (3.5 * 10^-7)
	// we don't use the following but a normalized version of it:
	nom   = 0.999999986 * x - 0.0958010197 * x3;
	denom = 1.0 - 0.429135022 * x2 + 0.00971659383 * x4;

	float tanx = nom/denom;

	return tanx;
}
