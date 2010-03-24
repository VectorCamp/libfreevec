/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define SWAB_NIBBLE(dst, src, len)     \
  if (len == 4) {                      \
    dst[0] = src[1]; dst[1] = src[0];  \
    dst[2] = src[3]; dst[3] = src[2];  \
  } else if (len == 2) {               \
    dst[0] = src[1]; dst[1] = src[0];  \
  }

#define SWAB_FIND_IF_HAS_CARRY(dst, src, len, carry, has_carry)  \
  if (((uint32_t)dst) % sizeof(uint16_t)) {                      \
    *dst++ = src[1];                                             \
    carry = src[0];                                              \
    has_carry = 1;                                               \
    src++; len--;                                                \
  }

#define SWAB_WORD_UNTIL_ALTIVEC_ALIGNED_HAS_CARRY(dst16, src, len)  \
  while (((uint32_t)(dst16) % 16) && len >= sizeof(uint16_t)) {     \
    *dst16++ = ((uint16_t)carry << 8) | ((uint16_t)src[2]);         \
    carry = src[1];                                                 \
    src += sizeof(uint16_t);                                        \
    len -= sizeof(uint16_t);                                        \
  }

#define SWAB_WORD_UNTIL_ALTIVEC_ALIGNED_NO_CARRY(dst16, src, len)              \
  while (((uint32_t)(dst16) % ALTIVECWORD_SIZE) && len >= sizeof(uint16_t)) {  \
    *dst16++ = ((uint16_t)src[0]) | ((uint16_t)src[1] << 8);                   \
    src += sizeof(uint16_t);                                                   \
    len -= sizeof(uint16_t);                                                   \
  }

#define SWAB_REST_WORDS_HAS_CARRY(dst16, src, len)           \
  while (len >= sizeof(uint16_t)) {                          \
    *dst16++ = ((uint16_t)carry << 8) | ((uint16_t)src[2]);  \
    carry = src[1];                                          \
    src += sizeof(uint16_t);                                 \
    len -= sizeof(uint16_t);                                 \
  }

#define SWAB_REST_WORDS_NO_CARRY(dst16, src, len)             \
  while (len >= sizeof(uint16_t)) {                           \
    *dst16++ = ((uint16_t)src[0]) | ((uint16_t)src[1] << 8);  \
    src += sizeof(uint16_t);                                  \
    len -= sizeof(uint16_t);                                  \
  }

#define SWAB_SINGLE_ALTIVEC_ALIGNED_NO_CARRY(vsrc, vdst, src, dst, vpermute_mask)  \
{                                                                                  \
  vsrc = vec_ld(0, (uint8_t *)src);                                                \
  vdst = vec_perm(vsrc, vsrc, vpermute_mask);                                      \
  vec_st(vdst, 0, (uint8_t *)dst);                                                 \
}

#define SWAB_SINGLE_ALTIVEC_ALIGNED_HAS_CARRY(vsrc, vdst, src, dst, vpermute_mask)  \
{                                                                                   \
  vsrc = vec_sld(vec_ld(0, (uint8_t *)src), vec_ld(16, (uint8_t *)src), 1);         \
  vdst = vec_perm(vsrc, vsrc, vdst_permute_mask);                                   \
  vec_st(vdst, 0, (uint8_t *)dst);                                                  \
}

#define SWAB_SINGLE_ALTIVEC_UNALIGNED(MSQ, LSQ, vsrc, vdst, src, dst, vpermute_mask)  \
{                                                                                     \
  MSQ = vec_ld(0, src);                                                               \
  LSQ = vec_ld(15, src);                                                              \
  vsrc = vec_perm(MSQ, LSQ, mask);                                                    \
  vdst = vec_perm(vsrc, vsrc, vdst_permute_mask);                                     \
  vec_st(vdst, 0, (unsigned char *)dst16);                                            \
}

#define SWAB_LOOP_ALTIVEC_ALIGNED_HAS_CARRY(dst16, src, len, carry)                        \
{                                                                                          \
  vector uint8_t vdst_permute_mask = {  0, 1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15 };  \
  vector uint8_t vsrc, vdst;                                                               \
  while (len >= ALTIVECWORD_SIZE) {                                                        \
    SWAB_SINGLE_ALTIVEC_ALIGNED_HAS_CARRY(vsrc, vdst, src, dst16, vdst_permute_mask);      \
    *((uint8_t *)dst16) = carry;                                                           \
    carry = src[15];                                                                       \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                          \
    READ_PREFETCH_START1(src);                                                              \
    WRITE_PREFETCH_START2(dst);                                                             \
  }                                                                                        \
}

#define SWAB_LOOP_ALTIVEC_UNALIGNED_HAS_CARRY(dst16, src, len, carry)                           \
{                                                                                               \
  vector uint8_t vdst_permute_mask = { 0, 2, 1, 4, 3, 6, 5, 8, 7, 10, 9, 12, 11, 14, 13, 15 };  \
  vector uint8_t vsrc, vdst, MSQ, LSQ, mask;                                                    \
  mask = vec_lvsl(0, src);                                                                      \
  while (len >= ALTIVECWORD_SIZE) {                                                             \
    SWAB_SINGLE_ALTIVEC_UNALIGNED(MSQ, LSQ, vsrc, vdst, src, dst16, vdst_permute_mask);         \
    dst = (uint8_t *)dst16;                                                                     \
    dst[0] = carry; carry = src[15]; dst[15] = src[16];                                         \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                               \
    READ_PREFETCH_START1(src);                                                                   \
    WRITE_PREFETCH_START2(dst);                                                                  \
  }                                                                                             \
}

#define SWAB_LOOP_ALTIVEC_ALIGNED_NO_CARRY(dst16, src, len)                                \
{                                                                                          \
  vector uint8_t vdst_permute_mask = {  1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15,14 };  \
  vector uint8_t vsrc, vdst;                                                               \
  while (len >= ALTIVECWORD_SIZE) {                                                        \
    SWAB_SINGLE_ALTIVEC_ALIGNED_NO_CARRY(vsrc, vdst, src, dst16, vdst_permute_mask);       \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                          \
    READ_PREFETCH_START1(src);                                                              \
    WRITE_PREFETCH_START2(dst);                                                             \
  }                                                                                        \
}

#define SWAB_LOOP_ALTIVEC_UNALIGNED_NO_CARRY(dst16, src, len)                              \
{                                                                                          \
  vector uint8_t vdst_permute_mask = {  1, 0, 3, 2, 5, 4, 7, 6, 9, 8,11,10,13,12,15,14 };  \
  vector uint8_t vsrc, vdst, MSQ, LSQ, mask;                                               \
  mask = vec_lvsl(0, src);                                                                 \
  while (len >= ALTIVECWORD_SIZE) {                                                        \
    SWAB_SINGLE_ALTIVEC_UNALIGNED(MSQ, LSQ, vsrc, vdst, src, dst16, vdst_permute_mask);    \
    dst16 += 8; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                          \
    READ_PREFETCH_START1(src);                                                              \
    WRITE_PREFETCH_START2(dst);                                                             \
  }                                                                                        \
}
