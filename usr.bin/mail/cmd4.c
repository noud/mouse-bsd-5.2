/*	$NetBSD: cmd4.c,v 1.4 2008/04/28 20:24:14 martin Exp $	*/

/*-
 * Copyright (c) 2006 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Anon Ymous.
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


#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)cmd3.c	8.2 (Berkeley) 4/20/95";
#else
__RCSID("$NetBSD: cmd4.c,v 1.4 2008/04/28 20:24:14 martin Exp $");
#endif
#endif /* not lint */

#include "rcv.h"
#include <util.h>
#include "extern.h"

/*
 * Mail -- a mail program
 *
 * Still more user commands.
 * XXX - should this be renamed smopts.c?
 */

#if 0	/* XXX - debugging stuff - to be removed */
void showname(struct name *);
void
showname(struct name *np)
{
	for (/*EMPTY*/; np; np = np->n_flink)
		(void)printf("np: %p  np->n_type: %d  np->n_name: '%s' (%p)\n",
		    np, np->n_type, np->n_name, np->n_name);
}

__unused
static void
showsmopts(struct smopts_s *sp)
{
	(void)printf("%s (%p)\n", sp->s_name, sp);
	showname(sp->s_smopts);
}
#endif	/* XXX - debugging stuff - to be removed */


static int
hashcase(const char *key)
{
	char *lckey;
	lckey = salloc(strlen(key) + 1);
	istrcpy(lckey, key);
	return hash(lckey);
}

static struct smopts_s *
findsmopts_core(const char *name)
{
	struct smopts_s *sh;

	for (sh = smoptstbl[hashcase(name)]; sh; sh = sh->s_link)
		if (strcasecmp(sh->s_name, name) == 0)
			return sh;
	return NULL;
}

/*
 * The exported smopts lookup routine.
 */
PUBLIC struct smopts_s *
findsmopts(const char *name, int top_only)
{
	const char *cp;
	struct smopts_s *sh;

	if ((sh = findsmopts_core(name)) != NULL)
		return sh;

	if (top_only)
		return NULL;

	for (cp = strchr(name, '@'); cp; cp = strchr(cp + 1, '.'))
		if ((sh = findsmopts_core(cp)) != NULL)
			return sh;

	return findsmopts_core(".");
}

static void
printsmopts(const char *name)
{
	struct smopts_s *sp;

	if ((sp = findsmopts(name, 1)) == NULL) {
		(void)printf("%s:\n", name);
		return;
	}
	(void)printf("%s:\t%s\n", sp->s_name, detract(sp->s_smopts, GSMOPTS));
}

static void
printsmoptstbl(void)
{
	struct smopts_s *sp;
	const char **argv, **ap;
	int h;
	int cnt;

	cnt = 1;
	for (h = 0; h < (int)sizeofarray(smoptstbl); h++ )
		for (sp = smoptstbl[h]; sp && sp->s_name != NULL; sp = sp->s_link)
			cnt++;

	argv = salloc(cnt * sizeof(*argv));
	ap = argv;
	for (h = 0; h < (int)sizeofarray(smoptstbl); h++ )
		for (sp = smoptstbl[h]; sp && sp->s_name != NULL; sp = sp->s_link)
			*ap++ = sp->s_name;
	*ap = NULL;
	sort(argv);
	for (ap = argv; *ap != NULL; ap++)
		printsmopts(*ap);
}

static struct name *
name_expand(char *sname, int ntype)
{
	struct grouphead *gh;
	struct name *np;

	if ((gh = findgroup(sname)) != NULL) {
		np = gexpand(NULL, gh, 0, ntype);
	}
	else {
		np = csalloc(1, sizeof(*np));
		np->n_name = sname;
		np->n_type = ntype;
	}
	return np;
}

static struct name *
ncalloc(char *str, int ntype)
{
	struct name *np;

	np = ecalloc(1, sizeof(*np));
	np->n_type = ntype;
	np->n_name = vcopy(str);
	return np;
}

static void
smopts_core(const char *sname, char **argv)
{
	struct smopts_s *sp;
	struct name *np, *t;
	int h;
	char **ap;

	if ((sp = findsmopts(sname, 1)) != NULL) {
		char *cp;
		cp = detract(sp->s_smopts, GSMOPTS);
		(void)printf("%s already defined as: %s\n", sname, cp);
		return;
	}
	h = hashcase(sname);
	sp = ecalloc(1, sizeof(*sp));
	sp->s_name = vcopy(sname);
	if (smoptstbl[h])
		sp->s_link = smoptstbl[h];
	smoptstbl[h] = sp;

	np = NULL;
	for (ap = argv + 1; *ap != NULL; ap++) {
		t = ncalloc(*ap, GSMOPTS);
		if (sp->s_smopts == NULL)
			sp->s_smopts = t;
		else
			np->n_flink = t;
		t->n_blink = np;
		np = t;
	}
}

/*
 * Takes a list of entries, expands them, and adds the results to the
 * smopts table.
 */
PUBLIC int
smoptscmd(void *v)
{
	struct name *np;
	char **argv = v;

	if (*argv == NULL) {
		printsmoptstbl();
		return 0;
	}
	np = name_expand(argv[0], GTO);

	if (argv[1] == NULL) {
		for (/*EMPTY*/; np; np = np->n_flink)
			printsmopts(np->n_name);
		return 0;
	}
	for (/*EMPTY*/; np; np = np->n_flink)
		smopts_core(np->n_name, argv);

	return 0;
}

static void
free_name(struct name *np)
{
	struct name *next_np;
	for (/*EMPTY*/; np; np = next_np) {
		next_np = np->n_flink;
		free(next_np);
	}
}

static void
delsmopts(char *name)
{
	struct smopts_s *sp;
	struct smopts_s **last_link;

	last_link = &smoptstbl[hashcase(name)];
	for (sp = *last_link; sp; sp = sp->s_link) {
		if (strcasecmp(sp->s_name, name) == 0) {
			*last_link = sp->s_link;
			free_name(sp->s_smopts);
			free(sp);
		}
	}
}

/*
 * Takes a list of entries and removes them from the smoptstbl.
 */
PUBLIC int
unsmoptscmd(void *v)
{
	struct name *np;
	char **argv, **ap;

	argv = v;
	for (ap = argv; *ap != NULL; ap++)
		for (np = name_expand(*ap, GTO); np; np = np->n_flink)
			delsmopts(np->n_name);
	return 0;
}
