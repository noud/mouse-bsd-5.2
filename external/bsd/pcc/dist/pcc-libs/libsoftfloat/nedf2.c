/* $NetBSD: nedf2.c,v 1.1.1.1 2008/08/24 05:34:48 gmcgarry Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

flag __nedf2(float64, float64);

flag
__nedf2(float64 a, float64 b)
{

	/* libgcc1.c says a != b */
	return !float64_eq(a, b);
}
