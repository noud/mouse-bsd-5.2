# $Heimdal: Makefile.am 22413 2008-01-12 05:58:14Z lha $
# $NetBSD: Makefile.am,v 1.3 2008/03/22 08:37:23 mlelstv Exp $

include $(top_srcdir)/Makefile.am.common

bin_SCRIPTS = krb5-config

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = heimdal-gssapi.pc

man_MANS = krb5-config.1

if PKINIT
LIB_pkinit = -lhx509
endif

subst = sed	-e "s!@PACKAGE\@!$(PACKAGE)!g" \
		-e "s!@VERSION\@!$(VERSION)!g" \
		-e "s!@prefix\@!$(prefix)!g" \
		-e "s!@exec_prefix\@!$(exec_prefix)!g" \
		-e "s!@libdir\@!$(libdir)!g" \
		-e "s!@includedir\@!$(includedir)!g" \
		-e "s!@LIB_crypt\@!$(LIB_crypt)!g" \
		-e "s!@LIB_dbopen\@!$(LIB_dbopen)!g" \
		-e "s!@INCLUDE_hcrypto\@!$(INCLUDE_hcrypto)!g" \
		-e "s!@LIB_hcrypto_appl\@!$(LIB_hcrypto_appl)!g" \
		-e "s!@LIB_dlopen\@!$(LIB_dlopen)!g" \
		-e "s!@LIB_door_create\@!$(LIB_door_create)!g" \
		-e "s!@LIB_pkinit\@!$(LIB_pkinit)!g" \
		-e "s!@LIBS\@!$(LIBS)!g"

krb5-config: krb5-config.in
	$(subst) $(srcdir)/krb5-config.in > $@.new
	mv $@.new $@
	chmod +x $@

heimdal-gssapi.pc: heimdal-gssapi.pc.in
	$(subst) $(srcdir)/heimdal-gssapi.pc.in > $@.new
	mv $@.new $@

EXTRA_DIST = \
	$(man_MANS) \
	heimdal-build.sh \
	krb5-config.in \
	heimdal-gssapi.pc.in \
	kdc-log-analyze.pl

CLEANFILES = \
	krb5-config \
	krb5-config.new \
	heimdal-gssapi.pc \
	heimdal-gssapi.pc.new

