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
float tanhf(float x) {
#else
float vec_tanhf(float x) {
#endif

	float nom, denom, x2, x3, x4, x5;
	x2 = x * x;
	x3 = x * x2;
	x4 = x2 * x2;
	x5 = x3 * x2;

	nom   = x * 945.0 + x3 * 105.0 + x5;
	denom =     945.0 + x2 * 420.0 + x4 * 15.0;

	float tanhx = nom/denom;

	return tanhx;
}
