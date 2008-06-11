/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#ifdef HAVE_ALTIVEC_H
#include <altivec.h>

#define POW2(x)		(x*x)

#ifdef VEC_GLIBC
float tanf(float x) {
#else
float vtanf(float x) {
#endif

  float res, nom, denom, x2, x3, x4;
  x2 = POW2(x);
  x3 = x2*x;
  x4 = x2*x2;

  nom   = 0.999999986 * x - 0.0958010197 * x3;
  denom = 1.0 - 0.429135022 * x2 + 0.00971659383 * x4;

  res = nom/denom;
  return res;
}
#endif
