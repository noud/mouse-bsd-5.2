# $NetBSD: Makefile,v 1.3.2.3 2009/10/10 19:53:24 sborrill Exp $

XMODULEDIR=	${X11USRLIBDIR}/modules/extensions

.include "../../../Makefile.serverlib"
.include "../../../Makefile.servermod"

LIB=	dri2

.PATH:	${X11SRCDIR.xorg-server}/hw/xfree86/${LIB}
SRCS=	dri2.c dri2ext.c

CPPFLAGS+=	-I${X11SRCDIR.xorg-server}/hw/xfree86/${LIB} \
		-I${X11SRCDIR.xorg-server}/include \
		-I${X11SRCDIR.xorg-server}/hw/xfree86/common \
		-I${X11SRCDIR.xorg-server}/hw/xfree86/os-support \
		-I${X11SRCDIR.xorg-server}/hw/xfree86/os-support/bus \
		-I${X11SRCDIR.xorg-server}/xfixes \
		-I${X11SRCDIR.xorg-server}/glx \
		-I${X11SRCDIR.xorg-server}/GL/include \
		-I${X11SRCDIR.xorg-server}/modes \
		-I${X11SRCDIR.xorg-server}/ddc \
		-I${X11SRCDIR.xorg-server}/i2c \
		-I${X11SRCDIR.xorg-server}/parser \
		-I${X11SRCDIR.xorg-server}/ramdac \
		-I${X11SRCDIR.xorg-server}/Xext \
		-I${X11SRCDIR.MesaLib}/include \
		-I${DESTDIR}${X11INCDIR}/X11 \
		-I${DESTDIR}${X11INCDIR}/X11/dri \
		-I${DESTDIR}${X11INCDIR}/drm \
		-I${DESTDIR}${X11INCDIR}/pixman-1 \
		-I${DESTDIR}${X11INCDIR}/xorg

CPPFLAGS+=	-DXF86DRI \
		${X11FLAGS.DIX}

LIBDPLIBS=	drm ${.CURDIR}/../../../../../lib/libdrm

COPTS.dri2ext.c+=	-Wno-error

.include <bsd.x11.mk>
LIBDIR=	${XMODULEDIR}
.include <bsd.lib.mk>
