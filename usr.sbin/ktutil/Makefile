# $NetBSD: Makefile,v 1.19 2008/04/10 15:42:06 tsutsui Exp $

NOLINT= # defined

.include <bsd.own.mk>

USE_FORT?= yes	# network client

DIST=		${NETBSDSRCDIR}/crypto/dist
.PATH: ${DIST}/heimdal/admin

WARNS?=  1

PROG= ktutil

SRCS=		 add.c				\
		 change.c			\
		 copy.c				\
		 get.c				\
		 ktutil.c			\
		 list.c				\
		 purge.c			\
		 remove.c			\
		 rename.c			\
		 ktutil-commands.c

CPPFLAGS+= -I.					\
	-I${DIST}/heimdal/admin			\
	-I${NETBSDSRCDIR}/include/heimdal	\
	-I${DESTDIR}/usr/include/kadm5		\
	-I${DESTDIR}/usr/include/krb5		\
	-DHAVE_CONFIG_H

.if (${USE_INET6} != "no")
CPPFLAGS+=-DHAVE_IPV6
.endif

VERS!=	cd ${NETBSDSRCDIR}/lib/libvers && ${PRINTOBJDIR}

LDADD+=	-lkadm5clnt -lkrb5 -lhx509 -lcrypto -lasn1 \
	-lcom_err -lsl -ledit -ltermcap \
	-L${VERS} -lvers -lroken -lcrypt -lutil
DPADD+=	${LIBKADM5CLNT} ${LIBKRB5} ${LIBHX509} ${LIBCRYPTO} ${LIBASN1} \
	${LIBCOM_ERR} ${LIBSL} ${LIBEDIT} ${LIBTERMCAP} \
	${VERS}/libvers.a ${LIBROKEN} ${LIBCRYPT} ${LIBUTIL}

MAN = ktutil.8

.include <bsd.prog.mk>
