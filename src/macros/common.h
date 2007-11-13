/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under a BSD-type license                     *
 ***************************************************************************/

#define POSINWORD	0x00010203

#define FIND_LEFTFIRST_IN_WORD(x, p)			\
	x |= (x >> 1);								\
	x |= (x >> 2);								\
	x |= (x >> 4);								\
	x |= (x >> 8);								\
	x |= (x >> 16);								\
	x = x ^ (x >> 1);							\
	x |= (x >> 1);								\
	x |= (x >> 2);								\
	x |= (x >> 4);								\
	x &= POSINWORD;								\
	p = x + (x >> 8) + (x >> 16) + (x >> 24);	\

#define FIND_RIGHTFIRST_IN_WORD(x, p)			\
	x &= -x;									\
	x |= (x >> 1);								\
	x |= (x >> 2);								\
	x |= (x >> 4);								\
	x &= POSINWORD;								\
	p = x + (x >> 8) + (x >> 16) + (x >> 24);	\

size_t find_leftfirst_in_word(uint32_t x);
size_t find_rightfirst_in_word(uint32_t x);
uint32_t inline find_first_nonzero_char( uint32_t x);
