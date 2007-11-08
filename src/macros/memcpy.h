/***************************************************************************
 *   Copyright (C) 2005 by Konstantinos Margaritis                         *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#include "libfreevec.h"

#define MYNIBBLE_COPY_FWD(dst, src, len)  \
  switch(len) {                           \
  case 3:                                 \
    *dst++ = *src++;                      \
  case 2:                                 \
    *dst++ = *src++;                      \
  case 1:                                 \
    *dst = *src;                          \
  }


#define MYCOPY_FWD_UNTIL_DEST_IS_WORD_ALIGNED(dst, src, len)    \
  {                                                             \
    int dstal = ((uint32_t)dst) % sizeof(uint32_t);             \
    if (dstal == 1) {                                           \
      *dst++ = *src++;                                          \
      *dst++ = *src++;                                          \
      *dst++ = *src++;                                          \
      len -= 3;                                                 \
    } else if (dstal == 2) {                                    \
      *dst++ = *src++;                                          \
      *dst++ = *src++;                                          \
      len -= 2;                                                 \
    } else if (dstal == 3) {                                    \
      *dst++ = *src++;                                          \
      len--;                                                    \
    }                                                           \
  }

#define MYCOPY_FWD_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, src, len, srcofst) \
  while (len >= sizeof(uint32_t) && ((uint32_t)(dst) & 15)) {     \
    if (srcofst == 0) {                                         \
      *dst++ = *src++;                                      \
    } else if (srcofst == 3) {                  \
      *dst++ = (*(src) << 24) | (*(src+1) >> 8);            \
      src++;                                                \
    } else if (srcofst == 2) {                  \
      *dst++ = (*(src) << 16) | (*(src+1) >> 16);           \
      src++;                                                \
    } else if (srcofst == 1) {                  \
      *dst++ = (*(src) << 8) | (*(src+1) >> 24);            \
      src++;                                                \
    }                                                           \
    len -= sizeof(uint32_t);                                    \
  }

#define MYCOPY_SINGLEQUADWORD_ALTIVEC_ALIGNED(dst, src, step)       \
    vec_st((vector uint8_t) vec_ld(step, (uint8_t *)src),           \
                step, (uint8_t *)dst);

#define MYCOPY_SINGLEQUADWORD_ALTIVEC_UNALIGNED(dst, src, step)     \
    mask = vec_lvsl(0, src);                                        \
    MSQ = vec_ld(step, src);                                        \
    LSQ = vec_ld(step+15, src);                                     \
    vec_st(vec_perm(MSQ, LSQ, mask), step, (uint8_t *)dst);

#define MYCOPY_FWD_LOOP_QUADWORD_ALTIVEC_ALIGNED(dst, src, blocks)                      \
    len -= blocks << 6;                                                             \
    while (blocks--) {                                                              \
        vec_st((vector uint8_t) vec_ld(0, (uint8_t *)src), 0, (uint8_t *)dst);      \
        vec_st((vector uint8_t) vec_ld(16, (uint8_t *)src), 16, (uint8_t *)dst);    \
        vec_st((vector uint8_t) vec_ld(32, (uint8_t *)src), 32, (uint8_t *)dst);    \
        vec_st((vector uint8_t) vec_ld(48, (uint8_t *)src), 48, (uint8_t *)dst);    \
        dst += 16; src += 64;                                                       \
    }

#define MYCOPY_FWD_LOOP_QUADWORD_ALTIVEC_UNALIGNED(dst, src, blocks)        \
    vector uint8_t mask, MSQ1, LSQ1, LSQ2, LSQ3, LSQ4;                  \
    mask = vec_lvsl(0, src);                                            \
    len -= blocks << 6;                                                 \
    while (blocks--) {                                                  \
        MSQ1 = vec_ld(0, src);                                          \
        LSQ1 = vec_ld(15, src);                                         \
        LSQ2 = vec_ld(31, src);                                         \
        LSQ3 = vec_ld(47, src);                                         \
        LSQ4 = vec_ld(63, src);                                         \
        vec_st(vec_perm(MSQ1, LSQ1, mask), 0, (uint8_t *)dst);          \
        vec_st(vec_perm(LSQ1, LSQ2, mask), 16, (uint8_t *)dst);         \
        vec_st(vec_perm(LSQ2, LSQ3, mask), 32, (uint8_t *)dst);         \
        vec_st(vec_perm(LSQ3, LSQ4, mask), 48, (uint8_t *)dst);         \
        dst += 16; src += 64;                                           \
        vec_dst(src, DST_CTRL(2,2,32), DST_CHAN_SRC);                   \
        vec_dstst(dst, DST_CTRL(2,2,32), DST_CHAN_DEST);                \
    }

#define MYCOPY_FWD_REST_WORDS(dst, src, len, srcofst)           \
    while (len >= sizeof(uint32_t)) {                           \
        if (srcofst == 0) {                                     \
      *dst++ = *src++;                                  \
    } else if (srcofst == 3) {                              \
      *dst++ = (*(src) << 24) | (*(src+1) >> 8);        \
      src++;                                            \
    } else if (srcofst == 2) {                              \
      *dst++ = (*(src) << 16) | (*(src+1) >> 16);       \
      src++;                                            \
    } else if (srcofst == 1) {                              \
      *dst++ = (*(src) << 8) | (*(src+1) >> 24);        \
      src++;                                            \
        }                                                       \
        len -= sizeof(uint32_t);                                \
    }

#define MYMEMCCPY_UNTIL_DEST_WORD_ALIGNED(dst, src, c, len)     \
    while (((uint32_t)(dst) % sizeof(uint32_t)) && len--) {     \
        if ((*dst++ = *src++) == c) return dst;                 \
    }

#define MYMEMCCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset)  \
  if (srcoffset == 0) {                                   \
    srct = *srcl;                                       \
  } else if (srcoffset == 3) {                            \
    srct = (*(srcl) << 24) | (*(srcl+1) >> 8);          \
  } else if (srcoffset == 2) {                            \
    srct = (*(srcl) << 16) | (*(srcl+1) >> 16);         \
  } else if (srcoffset == 1) {                            \
    srct = (*(srcl) << 8) | (*(srcl+1) >> 24);          \
  }

#define MYMEMCCPY_SINGLE_WORD_FWD(dst, dstl, src, srcl, srct, srcoffset, mask, c)      \
    {                                                                                  \
        uint32_t lw = srct ^ mask;                                                     \
        if (( lw - lomagic) & himagic) {                                               \
            src = (uint8_t *) srcl +srcoffset;                                         \
            dst = (uint8_t *) dstl;                                                    \
            if ((*dst++ = *src++) == c ) return dst;                                   \
            if ((*dst++ = *src++) == c ) return dst;                                   \
            if ((*dst++ = *src++) == c ) return dst;                                   \
            if ((*dst++ = *src++) == c ) return dst;                                   \
        }                                                                              \
    }                                                                                  \
    *dstl++ = srct;                                                                    \
    srcl++;

#define MYMEMCCPY_QUADWORD(dst, dstl, src, srcl, srcoffset, mask, c)                \
    int i;                                                                          \
    for (i=0; i < 4; i++) {                                                         \
        uint32_t srct = 0;                                                          \
        MYMEMCCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                         \
        MYMEMCCPY_SINGLE_WORD_FWD(dst, dstl, src, srcl, srct, srcoffset, mask, c);  \
    }

#define MYMEMCCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED(dst, dstl, src, srcl, len, srcoffset, mask, c)  \
    while (((uint32_t)(dstl) % ALTIVECWORD_SIZE) && len >= sizeof(uint32_t)) {                  \
        uint32_t srct = 0;                                                                      \
        MYMEMCCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                                     \
        MYMEMCCPY_SINGLE_WORD_FWD(dst, dstl, src, srcl, srct, srcoffset, mask, c);              \
        len -= sizeof(uint32_t);                                                                \
    }

#define MYFILL_VECTOR(vecname, p)                   \
    union {                                         \
        vector uint8_t v;                           \
        uint8_t c[16];                              \
    } p_env;                                        \
    p_env.c[0] = p;                                 \
    vector uint8_t vecname = vec_splat(p_env.v, 0);

#define MYMEMCCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c) \
    {                                                                                       \
        vector uint8_t vsrc = (vector uint8_t) vec_ld(0, (uint8_t *)src);                   \
        if (vec_any_eq(vsrc, vc)) {                                                         \
            srcl = (uint32_t *)(src -srcoffset4);                                           \
            MYMEMCCPY_QUADWORD(dst, dstl, src, srcl, srcoffset, mask, c);                   \
        }                                                                                   \
        vec_st(vsrc, 0, (uint8_t *)dstl);                                                   \
    }

#define MYMEMCCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c)   \
    {                                                                                           \
        vector uint8_t vsrc, MSQ, LSQ, vmask;                                                   \
        vmask = vec_lvsl(0, src);                                                               \
        MSQ = vec_ld(0, src);                                                                   \
        LSQ = vec_ld(15, src);                                                                  \
        vsrc = vec_perm(MSQ, LSQ, vmask);                                                       \
        if (vec_any_eq(vsrc, vc)) {                                                             \
            srcl = (uint32_t *)(src -srcoffset4);                                               \
            MYMEMCCPY_QUADWORD(dst, dstl, src, srcl, srcoffset, mask, c);                       \
        }                                                                                       \
        vec_st(vsrc, 0, (uint8_t *)dstl);                                                       \
    }

#define MYMEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c)    \
    while (len >= ALTIVECWORD_SIZE) {                                                               \
        MYMEMCCPY_SINGLE_ALTIVEC_WORD_ALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c)         \
        dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                                 \
        vec_dst(src, DST_CTRL(2,2,16), DST_CHAN_SRC);                                               \
        vec_dstst(dst, DST_CTRL(2,2,16), DST_CHAN_DEST);                                            \
    }

#define MYMEMCCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c)  \
    while (len >= ALTIVECWORD_SIZE) {                                                               \
        MYMEMCCPY_SINGLE_ALTIVEC_WORD_UNALIGNED(dst, dstl, src, srcl, srcoffset, mask, vc, c)       \
        dstl += 4; src += ALTIVECWORD_SIZE; len -= ALTIVECWORD_SIZE;                                 \
        vec_dst(src, DST_CTRL(2,2,16), DST_CHAN_SRC);                                               \
        vec_dstst(dst, DST_CTRL(2,2,16), DST_CHAN_DEST);                                            \
    }

#define MYMEMCCPY_REST_WORDS(dst, dstl, src, srcl, len, srcoffset, mask, c)         \
    while (len >= sizeof(uint32_t)) {                                               \
        uint32_t srct = 0;                                                          \
        MYMEMCCPY_SRC_TO_SRC_ALIGNED(src, srct, srcoffset);                         \
        MYMEMCCPY_SINGLE_WORD_FWD(dst, dstl, src, srcl, srct, srcoffset, mask, c);  \
        len -= sizeof(uint32_t);                                                    \
    }

#define MYNIBBLE_MEMCCPY_FWD(dst, src, c, len)    \
  switch (len) {                                  \
  case 3:                                         \
    if ((*dst++ = *src++) == c) return dst;       \
  case 2:                                         \
    if ((*dst++ = *src++) == c) return dst;       \
  case 1:                                         \
    if ((*dst++ = *src++) == c) return dst;       \
  }
