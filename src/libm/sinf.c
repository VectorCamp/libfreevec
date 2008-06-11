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

#ifdef VEC_GLIBC
float sinf(float x) {
#else
float vsinf(float x) {
#endif
/*
  // Load x into an aligned float array
  float __attribute__((aligned(16))) xa[4];
  xa[0] = x;

  vector float vx = vec_ld(0, xa);
  vector float vres, vdenom, vest1, 
               vx2, vx02, vx13, vx24,
               v0 = (vector float)vec_splat_u32(0),
               v1 = vec_ctf(vec_splat_u32(1),0);

  // Load x into a vector and splat it all over
  vx = vec_splat(vx, 0);
  // get the vector with all elements: x^2 
  vx2 = vec_madd(vx, vx, v0);

  // We need a vector with | 1.0 | x^2 | 1.0 | x^2 |
  vx02 = vec_mergeh(v1, vx2);
  // Multiply with x -> | x | x^3 | x | x^3 |
  vx13 = vec_madd(vx, vx02, v0);
  // Now shift left and combine with vx02 -> | x | x^3 | 1.0 | x^2 |
  vx13 = vec_sld(vx13, vx02, 8);
  // Again with x^2 -> | x^3 | x^5 | x^2 | x^4 |
  vx24 = vec_madd(vx13, vx2, v0);

  // Multiply with the coefficients vectors:
  // First with C_SINF1 -> | a1*x | a3*x^3 | b0*1.0 | b2*x^2 |
  vres = vec_madd(vx13, C_SINF[0], v0);
  // Now with C_SINF2 (and add previous result) -> | a1*x + 0*x^3 | a3*x^3 + a5*x^5 | b0*1.0 + 0.0*x^2 | b2*x^2 + b4*x^4 |
  vres = vec_madd(vx24, C_SINF[1], vres);
  // Shift left by 4 and add the vectors -> | nom | .. | denom | .. |
  vres = vec_add(vres, vec_sld(vres, vres, 4));

  // Now splat denom (we don't have to splat nom, we'll just take the first element after the division.
  vdenom = vec_splat(vres, 2);

  vest1 = vec_re(vdenom);
  //1st round of Newton-Raphson refinement
  vdenom = vec_madd( vest1, vec_nmsub( vest1, vdenom, v1 ), vest1 );
  // 2nd round of Newton-Raphson refinement
  // vdenom = vec_madd( vest2, vec_nmsub( vest2, vdenom, v1 ), vest2 );
  vres = vec_madd(vres, vdenom, v0);
  vec_st(vres, 0, xa);
  // printf("vres = %2.7f %2.7f %2.7f %2.7f\n", xa[0], xa[1], xa[2], xa[3]);
*/
  float nom, denom, res, x2, x3, x4, x5;
  x2 = POW2(x);
  x3 = x2*x;
  x4 = x2*x2;
  x5 = x4*x;
  nom   = 166320.0 * x - 22260.0 * x3 + 551.0 * x5;
  // nom   = x*(166320.0 - x2 * (22260.0 + 551.0 * x2));
  denom = 166320.0 + 5460.0 * x2 + 75.0 * x4;
  // denom = 166320.0 + x2 * (5460.0 + 75.0 * x2);

  //printf("nom = %2.7f, denom = %2.7f\n", nom, denom);

  res = nom/denom;
  //printf("res = %2.7f\n", res);
  return res;
  //return xa[0];
}
#endif
