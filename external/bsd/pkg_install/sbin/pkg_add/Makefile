# $NetBSD: Makefile,v 1.1.6.1 2009/05/30 16:40:34 snj Exp $
# Original from FreeBSD, no rcs id.

PROG= pkg_add
SRCS= main.c perform.c

.include <bsd.init.mk>

.PATH:	${DIST}/add

CPPFLAGS+=	-DMACHINE_ARCH=\"${MACHINE_ARCH}\"

.include <bsd.prog.mk>
