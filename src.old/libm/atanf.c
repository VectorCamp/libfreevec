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
float atanf(float x) {
#else
float vatanf(float x) {
#endif

  float res, nom, denom, x2, x3, x4, x5;
  x2 = POW2(x);
  x3 = x2*x;
  x4 = x2*x2;
  x5 = x4*x;

  nom   = 1.0 * x + 0.7777777777777777778 * x3 + 0.06772486772486772487 * x5;
  denom = 1.0 + 1.111111111111111111 * x2 + 0.2380952380952380952 * x4;

  res = nom/denom;
  return res;
}
#endif
