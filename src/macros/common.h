/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under a BSD-type license                     *
 ***************************************************************************/

#define POSINWORD	0x00010203

size_t find_leftfirst_in_word(uint32_t x);
size_t find_rightfirst_in_word(uint32_t x);

uint32_t inline find_leftfirst_nzb( uint32_t x);
uint32_t inline find_rightfirst_nzb( uint32_t *x);

