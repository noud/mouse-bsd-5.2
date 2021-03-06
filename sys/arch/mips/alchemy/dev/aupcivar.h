/* $NetBSD: aupcivar.h,v 1.2 2006/02/16 01:55:17 gdamore Exp $ */

/*-
 * Copyright (c) 2006 Itronix Inc.
 * All rights reserved.
 *
 * Written by Garrett D'Amore for Itronix Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of Itronix Inc. may not be used to endorse
 *    or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ITRONIX INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ITRONIX INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#ifndef _MIPS_ALCHEMY_DEV_AUPCIVAR_H
#define	_MIPS_ALCHEMY_DEV_AUPCIVAR_H

#include <dev/pci/pcivar.h>

/*
 * PCI configuration space encompasses all 32-bits.
 *
 * PCI memory space encompasses all 32-bits, excepting that portion of
 * the address space that is decoded by the Alchemy core for accesses
 * to host memory.  (That range is determined dynamically.) 
 *
 * PCI I/O address range.  We want to start offset from zero to avoid
 * potential problems with devices.  These addresses do not
 * participate on the Alchemy system bus, hence we can choose any
 * range we like.  16 MB is plenty.
 */

#define	AUPCI_IO_START	0x1000000
#define	AUPCI_IO_END	0x1FFFFFF


/*
 * Machdep code must implement this.  Stores an IRQ number in
 * pci_intr_handle_t.  See pci_intr_map(9) for more detail.  Returns 0
 * on success, non-zero on failure.
 */
int aupci_intr_map(struct pci_attach_args *, pci_intr_handle_t *);

#endif	/* _MIPS_ALCHEMY_DEV_AUPCIVAR_H */
