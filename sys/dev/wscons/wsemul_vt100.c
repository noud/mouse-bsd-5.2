/* $NetBSD: wsemul_vt100.c,v 1.30.64.1 2012/03/21 21:27:36 jdc Exp $ */

/*
 * Copyright (c) 1998
 *	Matthias Drochner.  All rights reserved.
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
 *
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: wsemul_vt100.c,v 1.30.64.1 2012/03/21 21:27:36 jdc Exp $");

#include "opt_wsmsgattrs.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/malloc.h>
#include <sys/fcntl.h>

#include <dev/wscons/wsconsio.h>
#include <dev/wscons/wsdisplayvar.h>
#include <dev/wscons/wsemulvar.h>
#include <dev/wscons/wsemul_vt100var.h>
#include <dev/wscons/ascii.h>

void	*wsemul_vt100_cnattach(const struct wsscreen_descr *, void *,
			       int, int, long);
void	*wsemul_vt100_attach(int console, const struct wsscreen_descr *,
			     void *, int, int, void *, long);
void	wsemul_vt100_output(void *cookie, const u_char *data, u_int count, int);
void	wsemul_vt100_detach(void *cookie, u_int *crowp, u_int *ccolp);
void	wsemul_vt100_resetop(void *, enum wsemul_resetops);
#ifdef WSDISPLAY_CUSTOM_OUTPUT
static void wsemul_vt100_getmsgattrs(void *, struct wsdisplay_msgattrs *);
static void wsemul_vt100_setmsgattrs(void *, const struct wsscreen_descr *,
                                     const struct wsdisplay_msgattrs *);
#endif /* WSDISPLAY_CUSTOM_OUTPUT */

const struct wsemul_ops wsemul_vt100_ops = {
	"vt100",
	wsemul_vt100_cnattach,
	wsemul_vt100_attach,
	wsemul_vt100_output,
	wsemul_vt100_translate,
	wsemul_vt100_detach,
	wsemul_vt100_resetop,
#ifdef WSDISPLAY_CUSTOM_OUTPUT
	wsemul_vt100_getmsgattrs,
	wsemul_vt100_setmsgattrs,
#else
	NULL,
	NULL,
#endif
};

struct wsemul_vt100_emuldata wsemul_vt100_console_emuldata;

static void wsemul_vt100_init(struct wsemul_vt100_emuldata *,
			      const struct wsscreen_descr *,
			      void *, int, int, long);

static void wsemul_vt100_output_normal(struct wsemul_vt100_emuldata *,
				       u_char, int);
static void wsemul_vt100_output_c0c1(struct wsemul_vt100_emuldata *,
				     u_char, int);
static void wsemul_vt100_nextline(struct wsemul_vt100_emuldata *);
typedef u_int vt100_handler(struct wsemul_vt100_emuldata *, u_char);

static vt100_handler
wsemul_vt100_output_esc,
wsemul_vt100_output_csi,
wsemul_vt100_output_scs94,
wsemul_vt100_output_scs94_percent,
wsemul_vt100_output_scs96,
wsemul_vt100_output_scs96_percent,
wsemul_vt100_output_esc_hash,
wsemul_vt100_output_esc_spc,
wsemul_vt100_output_string,
wsemul_vt100_output_string_esc,
wsemul_vt100_output_dcs,
wsemul_vt100_output_dcs_dollar;

#define	VT100_EMUL_STATE_NORMAL		0	/* normal processing */
#define	VT100_EMUL_STATE_ESC		1	/* got ESC */
#define	VT100_EMUL_STATE_CSI		2	/* got CSI (ESC[) */
#define	VT100_EMUL_STATE_SCS94		3	/* got ESC{()*+} */
#define	VT100_EMUL_STATE_SCS94_PERCENT	4	/* got ESC{()*+}% */
#define	VT100_EMUL_STATE_SCS96		5	/* got ESC{-./} */
#define	VT100_EMUL_STATE_SCS96_PERCENT	6	/* got ESC{-./}% */
#define	VT100_EMUL_STATE_ESC_HASH	7	/* got ESC# */
#define	VT100_EMUL_STATE_ESC_SPC	8	/* got ESC<SPC> */
#define	VT100_EMUL_STATE_STRING		9	/* waiting for ST (ESC\) */
#define	VT100_EMUL_STATE_STRING_ESC	10	/* waiting for ST, got ESC */
#define	VT100_EMUL_STATE_DCS		11	/* got DCS (ESC P) */
#define	VT100_EMUL_STATE_DCS_DOLLAR	12	/* got DCS<p>$ */

