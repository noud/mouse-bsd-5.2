#	$NetBSD: Makefile,v 1.1.1.1.8.1 2009/09/17 04:24:29 snj Exp $

.include <bsd.own.mk>

PROG=	xwd
SRCS=	xwd.c clientwin.c dsimple.c list.c multiVis.c

CPPFLAGS+=-I${X11SRCDIR./xlsfonts}
.PATH:	${X11SRCDIR./xlsfonts}

LDADD+=	-lXmuu -lXt -lSM -lICE -lXext -lX11
DPADD+=	${LIBXMUU} ${LIBXT} ${LIBSM} ${LIBICE} ${LIBXEXT} ${LIBX11}

.PATH:	${X11SRCDIR.${PROG}}

.include <bsd.x11.mk>
.include <bsd.prog.mk>
