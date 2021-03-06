/*	$NetBSD: rtld_start.S,v 1.13 2002/12/04 01:19:37 thorpej Exp $	*/

/*-
 * Copyright (C) 1998	Tsubai Masanari
 * Portions copyright 2002 Charles M. Hannum <root@ihack.net>
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <machine/asm.h>

	.globl	_rtld_start
	.globl	_rtld

	.text

_rtld_start:
	stwu	%r1,-48(%r1)
	stw	%r3,12(%r1)		# argc
	stw	%r4,16(%r1)		# argv
	stw	%r5,20(%r1)		# envp
/*	stw	%r6,24(%r1)		# obj		(always 0) */
/*	stw	%r7,28(%r1)		# cleanup	(always 0) */
	stw	%r8,32(%r1)		# ps_strings

	bl	_GLOBAL_OFFSET_TABLE_-4@local
	mflr	%r31			# r31 = (real) GOT
	lwz	%r28,0(%r31)		# base-relative &_DYNAMIC

	bl	0f			# lr = next instruction
	b	_DYNAMIC@local
0:	mflr	%r30
	lwz	%r29,0(%r30)		# load instruction contents
	rlwinm	%r29,%r29,0,6,29	# extract PC offset
	add	%r3,%r29,%r30		# r3 = &_DYNAMIC
	sub	%r28,%r3,%r28
	mr	%r4,%r28		# r4 = relocbase
	bl	_rtld_relocate_nonplt_self@plt

	lwz	%r3,16(%r1)
	addi	%r3,%r3,-12		# sp = &argv[-3]	/* XXX */
	mr	%r4,%r28		# r4 = relocbase
	bl	_rtld@plt		# _start = _rtld(sp, relocbase)
	mtlr	%r3

	lwz	%r3,12(%r1)		# argc
	lwz	%r4,16(%r1)		# argv
	lwz	%r5,20(%r1)		# envp
	lwz	%r6,-8(%r4)		# obj = sp[1] (== argv[-2])
	lwz	%r7,-12(%r4)		# cleanup = sp[0] (== argv[-3])
	lwz	%r8,32(%r1)		# ps_strings

	addi	%r1,%r1,48
	blrl		# _start(argc, argv, envp, obj, cleanup, ps_strings)

	li	%r0,1			# _exit()
	sc


	.globl	_rtld_bind_start
	.globl	_rtld_bind

_rtld_bind_start:
	stwu	%r1,-160(%r1)

	stw	%r0,20(%r1)
	mflr	%r0
	stw	%r0,16(%r1)		# save lr
	mfcr	%r0
	stw	%r0,12(%r1)		# save cr
	stmw	%r3,24(%r1)		# save r3-r31

	mr	%r3,%r12		# obj
	mr	%r4,%r11		# reloff
	bl	_rtld_bind@plt		# _rtld_bind(obj, reloff)
	mtctr	%r3

	lmw	%r3,24(%r1)		# load r3-r31
	lwz	%r0,12(%r1)		# restore cr
	mtcr	%r0
	lwz	%r0,16(%r1)		# restore lr
	mtlr	%r0
	lwz	%r0,20(%r1)

	addi	%r1,%r1,160
	bctr

	.globl	_rtld_powerpc_pltcall
	.globl	_rtld_powerpc_pltresolve

_rtld_powerpc_pltcall:
	slwi	%r11,%r11,2
	addis	%r11,%r11,0		# addis	11,11,jmptab@ha
	lwz	%r11,0(%r11)		# lwz	11,jmptab@l(11)
	mtctr	%r11
	bctr

_rtld_powerpc_pltresolve:
	lis	%r12,0			# lis	12,_rtld_bind_start@ha
	addi	%r12,%r12,0		# addi	12,12,_rtld_bind_start@l
	mtctr	%r12
	lis	%r12,0			# lis	12,obj@ha
	addi	%r12,%r12,0		# addi	12,12,obj@l
	bctr
