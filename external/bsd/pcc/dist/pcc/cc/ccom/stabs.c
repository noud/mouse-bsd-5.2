/*	$Id: stabs.c,v 1.1.1.1 2008/08/24 05:33:03 gmcgarry Exp $	*/

/*
 * Copyright (c) 2004 Anders Magnusson (ragge@ludd.luth.se).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Simple implementation of the "stabs" debugging format.
 * Not complete but at least makes it possible to set breakpoints,
 * examine simple variables and do stack traces.
 * Based on the stabs documentation that follows gdb.
 */

#include "pass1.h"

#ifdef STABS

#include <sys/types.h>
#include <stdarg.h>
#include <string.h>

#define	STABHASH	256
#define	INTNUM		1	/* internal number of type "int" */
#define	BIT2BYTE(x)	((x)/SZCHAR)

#ifndef STABLBL
#error macdefs.h must define STABLBL
#endif

/* defines taken from BSD <stab.h> */
#define N_GSYM          0x20    /* global symbol */
#define N_FUN           0x24    /* procedure name */
#define N_LCSYM         0x28    /* bss segment variable */
#define N_RSYM          0x40    /* register variable */
#define N_SLINE         0x44    /* text segment line number */
#define N_SO            0x64    /* main source file name */
#define N_LSYM          0x80    /* stack variable */
#define N_SOL           0x84    /* included source file name */
#define N_PSYM          0xa0    /* parameter variable */
#define N_LBRAC         0xc0    /* left bracket */
#define N_RBRAC         0xe0    /* right bracket */

/*
 * Local type mapping
 * Types are defined as a typeword, a dimension pointer (in the case
 * of arrays) and struct/union/enum declarations.
 * Function prototypes are ignored.
 */
static struct stabtype {
	struct stabtype *next;	/* linked list */
	TWORD type;		/* pcc type number */
	union dimfun *df;	/* dimension of arrays */
	struct suedef *sue;	/* struct/union/enum declarations */
	int num;		/* local type number */
} *stabhash[STABHASH];
static int ntypes;
static char *curfun;
static int stablbl = 10;

void ptype(char *name, int num, int inhnum, long long min, long long max);
struct stabtype *addtype(TWORD, union dimfun *, struct suedef *);
struct stabtype *findtype(TWORD t, union dimfun *df, struct suedef *sue);
void printtype(struct symtab *s, char *str, int len);
void cprint(int p2, char *fmt, ...);

#define	MAXPSTR	100

extern int isinlining;
#define savestabs isinlining

/*
 * Output type definitions for the stab debugging format.
 * Note that "int" is always internal number 1.
 */
void
stabs_init()
{
	struct stabtype *st;

#define	ADDTYPE(y) addtype(y, NULL, MKSUE(y))

	ptype("int", ADDTYPE(INT)->num, INTNUM, MIN_INT, MAX_INT);

	st = ADDTYPE(CHAR);
	ptype("char", st->num, st->num, 0, MAX_CHAR);
	ptype("short", ADDTYPE(SHORT)->num, INTNUM, MIN_SHORT, MAX_SHORT);
	ptype("long", ADDTYPE(LONG)->num, INTNUM, MIN_LONG, MAX_LONG);
	ptype("long long", ADDTYPE(LONGLONG)->num, INTNUM,
	     MIN_LONGLONG, MAX_LONGLONG);
	ptype("unsigned char", ADDTYPE(UCHAR)->num, INTNUM, 0, MAX_UCHAR);
	ptype("unsigned short", ADDTYPE(USHORT)->num, INTNUM, 0, MAX_USHORT);
	ptype("unsigned int", ADDTYPE(UNSIGNED)->num, INTNUM, 0, MAX_UNSIGNED);
	ptype("unsigned long", ADDTYPE(ULONG)->num, INTNUM, 0, MAX_ULONG);
	ptype("unsigned long long", ADDTYPE(ULONGLONG)->num, INTNUM,
	    0, MAX_ULONGLONG);

	ptype("float", ADDTYPE(FLOAT)->num, INTNUM, 4, 0);
	ptype("double", ADDTYPE(DOUBLE)->num, INTNUM, 8, 0);
	ptype("long double", ADDTYPE(LDOUBLE)->num, INTNUM, 12, 0);
	st = ADDTYPE(VOID);
	cprint(savestabs, "\t.stabs \"void:t%d=r%d\",%d,0,0,0\n",
	    st->num, st->num, N_LSYM);

}

