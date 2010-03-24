/***************************************************************************
 *   Copyright (C) 2009 by Konstantinos Margaritis   *
 *   markos@codex.gr   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// This portion was stolen from SDL Altivec patches by Ryan C. Gordon :-)
#define DST_CHAN_1 1
#define DST_CHAN_2 2

#define DST_CTRL(size, count, stride) (((size) << 24) | ((count) << 16) | (stride))

#define READ_PREFETCH_START1(addr)    vec_dstt(addr, DST_CTRL(2,2,32), DST_CHAN_1)
#define READ_PREFETCH_START2(addr)    vec_dstt(addr, DST_CTRL(2,2,32), DST_CHAN_2)
#define WRITE_PREFETCH_START1(addr)   vec_dststt(addr, DST_CTRL(2,2,32), DST_CHAN_1)
#define WRITE_PREFETCH_START2(addr)   vec_dststt(addr, DST_CTRL(2,2,32), DST_CHAN_2)
#define PREFETCH_STOP1                vec_dss(DST_CHAN_1)
#define PREFETCH_STOP2                vec_dss(DST_CHAN_2)