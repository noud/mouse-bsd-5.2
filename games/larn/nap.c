/*	$NetBSD: nap.c,v 1.5 1997/10/18 20:03:36 christos Exp $	*/

/* nap.c		 Larn is copyrighted 1986 by Noah Morgan. */
#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: nap.c,v 1.5 1997/10/18 20:03:36 christos Exp $");
#endif				/* not lint */

#include <poll.h>
#include "header.h"
#include "extern.h"

/*
 *	routine to take a nap for n milliseconds
 */
void nap(int x)
{
	if (x <= 0)
		return;		/* eliminate chance for infinite loop */
	lflush();
	poll(0,0,x);
}