vt100_handler *vt100_output[] = {
	wsemul_vt100_output_esc,
	wsemul_vt100_output_csi,
	wsemul_vt100_output_scs94,
	wsemul_vt100_output_scs94_percent,
	wsemul_vt100_output_scs96,
	wsemul_vt100_output_scs96_percent,
	wsemul_vt100_output_esc_hash,
	wsemul_vt100_output_esc_spc,
	wsemul_vt100_output_string,
	wsemul_vt100_output_string_esc,
	wsemul_vt100_output_dcs,
	wsemul_vt100_output_dcs_dollar,
};

static void
wsemul_vt100_init(struct wsemul_vt100_emuldata *edp,
	const struct wsscreen_descr *type, void *cookie, int ccol, int crow,
	long defattr)
{
	int error;

	edp->emulops = type->textops;
	edp->emulcookie = cookie;
	edp->scrcapabilities = type->capabilities;
	edp->nrows = type->nrows;
	edp->ncols = type->ncols;
	edp->crow = crow;
	edp->ccol = ccol;

	/* The underlying driver has already allocated a default and simple
	 * attribute for us, which is stored in defattr.  We try to set the
	 * values specified by the kernel options below, but in case of
	 * failure we fallback to the value given by the driver. */

	if (type->capabilities & WSSCREEN_WSCOLORS) {
		edp->msgattrs.default_attrs = WS_DEFAULT_COLATTR |
		    WSATTR_WSCOLORS;
		edp->msgattrs.default_bg = WS_DEFAULT_BG;
		edp->msgattrs.default_fg = WS_DEFAULT_FG;

		edp->msgattrs.kernel_attrs = WS_KERNEL_COLATTR |
		    WSATTR_WSCOLORS;
		edp->msgattrs.kernel_bg = WS_KERNEL_BG;
		edp->msgattrs.kernel_fg = WS_KERNEL_FG;
	} else {
		edp->msgattrs.default_attrs = WS_DEFAULT_MONOATTR;
		edp->msgattrs.default_bg = edp->msgattrs.default_fg = 0;

		edp->msgattrs.kernel_attrs = WS_KERNEL_MONOATTR;
		edp->msgattrs.kernel_bg = edp->msgattrs.kernel_fg = 0;
	}

	error = (*edp->emulops->allocattr)(cookie,
					   edp->msgattrs.default_fg,
					   edp->msgattrs.default_bg,
					   edp->msgattrs.default_attrs,
					   &edp->defattr);
	if (error) {
		edp->defattr = defattr;
		/* XXX This assumes the driver has allocated white on black
		 * XXX as the default attribute, which is not always true.
		 * XXX Maybe we need an emulop that, given an attribute,
		 * XXX (defattr) returns its flags and colors? */
		edp->msgattrs.default_attrs = 0;
		edp->msgattrs.default_bg = WSCOL_BLACK;
		edp->msgattrs.default_fg = WSCOL_WHITE;
	} else {
		if (edp->emulops->replaceattr != NULL)
			(*edp->emulops->replaceattr)(cookie, defattr,
			                             edp->defattr);
	}

#if defined(WS_KERNEL_CUSTOMIZED)
	/* Set up kernel colors, in case they were customized by the user;
	 * otherwise default to the colors specified for the console.
	 * In case of failure, we use console colors too; we can assume
	 * they are good as they have been previously allocated and
	 * verified. */
	error = (*edp->emulops->allocattr)(cookie,
					   edp->msgattrs.kernel_fg,
					   edp->msgattrs.kernel_bg,
					   edp->msgattrs.kernel_attrs,
					   &edp->kernattr);
	if (error)
#endif
	edp->kernattr = edp->defattr;
}

void *
wsemul_vt100_cnattach(const struct wsscreen_descr *type, void *cookie,
	int ccol, int crow, long defattr)
{
	struct wsemul_vt100_emuldata *edp;

	edp = &wsemul_vt100_console_emuldata;
	wsemul_vt100_init(edp, type, cookie, ccol, crow, defattr);
#ifdef DIAGNOSTIC
	edp->console = 1;
#endif
	edp->cbcookie = NULL;

	edp->tabs = 0;
	edp->dblwid = 0;
	edp->dw = 0;
	edp->dcsarg = 0;
	edp->isolatin1tab = edp->decgraphtab = edp->dectechtab = 0;
	edp->nrctab = 0;
	wsemul_vt100_reset(edp);
	return (edp);
}

