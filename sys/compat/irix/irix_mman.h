/*	$NetBSD: irix_mman.h,v 1.2 2008/04/28 20:23:41 martin Exp $ */

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Emmanuel Dreyfus.
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

#ifndef _IRIX_MMAN_H_
#define _IRIX_MMAN_H_

#include <sys/mman.h>

/* From IRIX's <sys/mman.h> */

#define IRIX_MAP_SHARED		MAP_SHARED
#define IRIX_MAP_PRIVATE	MAP_PRIVATE
#define IRIX_MAP_COPY		0x0004
#define IRIX_MAP_FIXED		MAP_FIXED
#define IRIX_MAP_RENAME		MAP_RENAME
#define IRIX_MAP_AUTOGROW	0x0040
#define IRIX_MAP_LOCAL		0x0080
#define IRIX_MAP_AUTORESRV	0x0100
#define IRIX_MAP_TEXT		0x0200
#define IRIX_MAP_BRK		0x0400
#define IRIX_MAP_PRIMARY	0x0800
#define IRIX_MAP_SGI_ANYADDR	0x1000

#endif /* _IRIX_MMAN_H_ */
