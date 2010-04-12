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

#ifdef LIBFREEVEC_SIMD_ENGINE
#define LIBFREEVEC_SIMD_MACROS_INC MAKEINC(LIBFREEVEC_SIMD_ENGINE)
#else
#define LIBFREEVEC_SIMD_MACROS_INC MAKEINC(scalar)
#endif

#define LIBFREEVEC_SIMD_MACROS_EXP_H MAKESTR(LIBFREEVEC_SIMD_MACROS_INC)
#include LIBFREEVEC_SIMD_MACROS_EXP_H

#ifdef LIBFREEVEC_BUILD_AS_LIBM
float expf(float x) {
#else
float vec_expf(float x) {
#endif

    if (x > 127.0)
        return HUGE_VAL;

    if (x < -126.0)
        return 0.0;

    float expx = 1.0;
    float n  = floorf(x * M_1_LN2);
    float x0 = n * M_LN2;
    float b = x - x0;

    //debug("x = n *ln2 + b -> x = %5.7f, n = %3.0f, x0 = %5.7f, b = %1.7f\n", x, n, x0, b);
    if (n != 0.0)
        expx = powerof2f((uint32_t) n);

    //debug("exp(x0) = %f\n", expx);

    if (b != 0.0) {
        float nom, denom;
        float y1, y2, y3, y4, y5;
        y1 = 0.5 * b;
        y2 = 0.111111111111 * b * b;
        y3 = 0.0138888888889 * b * b * b;
        y4 = 0.000992063492063 * b * b * b * b;
        y5 = 3.30687830688e-05 * b * b * b * b * b;

        // In the range [0..ln2] we found that this Pade approximant gives exact results (maxerror ~1.2 * 10^-7):
        //
        // exp(x) ~ (1680 +840x +180x^2 +20x^3 + x^4)/(1680 -840x +180x^2 -20x^3 + x^4)
        // In fact, this proves to be accurate with a larger error, ~4 *10^(-7).
        // and that the normalized version is more accurate, with a maxerror 2 * 10^(-7).

        //P[4,4]
        //nom   = 1680.0 + 840.0 * b + 180.0 * x2 + 20.0 * x3 + x4;
        //denom = 1680.0 - 840.0 * b + 180.0 * x2 - 20.0 * x3 + x4;
        //P[6,6]
        nom   = 1.0 + y1 + y2 + y3 + y4 + y5;
        denom = 1.0 - y1 + y2 - y3 + y4 - y5;
        //nom   = 30240.0 + 15120.0 * b + 3360.0 * x2 + 420.0 * x3 + 30.0 * x4 + x5;
        //denom = 30240.0 - 15120.0 * b + 3360.0 * x2 - 420.0 * x3 + 30.0 * x4 - x5;

        expx *= nom / denom;

        //debug("exp(b) = %5.7f\n", nom/denom);
    }

    //debug("exp(x) = %5.7f\n", expx);
    return expx;
}
