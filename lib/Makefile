#	$NetBSD: Makefile,v 1.128.2.1 2009/01/16 21:41:10 bouyer Exp $
#	from: @(#)Makefile	5.25.1.1 (Berkeley) 5/7/91

.include <bsd.own.mk>

SUBDIR=	csu libc .WAIT libarch \
	libbsdmalloc libbluetooth libbz2 \
	libcompat libcrypt \
	libcurses libevent libintl libkvm libl libm \
	libossaudio libpcap libpci libpmc libposix libprop libpthread \
	libpthread_dbg libpuffs libresolv librmt librpcsvc librt libsdp \
	libterm libusbhid libutil libwrap liby libz

.if (${MKATF} != "no")
SUBDIR+= libatf-c libatf-c++
.endif

# libform and libmenu need libcurses
SUBDIR+=	.WAIT libform libmenu 

.if (${MKSKEY} != "no")
SUBDIR+= libskey
.endif

# XXX Crypto bits must be done before libtelnet.

.if (${MKCRYPTO} != "no")
# OpenSSL libraries.
SUBDIR+= libcrypto libdes

.if (${MKCRYPTO_IDEA} != "no")
SUBDIR+= libcrypto_idea
.endif	# MKCRYPTO_IDEA != no

.if (${MKCRYPTO_MDC2} != "no")
SUBDIR+= libcrypto_mdc2
.endif	# MKCRYPTO_MDC2 != no

.if (${MKCRYPTO_RC5} != "no")
SUBDIR+= libcrypto_rc5
.endif	# MKCRYPTO_RC5 != no

.endif	# MKCRYPTO != no

.if (${MKKERBEROS} != "no")
# Heimdal Kerberos 5 libraries
SUBDIR+= .WAIT libroken libvers libcom_err .WAIT libasn1
SUBDIR+= .WAIT libhx509

.endif	# MKKERBEROS != no

SUBDIR+=	libtelnet

# IPv6/IPsec
SUBDIR+=	libipsec

# I18N modules
SUBDIR+=	i18n_module


#
# Libraries that depend upon any listed previously
# (and those that depend upon these [and ...])
#
#==================== 1st library dependency barrier ====================
SUBDIR+=	.WAIT

SUBDIR+=	libedit		# depends on libterm

SUBDIR+=	libmagic	# depends on libz

SUBDIR+=	libradius	# depends on libcrypto - only if ${MKCRYPTO}

.if (${MKISCSI} != "no")
SUBDIR+=	libiscsi	# depends on libpthread
.endif

.if (${MKCRYPTO} != "no")
SUBDIR+=	libssl		# depends on libcrypto
SUBDIR+=	libssh		# depends on libcrypto
.endif

.if (${MKKERBEROS} != "no")
SUBDIR+=	libkrb5		# depends on libcrypto and more
.endif

SUBDIR+=	librefuse	# depends on libpuffs

# XXX These bind9/ISC libraries should depend on each other but the
# XXX dependency ordering requires considerable investigation.
# XXX Please preserve the order below so we know the order ISC links
# XXX them in in their Makefiles.
SUBDIR+= libbind9 libdns liblwres libisccfg libisccc libisc # dep. libpthread

#==================== 2nd library dependency barrier ====================
SUBDIR+=	.WAIT

.if (${MKKERBEROS} != "no")
# Heimdal Kerberos 5 libraries depending on libkrb5 and more
SUBDIR+=	libheimntlm
SUBDIR+=	.WAIT libgssapi libhdb
SUBDIR+=	.WAIT libkadm5srv libkadm5clnt libkafs libsl libss
.endif

.if (${MKLDAP} != "no")
				# depends on libcrypto - only if ${MKCRYPTO}
SUBDIR+=	../external/bsd/openldap/lib
.endif

#==================== 3rd library dependency barrier ====================
SUBDIR+=	.WAIT

.if (${MKPAM} != "no")
SUBDIR+=	libpam		# depends on libkrb5, libkafs and more
.endif

.include <bsd.subdir.mk>
