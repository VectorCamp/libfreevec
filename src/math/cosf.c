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

#ifdef LIBFREEVEC_BUILD_AS_LIBM
float cosf(float x) {
#else
float vec_cosf(float x) {
#endif

	// First perform range reduction to x in the range [-2*pi..2*pi] and get the absolute,
	// so end range is [0.. 2*pi]
	float x1 = reduce_pi_2f(x);

	float nom, denom;

	// In the range [0..pi/4] we found that this Pade approximant gives very good results (3.5 * 10^-7)
    if (x1 < M_PI_4) {
        // Best accuracy so far P[4,4] unnormalized
        nom   = 15120.0 - x1 * x1 * 6900.0 + x1 * x1 * x1 * x1 * 313.0;
        denom = 15120.0 + x1 * x1 * 660.0  + x1 * x1 * x1 * x1 * 13.0;
    } else {
        //Pade[6,6]
        nom   = 39251520.0 - 18471600.0 * x1 * x1 + 1075032.0 * x1 * x1 * x1 * x1 - 14615.0 * x1 * x1 * x1 * x1 * x1 * x1;
        denom = 39251520.0 + 1154160.0  * x1 * x1 + 16632.0   * x1 * x1 * x1 * x1 + 127.0   * x1 * x1 * x1 * x1 * x1 * x1;
    }

	float cosx = nom/denom;
    
	return cosx;
}