void *
wsemul_vt100_attach(int console, const struct wsscreen_descr *type,
	void *cookie, int ccol, int crow, void *cbcookie, long defattr)
{
	struct wsemul_vt100_emuldata *edp;

	if (console) {
		edp = &wsemul_vt100_console_emuldata;
#ifdef DIAGNOSTIC
		KASSERT(edp->console == 1);
#endif
	} else {
		edp = malloc(sizeof *edp, M_DEVBUF, M_WAITOK);
		wsemul_vt100_init(edp, type, cookie, ccol, crow, defattr);
#ifdef DIAGNOSTIC
		edp->console = 0;
#endif
	}
	edp->cbcookie = cbcookie;

	edp->tabs = malloc(edp->ncols, M_DEVBUF, M_NOWAIT);
	edp->dblwid = malloc(edp->nrows, M_DEVBUF, M_NOWAIT|M_ZERO);
	edp->dw = 0;
	edp->dcsarg = malloc(DCS_MAXLEN, M_DEVBUF, M_NOWAIT);
	edp->isolatin1tab = malloc(128 * sizeof(int), M_DEVBUF, M_NOWAIT);
	edp->decgraphtab = malloc(128 * sizeof(int), M_DEVBUF, M_NOWAIT);
	edp->dectechtab = malloc(128 * sizeof(int), M_DEVBUF, M_NOWAIT);
	edp->nrctab = malloc(128 * sizeof(int), M_DEVBUF, M_NOWAIT);
	vt100_initchartables(edp);
	wsemul_vt100_reset(edp);
	return (edp);
}

void
wsemul_vt100_detach(void *cookie, u_int *crowp, u_int *ccolp)
{
	struct wsemul_vt100_emuldata *edp = cookie;

	*crowp = edp->crow;
	*ccolp = edp->ccol;
#define f(ptr) if (ptr) {free(ptr, M_DEVBUF); ptr = 0;}
	f(edp->tabs)
	f(edp->dblwid)
	f(edp->dcsarg)
	f(edp->isolatin1tab)
	f(edp->decgraphtab)
	f(edp->dectechtab)
	f(edp->nrctab)
#undef f
	if (edp != &wsemul_vt100_console_emuldata)
		free(edp, M_DEVBUF);
}

void
wsemul_vt100_resetop(void *cookie, enum wsemul_resetops op)
{
	struct wsemul_vt100_emuldata *edp = cookie;

	switch (op) {
	case WSEMUL_RESET:
		wsemul_vt100_reset(edp);
		break;
	case WSEMUL_SYNCFONT:
		vt100_initchartables(edp);
		break;
	case WSEMUL_CLEARSCREEN:
		wsemul_vt100_ed(edp, 2);
		edp->ccol = edp->crow = 0;
		(*edp->emulops->cursor)(edp->emulcookie,
					edp->flags & VTFL_CURSORON, 0, 0);
		break;
	default:
		break;
	}
}

void
wsemul_vt100_reset(struct wsemul_vt100_emuldata *edp)
{
	int i;

	edp->state = VT100_EMUL_STATE_NORMAL;
	edp->flags = VTFL_DECAWM | VTFL_CURSORON;
	edp->bkgdattr = edp->curattr = edp->defattr;
	edp->attrflags = edp->msgattrs.default_attrs;
	edp->fgcol = edp->msgattrs.default_fg;
	edp->bgcol = edp->msgattrs.default_bg;
	edp->scrreg_startrow = 0;
	edp->scrreg_nrows = edp->nrows;
	if (edp->tabs) {
		memset(edp->tabs, 0, edp->ncols);
		for (i = 8; i < edp->ncols; i += 8)
			edp->tabs[i] = 1;
	}
	edp->dcspos = 0;
	edp->dcstype = 0;
	edp->chartab_G[0] = 0;
	edp->chartab_G[1] = edp->nrctab; /* ??? */
	edp->chartab_G[2] = edp->isolatin1tab;
	edp->chartab_G[3] = edp->isolatin1tab;
	edp->chartab0 = 0;
	edp->chartab1 = 2;
	edp->sschartab = 0;
}

/*
 * now all the state machine bits
 */

/*
 * Move the cursor to the next line if possible. If the cursor is at
 * the bottom of the scroll area, then scroll it up. If the cursor is
 * at the bottom of the screen then don't move it down.
 */
static void
wsemul_vt100_nextline(struct wsemul_vt100_emuldata *edp)
{
	if (ROWS_BELOW == 0) {
		/* Bottom of the scroll region. */
	  	wsemul_vt100_scrollup(edp, 1);
	} else {
		if ((edp->crow+1) < edp->nrows)
			/* Cursor not at the bottom of the screen. */
			edp->crow++;
		CHECK_DW;
	}
}

