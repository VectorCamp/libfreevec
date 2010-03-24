/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
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
