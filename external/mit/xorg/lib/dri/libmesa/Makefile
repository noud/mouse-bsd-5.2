#	$NetBSD: Makefile,v 1.1.2.6 2010/04/08 23:14:27 snj Exp $

.include <bsd.own.mk>

LIB=		mesa_dri
LIBISMODULE=	yes

SHLIB_MAJOR=	0

MESA_SRC_MODULES=	glapi main math vbo tnl shader swrast slang ss

CPPFLAGS+=	-I${X11SRCDIR.MesaLib}/src/mesa/main \
		-I${X11SRCDIR.MesaLib}/src/mesa/glapi \
		-I${X11SRCDIR.MesaLib}/src/mesa/shader \
		-I${X11SRCDIR.MesaLib}/src/mesa/drivers/dri/common \
		-I${X11SRCDIR.MesaLib}/src/mesa \
		-I${X11SRCDIR.MesaLib}/include \
		-I${DESTDIR}${X11INCDIR}/drm \
		-I${DESTDIR}${X11INCDIR}/X11

CPPFLAGS+=	${X11FLAGS.THREADLIB} \
		-DPTHREADS -D_NETBSD_SOURCE -DUSE_EXTERNAL_DXTN_LIB=1 \
		-DHAVE_ALIAS
CFLAGS+=	-pthread

.include "../../libOSMesa/libmesa.mk"

.include <bsd.x11.mk>

LIBDIR=		${X11USRLIBDIR}/modules/dri

.include <bsd.lib.mk>
