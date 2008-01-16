/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#define rRTN      r3
#define rTMPa     r4
#define rTMPb     r5

#define vIN       v2
#define vMASK     v3
#define vRES      v0

.text                       /* section declaration - begin code */

.globl find_leftfirst_nzb
.globl find_rightfirst_nzb
/* .globl vec_find_leftfirst_nzb_mask */

find_leftfirst_nzb:
    /* count zero bits in r3 (argument) before the first non-zero bit */
    cntlzw        rTMPa, rRTN
    /* divide the result by 3, that should give the number of bytes
      before the first non-zero bit */
    srawi         rRTN, rTMPa, 3
    blr           /* return back to caller */


find_rightfirst_nzb:
    /* load zero into r4 */
    li            rTMPa, 0
    /* load argument in r3 (which is an address) and byte reverse the input */
    lwbrx         rTMPb, rTMPa, rRTN
    /* do the same as find_leftfirst_nzb() */
    cntlzw        rTMPa, rTMPb
    srawi         rRTN, rTMPa, 3
    /* we have to make sure that we subtract 4 from the result, because we byte-reversed
      the input */
    li            rTMPa, 3
    sub           rRTN, rTMPa, rRTN
    blr           /* return back to caller */

vec_find_leftfirst_nzb_mask_2:
    /* compare the vIN vector to vMASK, set the CR bit and store the result in vRES */
    vcmpequb.     vRES, vIN, vMASK
    bne-          cr6, vec_find_leftfirst_nzb_end
    blr
vec_find_leftfirst_nzb_end:
    /* if we got here, the vectors might be equal */
    li            rTMPa, 0
    stvx          vRES, rRTN, rTMPa
    li            rRTN, 1
    blr
