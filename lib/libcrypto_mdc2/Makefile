#	$NetBSD: Makefile,v 1.1 2002/09/26 13:05:55 itojun Exp $

# RCSid:
#	Id: Makefile,v 1.33 1998/11/11 11:53:53 sjg Exp
#
#	@(#) Copyright (c) 1994 Simon J. Gerraty
#
#	This file is provided in the hope that it will
#	be of use.  There is absolutely NO WARRANTY.
#	Permission to copy, redistribute or otherwise
#	use this file is hereby granted provided that 
#	the above copyright notice and this notice are
#	left intact. 
#      
#	Please send copies of changes and bug-fixes to:
#	sjg@quick.com.au
#

NOPIC=		# defined
#NOPROFILE=	# defined
#NOLINT=	# defined

.include <bsd.own.mk>

# XXX There's a bit of work to do before we can enable warnings.
WARNS=		0

LIB=		crypto_mdc2
CPPFLAGS+=	-Dlib${LIB} -I. -I${OPENSSLSRC}/crypto -I${OPENSSLSRC}

CRYPTODIST=	${NETBSDSRCDIR}/crypto/dist

.include "${NETBSDSRCDIR}/crypto/Makefile.openssl"

.PATH: ${OPENSSLSRC} ${OPENSSLSRC}/crypto/mdc2 ${OPENSSLSRC}/crypto/evp

CPPFLAGS+=	-I${OPENSSLSRC}/crypto/mdc2
SRCS+=		mdc2dgst.c

# XXX
.if ${OBJECT_FMT} == "ELF"
AFLAGS+=	-DELF
.else
AFLAGS+=	-DOUT
.endif

.include <bsd.lib.mk>
