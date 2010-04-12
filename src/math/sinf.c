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
float sinf(float x) {
#else
float vec_sinf(float x) {
#endif

    // First perform range reduction to x in the range [-2*pi..2*pi] and get the absolute,
    // so end range is [0.. 2*pi]
    float x1 = reduce_pi_2f(x);

    float nom, denom;

    //if (x1 < M_PI_4) {
    // In the range [0..pi/4] we found that this Pade approximant gives exact results (3.5 * 10^-7)
    //nom   = 166320.0 * x1 * x1 - 22260.0 * x1 * x1 * x1 + 551.0 * x1 * x1 * x1 * x1 * x1;
    //denom = 166320.0           + 5460.0  * x1 * x1      + 75.0  * x1 * x1 * x1 * x1;
    nom = x1  - 0.133838  * x1 * x1 * x1 + 0.00331289  * x1 * x1 * x1 * x1 * x1;
    denom = 1.0 + 0.0328283 * x1 * x1      + 0.000450938 * x1 * x1 * x1 * x1;
    //} else {
    //P[6,6]
    //nom   = x1  - 0.129957  * x1 * x1 * x1 + 0.00290358  * x1 * x1 * x1 * x1 * x1;
    //denom = 1.0 + 0.0367101 * x1 * x1      + 0.000688601 * x1 * x1 * x1 * x1 + 7.26193E-6 * x1 * x1 * x1 * x1 * x1 * x1;
    //}

    float sinx = nom / denom;

    return sinx;
}
