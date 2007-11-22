/***************************************************************************
 *   Copyright (C) 2005 by CODEX                                           *
 *   markos@debian.gr                                                      *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#define rRTN      r3
#define rTMPa     r4
#define rTMPb     r5


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
    blr           /* return back to caller */
