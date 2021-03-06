# $Heimdal: Makefile.am 22304 2007-12-14 12:18:18Z lha $
# $NetBSD: Makefile.am,v 1.2 2008/03/22 08:37:06 mlelstv Exp $

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += $(INCLUDE_krb4)

WFLAGS += $(WFLAGS_NOIMPLICITINT)

DEFS = @DEFS@

## this is horribly ugly, but automake/libtool doesn't allow us to
## unconditionally build shared libraries, and it does not allow us to
## link with non-installed libraries

KAFS=$(top_builddir)/lib/kafs/.libs/libkafs.a
KAFS_S=$(top_builddir)/lib/kafs/.libs/libkafs.so

if KRB5
L = \
	$(KAFS)						\
	$(top_builddir)/lib/krb5/.libs/libkrb5.a	\
	$(top_builddir)/lib/asn1/.libs/libasn1.a	\
	$(LIB_krb4)					\
	$(LIB_hcrypto_a)					\
	$(LIB_com_err_a)				\
	$(top_builddir)/lib/roken/.libs/libroken.a	\
	$(LIB_getpwnam_r)				\
	-lc

L_shared = \
	$(KAFS_S)					\
	$(top_builddir)/lib/krb5/.libs/libkrb5.so	\
	$(top_builddir)/lib/asn1/.libs/libasn1.so	\
	$(LIB_krb4)					\
	$(LIB_hcrypto_so)					\
	$(LIB_com_err_so)				\
	$(top_builddir)/lib/roken/.libs/libroken.so	\
	$(LIB_getpwnam_r)				\
	-lc

MOD = libsia_krb5.so

else

L = \
	$(KAFS)						\
	$(top_builddir)/lib/kadm/.libs/libkadm.a	\
	$(top_builddir)/lib/krb/.libs/libkrb.a		\
	$(LIB_hcrypto_a)		\
	$(top_builddir)/lib/com_err/.libs/libcom_err.a	\
	$(top_builddir)/lib/roken/.libs/libroken.a	\
	$(LIB_getpwnam_r)				\
	-lc

L_shared = \
	$(KAFS_S)					\
	$(top_builddir)/lib/kadm/.libs/libkadm.so	\
	$(top_builddir)/lib/krb/.libs/libkrb.so		\
	$(LIB_hcrypto_so)		\
	$(top_builddir)/lib/com_err/.libs/libcom_err.so	\
	$(top_builddir)/lib/roken/.libs/libroken.so	\
	$(LIB_getpwnam_r)				\
	-lc

MOD = libsia_krb4.so

endif

foodir = $(libdir)
foo_DATA = $(MOD)

LDFLAGS = @LDFLAGS@ -rpath $(libdir) -Wl,-hidden -Wl,-exported_symbol -Wl,siad_\*

SRCS = sia.c posix_getpw.c sia_locl.h
OBJS = sia.o posix_getpw.o

libsia_krb5.so: $(OBJS)
	@if test -f $(top_builddir)/lib/krb5/.libs/libkrb5.a; then \
		echo "$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L)`"; \
		$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L)`; \
	elif test -f $(top_builddir)/lib/krb5/.libs/libkrb5.so; then \
		echo "$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L_shared)`"; \
		$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L_shared)`; \
	else \
		echo "missing libraries"; exit 1; \
	fi
	ostrip -x $@

libsia_krb4.so: $(OBJS)
	@if test -f $(top_builddir)/lib/krb/.libs/libkrb.a; then \
		echo "$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L)`"; \
		$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L)`; \
	elif test -f $(top_builddir)/lib/krb/.libs/libkrb.so; then \
		echo "$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L_shared)`"; \
		$(CC) -shared -o $@ `$(SHELL) $(srcdir)/make-rpath $(LDFLAGS) $(OBJS) $(L_shared)`; \
	else \
		echo "missing libraries"; exit 1; \
	fi
	ostrip -x $@

CLEANFILES = $(MOD) $(OBJS) so_locations

SUFFIXES += .c .o

# XXX inline COMPILE since automake wont add it

.c.o:
	$(CC) $(DEFS) $(DEFAULT_AM_CPPFLAGS) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) \
	-c `test -f '$<' || echo '$(srcdir)/'`$<

EXTRA_DIST = sia.c sia_locl.h posix_getpw.c \
	krb4_matrix.conf krb4+c2_matrix.conf \
	krb5_matrix.conf krb5+c2_matrix.conf \
	security.patch \
	make-rpath $(SRCS)
