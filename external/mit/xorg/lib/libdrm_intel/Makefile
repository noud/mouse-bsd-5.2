#      $NetBSD: Makefile,v 1.2.2.3 2009/10/10 19:53:22 sborrill Exp $

.include <bsd.own.mk>

LIB=	drm_intel
.PATH: ${X11SRCDIR.drm}/libdrm/intel

SRCS=	intel_bufmgr.c intel_bufmgr_fake.c intel_bufmgr_gem.c mm.c

CPPFLAGS+=	-I${DESTDIR}${X11INCDIR}/drm
CPPFLAGS+=	-I${DESTDIR}${X11INCDIR}/X11 -I${X11SRCDIR.drm}/libdrm

INCS=	intel_bufmgr.h
INCSDIR=${X11INCDIR}

# XXX not yet
#PKGCONFIG=	libdrm_intel
#PKGDIST=	libdrm

.include <bsd.x11.mk>
.include <bsd.lib.mk>
