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
float cosf(float x) {
#else
float vcosf(float x) {
#endif

  float nom, res, denom, x2, x4, x6;
  // Instead calculate cos(x/2) and use the formula, cos(x) = 2*cos(x/2)^2 - 1
  x2 = x*x;
  x4 = x2*x2;
  x6 = x2*x4;

  // we don't use the following but a normalized version of it:
  //nom   = 39251520.0 - 18471600.0 * POW2(x) + 1075032.0 * POW4(x) -14615.0 * POW6(x);
  //denom = 39251520.0 + 1154160.0 * POW2(x) + 16632.0 * POW4(x) + 127.0 * POW6(x);

  nom   = 1.0 - x2 * 0.4705957883923985619  + x4 * 0.02738828967642526965   - x6 * 0.0003723422685287092067;
  denom = 1.0 + x2 * 0.02940421160760143811 + x4 * 0.0004237288135593220339 + x6 * 3.235543489780777916e-06;

  res = nom/denom;

  return res;
}

// vsinf()
// We want to calculate these:
// nom   = 166320.0 * x - 22260.0 * POW3(x) + 551.0 * POW5(x);
// denom = 166320.0 + 5460.0 * POW2(x) + 75.0 * POW4(x);
// res = nom/denom;
//
// We first setup our constants:
// vc1 = | a1  | a3 | b0  |  b2 |
// vc2 = | 0.0 | a5 | 0.0 | 0.0 |
static const vector float C_SINF[] =
{
  { 166320.0, -22260, 166320.0, 5460.0 },
  { 0.0, 551.0, 0.0, 75.0 }
};

#endif
