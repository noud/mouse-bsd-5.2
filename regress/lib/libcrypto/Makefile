#	$NetBSD: Makefile,v 1.7 2005/11/25 20:07:14 christos Exp $

.include <bsd.own.mk>

SUBDIR+=bf bn cast des dh dsa ec ecdh ecdsa engine hmac lhash \
	md2 md4 md5 rand rc2 rc4 ripemd rsa sha sha1 x509v3
#SUBDIR+=conf threads

.if ${MKCRYPTO_IDEA} != "no"
SUBDIR+=idea
.endif
.if ${MKCRYPTO_RC5} != "no"
SUBDIR+=rc5
.endif
.if ${MKCRYPTO_MDC2} != "no"
SUBDIR+=mdc2
.endif

.include <bsd.subdir.mk>
