/*	$NetBSD: attributes.c,v 1.19 2008/06/13 03:15:50 yamt Exp $	*/

/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Julian Coleman.
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
__RCSID("$NetBSD: attributes.c,v 1.19 2008/06/13 03:15:50 yamt Exp $");
#endif				/* not lint */

#include "curses.h"
#include "curses_private.h"

void __wcolor_set(WINDOW *, attr_t);

#ifndef _CURSES_USE_MACROS
/*
 * attr_get --
 *	Get wide attributes and color pair from stdscr
 */
/* ARGSUSED */
int
attr_get(attr_t *attr, short *pair, void *opt)
{
	return wattr_get(stdscr, attr, pair, opt);
}

/*
 * attr_on --
 *	Test and set wide attributes on stdscr
 */
/* ARGSUSED */
int
attr_on(attr_t attr, void *opt)
{
	return wattr_on(stdscr, attr, opt);
}

/*
 * attr_off --
 *	Test and unset wide attributes on stdscr
 */
/* ARGSUSED */
int
attr_off(attr_t attr, void *opt)
{
	return wattr_off(stdscr, attr, opt);
}

/*
 * attr_set --
 *	Set wide attributes and color pair on stdscr
 */
/* ARGSUSED */
int
attr_set(attr_t attr, short pair, void *opt)
{
	return wattr_set(stdscr, attr, pair, opt);
}

/*
 * color_set --
 *	Set color pair on stdscr
 */
/* ARGSUSED */
int
color_set(short pair, void *opt)
{
	return wcolor_set(stdscr, pair, opt);
}

/*
 * attron --
 *	Test and set attributes on stdscr
 */
int
attron(int attr)
{
	return wattr_on(stdscr, (attr_t) attr, NULL);
}

/*
 * attroff --
 *	Test and unset attributes on stdscr.
 */
int
attroff(int attr)
{
	return wattr_off(stdscr, (attr_t) attr, NULL);
}

/*
 * attrset --
 *	Set specific attribute modes.
 *	Unset others.  On stdscr.
 */
int
attrset(int attr)
{
	return wattrset(stdscr, attr);
}
#endif	/* _CURSES_USE_MACROS */

/*
 * wattr_get --
 *	Get wide attributes and colour pair from window
 *	Note that attributes also includes colour.
 */
/* ARGSUSED */
int
wattr_get(WINDOW *win, attr_t *attr, short *pair, void *opt)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "wattr_get: win %p\n", win);
#endif
	if (attr != NULL) {
		*attr = win->wattr;
#ifdef HAVE_WCHAR
		*attr &= WA_ATTRIBUTES;
#endif
	}

	if (pair != NULL)
		*pair = PAIR_NUMBER(win->wattr);
	return OK;
}

/*
 * wattr_on --
 *	Test and set wide attributes on window
 */
/* ARGSUSED */
int
wattr_on(WINDOW *win, attr_t attr, void *opt)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "wattr_on: win %p, attr %08x\n", win, attr);
#endif
	/* If can enter modes, set the relevent attribute bits. */
	if (__tc_me != NULL) {
		if (attr & __BLINK && __tc_mb != NULL)
			win->wattr |= __BLINK;
		if (attr & __BOLD && __tc_md != NULL)
			win->wattr |= __BOLD;
		if (attr & __DIM && __tc_mh != NULL)
			win->wattr |= __DIM;
		if (attr & __BLANK && __tc_mk != NULL)
			win->wattr |= __BLANK;
		if (attr & __PROTECT && __tc_mp != NULL)
			win->wattr |= __PROTECT;
		if (attr & __REVERSE && __tc_mr != NULL)
			win->wattr |= __REVERSE;
#ifdef HAVE_WCHAR
		if (attr & WA_LOW && __tc_Xo != NULL)
			win->wattr |= WA_LOW;
		if (attr & WA_TOP && __tc_Xt != NULL)
			win->wattr |= WA_TOP;
		if (attr & WA_LEFT && __tc_Xl != NULL)
			win->wattr |= WA_LEFT;
		if (attr & WA_RIGHT && __tc_Xr != NULL)
			win->wattr |= WA_RIGHT;
		if (attr & WA_HORIZONTAL && __tc_Xh != NULL)
			win->wattr |= WA_HORIZONTAL;
		if (attr & WA_VERTICAL && __tc_Xv != NULL)
			win->wattr |= WA_VERTICAL;
#endif /* HAVE_WCHAR */
	}
	if (attr & __STANDOUT && __tc_so != NULL && __tc_se != NULL)
		wstandout(win);
	if (attr & __UNDERSCORE && __tc_us != NULL && __tc_ue != NULL)
		wunderscore(win);
	if ((attr_t) attr & __COLOR)
		__wcolor_set(win, (attr_t) attr);
	return OK;
}

/*
 * wattr_off --
 *	Test and unset wide attributes on window
 *
 *	Note that the 'me' sequence unsets all attributes.  We handle
 *	which attributes should really be set in refresh.c:makech().
 */
