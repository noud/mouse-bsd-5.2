#	$NetBSD: Makefile,v 1.4 2003/09/23 23:21:12 lukem Exp $

.include <bsd.own.mk>

XF86SRC=	${X11SRCDIR.xc}/programs/Xserver/hw/xfree86

BUILDSYMLINKS=	${XF86SRC}/common/xf86Module.h		xf86Module.h \
		${XF86SRC}/common/xf86Opt.h		xf86Opt.h \
		${XF86SRC}/os-support/xf86_libc.h	xf86_libc.h \
		${XF86SRC}/os-support/xf86_ansic.h	xf86_ansic.h \
		${XF86SRC}/os-support/bus/xf86Pci.h	xf86Pci.h \
		${XF86SRC}/xf86Version.h		xf86Version.h \
		${XF86SRC}/common/compiler.h		compiler.h

XTODO=		osdep.h

.include <bsd.x11.mk>
.include <bsd.prog.mk>
