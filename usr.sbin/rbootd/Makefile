#	from: @(#)Makefile	8.1 (Berkeley) 6/4/93
#	$NetBSD: Makefile,v 1.12 2008/08/29 00:02:25 gmcgarry Exp $

USE_FORT?= yes	# network server

PROG=	rbootd
SRCS=	bpf.c conf.c parseconf.c rbootd.c rmpproto.c utils.c
MAN=	rbootd.8

LDADD+=	-lutil
DPADD+=	${LIBUTIL}

.include <bsd.own.mk>

.if defined(HAVE_GCC) && ${HAVE_GCC} == 4
COPTS.utils.c+=	-fno-strict-aliasing
.endif

.include <bsd.prog.mk>
