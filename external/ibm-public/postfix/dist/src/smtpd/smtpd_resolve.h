/*	$NetBSD: smtpd_resolve.h,v 1.1.1.1.2.3 2011/01/07 01:24:14 riz Exp $	*/

/*++
/* NAME
/*	smtpd_resolve 3h
/* SUMMARY
/*	caching resolve client
/* SYNOPSIS
/*	include <smtpd_resolve.h>
/* DESCRIPTION
/* .nf

 /*
  * Global library.
  */
#include <resolve_clnt.h>

 /*
  * External interface.
  */
extern void smtpd_resolve_init(int);
extern const RESOLVE_REPLY *smtpd_resolve_addr(const char *);

/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*
/*	TLS support originally by:
/*	Lutz Jaenicke
/*	BTU Cottbus
/*	Allgemeine Elektrotechnik
/*	Universitaetsplatz 3-4
/*	D-03044 Cottbus, Germany
/*--*/
