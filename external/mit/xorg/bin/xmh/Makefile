#	$NetBSD: Makefile,v 1.1.1.1 2008/07/29 05:01:22 mrg Exp $

.include <bsd.own.mk>

PROG=	xmh
SRCS=	bbox.c command.c compfuncs.c folder.c init.c main.c \
	menu.c mlist.c msg.c pick.c popup.c screen.c toc.c \
	tocfuncs.c tocutil.c tsource.c util.c viewfuncs.c \
	miscfuncs.c

CPPFLAGS+=	-DHAS_VFORK -DXVENDORNAME=${XVENDORNAME} \
		-DXORG_RELEASE=${XORG_RELEASE}

FILES=	black6 box6
FILESDIR=${X11INCDIR}/X11/bitmaps

APPDEFS=Xmh.ad

LDADD+=	-lXaw -lXmu -lXt -lSM -lICE -lXpm -lXext -lX11
DPADD+=	${LIBXAW} ${LIBXMU} ${LIBXT} ${LIBSM} ${LIBICE} \
	${LIBXPM} ${LIBXEXT} ${LIBX11}

.PATH:	${X11SRCDIR.${PROG}}

.include <bsd.x11.mk>
.include <bsd.prog.mk>
