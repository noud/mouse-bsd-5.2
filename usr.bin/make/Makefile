#	$NetBSD: Makefile,v 1.44 2008/10/06 22:09:21 joerg Exp $
#	@(#)Makefile	5.2 (Berkeley) 12/28/90

PROG=	make
SRCS=	arch.c buf.c compat.c cond.c dir.c for.c hash.c job.c main.c \
	make.c parse.c str.c suff.c targ.c trace.c var.c util.c 
SRCS+=	lstAppend.c lstAtEnd.c lstAtFront.c lstClose.c lstConcat.c \
	lstDatum.c lstDeQueue.c lstDestroy.c lstDupl.c lstEnQueue.c \
	lstFind.c lstFindFrom.c lstFirst.c lstForEach.c lstForEachFrom.c \
	lstInit.c lstInsert.c lstIsAtEnd.c lstIsEmpty.c lstLast.c \
	lstMember.c lstNext.c lstOpen.c lstRemove.c lstReplace.c lstSucc.c
SRCS += lstPrev.c

.PATH:	${.CURDIR}/lst.lib
WARNS=4
.if make(install)
SUBDIR=	PSD.doc
.endif
.if make(obj) || make(clean)
SUBDIR+= unit-tests
.endif

.include <bsd.prog.mk>
.include <bsd.subdir.mk>

# provide a clue as to what we are using
BUILD_DATE!= date +%Y%m%d
MAKE_VERSION:= netbsd-${BUILD_DATE}
CPPFLAGS_main.o:= "-DMAKE_VERSION=\"${MAKE_VERSION}\""
CPPFLAGS+= ${CPPFLAGS_${.TARGET}} -DMAKE_NATIVE
main.o:	${OBJS:Nmain.o} ${MAKEFILE}
COPTS.var.c+= -Wno-cast-qual

.ifdef TOOLDIR
# this is a native netbsd build, 
# use libutil rather than the local emalloc etc.
CPPFLAGS+= -DUSE_EMALLOC
LDADD+=-lutil
DPADD+=${LIBUTIL}
.endif

# A simple unit-test driver to help catch regressions
accept test:
	cd ${.CURDIR}/unit-tests && ${.MAKE:S,^./,${.CURDIR}/,} TEST_MAKE=${TEST_MAKE:U${.OBJDIR}/${PROG:T}} ${.TARGET}