static void
wsemul_vt100_output_normal(struct wsemul_vt100_emuldata *edp, u_char c,
	int kernel)
{
	u_int *ct, dc;

	if ((edp->flags & (VTFL_LASTCHAR | VTFL_DECAWM)) ==
	    (VTFL_LASTCHAR | VTFL_DECAWM)) {
		wsemul_vt100_nextline(edp);
		edp->ccol = 0;
		edp->flags &= ~VTFL_LASTCHAR;
	}

	if (c & 0x80) {
		c &= 0x7f;
		ct = edp->chartab_G[edp->chartab1];
	} else {
		if (edp->sschartab) {
			ct = edp->chartab_G[edp->sschartab];
			edp->sschartab = 0;
		} else
			ct = edp->chartab_G[edp->chartab0];
	}
	dc = (ct ? ct[c] : c);

	if ((edp->flags & VTFL_INSERTMODE) && COLS_LEFT)
		COPYCOLS(edp->ccol, edp->ccol + 1, COLS_LEFT);

	(*edp->emulops->putchar)(edp->emulcookie, edp->crow,
				 edp->ccol << edp->dw, dc,
				 kernel ? edp->kernattr : edp->curattr);

	if (COLS_LEFT)
		edp->ccol++;
	else
		edp->flags |= VTFL_LASTCHAR;
}

static void
wsemul_vt100_output_c0c1(struct wsemul_vt100_emuldata *edp, u_char c,
	int kernel)
{
	u_int n;

	switch (c) {
	case ASCII_NUL:
	default:
		/* ignore */
		break;
	case ASCII_BEL:
		wsdisplay_emulbell(edp->cbcookie);
		break;
	case ASCII_BS:
		if (edp->ccol > 0) {
			edp->ccol--;
			edp->flags &= ~VTFL_LASTCHAR;
		}
		break;
	case ASCII_CR:
		edp->ccol = 0;
		edp->flags &= ~VTFL_LASTCHAR;
		break;
	case ASCII_HT:
		if (edp->tabs) {
			if (!COLS_LEFT)
				break;
			for (n = edp->ccol + 1; n < NCOLS - 1; n++)
				if (edp->tabs[n])
					break;
		} else {
			n = edp->ccol + min(8 - (edp->ccol & 7), COLS_LEFT);
		}
		edp->ccol = n;
		break;
	case ASCII_SO: /* LS1 */
		edp->chartab0 = 1;
		break;
	case ASCII_SI: /* LS0 */
		edp->chartab0 = 0;
		break;
	case ASCII_ESC:
		if (kernel) {
			printf("wsemul_vt100_output_c0c1: ESC in kernel output ignored\n");
			break;	/* ignore the ESC */
		}

		if (edp->state == VT100_EMUL_STATE_STRING) {
			/* might be a string end */
			edp->state = VT100_EMUL_STATE_STRING_ESC;
		} else {
			/* XXX cancel current escape sequence */
			edp->state = VT100_EMUL_STATE_ESC;
		}
		break;
#if 0
	case CSI: /* 8-bit */
		/* XXX cancel current escape sequence */
		edp->nargs = 0;
		memset(edp->args, 0, sizeof (edp->args));
		edp->modif1 = edp->modif2 = '\0';
		edp->state = VT100_EMUL_STATE_CSI;
		break;
	case DCS: /* 8-bit */
		/* XXX cancel current escape sequence */
		edp->nargs = 0;
		memset(edp->args, 0, sizeof (edp->args));
		edp->state = VT100_EMUL_STATE_DCS;
		break;
	case ST: /* string end 8-bit */
		/* XXX only in VT100_EMUL_STATE_STRING */
		wsemul_vt100_handle_dcs(edp);
		return (VT100_EMUL_STATE_NORMAL);
#endif
	case ASCII_LF:
	case ASCII_VT:
	case ASCII_FF:
		wsemul_vt100_nextline(edp);
		break;
	}
}

