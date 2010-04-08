/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#ifndef LIBFREEVEC_MATH_SCALAR_TRIG_H
#define LIBFREEVEC_MATH_SCALAR_TRIG_H

#include <math.h>

/*static float inline fmin(float a, float b) {
    return a >= b ? b : a;
}

static float inline fmax(float a, float b) {
    return a >= b ? a : b;
}*/

static inline float reduce_2pif(float x) {
    return fmodf(x, TWO_PI);
}

static inline float reduce_pi_2f(float x) {
    x = fmodf(x, TWO_PI);

    //x = fmin(x,  M_PI -x);
    //x = fmax(x, -M_PI -x);
    //x = fmin(x,  M_PI -x);

    return x;
}

static inline double reduce_2pi(double x) {
    return fmod(x, TWO_PI);
}

static inline double reduce_pi_2(double x) {
    x = fmod(x, TWO_PI);

    //x = fmin(x,  M_PI -x);
    //x = fmax(x, -M_PI -x);
    //x = fmin(x,  M_PI -x);

    return x;
}

#endif // LIBFREEVEC_MATH_SCALAR_TRIG_H