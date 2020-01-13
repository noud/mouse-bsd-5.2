#	$NetBSD: Makefile.inc,v 1.5 2008/08/06 20:14:08 pooka Exp $

USE_FORT?= yes	# network client

.if exists(${.CURDIR}/../../Makefile.inc)
.include "${.CURDIR}/../../Makefile.inc"
.endif
