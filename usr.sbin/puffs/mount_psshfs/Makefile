#	$NetBSD: Makefile,v 1.3 2007/07/18 22:38:59 pooka Exp $

PROG=	mount_psshfs
SRCS=	psshfs.c fs.c node.c psbuf.c subr.c
LDADD+= -lpuffs -lutil
WARNS=2

MAN=	mount_psshfs.8

.include <bsd.prog.mk>
