#	$NetBSD: Makefile,v 1.4.2.1 2008/11/07 23:23:35 snj Exp $

.include <bsd.own.mk>

LIB=	Xft

.PATH:	${X11SRCDIR.${LIB}}/src
.PATH:	${X11SRCDIR.${LIB}}/include/X11/Xft
.PATH:	${X11SRCDIR.${LIB}}/man
SRCS=	xftcolor.c xftcore.c xftdbg.c xftdpy.c xftdraw.c xftextent.c \
	xftfont.c xftfreetype.c xftglyphs.c xftinit.c xftlist.c \
	xftname.c xftrender.c xftstr.c xftswap.c xftxlfd.c

INCS=	Xft.h XftCompat.h
INCSDIR=${X11INCDIR}/X11/Xft

MAN=	Xft.3

CPPFLAGS+=	${X11FLAGS.THREADLIB}
CPPFLAGS+=	-DFREETYPE2 -I${DESTDIR}${X11INCDIR}/freetype2 -DXFREE86_FT2
CPPFLAGS+=	-I${DESTDIR}${X11INCDIR}/X11/Xft

LIBDPLIBS=	X11		${.CURDIR}/../libX11/dynamic \
		fontconfig	${.CURDIR}/../fontconfig/src \
		freetype	${.CURDIR}/../freetype \
		Xrandr		${.CURDIR}/../libXrandr \
		Xrender		${.CURDIR}/../libXrender

PKGDIST=	${LIB}

.include <bsd.x11.mk>
.include <bsd.lib.mk>
