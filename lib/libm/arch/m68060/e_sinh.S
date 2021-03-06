/* $NetBSD: e_sinh.S,v 1.5 2000/05/16 19:07:35 is Exp $ */

/*
 * FPLSP wrapper for __ieee754_sinh
 * DO NOT EDIT - this file is automatically generated.
 */

#include <machine/asm.h>

ENTRY(__ieee754_sinh)
#ifdef __SVR4_ABI__
	jbra _C_LABEL(__fplsp060_01d0)
#else
	movel %sp@(8),%sp@-
	movel %sp@(8),%sp@-
	jbsr _C_LABEL(__fplsp060_01d0)
	fmoved %fp0,%sp@
	movel %sp@+,%d0
	movel %sp@+,%d1
	rts
#endif
