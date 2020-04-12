/*****************************************************************************
 * Free42 -- an HP-42S calculator simulator
 * Copyright (C) 2004-2020  Thomas Okken
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see http://www.gnu.org/licenses/.
 *****************************************************************************/

#ifndef FREE42_H
#define FREE42_H 1


#ifndef BCD_MATH
#define _REENTRANT 1
#include <math.h>
#endif

typedef short           int2;
typedef unsigned short  uint2;
typedef int             int4;
typedef unsigned int    uint4;
typedef long long       int8;
typedef unsigned long long uint8;
typedef unsigned int    uint;

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
/* I have tested big-endian state file compatibility in Fedora 12
 * running on qemu-system-ppc. I found that I needed to explicitly
 * set F42_BIG_ENDIAN for it to work; apparently the __BYTE_ORDER__
 * macro is not defined in such old compilers.
 * Also see the comment about setting BID_BIG_ENDIAN in
 * gtk/build-intel-lib.sh.
 */
#define F42_BIG_ENDIAN 1
#endif

/* Magic number "24kF" for the state file. */
#define FREE42_MAGIC 0x466b3432
#define FREE42_MAGIC_STR "24kF"


#endif
