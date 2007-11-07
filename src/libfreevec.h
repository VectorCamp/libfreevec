/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under a BSD-type license                     *
 ***************************************************************************/
 
#ifndef LIBFREEVEC_H
#define LIBFREEVEC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <altivec.h>
#include <sys/types.h>
#include <stdint.h>

#define ALTIVECWORD_SIZE    16
#define ALTIVEC_BIGLOOP		64

// This portion was stolen from SDL Altivec patches by Ryan C. Gordon :-) 
#define DST_CHAN_SRC 1
#define DST_CHAN_DEST 2

#define DST_CTRL(size, count, stride) (((size) << 24) | ((count) << 16) | (stride))

#define VEC_DSS()   vec_dss(DST_CHAN_SRC);

#define ptrdiff_t(a, b)     ((uint32_t)(a)-(uint32_t)(b))

#define CMP_LT_OR_GT(a, b) ((a) > (b) ? 1 : -1)

#define MIN(a,b)    ((a) <= (b) ? (a) : (b))

#define DIFF(a, b) ((a)-(b))

#define himagic         0x80808080L
#define lomagic         0x01010101L
#define magic_bits32    0x7efefeffL
#define magic_bits64    (((unsigned long int) 0x7efefefe << 32) | 0xfefefeff)

#define charmask16(c)   (uint16_t)((c) | ((c) << 8))
#define charmask32(c)   (uint32_t)((charmask16(c)) | (charmask16(c) << 16))
#define charmask64(c)   (uint64_t)((charmask32(c)) | (charmask32(c) << 32))

void printbuf16(char *label, uint8_t *buf);
void printbuf4(char *label, uint32_t *buf);
void printvec_text(char *label, vector uint8_t vc);
void printvec_char(char *label, vector uint8_t vc);
void printvec_short(char *label, vector uint16_t vs);
void printvec_long(char *label, vector uint32_t vl);

#endif
