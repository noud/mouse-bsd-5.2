# $Heimdal: Makefile.am 14164 2004-08-26 11:55:29Z joda $
# $NetBSD: Makefile.am,v 1.2 2008/03/22 08:37:04 mlelstv Exp $

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += $(INCLUDE_krb4)

lib_LIBRARIES = @EXTRA_LIB45@

EXTRA_LIBRARIES = lib45.a

lib45_a_SOURCES = get_ad_tkt.c mk_req.c 45_locl.h
