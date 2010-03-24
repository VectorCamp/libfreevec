/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include <math.h>
#include <stdint.h>

/*	Clever trick to calculate a float representation of a power of two
	Actually, the same thing that vec_expte() returns in AltiVec, but 
	using plain C
 */
static inline float powerof2(uint32_t n) {
	/* We need to mess with binary arithmetic, but the result has
	   to be float, so we use a union */
	union {
		float f;
		uint32_t u;
	} x;
	
	// Start with 1.0
	x.u = 0x3f800000;

	// Multiply or divide (if n <0) by 2.0, n times (n is supposed to be an uint32_t)
	x.u += n * 0x800000;

	return x.f;
}
