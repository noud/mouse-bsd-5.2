# $NetBSD: Makefile,v 1.2 2008/04/10 15:05:24 tsutsui Exp $

NOLINT= # defined

.include <bsd.own.mk>

DIST=		${NETBSDSRCDIR}/crypto/dist
.PATH: ${DIST}/heimdal/lib/krb5

WARNS?=  1

PROG= verify_krb5_conf
MAN= verify_krb5_conf.8

SRCS= verify_krb5_conf.c

CPPFLAGS+= -I.					\
	-I${DIST}/heimdal/lib/asn1		\
	-I${NETBSDSRCDIR}/include/heimdal	\
	-I${DESTDIR}/usr/include/krb5		\
	-DHAVE_CONFIG_H

.if (${USE_INET6} != "no")
CPPFLAGS+=-DHAVE_IPV6
.endif

VERS!=	cd ${NETBSDSRCDIR}/lib/libvers && ${PRINTOBJDIR}

LDADD+=	-lkrb5 -lhx509 -lcrypto -lasn1 -lcom_err \
	-L${VERS} -lvers -lroken -lcrypt
DPADD+=	${LIBKRB5} ${LIBHX509} ${LIBCRYPTO} ${LIBASN1} ${LIBCOM_ERR} \
	${VERS}/libvers.a ${LIBROKEN} ${LIBCRYPT}

.include <bsd.prog.mk>
