#	$NetBSD: conf.inc,v 1.7 2007/12/09 22:44:14 adrianp Exp $
#
#	@(#) Copyright (c) 1995 Simon J. Gerraty
#
#	SRCS extracted from src/crypto/dist/openssl/crypto/conf/Makefile
#

.PATH:	${OPENSSLSRC}/crypto/conf


CONF_SRCS = conf_err.c conf_lib.c conf_api.c conf_def.c conf_mod.c \
	conf_mall.c conf_sap.c
SRCS += ${CONF_SRCS}

.for cryptosrc in ${CONF_SRCS}
CPPFLAGS.${cryptosrc} = -I${OPENSSLSRC}/crypto/conf
.endfor
