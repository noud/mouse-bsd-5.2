#	$NetBSD: Makefile,v 1.22 2007/12/04 21:24:10 pooka Exp $
#

.include <bsd.own.mk>

USE_FORT?= yes	# data-driven bugs?

WARNS=		4

LIB=		puffs

# don't enable this unless you know what you're doing (or if you don't
# know what you're doing, it's still ok to enable this if you're me)
#LIBDPLIBS+=	pthread	${.CURDIR}/../libpthread
#CPPFLAGS+=	-DPUFFS_WITH_THREADS

SRCS=		puffs.c callcontext.c creds.c dispatcher.c flush.c	\
		framebuf.c null.c opdump.c paths.c pnode.c requests.c	\
		subr.c suspend.c
MAN=		puffs.3 puffs_cc.3 puffs_cred.3 puffs_flush.3		\
		puffs_framebuf.3 puffs_node.3 puffs_ops.3 puffs_path.3	\
		puffs_suspend.3
INCS=		puffs.h puffsdump.h
INCSDIR=	/usr/include
LINTFLAGS+=-S -w

.include <bsd.lib.mk>