/*
 * Print a type in stabs format
 */
void
ptype(char *name, int num, int inhnum, long long min, long long max)
{
	cprint(savestabs, "\t.stabs \"%s:t%d=r%d;%lld;%lld;\",%d,0,0,0\n",
	    name, num, inhnum, min, max, N_LSYM);
}

/*
 * Add a new local type to the hash table.
 * The search key is the (type, df, sue) triple.
 */
struct stabtype *
addtype(TWORD t, union dimfun *df, struct suedef *sue)
{
	struct stabtype *st;

	st = permalloc(sizeof(struct stabtype));
	st->type = t;
	st->df = df;
	st->sue = sue;
	st->num = ++ntypes;
	st->next = stabhash[t & (STABHASH-1)];
	stabhash[t & (STABHASH-1)] = st;
	return st;
}

/*
 * Search for a given type and return a type pointer (or NULL).
 */
struct stabtype *
findtype(TWORD t, union dimfun *df, struct suedef *sue)
{
	struct stabtype *st;
	union dimfun *dw, *dx;
	TWORD tw;

	st = stabhash[t & (STABHASH-1)];
	for (; st; st = st->next) {
		if (t != st->type || sue != st->sue)
			continue;
		/* Ok, type and sue matches, check dimensions */
		if (st->df == NULL)
			return st; /* no arrays, got match */
		dw = st->df;
		dx = df;
		tw = t;
		for (; tw > BTMASK; tw = DECREF(tw)) {
			if (ISARY(tw)) {
				if (dw->ddim == dx->ddim)
					dw++, dx++;
				else
					break;
			}
		}
		if (tw <= BTMASK)
			return st;
	}
	return NULL;
}

/*
 * Print current line number.
 */
void
stabs_line(int line)
{
#ifdef STAB_LINE_ABSOLUTE
	cprint(savestabs, "\t.stabn %d,0,%d," STABLBL "\n", N_SLINE, line, stablbl);
#else
	cprint(savestabs, "\t.stabn %d,0,%d," STABLBL "-%s\n", N_SLINE, line, stablbl, exname(curfun));
#endif
	cprint(1, STABLBL ":\n", stablbl++);
}

/*
 * Start of block.
 */
void
stabs_lbrac(int blklvl)
{
#ifdef STAB_LINE_ABSOLUTE
	cprint(savestabs, "\t.stabn %d,0,%d," STABLBL "\n", N_LBRAC, blklvl, stablbl);
#else
	cprint(savestabs, "\t.stabn %d,0,%d," STABLBL "-%s\n",
	    N_LBRAC, blklvl, stablbl, exname(curfun));
#endif
	cprint(1, STABLBL ":\n", stablbl++);
}

/*
 * End of block.
 */
void
stabs_rbrac(int blklvl)
{
#ifdef STAB_LINE_ABSOLUTE
	cprint(savestabs, "\t.stabn %d,0,%d," STABLBL "\n",
	    N_RBRAC, blklvl, stablbl);
#else
	cprint(savestabs, "\t.stabn %d,0,%d," STABLBL "-%s\n",
	    N_RBRAC, blklvl, stablbl, exname(curfun));
#endif
	cprint(1, STABLBL ":\n", stablbl++);
}

/*
 * Print current file and set mark.
 */
void
stabs_file(char *fname)
{
	static char *mainfile;

	if (mainfile == NULL)
		mainfile = fname; /* first call */
	cprint(savestabs, "\t.stabs	\"%s\",%d,0,0," STABLBL "\n",
	    fname, fname == mainfile ? N_SO : N_SOL, stablbl);
	cprint(savestabs, STABLBL ":\n", stablbl++);
}

/*
 * Print beginning of function.
 */
void
stabs_func(struct symtab *s)
{
	char str[MAXPSTR];

	curfun = s->soname;
	printtype(s, str, sizeof(str));
	cprint(savestabs, "\t.stabs	\"%s:%c%s\",%d,0,%d,%s\n",
	    curfun, s->sclass == STATIC ? 'f' : 'F', str,
	    N_FUN, BIT2BYTE(s->ssue->suesize), exname(curfun));
}

/*
 * Print a (complex) type.
 * Will also create subtypes.
 * Printed string is like "20=*21=*1".
 */
