dnl $Heimdal: acinclude.m4 13337 2004-02-12 14:19:16Z lha $
dnl $NetBSD: acinclude.m4,v 1.3 2008/03/22 08:36:49 mlelstv Exp $
dnl
dnl Only put things that for some reason can't live in the `cf'
dnl directory in this file.
dnl

dnl $xId: misc.m4,v 1.1 1997/12/14 15:59:04 joda Exp $
dnl
m4_define([upcase],`echo $1 | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`)dnl
