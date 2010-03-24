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
float coshf(float x) {
#else
float vec_coshf(float x) {
#endif

	float nom, denom, x2, x4, x6;
	x2 = x * x;
	x4 = x2 * x2;
	x6 = x4 * x2;

	// Pade approximant gives good results in the region [0,0.5]
	nom   = -3155040.0 - x2 * 1428000.0 - x4 * 60514.0;
	denom = -3155040.0 + x2 * 149520.0  - x4 * 3814.0  + x6 * 59.0;

	float coshx = nom/denom;

	return coshx;
}
