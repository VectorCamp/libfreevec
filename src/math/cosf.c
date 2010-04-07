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
	float x_2pi = reduce_pi_2(x);

	float nom, denom, x2, x3, x4, x6;
	x2 = x_2pi * x_2pi;
    //x3 = x2 * x_2pi;
	x4 = x2 * x2;

	// In the range [0..pi/4] we found that this Pade approximant gives very good results (3.5 * 10^-7)
	//nom   = 1.0 - x2 * 0.4563492063492063492  + x4 * 0.02070101020105820106;
	//denom = 1.0 + x2 * 0.04365079365079365079 + x4 * 0.0008597883597883597885;
    if (x_2pi < M_PI_4) {
        // Best accuracy so far P[4,4] unnormalized
        nom   = 15120.0 - x2 * 6900.0 + x4 * 313.0;
        denom = 15120.0 + x2 * 660.0  + x4 * 13.0;
    } else {
        x6 = x4 * x2;
        //Pade[6,6]
        nom   = 39251520.0 - 18471600.0 * x2 + 1075032.0 * x4 - 14615.0 * x6;
        denom = 39251520.0 + 1154160.0  * x2 + 16632.0   * x4 + 127.0 * x6;
    }
    //Pade[6,4];
    //nom   = 131040.0 - 62160.0 *x2 + 3814.0 * x4 - 59.0 * x6;
    //denom = 131040.0 + 3360.0 * x2 + 34.0 * x4;
    //Pade[6,6]
    //nom   = 39251520.0 - 18471600.0 * x2 + 1075032.0 * x4 - 14615.0 * x6;
    //denom = 39251520.0 + 1154160.0  * x2 + 16632.0   * x4 + 127.0 * x6;
    //Pade[4,4, pi/4]
    //nom   = 1.0 - 0.0256810437336 * x - 0.449095393189 * x2 + 0.0110106745486 * x3 + 0.0173724419677  * x4;
    //denom = 1.0 - 0.0256849972702 * x + 0.050924186230 * x2 -0.00188881149161 * x3 + 0.00127255304328 * x4;

	float cosx = nom/denom;
    
	return cosx;
}