static u_int
wsemul_vt100_output_esc(struct wsemul_vt100_emuldata *edp, u_char c)
{
	u_int newstate = VT100_EMUL_STATE_NORMAL;
	int i;

	switch (c) {
	case '[': /* CSI */
		edp->nargs = 0;
		memset(edp->args, 0, sizeof (edp->args));
		edp->modif1 = edp->modif2 = '\0';
		newstate = VT100_EMUL_STATE_CSI;
		break;
	case '7': /* DECSC */
		edp->flags |= VTFL_SAVEDCURS;
		edp->savedcursor_row = edp->crow;
		edp->savedcursor_col = edp->ccol;
		edp->savedattr = edp->curattr;
		edp->savedbkgdattr = edp->bkgdattr;
		edp->savedattrflags = edp->attrflags;
		edp->savedfgcol = edp->fgcol;
		edp->savedbgcol = edp->bgcol;
		for (i = 0; i < 4; i++)
			edp->savedchartab_G[i] = edp->chartab_G[i];
		edp->savedchartab0 = edp->chartab0;
		edp->savedchartab1 = edp->chartab1;
		break;
	case '8': /* DECRC */
		if ((edp->flags & VTFL_SAVEDCURS) == 0)
			break;
		edp->crow = edp->savedcursor_row;
		edp->ccol = edp->savedcursor_col;
		edp->curattr = edp->savedattr;
		edp->bkgdattr = edp->savedbkgdattr;
		edp->attrflags = edp->savedattrflags;
		edp->fgcol = edp->savedfgcol;
		edp->bgcol = edp->savedbgcol;
		for (i = 0; i < 4; i++)
			edp->chartab_G[i] = edp->savedchartab_G[i];
		edp->chartab0 = edp->savedchartab0;
		edp->chartab1 = edp->savedchartab1;
		break;
	case '=': /* DECKPAM application mode */
		edp->flags |= VTFL_APPLKEYPAD;
		break;
	case '>': /* DECKPNM numeric mode */
		edp->flags &= ~VTFL_APPLKEYPAD;
		break;
	case 'E': /* NEL */
		edp->ccol = 0;
		/* FALLTHRU */
	case 'D': /* IND */
		wsemul_vt100_nextline(edp);
		break;
	case 'H': /* HTS */
		KASSERT(edp->tabs != 0);
		edp->tabs[edp->ccol] = 1;
		break;
	case '~': /* LS1R */
		edp->chartab1 = 1;
		break;
	case 'n': /* LS2 */
		edp->chartab0 = 2;
		break;
	case '}': /* LS2R */
		edp->chartab1 = 2;
		break;
	case 'o': /* LS3 */
		edp->chartab0 = 3;
		break;
	case '|': /* LS3R */
		edp->chartab1 = 3;
		break;
	case 'N': /* SS2 */
		edp->sschartab = 2;
		break;
	case 'O': /* SS3 */
		edp->sschartab = 3;
		break;
	case 'M': /* RI */
		if (ROWS_ABOVE > 0) {
			edp->crow--;
			CHECK_DW;
			break;
		}
		wsemul_vt100_scrolldown(edp, 1);
		break;
	case 'P': /* DCS */
		edp->nargs = 0;
		memset(edp->args, 0, sizeof (edp->args));
		newstate = VT100_EMUL_STATE_DCS;
		break;
	case 'c': /* RIS */
		wsemul_vt100_reset(edp);
		wsemul_vt100_ed(edp, 2);
		edp->ccol = edp->crow = 0;
		break;
	case '(': case ')': case '*': case '+': /* SCS */
		edp->designating = c - '(';
		newstate = VT100_EMUL_STATE_SCS94;
		break;
	case '-': case '.': case '/': /* SCS */
		edp->designating = c - '-' + 1;
		newstate = VT100_EMUL_STATE_SCS96;
		break;
	case '#':
		newstate = VT100_EMUL_STATE_ESC_HASH;
		break;
	case ' ': /* 7/8 bit */
		newstate = VT100_EMUL_STATE_ESC_SPC;
		break;
	case ']': /* OSC operating system command */
	case '^': /* PM privacy message */
	case '_': /* APC application program command */
		/* ignored */
		newstate = VT100_EMUL_STATE_STRING;
		break;
	case '<': /* exit VT52 mode - ignored */
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("ESC%c unknown\n", c);
#endif
		break;
	}

	return (newstate);
}

static u_int
wsemul_vt100_output_scs94(struct wsemul_vt100_emuldata *edp, u_char c)
{
	u_int newstate = VT100_EMUL_STATE_NORMAL;

	switch (c) {
	case '%': /* probably DEC supplemental graphic */
		newstate = VT100_EMUL_STATE_SCS94_PERCENT;
		break;
	case 'A': /* british / national */
		edp->chartab_G[edp->designating] = edp->nrctab;
		break;
	case 'B': /* ASCII */
		edp->chartab_G[edp->designating] = 0;
		break;
	case '<': /* user preferred supplemental */
		/* XXX not really "user" preferred */
		edp->chartab_G[edp->designating] = edp->isolatin1tab;
		break;
	case '0': /* DEC special graphic */
		edp->chartab_G[edp->designating] = edp->decgraphtab;
		break;
	case '>': /* DEC tech */
		edp->chartab_G[edp->designating] = edp->dectechtab;
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("ESC%c%c unknown\n", edp->designating + '(', c);
#endif
		break;
	}
	return (newstate);
}