void
printtype(struct symtab *s, char *ostr, int len)
{
	struct stabtype *st;
	union dimfun *df = s->sdf;
	struct suedef *sue = s->ssue;
	TWORD t = s->stype;
	int op = 0;

	/* Print out not-yet-found types */
	if (ISFTN(t))
		t = DECREF(t);
	st = findtype(t, df, sue);
	while (st == NULL && t > BTMASK) {
		st = addtype(t, df, sue);
		op+=snprintf(ostr+op, len - op, "%d=", st->num);
		if (ISFTN(t))
			ostr[op++] = 'f';
		else if (ISPTR(t))
			ostr[op++] = '*';
		else if (ISARY(t)) {
			op+=snprintf(ostr+op, len - op, "ar%d;0;%d;", INTNUM, df->ddim-1);
		} else
			cerror("printtype: notype");
		if (ISARY(t))
			df++;
		t = DECREF(t);
		st = findtype(t, df, sue);
		if (op > MAXPSTR-10)
			cerror("printtype: too difficult expression");
	}
	/* print out basic type. may have to be entered in case of sue */
	snprintf(ostr+op, len - op, "%d", st == NULL ? 1 : st->num);
	/* snprintf here null-terminated the string */
}

void
stabs_newsym(struct symtab *s)
{
	extern int fun_inline;
	char *sname;
	char ostr[MAXPSTR];
	int suesize;

	if (ISFTN(s->stype))
		return; /* functions are handled separate */

	if (s->sclass == STNAME || s->sclass == UNAME || s->sclass == MOS ||
	    s->sclass == ENAME || s->sclass == MOU || s->sclass == MOE ||
	    s->sclass == TYPEDEF || (s->sclass & FIELD))
		return; /* XXX - fix structs */

	sname = s->soname;
	suesize = BIT2BYTE(s->ssue->suesize);
	if (suesize > 32767)
		suesize = 32767;
	else if (suesize < -32768)
		suesize = -32768;

	printtype(s, ostr, sizeof(ostr));
	switch (s->sclass) {
	case PARAM:
		cprint(savestabs, "\t.stabs \"%s:p%s\",%d,0,%d,%d\n", sname, ostr,
		    N_PSYM, suesize, BIT2BYTE(s->soffset));
		break;

	case AUTO:
		cprint(savestabs, "\t.stabs \"%s:%s\",%d,0,%d,%d\n", sname, ostr,
		    N_LSYM, suesize, BIT2BYTE(s->soffset));
		break;

	case STATIC:
		if (blevel)
			cprint(savestabs, "\t.stabs \"%s:V%s\",%d,0,%d," LABFMT "\n", sname, ostr,
			    N_LCSYM, suesize, s->soffset);
		else
			cprint(savestabs, "\t.stabs \"%s:S%s\",%d,0,%d,%s\n", sname, ostr,
			    N_LCSYM, suesize, exname(sname));
		break;

	case EXTERN:
	case EXTDEF:
		cprint(savestabs, "\t.stabs \"%s:G%s\",%d,0,%d,0\n", sname, ostr,
		    N_GSYM, suesize);
		break;

	case REGISTER:
		cprint(savestabs, "\t.stabs \"%s:r%s\",%d,0,%d,%d\n", sname, ostr,
		    N_RSYM, 1, s->soffset);
		break;
	case SNULL:
		if (fun_inline)
			break;
		/* FALLTHROUGH */
	default:
		cerror("fix stab_newsym; class %d", s->sclass);
	}
}

void
stabs_chgsym(struct symtab *s)
{
}

/*
 * define a struct.
 */
void
stabs_struct(struct symtab *p, struct suedef *sue)
{
}

static struct foo {
	struct foo *next;
	char *str;
} *foopole;

void
cprint(int p2, char *fmt, ...)
{
	extern int inftn;
	va_list ap;
	char *str;

	if (isinlining)
		return; /* XXX do not save any inline functions currently */

	va_start(ap, fmt);
	if (p2) {
		str = tmpvsprintf(fmt, ap);
		str = newstring(str, strlen(str)); /* XXX - for inlines */
		if (inftn == 0) {
			struct foo *w = tmpalloc(sizeof(struct foo));
			w->str = str;
			w->next = foopole;
			foopole = w;
		} else {
			while (foopole)
				send_passt(IP_ASM, foopole->str),
				    foopole = foopole->next;
			send_passt(IP_ASM, str);
		}
	} else
		vprintf(fmt, ap);
	va_end(ap);
}

#endif
