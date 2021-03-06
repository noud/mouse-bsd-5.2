/*	$NetBSD: visual.c,v 1.7 2007/12/15 19:44:45 perry Exp $	*/

/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)visual.c	8.1 (Berkeley) 5/31/93";
#else
__RCSID("$NetBSD: visual.c,v 1.7 2007/12/15 19:44:45 perry Exp $");
#endif
#endif /* not lint */

#include <stdio.h>
#include "trek.h"
#include "getpar.h"

/*
**  VISUAL SCAN
**
**	A visual scan is made in a particular direction of three sectors
**	in the general direction specified.  This takes time, and
**	Klingons can attack you, so it should be done only when sensors
**	are out.
*/

static signed char Visdelta[8][2]
 = { { -1, -1 },
     { -1,  0 },
     { -1,  1 },
     {  0,  1 },
     {  1,  1 },
     {  1,  0 },
     {  1, -1 },
     {  0, -1 } };

/*ARGSUSED*/
void
visual(z)
	int z __unused;
{
	int		ix, iy;
	int		co;
	int vx;

	co = getintpar("direction");
	if (co < 0 || co > 360)
		return;
	vx = ((co + 22) / 45) & 7;
	ix = Ship.sectx + Visdelta[vx][0];
	iy = Ship.secty + Visdelta[vx][1];
	if (ix < 0 || ix >= NSECTS || iy < 0 || iy >= NSECTS)
		co = '?';
	else
		co = Sect[ix][iy];
	printf("%d,%d %c ", ix, iy, co);
	vx = (vx + 1) & 7;
	ix = Ship.sectx + Visdelta[vx][0];
	iy = Ship.secty + Visdelta[vx][1];
	if (ix < 0 || ix >= NSECTS || iy < 0 || iy >= NSECTS)
		co = '?';
	else
		co = Sect[ix][iy];
	printf("%c ", co);
	vx = (vx + 1) & 7;
	ix = Ship.sectx + Visdelta[vx][0];
	iy = Ship.secty + Visdelta[vx][1];
	if (ix < 0 || ix >= NSECTS || iy < 0 || iy >= NSECTS)
		co = '?';
	else
		co = Sect[ix][iy];
	printf("%c %d,%d\n", co, ix, iy);
	Move.time = 0.05;
	Move.free = 0;
}