static u_int
wsemul_vt100_output_scs94_percent(struct wsemul_vt100_emuldata *edp, u_char c)
{
	switch (c) {
	case '5': /* DEC supplemental graphic */
		/* XXX there are differences */
		edp->chartab_G[edp->designating] = edp->isolatin1tab;
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("ESC%c%%%c unknown\n", edp->designating + '(', c);
#endif
		break;
	}
	return (VT100_EMUL_STATE_NORMAL);
}

static u_int
wsemul_vt100_output_scs96(struct wsemul_vt100_emuldata *edp, u_char c)
{
	u_int newstate = VT100_EMUL_STATE_NORMAL;
	int nrc;

	switch (c) {
	case '%': /* probably portuguese */
		newstate = VT100_EMUL_STATE_SCS96_PERCENT;
		break;
	case 'A': /* ISO-latin-1 supplemental */
		edp->chartab_G[edp->designating] = edp->isolatin1tab;
		break;
	case '4': /* dutch */
		nrc = 1;
		goto setnrc;
	case '5': case 'C': /* finnish */
		nrc = 2;
		goto setnrc;
	case 'R': /* french */
		nrc = 3;
		goto setnrc;
	case 'Q': /* french canadian */
		nrc = 4;
		goto setnrc;
	case 'K': /* german */
		nrc = 5;
		goto setnrc;
	case 'Y': /* italian */
		nrc = 6;
		goto setnrc;
	case 'E': case '6': /* norwegian / danish */
		nrc = 7;
		goto setnrc;
	case 'Z': /* spanish */
		nrc = 9;
		goto setnrc;
	case '7': case 'H': /* swedish */
		nrc = 10;
		goto setnrc;
	case '=': /* swiss */
		nrc = 11;
setnrc:
		vt100_setnrc(edp, nrc); /* what table ??? */
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("ESC%c%c unknown\n", edp->designating + '-' - 1, c);
#endif
		break;
	}
	return (newstate);
}

static u_int
wsemul_vt100_output_scs96_percent(struct wsemul_vt100_emuldata *edp, u_char c)
{
	switch (c) {
	case '6': /* portuguese */
		vt100_setnrc(edp, 8);
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("ESC%c%%%c unknown\n", edp->designating + '-', c);
#endif
		break;
	}
	return (VT100_EMUL_STATE_NORMAL);
}

static u_int
wsemul_vt100_output_esc_spc(struct wsemul_vt100_emuldata *edp,
    u_char c)
{
	switch (c) {
	case 'F': /* 7-bit controls */
	case 'G': /* 8-bit controls */
#ifdef VT100_PRINTNOTIMPL
		printf("ESC<SPC>%c ignored\n", c);
#endif
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("ESC<SPC>%c unknown\n", c);
#endif
		break;
	}
	return (VT100_EMUL_STATE_NORMAL);
}

static u_int
wsemul_vt100_output_string(struct wsemul_vt100_emuldata *edp, u_char c)
{
	if (edp->dcstype && edp->dcspos < DCS_MAXLEN)
		edp->dcsarg[edp->dcspos++] = c;
	return (VT100_EMUL_STATE_STRING);
}

static u_int
wsemul_vt100_output_string_esc(struct wsemul_vt100_emuldata *edp, u_char c)
{
	if (c == '\\') { /* ST complete */
		wsemul_vt100_handle_dcs(edp);
		return (VT100_EMUL_STATE_NORMAL);
	} else
		return (VT100_EMUL_STATE_STRING);
}

static u_int
wsemul_vt100_output_dcs(struct wsemul_vt100_emuldata *edp, u_char c)
{
	u_int newstate = VT100_EMUL_STATE_DCS;

	switch (c) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		/* argument digit */
		if (edp->nargs > VT100_EMUL_NARGS - 1)
			break;
		edp->args[edp->nargs] = (edp->args[edp->nargs] * 10) +
		    (c - '0');
		break;
	case ';': /* argument terminator */
		edp->nargs++;
		break;
	default:
		edp->nargs++;
		if (edp->nargs > VT100_EMUL_NARGS) {
#ifdef VT100_DEBUG
			printf("vt100: too many arguments\n");
#endif
			edp->nargs = VT100_EMUL_NARGS;
		}
		newstate = VT100_EMUL_STATE_STRING;
		switch (c) {
		case '$':
			newstate = VT100_EMUL_STATE_DCS_DOLLAR;
			break;
		case '{': /* DECDLD soft charset */
		case '!': /* DECRQUPSS user preferred supplemental set */
			/* 'u' must follow - need another state */
		case '|': /* DECUDK program F6..F20 */
#ifdef VT100_PRINTNOTIMPL
			printf("DCS%c ignored\n", c);
#endif
			break;
		default:
#ifdef VT100_PRINTUNKNOWN
			printf("DCS%c (%d, %d) unknown\n", c, ARG(0), ARG(1));
#endif
			break;
		}
	}

	return (newstate);
}

