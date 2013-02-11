/*	$NetBSD: putenv.c,v 1.1.1.1.4.2 2011/01/06 21:42:11 riz Exp $	*/

#ifndef LINT
static const char rcsid[] = "Id: putenv.c,v 1.2 2005/04/27 04:56:11 sra Exp";
#endif

#include "port_before.h"
#include "port_after.h"

/*%
 * To give a little credit to Sun, SGI,
 * and many vendors in the SysV world.
 */

#if !defined(NEED_PUTENV)
int __bindcompat_putenv;
#else
int
putenv(char *str) {
	char *tmp;

	for (tmp = str; *tmp && (*tmp != '='); tmp++)
		;

	return (setenv(str, tmp, 1));
}
#endif

/*! \file */
