#	$NetBSD: Makefile,v 1.4.2.4 2011/01/06 05:20:26 riz Exp $

.include <bsd.own.mk>
.include <bsd.init.mk>

CPPFLAGS+=	-DELFSIZE=32
LIB=	ldd_elf32

# XXX Force one member
SRCS=	dummy.c

LIBISPRIVATE=	yes
.PATH: ${.CURDIR}/..

.ifdef MLIBDIR
CPPFLAGS+=	-DRTLD_ARCH_SUBDIR=\"${MLIBDIR}\"
.endif

.if ${MACHINE} != "alpha"
.include "../Makefile.elf"
.endif

.include <bsd.lib.mk>
