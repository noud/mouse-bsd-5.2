# $NetBSD: Makefile,v 1.2 2008/04/10 15:05:23 tsutsui Exp $

NOLINT= # defined

.include <bsd.own.mk>

DIST=		${NETBSDSRCDIR}/crypto/dist
.PATH: ${DIST}/heimdal/lib/gssapi

WARNS?=  1

PROG= gss
# no man page yet
MAN=

SRCS= \
	gss.c				\
	gss-commands.c

CPPFLAGS+= -I.					\
	-I${DIST}/heimdal/lib/gssapi		\
	-I${NETBSDSRCDIR}/include/heimdal	\
	-I${DESTDIR}/usr/include/krb5		\
	-DHAVE_CONFIG_H

.if (${USE_INET6} != "no")
CPPFLAGS+=-DHAVE_IPV6
.endif

VERS!=	cd ${NETBSDSRCDIR}/lib/libvers && ${PRINTOBJDIR}

LDADD+=	-lgssapi -lkrb5 -lheimntlm -lhx509 -lsl \
	-lasn1 -lcrypto -lcom_err -L${VERS} -lvers \
	-lroken -lcrypt -lutil -ledit -ltermcap
DPADD+=	${LIBGSSAPI} ${LIBKRB5} ${LIBHEIMNTLM} ${LIBHX509} ${LIBSL} \
	${LIBASN1} ${LIBCRYPTO} ${LIBCOM_ERR} ${VERS}/libvers.a \
	${LIBROKEN} ${LIBCRYPT} ${LIBUTIL} ${LIBEDIT} ${LIBTERMCAP}

.include <bsd.prog.mk>
