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
float sinf(float x) {
#else
float vec_sinf(float x) {
#endif

	// First perform range reduction to x in the range [-2*pi..2*pi] and get the absolute,
	// so end range is [0.. 2*pi]
	float x_2pi = reduce_2pi(x);

	float nom, denom, x2, x3, x4, x5, x6;
	x2 = x_2pi * x_2pi;
	x4 = x2 * x2;
	x5 = x_2pi * x4;
    
    //if (x_2pi < M_PI_2) {
        // In the range [0..pi/4] we found that this Pade approximant gives exact results (3.5 * 10^-7)
        nom   = 166320.0 * x_2pi - 22260.0 * x3 + 551.0 * x5;
        denom = 166320.0         + 5460.0  * x2 + 75.0 * x4;
    /*} else {
        //P[6,6]
        x6 = x2 * x4;
        nom   = 183284640.0 * x_2pi - 23819040.0 * x3 + 532182.0 * x5;
        denom = 183284640           + 6728400    * x2 + 126210.0 * x4 + 1331 * x6;
    }*/

	float sinx = nom/denom;

	return sinx;
}
