#	$NetBSD: Makefile,v 1.4 2007/10/17 19:58:41 garbled Exp $

S=	${.CURDIR}/../../../..

OBJS=	zbsdmod.o
SRCS=	zbsdmod.c
NOMAN=	yes

realall: ${OBJS}

cleandir:
	rm -f machine arm

.include <bsd.prog.mk>

afterinstall:
	${INSTALL_FILE} ${INSTALL_COPY} -o root -g wheel -m 644 \
		${OBJS} ${DESTDIR}/${BINDIR}

CFLAGS=		-Wall -Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=	-DMACHINE=\"${MACHINE}\" -DUTS_RELEASE=\"2.4.20\"
CPPFLAGS+=      -nostdinc -D_STANDALONE
CPPFLAGS+=	-I${.OBJDIR} -I${S}

.if !make(obj) && !make(clean) && !make(cleandir) && !make(release)
.BEGIN: machine arm
.NOPATH: machine arm

machine::
	-rm -f $@
	ln -s ${S}/arch/${MACHINE}/include $@

arm::
	-rm -f $@
	ln -s ${S}/arch/arm/include $@
.endif

release: check_RELEASEDIR
	${HOST_INSTALL_FILE} -m ${NONBINMODE} ${OBJS} \
	    ${RELEASEDIR}/${MACHINE}/installation
