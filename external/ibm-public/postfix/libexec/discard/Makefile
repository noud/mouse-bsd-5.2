#	$NetBSD: Makefile,v 1.1.2.2 2009/09/15 06:04:15 snj Exp $

NOMAN=	# defined

.include <bsd.own.mk>

PROG=	discard

DIST=	${NETBSDSRCDIR}/external/ibm-public/postfix/dist/src/${PROG}
.PATH:	${DIST}

SRCS	= discard.c

DPADD+= ${LIBPMASTER} ${LIBPGLOBAL} ${LIBPUTIL}
LDADD+= ${LIBPMASTER} ${LIBPGLOBAL} ${LIBPUTIL}

.include <bsd.prog.mk>
