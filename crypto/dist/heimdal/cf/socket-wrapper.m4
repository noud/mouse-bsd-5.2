dnl $Heimdal: socket-wrapper.m4 18077 2006-09-12 17:33:07Z lha $
dnl $NetBSD: socket-wrapper.m4,v 1.1 2008/03/22 08:36:58 mlelstv Exp $
dnl
AC_DEFUN([samba_SOCKET_WRAPPER], [

AC_ARG_ENABLE(socket-wrapper,
	AS_HELP_STRING([--enable-socket-wrapper],
		[use sambas socket-wrapper for testing]))

AM_CONDITIONAL(have_socket_wrapper, test "x$enable_socket_wrapper" = xyes)dnl

if test "x$enable_socket_wrapper" = xyes ; then
       AC_DEFINE(SOCKET_WRAPPER_REPLACE, 1,
               [Define if you want to use samba socket wrappers.])
fi

])
