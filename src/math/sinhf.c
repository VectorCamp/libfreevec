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
float sinhf(float x) {
#else
float vec_sinhf(float x) {
#endif
	float nom, denom, x2, x3, x4, x5;
	x2 = x * x;
	x3 = x2 * x;
	x4 = x2 * x2;
	x5 = x4 * x;

	// Pade approximant gives good results in the region [0,0.5]
	nom   = x * 166320.0 + x3 * 22260.0 + x5 * 551.0;
	denom =     166320.0 - x2 * 5460.0  + x4 * 75.0;

	float coshx = nom/denom;

	return coshx;
}
