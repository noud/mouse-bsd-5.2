/* $NetBSD: chmod.c,v 1.34 2008/07/20 00:52:39 lukem Exp $ */
/* mods by mouse: add -F, make -h actually work,
   handle moded symlinks, handle ls-style symbolic modes */

/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__COPYRIGHT(
"@(#) Copyright (c) 1989, 1993, 1994\
 The Regents of the University of California.  All rights reserved.");
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)chmod.c	8.8 (Berkeley) 4/1/94";
#else
__RCSID("$NetBSD: chmod.c,v 1.34 2008/07/20 00:52:39 lukem Exp $");
#endif
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fts.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int	main(int, char *[]);
void	usage(void);

int
main(int argc, char *argv[])
{
	FTS *ftsp;
	FTSENT *p;
	mode_t *set;
	int Fflag, Hflag, Lflag, Rflag, ch, fflag, fts_options, hflag, rval;
	char *mode;
	int (*change_mode)(const char *, mode_t);
	int (*get_mode)(const char *, struct stat *);

	setprogname(argv[0]);
	(void)setlocale(LC_ALL, "");

	Fflag = Hflag = Lflag = Rflag = fflag = hflag = 0;
	while ((ch = getopt(argc, argv, "FHLPRXfghorstuwx")) != -1)
		switch (ch) {
		case 'F':
			Fflag = 1;
			break;
		case 'H':
			Hflag = 1;
			Lflag = 0;
			break;
		case 'L':
			Lflag = 1;
			Hflag = 0;
			break;
		case 'P':
			Hflag = Lflag = 0;
			break;
		case 'R':
			Rflag = 1;
			break;
		case 'f':		/* XXX: undocumented. */
			fflag = 1;
			break;
		case 'h':
			/*
			 * In System V the -h option causes chmod to
			 * change the mode of the symbolic link.
			 * 4.4BSD's symbolic links didn't have modes,
			 * so it was an undocumented noop.  In NetBSD
			 * 1.3, lchmod(2) is introduced and this
			 * option does real work.
			 */
			hflag = 1;
			break;
		/*
		 * XXX
		 * "-[rwx]" are valid mode commands.  If they are the entire
		 * argument, getopt has moved past them, so decrement optind.
		 * Regardless, we're done argument processing.
		 */
		case 'g': case 'o': case 'r': case 's':
		case 't': case 'u': case 'w': case 'X': case 'x':
			if (argv[optind - 1][0] == '-' &&
			    argv[optind - 1][1] == ch &&
			    argv[optind - 1][2] == '\0')
				--optind;
			goto done;
		case '?':
		default:
			usage();
		}
done:	argv += optind;
	argc -= optind;

	/* If we have a mode beginning with --, like --x--x--x,
	   getopt will mistake it for a -- end-of-flags indicator. */
	if ( (argv[-1][0] == '-') &&
	     (argv[-1][1] == '-') &&
	     (strlen(argv[-1]) == 9) ) {
		argc ++;
		argv --;
	}

	if (argc < 2)
		usage();

	if (Fflag && Rflag)
		errx(EXIT_FAILURE,"-F is incompatible with -R");

	fts_options = FTS_PHYSICAL;
	if (Rflag) {
		if (Hflag)
			fts_options |= FTS_COMFOLLOW;
		if (Lflag) {
			fts_options &= ~FTS_PHYSICAL;
			fts_options |= FTS_LOGICAL;
		}
	} else if (!hflag)
		fts_options |= FTS_COMFOLLOW;
	change_mode = hflag ? &lchmod : &chmod;
	get_mode = hflag ? &lstat : &stat;

	mode = *argv;
	if ((set = setmode(mode)) == NULL) {
		err(EXIT_FAILURE, "Cannot set file mode `%s'", mode);
		/* NOTREACHED */
	}

	if (Fflag) {
		int i;
		int rv;
		rval = 0;
		for (i=1;argv[i];i++) {
			struct stat stb;
			rv = (*get_mode)(argv[i],&stb);
			if (rv == 0)
				rv = (*change_mode)(argv[i],getmode(set,stb.st_mode));
			if (rv < 0) {
				warn(argv[i]);
				rval = 1;
			}
		}
		exit(rval);
	}

	if ((ftsp = fts_open(++argv, fts_options, 0)) == NULL) {
		err(EXIT_FAILURE, "fts_open");
		/* NOTREACHED */
	}
	for (rval = 0; (p = fts_read(ftsp)) != NULL;) {
		switch (p->fts_info) {
		case FTS_D:
			if (!Rflag)
				(void)fts_set(ftsp, p, FTS_SKIP);
			break;
		case FTS_DNR:			/* Warn, chmod, continue. */
			warnx("%s: %s", p->fts_path, strerror(p->fts_errno));
			rval = 1;
			break;
		case FTS_DP:			/* Already changed at FTS_D. */
			continue;
		case FTS_ERR:			/* Warn, continue. */
		case FTS_NS:
			warnx("%s: %s", p->fts_path, strerror(p->fts_errno));
			rval = 1;
			continue;
		case FTS_SL:			/* Ignore, unless -h. */
		case FTS_SLNONE:
			/*
			 * The only symlinks that end up here are ones that
			 * don't point to anything and ones that we found
			 * doing a physical walk.
			 */
			if (!hflag)
				continue;
			/* else */
			/* FALLTHROUGH */
		default:
			break;
		}
		if ((*change_mode)(p->fts_accpath,
		    getmode(set, p->fts_statp->st_mode)) && !fflag) {
			warn("%s", p->fts_path);
			rval = 1;
		}
	}
	if (errno) {
		err(EXIT_FAILURE, "fts_read");
		/* NOTREACHED */
	}
	exit(rval);
	/* NOTREACHED */
}

void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: %s [-R [-H | -L | -P]] [-F] [-h] mode file ...\n",
	    getprogname());
	exit(1);
	/* NOTREACHED */
}
