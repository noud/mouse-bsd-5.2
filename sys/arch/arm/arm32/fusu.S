/*	$NetBSD: fusu.S,v 1.13 2008/04/27 18:58:44 matt Exp $	*/

/*
 * Copyright (c) 1996-1998 Mark Brinicombe.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Mark Brinicombe
 * 4. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "opt_multiprocessor.h"
#include "opt_cpuoptions.h"

#include "assym.h"
#include <machine/asm.h>
#include <machine/cpu.h>

#if !defined(PROCESS_ID_IS_CURCPU) && !defined(PROCESS_ID_IS_CURLWP)
#ifdef MULTIPROCESSOR
.Lcpu_info:
	.word	_C_LABEL(cpu_info)
#else
.Lcurpcb:
	.word	_C_LABEL(cpu_info_store) + CI_CURPCB
#endif
#endif

#if !defined(PROCESS_ID_IS_CURCPU) && !defined(PROCESS_ID_IS_CURLWP)
#if defined(MULTIPROCESSOR)
#define	GET_CURPCB(rX)				\
	stmfd	sp!, {r0, r1, r3, r14}		;\
	bl	_C_LABEL(cpu_number)		;\
	ldr	rX, .Lcpu_info			;\
	ldr	rX, [rX, r0, lsl #2]		;\
	ldr	rX, [rX, #CI_CURPCB]		;\
	ldmfd	sp!, {r0, r1, r3, r14}
#else
#define	GET_CURPCB(rX)				\
	ldr	rX, .Lcurpcb			;\
	ldr	rX, [rX]
#endif
#endif
/*
 * fuword(void *uaddr);
 * Fetch an int from the user's address space.
 */

ENTRY(fuword)
	GET_CURPCB(r2)

