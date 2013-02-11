/*	$NetBSD: mbrtowc.c,v 1.5 2005/02/11 06:21:21 simonb Exp $	*/

/*-
 * Copyright (c)2003 Citrus Project,
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * test code for mbrtowc
 * by YAMAMOTO Takashi
 *
 * this file uses following mb/ws functions.
 * setlocale, mbrtowc, mbsrtowcs, mbsinit, wcscmp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#define WBSIZE 100

/*
 * test mbrtowc
 */
int test_mbrtowc(char const *text)
{
	wchar_t* wbuf[10];
	//wchar_t wbuf[10][WBSIZE];
	size_t rv;
	size_t n;
	wchar_t *dst;
	mbstate_t *stp;
#ifdef USE_MBSTATE
	mbstate_t st;

	memset(&st, 0, sizeof(st));
//	mbrtowc(0, 0, 0, &st); /* XXX for ISO2022-JP */
	stp = &st;
#else
	stp = 0;
#endif

	for (n = 0; n < 10; n++) {
		wbuf[n] = malloc(sizeof(wchar_t) * WBSIZE);
		assert(wbuf[n]);
		memset(wbuf[n], 0xff, sizeof(wchar_t) * WBSIZE);
	}

	for (n = 9; n > 0; n--) {
		int m;
		char const *src = text;
		assert(mbsinit(stp));
		printf("n = %u:\t", n);
		dst = wbuf[n];
		m = n;
		for (;;) {
			rv = mbrtowc(dst, src, m, stp);
			printf("%d ", rv);
			if (rv == 0) {
				printf("(%d chars)", dst - wbuf[n]);
				printf("(%x)", (u_int)*dst);
				assert(!*dst);
				break;
			}
			if (rv == (size_t)-2) {
				src += m;
				continue;
			}
			if (rv == (size_t)-1) {
				assert(errno == EILSEQ);
				break;
			}

			printf("(%x) ", (u_int)*dst);
			src += rv;
			dst++;
		}
		printf("\n");
	}

	{
		wchar_t *pwc;
		size_t rv, len;
		char const *src = text;

		rv = mbsrtowcs(wbuf[0], &src, WBSIZE, 0);
		assert(!src);

		printf("mbsrtowcs: ");
		for (pwc=wbuf[0], len=0; *pwc; pwc++, len++) {
			printf("(%x) ", (u_int)*pwc);
		}
		assert(len == rv);
		printf("(%d chars)\n", len);
	}

	for (n = 1; n<10; n++) {
		if (wcscmp(wbuf[n], wbuf[0])) {
			fprintf(stderr, "err:%d\n", n);
			return EXIT_FAILURE;
		}
	}

	fprintf(stderr, "ok\n");
	return EXIT_SUCCESS;
}

int
main(int argc, char**argv)
{
	char *loc;
	loc = setlocale(LC_ALL, "");
	if (loc)
		fprintf(stderr, "===> Testing for locale %s... ", loc);
	else
		return EXIT_FAILURE;

	if (argc == 2)
		return test_mbrtowc(argv[1]);
	
	return EXIT_FAILURE;
}
