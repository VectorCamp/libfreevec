/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#define POSINWORD   0x00010203

// Taken from http://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
#define HAS_ZERO_BYTE(v)  (~((((v & 0x7F7F7F7FUL) + 0x7F7F7F7FUL) | v) | 0x7F7F7F7FUL))
// This one was used initially but it won't catch all cases
#define HAS_ZERO_BYTE_3(v)  (((v) - 0x01010101UL) & (~(v)) & 0x80808080UL)
#define HAS_ZERO_BYTE_2(v)    (((v) + 0xFEFEFEFFUL) & ((~v) & 0x80808080UL))

// Helper macro, fills a vector with a given char
#define FILL_VECTOR(vecname, p)                  \
  vector uint8_t vecname = vec_lde(0, &p);       \
  vecname = vec_splat(vecname, 0);

#define FIND_LEFTFIRST_ZB_IN_WORD(res, x, m)  \
{                                             \
  __asm__("\t"                                \
    "rlwinm   %%r0,%[input],7,0,31\n\n"       \
    "andc     %[mask],%[mask],%%r0\n\t"       \
    "cntlzw   %%r0, %[mask]\n\t"              \
    "srawi    %[output], %%r0, 3"             \
    : [output] "=r" (res)  /* outputs   */    \
    : [input] "r"(x), [mask] "r"(m)           \
    : "r0"                 /* clobbers: */    \
  );                                          \
}

#define FIND_LEFTFIRST_IN_WORD(res, x)       \
{                                            \
  __asm__("\t"                               \
    "cntlzw   %%r0, %[input]\n\t"            \
    "srawi    %[output], %%r0, 3"            \
    : [output] "=r" (res)  /* outputs:  */   \
    : [input] "r" (x)      /* inputs:   */   \
    : "r0"                 /* clobbers: */   \
  );                                         \
}

#define FIND_RIGHTFIRST_IN_WORD(res, x)      \
{                                            \
  asm("\t"                                   \
    "li       %%r4, 0\n\t"                   \
    "lwbrx    %%r5, %%r4, %[input]\n\t"      \
    "cntlzw   %%r4, %%r5\n\t"                \
    "srawi    %[output], %%r4, 3\n\t"        \
    "li       %%r4, 3\n\t"                   \
    "sub      %[output], %%r4, %[output]\n"  \
    : [output] "=r" (res)  /* outputs:  */   \
    : [input] "r" (x)      /* inputs:   */   \
    : "r4" "r5"            /* clobbers: */   \
  );                                         \
}

/*
inline uint32_t vec_find_leftfirst_nzb_mask(vector uint8_t v,
                vector uint8_t mask, uint32_t *lw) {
  int res = 0;
  __asm__("\t"
      "vcmpequb.  %[vin], %[vin], %[vmask]\n\t"
      "beq+       cr6, has_no_mask_byte\n\t"
      "li         %%r4, 0\n\t"
      "stvx       %[vin], %[vcopy], %%r4\n\t"
      "li         %[output], 1\n"
      "has_no_mask_byte:\n\t"
    : [output] "+r" (res)
    : [vin] "v"(v), [vmask] "v"(mask), [vcopy] "r"(lw)
    : "r4"
  );
  return res;
}*/

size_t find_leftfirst_in_word(uint32_t x);
size_t find_rightfirst_in_word(uint32_t x);

uint32_t find_leftfirst_nzb( uint32_t x);
uint32_t find_rightfirst_nzb( uint32_t *x);

