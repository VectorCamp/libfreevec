/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include <math.h>

static inline float reduce_2pi(float x) {
	return fmodf(x, TWO_PI);
}

static inline float reduce_pi_2(float x) {
	x = fmodf(x, TWO_PI);

	x = fmin(x,  M_PI -x);
	x = fmax(x, -M_PI -x);
	x = fmin(x,  M_PI -x);

	return x;
}