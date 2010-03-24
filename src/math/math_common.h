/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include <math.h>

#define TWO_PI		2.0*M_PI
#define M_1_LN2		1.442695040888963456

static float inline fmin(float a, float b) {
	return a >= b ? b : a;
}

static float inline fmax(float a, float b) {
	return a >= b ? a : b;
}