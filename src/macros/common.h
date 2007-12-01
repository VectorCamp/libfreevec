/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#define POSINWORD	0x00010203

// Helper macro, fills a vector with a given char
#define FILL_VECTOR(vecname, p)                   \
  union {                                         \
    vector uint8_t v;                             \
    uint8_t c[16];                                \
  } p_env;                                        \
  p_env.c[0] = p;                                 \
  vector uint8_t vecname = vec_splat(p_env.v, 0);

size_t find_leftfirst_in_word(uint32_t x);
size_t find_rightfirst_in_word(uint32_t x);

uint32_t inline find_leftfirst_nzb( uint32_t x);
uint32_t inline find_rightfirst_nzb( uint32_t *x);

