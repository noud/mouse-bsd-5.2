#	$NetBSD: Makefile,v 1.40 2008/09/13 15:43:07 tsutsui Exp $
#

.include <bsd.own.mk>

TARGETS+=	release

.ifndef ALLNOTES
.if exists(${RELEASEMACHINE}/Makefile)
SUBDIR=	${RELEASEMACHINE}
.endif
.else
SUBDIR+= acorn26
SUBDIR+= acorn32
SUBDIR+= alpha
SUBDIR+= amd64
SUBDIR+= amiga
SUBDIR+= arc
SUBDIR+= atari
SUBDIR+= cats
SUBDIR+= evbarm
SUBDIR+= evbppc
SUBDIR+= hp300
SUBDIR+= hp700
SUBDIR+= hpcmips
SUBDIR+= hpcsh
SUBDIR+= i386
# SUBDIR+= iyonix	# not yet
SUBDIR+= landisk
SUBDIR+= mac68k
SUBDIR+= macppc
SUBDIR+= mmeye
SUBDIR+= mvme68k
SUBDIR+= news68k
SUBDIR+= newsmips
SUBDIR+= next68k
SUBDIR+= pmax
SUBDIR+= prep
SUBDIR+= sandpoint
SUBDIR+= sgimips
SUBDIR+= shark
SUBDIR+= sparc
SUBDIR+= sparc64
SUBDIR+= sun2
SUBDIR+= sun3
SUBDIR+= vax
SUBDIR+= x68k
SUBDIR+= xen
.endif

.include <bsd.subdir.mk>