#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r1, .Lfusufault
	str	r1, [r2, #PCB_ONFAULT]

	ldrt	r3, [r0]

	mov	r1, #0x00000000
	str	r1, [r2, #PCB_ONFAULT]
	mov	r0, r3
	mov	pc, lr

/*
 * fusword(void *uaddr);
 * Fetch a short from the user's address space.
 */

ENTRY(fusword)
	GET_CURPCB(r2)

#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r1, .Lfusufault
	str	r1, [r2, #PCB_ONFAULT]

	ldrbt	r3, [r0], #1
	ldrbt	ip, [r0]
#ifdef __ARMEB__
	orr	r0, ip, r3, asl #8
#else
	orr	r0, r3, ip, asl #8
#endif
	mov	r1, #0x00000000
	str	r1, [r2, #PCB_ONFAULT]
	mov	pc, lr

/*
 * fuswintr(void *uaddr);
 * Fetch a short from the user's address space.  Can be called during an
 * interrupt.
 */

ENTRY(fuswintr)
	ldr	r2, Lblock_userspace_access
	ldr	r2, [r2]
	teq	r2, #0
	mvnne	r0, #0x00000000
	movne	pc, lr

	GET_CURPCB(r2)

#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r1, _C_LABEL(fusubailout)
	str	r1, [r2, #PCB_ONFAULT]

	ldrbt	r3, [r0], #1
	ldrbt	ip, [r0]
#ifdef __ARMEB__
	orr	r0, ip, r3, asl #8
#else
	orr	r0, r3, ip, asl #8
#endif

	mov	r1, #0x00000000
	str	r1, [r2, #PCB_ONFAULT]
	mov	pc, lr

Lblock_userspace_access:
	.word	_C_LABEL(block_userspace_access)

	.data
	.align	0
	.global	_C_LABEL(block_userspace_access)
_C_LABEL(block_userspace_access):
	.word	0
	.text

/*
 * fubyte(void *uaddr);
 * Fetch a byte from the user's address space.
 */

ENTRY(fubyte)
	GET_CURPCB(r2)

#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r1, .Lfusufault
	str	r1, [r2, #PCB_ONFAULT]

	ldrbt	r3, [r0]

	mov	r1, #0x00000000
	str	r1, [r2, #PCB_ONFAULT]
	mov	r0, r3
	mov	pc, lr

/*
 * Handle faults from [fs]u*().  Clean up and return -1.
 */

.Lfusufault:
	mov	r0, #0x00000000
	str	r0, [r2, #PCB_ONFAULT]
	mvn	r0, #0x00000000
	mov	pc, lr

/*
 * Handle faults from [fs]u*().  Clean up and return -1.  This differs from
 * fusufault() in that trap() will recognise it and return immediately rather
 * than trying to page fault.
 */

/* label must be global as fault.c references it */
	.global	_C_LABEL(fusubailout)
_C_LABEL(fusubailout):
	mov	r0, #0x00000000
	str	r0, [r2, #PCB_ONFAULT]
	mvn	r0, #0x00000000
	mov	pc, lr

#ifdef DIAGNOSTIC
/*
 * Handle earlier faults from [fs]u*(), due to no pcb
 */

.Lfusupcbfault:
	mov	r1, r0
	adr	r0, fusupcbfaulttext
	b	_C_LABEL(panic)

fusupcbfaulttext:
	.asciz	"Yikes - no valid PCB during fusuxxx() addr=%08x\n"
	.align	0
#endif

/*
 * suword(void *uaddr, int x);
 * Store an int in the user's address space.
 */

ENTRY(suword)
	GET_CURPCB(r2)

#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r3, .Lfusufault
	str	r3, [r2, #PCB_ONFAULT]

	strt	r1, [r0]

	mov	r0, #0x00000000
	str	r0, [r2, #PCB_ONFAULT]
	mov	pc, lr

/*
 * suswintr(void *uaddr, short x);
 * Store a short in the user's address space.  Can be called during an
 * interrupt.
 */

ENTRY(suswintr)
	ldr	r2, Lblock_userspace_access
	ldr	r2, [r2]
	teq	r2, #0
	mvnne	r0, #0x00000000
	movne	pc, lr

	GET_CURPCB(r2)

#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r3, _C_LABEL(fusubailout)
	str	r3, [r2, #PCB_ONFAULT]

#ifdef __ARMEB__
	mov	ip, r1, lsr #8
	strbt	ip, [r0], #1
#else
	strbt	r1, [r0], #1
	mov	r1, r1, lsr #8
#endif
	strbt	r1, [r0]

	mov	r0, #0x00000000
	str	r0, [r2, #PCB_ONFAULT]
	mov	pc, lr

/*
 * susword(void *uaddr, short x);
 * Store a short in the user's address space.
 */

ENTRY(susword)
	GET_CURPCB(r2)

#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r3, .Lfusufault
	str	r3, [r2, #PCB_ONFAULT]

#ifdef __ARMEB__
	mov	ip, r1, lsr #8
	strbt	ip, [r0], #1
#else
	strbt	r1, [r0], #1
	mov	r1, r1, lsr #8
#endif
	strbt	r1, [r0]

	mov	r0, #0x00000000
	str	r0, [r2, #PCB_ONFAULT]
	mov	pc, lr

/*
 * subyte(void *uaddr, char x);
 * Store a byte in the user's address space.
 */

ENTRY(subyte)
#if defined(PROCESS_ID_IS_CURCPU) || defined(PROCESS_ID_IS_CURLWP)
	GET_CURPCB(r2)
#elif defined(MULTIPROCESSOR)
	stmfd	sp!, {r0, r1, r3, r14}
	bl	_C_LABEL(cpu_number)
	ldr	r2, .Lcpu_info
	ldr	r2, [r2, r0, lsl #2]
	ldr	r2, [r2, #CI_CURPCB]
	ldmfd	sp!, {r0, r1, r3, r14}
#else
	ldr	r2, .Lcurpcb
	ldr	r2, [r2]
#endif


#ifdef DIAGNOSTIC
	teq	r2, #0x00000000
	beq	.Lfusupcbfault
#endif

	adr	r3, .Lfusufault
	str	r3, [r2, #PCB_ONFAULT]

	strbt	r1, [r0]

	mov	r0, #0x00000000
	str	r0, [r2, #PCB_ONFAULT]
	mov	pc, lr
