/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#ifdef HAVE_ALTIVEC_H
#include <altivec.h>

#include "libfreevec.h"
#include "macros/strcpy.h"

#ifdef VEC_GLIBC
int8_t *strcpy_old(int8_t *dstpp, const int8_t *srcpp)
{
#else
int8_t *vec_strcpy_old(int8_t *dstpp, const int8_t *srcpp)
{
#endif

  uint8_t *src = (uint8_t *)srcpp;
  uint8_t *dst = (uint8_t *)dstpp;

  vec_dst(src, DST_CTRL(2,2,32), DST_CHAN_SRC);
  vec_dstst(dst, DST_CTRL(2,2,32), DST_CHAN_DEST);

  MYSTRCPY_UNTIL_DEST_IS_ALTIVEC_ALIGNED_new(dstpp, dst, src);

  // Take the word-aligned long pointers of src and dest.
  uint8_t srcoffset4 = ((uint32_t)(src) & (sizeof(uint32_t)-1));
  uint32_t *dstl = (uint32_t *)(dst);
  const uint32_t *srcl = (uint32_t *)(src -srcoffset4);

  vector uint8_t v0 = vec_splat_u8(0);
  // Check for the alignment of src
  if (((uint32_t)(src) % ALTIVECWORD_SIZE) == 0)
    {
      // Now, both buffers are 16-byte aligned, just copy everything directly
      MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_ALIGNED(dstpp, dst, dstl, src, srcl, srcoffset4, v0);
    }
  else
    {
      // src is not 16-byte aligned so we have to a little trick with Altivec.
      MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_UNALIGNED(dstpp, dst, dstl, src, srcl, srcoffset4, v0);
    }
}

#ifdef VEC_GLIBC
int8_t *strcpy(int8_t *dstpp, const int8_t *srcpp)
{
#else
int8_t *vec_strcpy(int8_t *dstpp, const int8_t *srcpp)
{
#endif
  /*	int len = strlen(srcpp);
  	printf("strlen(srcpp) = %d\n", */
  uint8_t *src = (uint8_t *)srcpp;
  uint8_t *dst = (uint8_t *)dstpp;

  vec_dst(src, DST_CTRL(2,2,32), DST_CHAN_SRC);
  vec_dstst(dst, DST_CTRL(2,2,32), DST_CHAN_DEST);

  //MYSTRCPY_UNTIL_SRC_IS_ALTIVEC_ALIGNED_new(dstpp, dst, src);

  // Take the word-aligned long pointers of src and dest.
  // uint8_t dstoffset4 = ((uint32_t)(dst) & (sizeof(uint32_t)-1));
  // uint32_t *dstl = (uint32_t *)(dst) - dstoffset4;
  // const uint32_t *srcl = (uint32_t *)(src);

  //vector uint8_t v0 = vec_splat_u8(0);

  //MYSTRCPY_LOOP_SINGLE_ALTIVEC_WORD_new(dstpp, dst, dstl, src, srcl, srcoffset4, v0);
	return NULL;
}
#endif
