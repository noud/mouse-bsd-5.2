/*	$NetBSD: uinttoa.c,v 1.2 2003/12/04 16:23:37 drochner Exp $	*/

/*
 * uinttoa - return an asciized unsigned integer
 */
#include <stdio.h>

#include "lib_strbuf.h"
#include "ntp_stdlib.h"

char *
uinttoa(
	u_long uval
	)
{
	register char *buf;

	LIB_GETBUF(buf);

	(void) sprintf(buf, "%lu", (u_long)uval);
	return buf;
}
