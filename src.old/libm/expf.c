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

#define M_LN2		0.69314718055994530942  /* Log(2)   */
#define M_LOG2E		1.4426950408889634074	/* Log2(e)  */

#ifdef VEC_GLIBC
float exp2f(float x) {
#else
float vexp2f(float x) {
#endif
  vector float vexpa, va;
  float exp, a, b, b2, b3, b4, b5, b6;
  float __attribute__((aligned(16))) xa[4];

  xa[0] = floorf(x);
  va = vec_ld(0, xa);
  vexpa = vec_expte(va);
  a = xa[0] * M_LN2;
  vec_st(vexpa, 0, xa);

  b = x - a;
  b2 = b*b;
  b3 = b2*b;
  b4 = b3*b;
  b5 = b4*b;
  b6 = b5*b;
  exp =   1.000000002644271861135635165803    + 0.999999630676466160313515392503*b
        + 0.500008415450545865703228178366*b2 + 0.166594937239279108432157593360*b3
        + 0.041956255273539448094181805064*b4 + 0.007740059180886275586488149490*b5
        + 0.001973684356346945984956068724*b6;

  exp *= xa[0];
  return xa[0];
}

static const vector float C_EXPF[] =
{
  { 1.4426950408889634074, 0.69314718055994530942, 1.4426950408889634074, 1.4426950408889634074 }, /* LOG2E, LN2 */
  { 166320.0, -22260, 166320.0, 5460.0 },
  { 0.0, 551.0, 0.0, 75.0 }
};

#ifdef VEC_GLIBC
float expf(float x) {
#else
float vexpf(float x) {
#endif
  vector float vexpa, va;//, vx, vn, va, vb,
//               v0, vlog2, vln2;
  
  register float exp, a;//, b, b2, b3, b4;//, b6, b8, b10, R0, R;
  float __attribute__((aligned(16))) xa[4];
  xa[0] = x;

/*  // set up a few constants
  vlog2 = vec_ld(0, &C_EXPF[0]);
  v0    = (vector float) vec_splat_u32(0);
  vln2  = vec_splat(vlog2, 1);
  vlog2 = vec_splat(vlog2, 0);

  // Load x into a vector float
  vx = vec_ld(0, xa);
  vx = vec_splat(vx, 0);

  // Split x = n*log2e + b
  vn = vec_madd(vx, vlog2e, v0);
  vn = vec_floor(vn);*/
    
  
  xa[0] = truncf(x*M_LOG2E);
  va = vec_ld(0, xa);
  vexpa = vec_expte(va);
  a = xa[0] * M_LN2;
  vec_st(vexpa, 0, xa);
/*
  b = x - a;
  b2 = b*b;
  b3 = b2*b;
  b4 = b2*b2;
  b6 = b4*b2;
  b8 = b6*b2;
  b10 = b8*b2;
  R0 =       0.1666666666666666019037   *b2 - 0.00277777777770155933842  *b4
           + 6.61375632143793436117e-05 *b6 - 1.65339022054652515390e-06 *b8
           + 4.13813679705723846039e-08 *b10;
  R = b - R0;
  //exp = 1.0 + 2.0*b/(2.0 - R);
  exp = (1680.0 + 840*b + 180*b2 + 20*b3 + b4)/(1680 - 840*b + 180*b2 - 20*b3 + b4);
*/
  exp = xa[0];
  return exp;
}

#endif