static u_int
wsemul_vt100_output_dcs_dollar(struct wsemul_vt100_emuldata *edp, u_char c)
{
	switch (c) {
	case 'p': /* DECRSTS terminal state restore */
	case 'q': /* DECRQSS control function request */
#ifdef VT100_PRINTNOTIMPL
		printf("DCS$%c ignored\n", c);
#endif
		break;
	case 't': /* DECRSPS restore presentation state */
		switch (ARG(0)) {
		case 0: /* error */
			break;
		case 1: /* cursor information restore */
#ifdef VT100_PRINTNOTIMPL
			printf("DCS1$t ignored\n");
#endif
			break;
		case 2: /* tab stop restore */
			edp->dcspos = 0;
			edp->dcstype = DCSTYPE_TABRESTORE;
			break;
		default:
#ifdef VT100_PRINTUNKNOWN
			printf("DCS%d$t unknown\n", ARG(0));
#endif
			break;
		}
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("DCS$%c (%d, %d) unknown\n", c, ARG(0), ARG(1));
#endif
		break;
	}
	return (VT100_EMUL_STATE_STRING);
}

static u_int
wsemul_vt100_output_esc_hash(struct wsemul_vt100_emuldata *edp, u_char c)
{
	int i, j;

	switch (c) {
	case '5': /*  DECSWL single width, single height */
		if (edp->dw) {
			for (i = 0; i < edp->ncols / 2; i++)
				(*edp->emulops->copycols)(edp->emulcookie,
							  edp->crow,
							  2 * i, i, 1);
			(*edp->emulops->erasecols)(edp->emulcookie, edp->crow,
						   i, edp->ncols - i,
						   edp->bkgdattr);
			edp->dblwid[edp->crow] = 0;
			edp->dw = 0;
		}
		break;
	case '6': /*  DECDWL double width, single height */
	case '3': /*  DECDHL double width, double height, top half */
	case '4': /*  DECDHL double width, double height, bottom half */
		if (!edp->dw) {
			for (i = edp->ncols / 2 - 1; i >= 0; i--)
				(*edp->emulops->copycols)(edp->emulcookie,
							  edp->crow,
							  i, 2 * i, 1);
			for (i = 0; i < edp->ncols / 2; i++)
				(*edp->emulops->erasecols)(edp->emulcookie,
							   edp->crow,
							   2 * i + 1, 1,
							   edp->bkgdattr);
			edp->dblwid[edp->crow] = 1;
			edp->dw = 1;
			if (edp->ccol > (edp->ncols >> 1) - 1)
				edp->ccol = (edp->ncols >> 1) - 1;
		}
		break;
	case '8': /* DECALN */
		for (i = 0; i < edp->nrows; i++)
			for (j = 0; j < edp->ncols; j++)
				(*edp->emulops->putchar)(edp->emulcookie, i, j,
							 'E', edp->curattr);
		edp->ccol = 0;
		edp->crow = 0;
		break;
	default:
#ifdef VT100_PRINTUNKNOWN
		printf("ESC#%c unknown\n", c);
#endif
		break;
	}
	return (VT100_EMUL_STATE_NORMAL);
}

static u_int
wsemul_vt100_output_csi(struct wsemul_vt100_emuldata *edp, u_char c)
{
	u_int newstate = VT100_EMUL_STATE_CSI;

	switch (c) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		/* argument digit */
		if (edp->nargs > VT100_EMUL_NARGS - 1)
			break;
		edp->args[edp->nargs] = (edp->args[edp->nargs] * 10) +
		    (c - '0');
		break;
	case ';': /* argument terminator */
		edp->nargs++;
		break;
	case '?': /* DEC specific */
	case '>': /* DA query */
		edp->modif1 = c;
		break;
	case '!':
	case '"':
	case '$':
	case '&':
		edp->modif2 = c;
		break;
	default: /* end of escape sequence */
		edp->nargs++;
		if (edp->nargs > VT100_EMUL_NARGS) {
#ifdef VT100_DEBUG
			printf("vt100: too many arguments\n");
#endif
			edp->nargs = VT100_EMUL_NARGS;
		}
		wsemul_vt100_handle_csi(edp, c);
		newstate = VT100_EMUL_STATE_NORMAL;
		break;
	}
	return (newstate);
}

