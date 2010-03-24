/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the LGPL license                       *
 *   See http://www.gnu.org/copyleft/lesser.html                           *
 ***************************************************************************/

#ifndef LIBFREEVEC_H
#define LIBFREEVEC_H

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"

#define SIMD_PACKETSIZE		16
#define QUADPACKET_SIZE		64
// log2(64) = 6
#define LOG_QUADPACKETSIZE	6

#define MACROFILE prefetch.h

#include "common.h"

#ifdef SIMD_ENGINE
	#define PREFETCH_MACROS_INC MAKEINC(SIMD_ENGINE)
	#define PREFETCH_MACROS_H MAKESTR(PREFETCH_MACROS_INC)
	#include PREFETCH_MACROS_H
#else
	#define READ_PREFETCH_START1(addr)
	#define READ_PREFETCH_START2(addr)
	#define WRITE_PREFETCH_START1(addr)
	#define WRITE_PREFETCH_START2(addr)
	#define PREFETCH_STOP1
	#define PREFETCH_STOP2
#endif
#undef MACROFILE

#endif
