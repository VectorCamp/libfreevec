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

#define MACROFILE exp.h

#include "common.h"
#include "math_common.h"

#ifdef SIMD_ENGINE
	#define SIMD_MACROS_INC MAKEINC(SIMD_ENGINE)
#else
	#define SIMD_MACROS_INC MAKEINC(scalar)
#endif

#define SIMD_MACROS_EXP_H MAKESTR(SIMD_MACROS_INC)
#include SIMD_MACROS_EXP_H

#ifdef TEST_LIBC
float expf(float x) {
#else
float vec_expf(float x) {
#endif

	if (x > 127.0)
		return HUGE_VAL;
	if (x < -126.0)
		return 0.0;

	float expx0 = 1.0;
	float n  = floorf(x*M_1_LN2);
	float x0 = n * M_LN2;
	float b = x - x0;

	//printf("x = n *ln2 + b -> x = %f, n = %f, x0 = %f, b = %f\n", x, n, x0, b);
	if (n != 0.0)
		expx0 = powerof2((uint32_t) n);
	//printf("exp(x0) = %f\n", expx0);

	float nom, denom, x2, x3, x4;
	x2 = b * b;
	x3 = x2 * x;
	x4 = x2 * x2;

	float y, y2, y3, y4;
	y  = b * 0.5;
	y2 = x2 * 0.1071428571428571429;
	y3 = x3 * 0.01190476190476190476;
	y4 = x4 * 0.0005952380952380952381;

	// In the range [0..ln2] we found that this Pade approximant gives exact results (maxerror ~1.2 * 10^-7):
	// 
	// exp(x) ~ (1680 +840x +180x^2 +20x^3 + x^4)/(1680 -840x +180x^2 -20x^3 + x^4)
	// In fact, this proves to be accurate with a larger error, ~4 *10^(-7).
	// and that the normalized version is more accurate, with a maxerror 2 * 10^(-7).

	nom   = 1.0 + y + y2 + y3 + y4;
	denom = 1.0 - y + y2 - y3 + y4;

	float expx = nom/denom;

	//printf("exp(b) = %f\n", expx);

	expx *= expx0;

	//printf("exp(x) = %f\n", expx);
	return expx;
}