void
wsemul_vt100_output(void *cookie, const u_char *data, u_int count, int kernel)
{
	struct wsemul_vt100_emuldata *edp = cookie;

#ifdef DIAGNOSTIC
	if (kernel && !edp->console)
		panic("wsemul_vt100_output: kernel output, not console");
#endif

	if (edp->flags & VTFL_CURSORON)
		(*edp->emulops->cursor)(edp->emulcookie, 0,
					edp->crow, edp->ccol << edp->dw);
	for (; count > 0; data++, count--) {
		if ((*data & 0x7f) < 0x20) {
			wsemul_vt100_output_c0c1(edp, *data, kernel);
			continue;
		}
		if (edp->state == VT100_EMUL_STATE_NORMAL || kernel) {
			wsemul_vt100_output_normal(edp, *data, kernel);
			continue;
		}
#ifdef DIAGNOSTIC
		if (edp->state > sizeof(vt100_output) / sizeof(vt100_output[0]))
			panic("wsemul_vt100: invalid state %d", edp->state);
#endif
		edp->state = vt100_output[edp->state - 1](edp, *data);
	}
	if (edp->flags & VTFL_CURSORON)
		(*edp->emulops->cursor)(edp->emulcookie, 1,
					edp->crow, edp->ccol << edp->dw);
}

#ifdef WSDISPLAY_CUSTOM_OUTPUT
static void
wsemul_vt100_getmsgattrs(void *cookie, struct wsdisplay_msgattrs *ma)
{
	struct wsemul_vt100_emuldata *edp = cookie;

	*ma = edp->msgattrs;
}

static void
wsemul_vt100_setmsgattrs(void *cookie, const struct wsscreen_descr *type,
                         const struct wsdisplay_msgattrs *ma)
{
	int error;
	long tmp;
	struct wsemul_vt100_emuldata *edp = cookie;

	edp->msgattrs = *ma;
	if (type->capabilities & WSSCREEN_WSCOLORS) {
		edp->msgattrs.default_attrs |= WSATTR_WSCOLORS;
		edp->msgattrs.kernel_attrs |= WSATTR_WSCOLORS;
	} else {
		edp->msgattrs.default_bg = edp->msgattrs.kernel_bg = 0;
		edp->msgattrs.default_fg = edp->msgattrs.kernel_fg = 0;
	}

	error = (*edp->emulops->allocattr)(edp->emulcookie,
	                                   edp->msgattrs.default_fg,
					   edp->msgattrs.default_bg,
	                                   edp->msgattrs.default_attrs,
	                                   &tmp);
#ifdef VT100_DEBUG
	if (error)
		printf("vt100: failed to allocate attribute for default "
		       "messages\n");
	else
#endif
	{
		if (edp->curattr == edp->defattr) {
			edp->bkgdattr = edp->curattr = tmp;
			edp->attrflags = edp->msgattrs.default_attrs;
			edp->bgcol = edp->msgattrs.default_bg;
			edp->fgcol = edp->msgattrs.default_fg;
		} else {
			edp->savedbkgdattr = edp->savedattr = tmp;
			edp->savedattrflags = edp->msgattrs.default_attrs;
			edp->savedbgcol = edp->msgattrs.default_bg;
			edp->savedfgcol = edp->msgattrs.default_fg;
		}
		if (edp->emulops->replaceattr != NULL)
			(*edp->emulops->replaceattr)(edp->emulcookie,
			                             edp->defattr, tmp);
		edp->defattr = tmp;
	}

	error = (*edp->emulops->allocattr)(edp->emulcookie,
	                                   edp->msgattrs.kernel_fg,
					   edp->msgattrs.kernel_bg,
	                                   edp->msgattrs.kernel_attrs,
	                                   &tmp);
#ifdef VT100_DEBUG
	if (error)
		printf("vt100: failed to allocate attribute for kernel "
		       "messages\n");
	else
#endif
	{
		if (edp->emulops->replaceattr != NULL)
			(*edp->emulops->replaceattr)(edp->emulcookie,
			                             edp->kernattr, tmp);
		edp->kernattr = tmp;
	}
}
#endif /* WSDISPLAY_CUSTOM_OUTPUT */
