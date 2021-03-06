#	$NetBSD: Makefile,v 1.2.8.2 2009/09/17 04:24:31 snj Exp $

NOLINT=		1	# XTODO: tess.ln SIGSEGVs lint :(

.include <bsd.own.mk>

LIB=		GLU
LIBISCXX=	yes

GLUDIR=		${X11SRCDIR.MesaLib}/src/glu/sgi

SRCS.libutil=	error.c glue.c mipmap.c project.c quad.c registry.c
.PATH:		${GLUDIR}/libutil

SRCS.libtess=	dict.c geom.c memalloc.c mesh.c normal.c priorityq.c \
		render.c sweep.c tess.c tessmono.c
.PATH:		${GLUDIR}/libtess

SRCS.interface=	bezierEval.cc bezierPatch.cc bezierPatchMesh.cc \
		glcurveval.cc glinterface.cc glrenderer.cc \
		glsurfeval.cc incurveeval.cc insurfeval.cc
.PATH:		${GLUDIR}/libnurbs/interface
.for fn in ${SRCS.interface}
CPPFLAGS.${fn}+=-I${GLUDIR}/libnurbs/internals \
		-I${GLUDIR}/libnurbs/nurbtess \
		-DLIBRARYBUILD
.endfor

SRCS.internals=	arc.cc arcsorter.cc arctess.cc backend.cc \
		basiccrveval.cc basicsurfeval.cc bin.cc bufpool.cc \
		cachingeval.cc ccw.cc coveandtiler.cc curve.cc \
		curvelist.cc curvesub.cc dataTransform.cc displaylist.cc \
		flist.cc flistsorter.cc hull.cc intersect.cc \
		knotvector.cc mapdesc.cc mapdescv.cc maplist.cc \
		mesher.cc monoTriangulationBackend.cc monotonizer.cc \
		mycode.cc nurbsinterfac.cc nurbstess.cc patch.cc \
		patchlist.cc quilt.cc reader.cc renderhints.cc \
		slicer.cc sorter.cc splitarcs.cc subdivider.cc \
		tobezier.cc trimline.cc trimregion.cc trimvertpool.cc \
		uarray.cc varray.cc
.PATH:		${GLUDIR}/libnurbs/internals
.for fn in ${SRCS.internals}
CPPFLAGS.${fn}+=-I${GLUDIR}/libnurbs/internals \
		-I${GLUDIR}/libnurbs/nurbtess \
		-I${GLUDIR}/libutil \
		-DLIBRARYBUILD -D_EXTENSIONS_
.endfor

SRCS.nurbtess=	directedLine.cc gridWrap.cc monoChain.cc monoPolyPart.cc \
		monoTriangulation.cc partitionX.cc partitionY.cc \
		polyDBG.cc polyUtil.cc primitiveStream.cc quicksort.cc \
		rectBlock.cc sampleComp.cc sampleCompBot.cc \
		sampleCompRight.cc sampleCompTop.cc sampleMonoPoly.cc \
		sampledLine.cc searchTree.cc
.PATH:		${GLUDIR}/libnurbs/nurbtess
.for fn in ${SRCS.nurbtess}
CPPFLAGS.${fn}+=-I${GLUDIR}/libnurbs/internals \
		-I${GLUDIR}/libnurbs/nurbtess \
		-DLIBRARYBUILD
.endfor

CPPFLAGS+=	-I${GLUDIR}/include
CPPFLAGS+=	-DPTHREADS -DUSE_XSHM -DHZ=100 -DNDEBUG -DLIBRARYBUILD

SRCS=		${SRCS.libutil} ${SRCS.libtess} \
		${SRCS.interface} ${SRCS.internals} ${SRCS.nurbtess}

CFLAGS+=	-fno-strict-aliasing -fvisibility=hidden -pthread

.PATH:		${X11SRCDIR.MesaLib}/include/GL
INCS=	glu.h glu_mangle.h
INCSDIR=${X11INCDIR}/GL

LIBDPLIBS=	Xext	${.CURDIR}/../libXext \
		X11	${.CURDIR}/../libX11/dynamic \
		GL	${.CURDIR}/../libGL \
		Xxf86vm	${.CURDIR}/../libXxf86vm \
		Xfixes	${.CURDIR}/../libXfixes \
		Xdamage	${.CURDIR}/../libXdamage \
		drm	${.CURDIR}/../libdrm

LDADD+=		-lstdc++ -lm
DPADD+=		${LIBSTDCXX} ${LIBM}

PKGCONFIG=	glu
PKGDIST.glu=	${X11SRCDIR.MesaLib}/src/glu
PKGCONFIG_VERSION.glu=	7.4.2

.include <bsd.x11.mk>
.include <bsd.lib.mk>
