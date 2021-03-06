/*      $NetBSD: xen_shm.h,v 1.7 2006/06/25 15:18:53 bouyer Exp $      */

/*
 * Copyright (c) 2005 Manuel Bouyer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Manuel Bouyer.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

#include "opt_xen.h"
#include <machine/param.h>

#define XENSHM_MAX_PAGES_PER_REQUEST (MAXPHYS >> PAGE_SHIFT)

/*
 * Helper routines for the backend drivers. This implement the necessary
 * functions to map a bunch of pages from foreign domains in our kernel VM
 * space, do I/O to it, and unmap it.
 */

#ifdef XEN3
int  xen_shm_map(int, int, grant_ref_t *, vaddr_t *, grant_handle_t *, int);
void xen_shm_unmap(vaddr_t, int, grant_handle_t *);
#else
int  xen_shm_map(paddr_t *, int, int, vaddr_t *, int);
void xen_shm_unmap(vaddr_t, paddr_t *, int, int);
#endif
int xen_shm_callback(int (*)(void *), void *);

/* flags for xen_shm_map() */
#define XSHM_CALLBACK 0x01	/* called from a callback */
#define XSHM_RO 0x02		/* map the guest's memory read-only */
