#	$NetBSD: Makefile,v 1.12 2008/08/29 00:02:24 gmcgarry Exp $

USE_FORT?= yes	# data-driven bugs?

PROG=		gzip
MAN=		gzip.1 gzexe.1 zdiff.1 zforce.1 zgrep.1 zmore.1 znew.1

DPADD=		${LIBZ} ${LIBBZ2}
LDADD=		-lz -lbz2
WARNS=		3

SCRIPTS=	gzexe zdiff zforce zgrep zmore znew

MLINKS+=	gzip.1 gunzip.1 \
		gzip.1 gzcat.1 \
		gzip.1 zcat.1 \
		zdiff.1 zcmp.1 \
		zgrep.1 zegrep.1 \
		zgrep.1 zfgrep.1

LINKS+=		${BINDIR}/gzip ${BINDIR}/gunzip \
		${BINDIR}/gzip ${BINDIR}/gzcat \
		${BINDIR}/gzip ${BINDIR}/zcat \
		${BINDIR}/zdiff ${BINDIR}/zcmp \
		${BINDIR}/zgrep ${BINDIR}/zegrep \
		${BINDIR}/zgrep ${BINDIR}/zfgrep

.include <bsd.prog.mk>

.if (defined(HAVE_GCC) && ${HAVE_GCC} == 4) || defined(HAVE_PCC)
COPTS.gzip.c+=	-Wno-pointer-sign
.endif
