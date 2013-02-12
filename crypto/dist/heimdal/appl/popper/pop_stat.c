/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <popper.h>
__RCSID("$Heimdal: pop_stat.c 1707 1997-05-11 11:04:38Z assar $"
        "$NetBSD: pop_stat.c,v 1.2 2008/03/22 08:36:55 mlelstv Exp $");

/*
 *  stat:   Display the status of a POP maildrop to its client
 */

int
pop_stat (POP *p)
{
#ifdef DEBUG
    if (p->debug) pop_log(p,POP_DEBUG,"%d message(s) (%ld octets).",
			  p->msg_count-p->msgs_deleted,
			  p->drop_size-p->bytes_deleted);
#endif /* DEBUG */
    return (pop_msg (p,POP_SUCCESS,
		     "%d %ld",
		     p->msg_count-p->msgs_deleted,
		     p->drop_size-p->bytes_deleted));
}