/* ARGSUSED */
int
wattr_off(WINDOW *win, attr_t attr, void *opt)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "wattr_off: win %p, attr %08x\n", win, attr);
#endif
	/* If can do exit modes, unset the relevent attribute bits. */
	if (__tc_me != NULL) {
		if (attr & __BLINK)
			win->wattr &= ~__BLINK;
		if (attr & __BOLD)
			win->wattr &= ~__BOLD;
		if (attr & __DIM)
			win->wattr &= ~__DIM;
		if (attr & __BLANK)
			win->wattr &= ~__BLANK;
		if (attr & __PROTECT)
			win->wattr &= ~__PROTECT;
		if (attr & __REVERSE)
			win->wattr &= ~__REVERSE;
#ifdef HAVE_WCHAR
		if (attr & WA_LOW)
			win->wattr &= ~WA_LOW;
		if (attr & WA_TOP)
			win->wattr &= ~WA_TOP;
		if (attr & WA_LEFT)
			win->wattr &= ~WA_LEFT;
		if (attr & WA_RIGHT)
			win->wattr &= ~WA_RIGHT;
		if (attr & WA_HORIZONTAL)
			win->wattr &= ~WA_HORIZONTAL;
		if (attr & WA_VERTICAL)
			win->wattr &= ~WA_VERTICAL;
#endif /* HAVE_WCHAR */
	}
	if (attr & __STANDOUT)
		wstandend(win);
	if (attr & __UNDERSCORE)
		wunderend(win);
	if ((attr_t) attr & __COLOR) {
		if (__tc_Co != 0)
			win->wattr &= ~__COLOR;
	}
	return OK;
}

/*
 * wattr_set --
 *	Set wide attributes and color pair on window
 */
int
wattr_set(WINDOW *win, attr_t attr, short pair, void *opt)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "wattr_set: win %p, attr %08x, pair %d\n",
	    win, attr, pair);
#endif
 	wattr_off(win, __ATTRIBUTES, opt);
	/*
	 * This overwrites any colour setting from the attributes
	 * and is compatible with ncurses.
	 */
 	attr = (attr & ~__COLOR) | COLOR_PAIR(pair);
 	wattr_on(win, attr, opt);
	return OK;
}

/*
 * wattron --
 *	Test and set attributes.
 */
int
wattron(WINDOW *win, int attr)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "wattron: win %p, attr %08x\n", win, attr);
#endif
	return wattr_on(win, (attr_t) attr, NULL);
}

/*
 * wattroff --
 *	Test and unset attributes.
 */
int
wattroff(WINDOW *win, int attr)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "wattroff: win %p, attr %08x\n", win, attr);
#endif
	return wattr_off(win, (attr_t) attr, NULL);
}

/*
 * wattrset --
 *	Set specific attribute modes.
 *	Unset others.
 */
int
wattrset(WINDOW *win, int attr)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "wattrset: win %p, attr %08x\n", win, attr);
#endif
	wattr_off(win, __ATTRIBUTES, NULL);
	wattr_on(win, (attr_t) attr, NULL);
	return OK;
}

/*
 * wcolor_set --
 *	Set color pair on window
 */
/* ARGSUSED */
int
wcolor_set(WINDOW *win, short pair, void *opt)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_COLOR, "wolor_set: win %p, pair %d\n", win, pair);
#endif
	__wcolor_set(win, (attr_t) COLOR_PAIR(pair));
	return OK;
}

/*
 * getattrs --
 *	Get window attributes.
 */
chtype
getattrs(WINDOW *win)
{
#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "getattrs: win %p\n", win);
#endif
	return((chtype) win->wattr);
}

/*
 * termattrs --
 *	Get terminal attributes
 */
chtype
termattrs(void)
{
	chtype ch = 0;

#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "termattrs\n");
#endif
	if (__tc_me != NULL) {
		if (__tc_mb != NULL)
			ch |= __BLINK;
		if (__tc_md != NULL)
			ch |= __BOLD;
		if (__tc_mh != NULL)
			ch |= __DIM;
		if (__tc_mk != NULL)
			ch |= __BLANK;
		if (__tc_mp != NULL)
			ch |= __PROTECT;
		if (__tc_mr != NULL)
			ch |= __REVERSE;
	}
	if (__tc_so != NULL && __tc_se != NULL)
		ch |= __STANDOUT;
	if (__tc_us != NULL && __tc_ue != NULL)
		ch |= __UNDERSCORE;
	if (__tc_as != NULL && __tc_ae != NULL)
		ch |= __ALTCHARSET;

	return ch;
}

/*
 * term_attrs --
 *	Get terminal wide attributes
 */
attr_t
term_attrs(void)
{
	attr_t attr = 0;

#ifdef DEBUG
	__CTRACE(__CTRACE_ATTR, "term_attrs\n");
#endif
	if (__tc_me != NULL) {
		if (__tc_mb != NULL)
			attr |= __BLINK;
		if (__tc_md != NULL)
			attr |= __BOLD;
		if (__tc_mh != NULL)
			attr |= __DIM;
		if (__tc_mk != NULL)
			attr |= __BLANK;
		if (__tc_mp != NULL)
			attr |= __PROTECT;
		if (__tc_mr != NULL)
			attr |= __REVERSE;
#ifdef HAVE_WCHAR
		if (__tc_Xo != NULL)
			attr |= WA_LOW;
		if (__tc_Xt != NULL)
			attr |= WA_TOP;
		if (__tc_Xl != NULL)
			attr |= WA_LEFT;
		if (__tc_Xr != NULL)
			attr |= WA_RIGHT;
		if (__tc_Xh != NULL)
			attr |= WA_HORIZONTAL;
		if (__tc_Xv != NULL)
			attr |= WA_VERTICAL;
#endif /* HAVE_WCHAR */
	}
	if (__tc_so != NULL && __tc_se != NULL)
		attr |= __STANDOUT;
	if (__tc_us != NULL && __tc_ue != NULL)
		attr |= __UNDERSCORE;
	if (__tc_as != NULL && __tc_ae != NULL)
		attr |= __ALTCHARSET;

	return attr;
}

/*
 * __wcolor_set --
 * Set color attribute on window
 */
void
__wcolor_set(WINDOW *win, attr_t attr)
{
	/* If another color pair is set, turn that off first. */
	win->wattr &= ~__COLOR;
	/* If can do color video, set the color pair bits. */
	if (__tc_Co != 0 && attr & __COLOR)
		win->wattr |= attr & __COLOR;
}
