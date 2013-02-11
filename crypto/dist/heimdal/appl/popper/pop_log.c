/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <popper.h>
__RCSID("$Heimdal: pop_log.c 3609 1997-10-14 21:59:37Z joda $"
        "$NetBSD: pop_log.c,v 1.2 2008/03/22 08:36:55 mlelstv Exp $");

/* 
 *  log:    Make a log entry
 */

int
pop_log(POP *p, int stat, char *format, ...)
{
    char msgbuf[MAXLINELEN];
    va_list     ap;

    va_start(ap, format);
    vsnprintf(msgbuf, sizeof(msgbuf), format, ap);

    if (p->debug && p->trace) {
        fprintf(p->trace,"%s\n",msgbuf);
        fflush(p->trace);
    } else {
#ifdef KRB5
	krb5_log(p->context, p->logf, stat, "%s", msgbuf);
#else
        syslog (stat,"%s",msgbuf);
#endif
    }
    va_end(ap);

    return(stat);
}
