/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#ifndef LIBFREEVEC_H
#define LIBFREEVEC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <altivec.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#define ALTIVECWORD_SIZE    16
#define QUAD_ALTIVECWORD    64
// log2(64) = 6
#define LOG_ALTIVECQUAD     6

// This portion was stolen from SDL Altivec patches by Ryan C. Gordon :-)
#define DST_CHAN_1 1
#define DST_CHAN_2 2

#define DST_CTRL(size, count, stride) (((size) << 24) | ((count) << 16) | (stride))

#define READ_PREFETCH_START1(addr)    vec_dstt(addr, DST_CTRL(2,2,32), DST_CHAN_1)
#define READ_PREFETCH_START2(addr)    vec_dstt(addr, DST_CTRL(2,2,32), DST_CHAN_2)
#define WRITE_PREFETCH_START1(addr)   vec_dststt(addr, DST_CTRL(2,2,32), DST_CHAN_1)
#define WRITE_PREFETCH_START2(addr)   vec_dststt(addr, DST_CTRL(2,2,32), DST_CHAN_2)
#define PREFETCH_STOP1                vec_dss(DST_CHAN_1)
#define PREFETCH_STOP2                vec_dss(DST_CHAN_2)

#define ptrdiff_t(a, b)     ((uint32_t)(a)-(uint32_t)(b))

#define CMP_LT_OR_GT(a, b) ((a) - (b))

#define MIN(a,b)    ((a) <= (b) ? (a) : (b))

#define DIFF(a, b) ((a)-(b))

#define himagic         0x80808080L
#define lomagic         0x01010101L
#define magic_bits32    0x07efefeff
#define magic_bits64    (((unsigned long int) 0x7efefefe << 32) | 0xfefefeff)

#define charmask8(c)    ((uint8_t)(c & 0xff))
#define charmask16(c)   (uint16_t)((charmask8(c)) | (charmask8(c) << 8))
#define charmask32(c)   (uint32_t)((charmask16(c)) | (charmask16(c) << 16))
#define charmask64(c)   (uint64_t)((charmask32(c)) | (charmask32(c) << 32))

void printbuf16 ( char *label, uint8_t *buf );
void printbuf4 ( char *label, uint32_t *buf );
void printbuf ( char *buf, size_t len );
void printvec_text ( char *label, vector uint8_t vc );
void printvec_char ( char *label, vector uint8_t vc );
void printvec_short ( char *label, vector uint16_t vs );
void printvec_long ( char *label, vector uint32_t vl );
void fprintvec_long ( FILE *fp, vector uint32_t vl );
int detect_altivec();

#endif
