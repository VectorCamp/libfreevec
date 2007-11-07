/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under a BSD-type license                     *
 ***************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include "libfreevec.h"
#include "macros/common.h"

size_t find_leftfirst_in_word(uint32_t x)
{
	uint8_t sum = 0;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x = x ^ (x >> 1);
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x &= POSINWORD;
	sum = x + (x >> 8) + (x >> 16) + (x >> 24);
	return (sum);
}

size_t find_rightfirst_in_word(uint32_t x)
{
	uint8_t sum = 0;
	x &= -x;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x &= POSINWORD;
	sum = x + (x >> 8) + (x >> 16) + (x >> 24);
	return (sum);
}

size_t find_leftfirst_in_word_new(uint32_t x)
{
	//(x - lobits) & ~x & hibits
	//do cntlz on it, shift right by 3, done

	printf("x = %08x\n", x);
	printf("x = %08x\n", x);
	return 0;
}
