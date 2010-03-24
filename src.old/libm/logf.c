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


float vvsinf(float *x) {

  // Load x into an aligned float array
  float __attribute__((aligned(16))) xa[4];
  xa[0] = x;

  // We want to calculate these:
  // nom   = 166320.0 * x - 22260.0 * POW3(x) + 551.0 * POW5(x);
  // denom = 166320.0 + 5460.0 * POW2(x) + 75.0 * POW4(x);
  // res = nom/denom;
  //
  // We first setup our constants:
  // vc1 = | a1  | a3 | b0  |  b2 |
  // vc2 = | 0.0 | a5 | 0.0 | 0.0 |
  vector float vc1 = { 166320.0, -22260, 166320.0, 5460.0 },
	       vc2 = { 0.0, 551.0, 0.0, 75.0 };
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
  // First with vc1 -> | a1*x | a3*x^3 | b0*1.0 | b2*x^2 |
  vres = vec_madd(vx13, vc1, v0);
  // Now with vc2 (and add previous result) -> | a1*x + 0*x^3 | a3*x^3 + a5*x^5 | b0*1.0 + 0.0*x^2 | b2*x^2 + b4*x^4 |
  vres = vec_madd(vx24, vc2, vres);
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
  //printf("vres = %2.7f %2.7f %2.7f %2.7f\n", xa[0], xa[1], xa[2], xa[3]);

/*  float nom, denom, res;
  nom   = 166320.0 * x - 22260.0 * POW3(x) + 551.0 * POW5(x);
  denom = 166320.0 + 5460.0 * POW2(x) + 75.0 * POW4(x);

  printf("nom = %2.7f, denom = %2.7f\n", nom, denom);

  res = nom/denom;
  printf("res = %2.7f\n", res);*/
  return xa[0];
}

float vsincos2f(float x) {

  // Load x into an aligned float array
  float __attribute__((aligned(16))) xa[4];
  xa[0] = x;

  // We want to calculate these:
  // nom   = 166320.0 * x - 22260.0 * POW3(x) + 551.0 * POW5(x);
  // denom = 166320.0 + 5460.0 * POW2(x) + 75.0 * POW4(x);
  // res = nom/denom;
  //
  // We first setup our constants:
  // vc1 = | a1  | a3 | b0  |  b2 |
  // vc2 = | 0.0 | a5 | 0.0 | 0.0 |
  vector float vc1 = { 166320.0, -22260, 166320.0, 5460.0 },
	       vc2 = { 0.0, 551.0, 0.0, 75.0 };
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
  // First with vc1 -> | a1*x | a3*x^3 | b0*1.0 | b2*x^2 |
  vres = vec_madd(vx13, vc1, v0);
  // Now with vc2 (and add previous result) -> | a1*x + 0*x^3 | a3*x^3 + a5*x^5 | b0*1.0 + 0.0*x^2 | b2*x^2 + b4*x^4 |
  vres = vec_madd(vx24, vc2, vres);
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
  //printf("vres = %2.7f %2.7f %2.7f %2.7f\n", xa[0], xa[1], xa[2], xa[3]);

/*  float nom, denom, res;
  nom   = 166320.0 * x - 22260.0 * POW3(x) + 551.0 * POW5(x);
  denom = 166320.0 + 5460.0 * POW2(x) + 75.0 * POW4(x);

  printf("nom = %2.7f, denom = %2.7f\n", nom, denom);

  res = nom/denom;
  printf("res = %2.7f\n", res);*/
  printf("res = %2.7f\n", xa[0]);
  return xa[0];
}

#endif
