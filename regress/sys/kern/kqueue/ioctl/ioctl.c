/*	$NetBSD: ioctl.c,v 1.2 2008/04/28 20:23:06 martin Exp $	*/

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Luke Mewburn.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/event.h>
#include <sys/ioctl.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
	struct kfilter_mapping	km;
	int	kq;
	long	filter;
	char	*ep, buf[100];

	if (argc != 2)
		errx(1, "Usage: %s filter", argv[0]);
	
        kq = kqueue();
        if (kq < 0)
                err(1, "kqueue");

	filter = strtol(argv[1], &ep, 10);
	if (ep[0] == '\0') {		/* do number -> name */
		km.name = buf;
		km.len = sizeof(buf) - 1;
		km.filter = filter;
		if (ioctl(kq, KFILTER_BYFILTER, &km) == -1)
			err(1, "KFILTER_BYFILTER `%d'", km.filter);
		printf("Map %d -> %s\n", km.filter, km.name);
	} else {			/* do name -> number */
		km.name = argv[1];
		if (ioctl(kq, KFILTER_BYNAME, &km) == -1)
			err(1, "KFILTER_BYNAME `%s'", km.name);
		printf("Map %s -> %d\n", km.name, km.filter);
	}

	return (0);
}
