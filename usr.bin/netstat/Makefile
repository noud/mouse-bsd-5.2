#	$NetBSD: Makefile,v 1.28 2007/05/28 12:06:29 tls Exp $
#	from: @(#)Makefile	8.1 (Berkeley) 6/12/93

.include <bsd.own.mk>

USE_FORT?= yes	# setgid

PROG=	netstat
SRCS=	atalk.c bpf.c fast_ipsec.c if.c inet.c inet6.c ipsec.c iso.c \
	main.c mbuf.c mroute.c mroute6.c show.c route.c tp_astring.c \
	unix.c
.PATH:	${NETBSDSRCDIR}/sys/netiso
BINGRP=	kmem
BINMODE=2555
LDADD=	-lkvm
DPADD=	${LIBKVM}
CPPFLAGS+= -DIPSEC

.if (${USE_INET6} != "no")
CPPFLAGS+= -DINET6
.endif

.include <bsd.prog.mk>
