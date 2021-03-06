/*	$NetBSD: machdep.c,v 1.102.4.14 2012/06/12 23:18:13 riz Exp $	*/

/*-
 * Copyright (c) 1996, 1997, 1998, 2000, 2006, 2007, 2008
 *     The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum and by Jason R. Thorpe of the Numerical Aerospace
 * Simulation Facility, NASA Ames Research Center.
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

/*
 * Copyright (c) 2006 Mathieu Ropert <mro@adviseo.fr>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copyright (c) 2007 Manuel Bouyer.
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

/*-
 * Copyright (c) 1982, 1987, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)machdep.c	7.4 (Berkeley) 6/3/91
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: machdep.c,v 1.102.4.14 2012/06/12 23:18:13 riz Exp $");

/* #define XENDEBUG_LOW  */

#include "opt_user_ldt.h"
#include "opt_ddb.h"
#include "opt_kgdb.h"
#include "opt_compat_netbsd.h"
#include "opt_compat_netbsd32.h"
#include "opt_compat_ibcs2.h"
#include "opt_cpureset_delay.h"
#include "opt_multiprocessor.h"
#include "opt_lockdebug.h"
#include "opt_mtrr.h"
#include "opt_realmem.h"
#include "opt_halted_message.h"
#include "opt_xen.h"
#ifndef XEN
#include "opt_physmem.h"
#endif
#include "isa.h"
#include "pci.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/signalvar.h>
#include <sys/kernel.h>
#include <sys/cpu.h>
#include <sys/user.h>
#include <sys/exec.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>
#include <sys/mount.h>
#include <sys/extent.h>
#include <sys/core.h>
#include <sys/kcore.h>
#include <sys/ucontext.h>
#include <machine/kcore.h>
#include <sys/ras.h>
#include <sys/sa.h>
#include <sys/savar.h>
#include <sys/syscallargs.h>
#include <sys/ksyms.h>

#ifdef KGDB
#include <sys/kgdb.h>
#endif

#include <dev/cons.h>

#include <uvm/uvm_extern.h>
#include <uvm/uvm_page.h>

#include <sys/sysctl.h>

#include <machine/cpu.h>
#include <machine/cpufunc.h>
#include <machine/gdt.h>
#include <machine/intr.h>
#include <machine/pio.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <machine/specialreg.h>
#include <machine/bootinfo.h>
#include <machine/fpu.h>
#include <machine/mtrr.h>
#include <machine/mpbiosvar.h>

#include <x86/cputypes.h>
#include <x86/cpu_msr.h>
#include <x86/cpuvar.h>

#include <x86/x86/tsc.h>

#include <dev/isa/isareg.h>
#include <machine/isa_machdep.h>
#include <dev/ic/i8042reg.h>

#ifdef XEN
#include <xen/xen.h>
#include <xen/hypervisor.h>
#include <xen/evtchn.h>
#endif

#ifdef DDB
#include <machine/db_machdep.h>
#include <ddb/db_extern.h>
#include <ddb/db_output.h>
#include <ddb/db_interface.h>
#endif

#include "acpi.h"

#if NACPI > 0
#include <dev/acpi/acpivar.h>
#define ACPI_MACHDEP_PRIVATE
#include <machine/acpi_machdep.h>
#endif

#include "isa.h"
#include "isadma.h"
#include "ksyms.h"

/* the following is used externally (sysctl_hw) */
char machine[] = "amd64";		/* CPU "architecture" */
char machine_arch[] = "x86_64";		/* machine == machine_arch */

/* Our exported CPU info; we have only one right now. */  
struct cpu_info cpu_info_primary;
struct cpu_info *cpu_info_list;

extern struct bi_devmatch *x86_alldisks;
extern int x86_ndisks;

#ifdef CPURESET_DELAY
int	cpureset_delay = CPURESET_DELAY;
#else
int     cpureset_delay = 2000; /* default to 2s */
#endif

int	cpu_class = CPUCLASS_686;

#ifdef MTRR
struct mtrr_funcs *mtrr_funcs;
#endif

int	physmem;
uint64_t	dumpmem_low;
uint64_t	dumpmem_high;
int	cpu_class;

vaddr_t	msgbuf_vaddr;
paddr_t msgbuf_paddr;

struct {
	paddr_t paddr;
	psize_t sz;
} msgbuf_p_seg[VM_PHYSSEG_MAX];
unsigned int msgbuf_p_cnt = 0;
      
vaddr_t	idt_vaddr;
paddr_t	idt_paddr;

vaddr_t lo32_vaddr;
paddr_t lo32_paddr;

vaddr_t lkm_start, lkm_end;
static struct vm_map lkm_map_store;
extern struct vm_map *lkm_map;
vaddr_t kern_end;

struct vm_map *mb_map = NULL;
struct vm_map *phys_map = NULL;

extern	paddr_t avail_start, avail_end;
#ifdef XEN
extern  vaddr_t first_bt_vaddr;
extern  paddr_t pmap_pa_start, pmap_pa_end;
#endif

#ifndef XEN
void (*delay_func)(unsigned int) = i8254_delay;
void (*initclock_func)(void) = i8254_initclocks;
#else /* XEN */
void (*delay_func)(unsigned int) = xen_delay;
void (*initclock_func)(void) = xen_initclocks;
#endif


#ifdef MTRR
struct mtrr_funcs *mtrr_funcs;
#endif

/*
 * Size of memory segments, before any memory is stolen.
 */
phys_ram_seg_t mem_clusters[VM_PHYSSEG_MAX];
int	mem_cluster_cnt;

char	x86_64_doubleflt_stack[4096];

int	cpu_dump(void);
int	cpu_dumpsize(void);
u_long	cpu_dump_mempagecnt(void);
void	dumpsys(void);
void	dodumpsys(void);
void	init_x86_64(paddr_t);

void add_mem_cluster(uint64_t, uint64_t, uint32_t);

/*
 * Machine-dependent startup code
 */
void
cpu_startup(void)
{
	int x, y;
	vaddr_t minaddr, maxaddr;
	psize_t sz;
	char pbuf[9];

	/*
	 * Initialize error message buffer (et end of core).
	 */
	if (msgbuf_p_cnt == 0)
		panic("msgbuf paddr map has not been set up");
	for (x = 0, sz = 0; x < msgbuf_p_cnt; sz += msgbuf_p_seg[x++].sz)
		continue;

	msgbuf_vaddr = uvm_km_alloc(kernel_map, sz, 0,
	    UVM_KMF_VAONLY);
	if (msgbuf_vaddr == 0)
		panic("failed to valloc msgbuf_vaddr");

	/* msgbuf_paddr was init'd in pmap */
	for (y = 0, sz = 0; y < msgbuf_p_cnt; y++) {
		for (x = 0; x < btoc(msgbuf_p_seg[y].sz); x++, sz += PAGE_SIZE)
			pmap_kenter_pa((vaddr_t)msgbuf_vaddr + sz,
				       msgbuf_p_seg[y].paddr + x * PAGE_SIZE,
				       VM_PROT_READ | UVM_PROT_WRITE);
	}
			  
	pmap_update(pmap_kernel());

	initmsgbuf((void *)msgbuf_vaddr, round_page(sz));

	printf("%s%s", copyright, version);

	format_bytes(pbuf, sizeof(pbuf), ptoa(physmem));
	printf("total memory = %s\n", pbuf);

	minaddr = 0;

	/*
	 * Allocate a submap for physio
	 */
	phys_map = uvm_km_suballoc(kernel_map, &minaddr, &maxaddr,
				   VM_PHYS_SIZE, 0, false, NULL);

	/*
	 * Finally, allocate mbuf cluster submap.
	 */
	mb_map = uvm_km_suballoc(kernel_map, &minaddr, &maxaddr,
	    nmbclusters * mclbytes, VM_MAP_INTRSAFE, false, NULL);

	uvm_map_setup(&lkm_map_store, lkm_start, lkm_end, 0);
	lkm_map_store.pmap = pmap_kernel();
	lkm_map = &lkm_map_store;

	format_bytes(pbuf, sizeof(pbuf), ptoa(uvmexp.free));
	printf("avail memory = %s\n", pbuf);

#if NISA > 0 || NPCI > 0
	/* Safe for i/o port / memory space allocation to use malloc now. */
	x86_bus_space_mallocok();
#endif

	gdt_init();
	x86_64_proc0_tss_ldt_init();

	cpu_init_tss(&cpu_info_primary);
#if !defined(XEN)
	ltr(cpu_info_primary.ci_tss_sel);
#endif /* !defined(XEN) */
}

#ifdef XEN
/* used in assembly */
void hypervisor_callback(void);
void failsafe_callback(void);
void x86_64_switch_context(struct pcb *);

void
x86_64_switch_context(struct pcb *new)
{
	struct cpu_info *ci;
	ci = curcpu();
	HYPERVISOR_stack_switch(GSEL(GDATA_SEL, SEL_KPL), new->pcb_rsp0);
	struct physdev_op physop;
	physop.cmd = PHYSDEVOP_SET_IOPL;
	physop.u.set_iopl.iopl = new->pcb_iopl;
	HYPERVISOR_physdev_op(&physop);
	if (new->pcb_fpcpu != ci) {
		HYPERVISOR_fpu_taskswitch(1);
	}
}

#endif

/*
 * Set up proc0's TSS and LDT.
 */
void
x86_64_proc0_tss_ldt_init(void)
{
	struct lwp *l;
	struct pcb *pcb;

	l = &lwp0;
	pcb = &l->l_addr->u_pcb;
	pcb->pcb_flags = 0;
	pcb->pcb_fs = 0;
	pcb->pcb_gs = 0;
	pcb->pcb_rsp0 = (USER_TO_UAREA(l->l_addr) + KSTACK_SIZE - 16) & ~0xf;
	pcb->pcb_iopl = SEL_KPL;

	pcb->pcb_ldt_sel = pmap_kernel()->pm_ldt_sel =
	    GSYSSEL(GLDT_SEL, SEL_KPL);
	pcb->pcb_cr0 = rcr0() & ~CR0_TS;
	l->l_md.md_regs = (struct trapframe *)pcb->pcb_rsp0 - 1;

#if !defined(XEN)
	lldt(pcb->pcb_ldt_sel);
#else
	{
	struct physdev_op physop;
	xen_set_ldt((vaddr_t) ldtstore, LDT_SIZE >> 3);
	/* Reset TS bit and set kernel stack for interrupt handlers */
	HYPERVISOR_fpu_taskswitch(1);
	HYPERVISOR_stack_switch(GSEL(GDATA_SEL, SEL_KPL), pcb->pcb_rsp0);
	physop.cmd = PHYSDEVOP_SET_IOPL;
	physop.u.set_iopl.iopl = pcb->pcb_iopl;
	HYPERVISOR_physdev_op(&physop);
	}
#endif /* XEN */
}

/*
 * Set up TSS and I/O bitmap.
 */
void
cpu_init_tss(struct cpu_info *ci)
{
	struct x86_64_tss *tss = &ci->ci_tss;
	uintptr_t p;

	tss->tss_iobase = IOMAP_INVALOFF << 16;
	/* tss->tss_ist[0] is filled by cpu_intr_init */

	/* double fault */
	tss->tss_ist[1] = (uint64_t)x86_64_doubleflt_stack + PAGE_SIZE - 16;

	/* NMI */
	p = uvm_km_alloc(kernel_map, PAGE_SIZE, 0, UVM_KMF_WIRED);
	tss->tss_ist[2] = p + PAGE_SIZE - 16;
	ci->ci_tss_sel = tss_alloc(tss);
}

/*  
 * machine dependent system variables.
 */ 
static int
sysctl_machdep_booted_kernel(SYSCTLFN_ARGS)
{
	struct btinfo_bootpath *bibp;
	struct sysctlnode node;

	bibp = lookup_bootinfo(BTINFO_BOOTPATH);
	if(!bibp)
		return(ENOENT); /* ??? */

	node = *rnode;
	node.sysctl_data = bibp->bootpath;
	node.sysctl_size = sizeof(bibp->bootpath);
	return (sysctl_lookup(SYSCTLFN_CALL(&node)));
}

static int
sysctl_machdep_diskinfo(SYSCTLFN_ARGS)
{
        struct sysctlnode node;

	if (x86_alldisks == NULL)
		return (ENOENT);

        node = *rnode;
        node.sysctl_data = x86_alldisks;
        node.sysctl_size = sizeof(struct disklist) +
	    (x86_ndisks - 1) * sizeof(struct nativedisk_info);
        return (sysctl_lookup(SYSCTLFN_CALL(&node)));
}

SYSCTL_SETUP(sysctl_machdep_setup, "sysctl machdep subtree setup")
{
	extern uint64_t tsc_freq;

	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_NODE, "machdep", NULL,
		       NULL, 0, NULL, 0,
		       CTL_MACHDEP, CTL_EOL);

	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_STRUCT, "console_device", NULL,
		       sysctl_consdev, 0, NULL, sizeof(dev_t),
		       CTL_MACHDEP, CPU_CONSDEV, CTL_EOL);
	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_STRING, "booted_kernel", NULL,
		       sysctl_machdep_booted_kernel, 0, NULL, 0,
		       CTL_MACHDEP, CPU_BOOTED_KERNEL, CTL_EOL);
	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_STRUCT, "diskinfo", NULL,
		       sysctl_machdep_diskinfo, 0, NULL, 0,
		       CTL_MACHDEP, CPU_DISKINFO, CTL_EOL);
	sysctl_createv(clog, 0, NULL, NULL,
		       CTLFLAG_PERMANENT,
		       CTLTYPE_QUAD, "tsc_freq", NULL,
		       NULL, 0, &tsc_freq, 0,
		       CTL_MACHDEP, CTL_CREATE, CTL_EOL);
}

void
buildcontext(struct lwp *l, void *catcher, void *f)
{
	struct trapframe *tf = l->l_md.md_regs;

	tf->tf_ds = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_es = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_fs = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_gs = GSEL(GUDATA_SEL, SEL_UPL);

	tf->tf_rip = (uint64_t)catcher;
	tf->tf_cs = GSEL(GUCODE_SEL, SEL_UPL);
	tf->tf_rflags &= ~PSL_CLEARSIG;
	tf->tf_rsp = (uint64_t)f;
	tf->tf_ss = GSEL(GUDATA_SEL, SEL_UPL);

	/* Ensure FP state is reset, if FP is used. */
	l->l_md.md_flags &= ~MDP_USEDFPU;
}

void
sendsig(const ksiginfo_t *ksi, const sigset_t *mask)
{
	struct lwp *l = curlwp;
	struct proc *p = l->l_proc;
	struct sigacts *ps = p->p_sigacts;
	int onstack, tocopy, error;
	int sig = ksi->ksi_signo;
	struct sigframe_siginfo *fp, frame;
	sig_t catcher = SIGACTION(p, sig).sa_handler;
	struct trapframe *tf = l->l_md.md_regs;
	char *sp;

	KASSERT(mutex_owned(p->p_lock));

	/* Do we need to jump onto the signal stack? */
	onstack =
	    (l->l_sigstk.ss_flags & (SS_DISABLE | SS_ONSTACK)) == 0 &&
	    (SIGACTION(p, sig).sa_flags & SA_ONSTACK) != 0;

	/* Allocate space for the signal handler context. */
	if (onstack)
		sp = ((char *)l->l_sigstk.ss_sp + l->l_sigstk.ss_size);
	else
		sp = (char *)tf->tf_rsp - 128;

	sp -= sizeof(struct sigframe_siginfo);
	/*
	 * Round down the stackpointer to a multiple of 16 for
	 * fxsave and the ABI.
	 */
	fp = (struct sigframe_siginfo *)(((unsigned long)sp & ~15) - 8);

	/* Build stack frame for signal trampoline. */
	switch (ps->sa_sigdesc[sig].sd_vers) {
	default:	/* unknown version */
		printf("nsendsig: bad version %d\n",
		    ps->sa_sigdesc[sig].sd_vers);
		sigexit(l, SIGILL);
	case 2:
		break;
	}

	/*
	 * Don't bother copying out FP state if there is none.
	 */
	if (l->l_md.md_flags & MDP_USEDFPU)
		tocopy = sizeof (struct sigframe_siginfo);
	else
		tocopy = sizeof (struct sigframe_siginfo) -
		    sizeof (frame.sf_uc.uc_mcontext.__fpregs);

	frame.sf_ra = (uint64_t)ps->sa_sigdesc[sig].sd_tramp;
	frame.sf_si._info = ksi->ksi_info;
	frame.sf_uc.uc_flags = _UC_SIGMASK;
	frame.sf_uc.uc_sigmask = *mask;
	frame.sf_uc.uc_link = l->l_ctxlink;
	frame.sf_uc.uc_flags |= (l->l_sigstk.ss_flags & SS_ONSTACK)
	    ? _UC_SETSTACK : _UC_CLRSTACK;
	memset(&frame.sf_uc.uc_stack, 0, sizeof(frame.sf_uc.uc_stack));
	sendsig_reset(l, sig);

	mutex_exit(p->p_lock);
	cpu_getmcontext(l, &frame.sf_uc.uc_mcontext, &frame.sf_uc.uc_flags);
	error = copyout(&frame, fp, tocopy);
	mutex_enter(p->p_lock);

	if (error != 0) {
		/*
		 * Process has trashed its stack; give it an illegal
		 * instruction to halt it in its tracks.
		 */
		sigexit(l, SIGILL);
		/* NOTREACHED */
	}

	buildcontext(l, catcher, fp);

	tf->tf_rdi = sig;
	tf->tf_rsi = (uint64_t)&fp->sf_si;
	tf->tf_rdx = tf->tf_r15 = (uint64_t)&fp->sf_uc;

	/* Remember that we're now on the signal stack. */
	if (onstack)
		l->l_sigstk.ss_flags |= SS_ONSTACK;

	if ((vaddr_t)catcher >= VM_MAXUSER_ADDRESS) {
		/* 
		 * process has given an invalid address for the
		 * handler. Stop it, but do not do it before so
		 * we can return the right info to userland (or in core dump)
		 */
		sigexit(l, SIGILL);
		/* NOTREACHED */
	}
}

void 
cpu_upcall(struct lwp *l, int type, int nevents, int ninterrupted, void *sas, void *ap, void *sp, sa_upcall_t upcall)
{
	struct trapframe *tf;

	tf = l->l_md.md_regs;

#if 0
	printf("proc %d: upcall to lwp %d, type %d ev %d int %d sas %p to %p\n",
	    (int)l->l_proc->p_pid, (int)l->l_lid, type, nevents, ninterrupted,
	    sas, (void *)upcall);
#endif

	tf->tf_rdi = type;
	tf->tf_rsi = (u_int64_t)sas;
	tf->tf_rdx = nevents;
	tf->tf_rcx = ninterrupted;
	tf->tf_r8 = (u_int64_t)ap;

	tf->tf_rip = (u_int64_t)upcall;
	tf->tf_rsp = ((unsigned long)sp & ~15) - 8;
	tf->tf_rbp = 0; /* indicate call-frame-top to debuggers */
	tf->tf_gs = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_fs = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_es = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_ds = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_cs = GSEL(GUCODE_SEL, SEL_UPL);
	tf->tf_ss = GSEL(GUDATA_SEL, SEL_UPL);
	tf->tf_rflags &= ~(PSL_T|PSL_VM|PSL_AC);

	l->l_md.md_flags |= MDP_IRET;
}

int	waittime = -1;
struct pcb dumppcb;

void
cpu_reboot(int howto, char *bootstr)
{

	if (cold) {
		howto |= RB_HALT;
		goto haltsys;
	}

	boothowto = howto;
	if ((howto & RB_NOSYNC) == 0 && waittime < 0) {
		waittime = 0;
		vfs_shutdown();
		/*
		 * If we've been adjusting the clock, the todr
		 * will be out of synch; adjust it now.
		 */
		resettodr();
	}

	/* Disable interrupts. */
	splhigh();

	/* Do a dump if requested. */
	if ((howto & (RB_DUMP | RB_HALT)) == RB_DUMP)
		dumpsys();

haltsys:
	doshutdownhooks();

        if ((howto & RB_POWERDOWN) == RB_POWERDOWN) {
#ifndef XEN
#if NACPI > 0
		acpi_enter_sleep_state(acpi_softc, ACPI_STATE_S5);
		printf("WARNING: powerdown failed!\n");
#endif
#else /* XEN */
		HYPERVISOR_shutdown();
#endif /* XEN */
	}

#ifdef MULTIPROCESSOR
	x86_broadcast_ipi(X86_IPI_HALT);
#endif

	if (howto & RB_HALT) {
#if NACPI > 0
		AcpiDisable();
#endif

#ifdef HALTED_MESSAGE
		printf("%s",HALTED_MESSAGE);
#else
		printf("\n");
		printf("The operating system has halted.\n");
		printf("Please press any key to reboot.\n\n");
#endif
		cnpollc(1);	/* for proper keyboard command handling */
		cngetc();
		cnpollc(0);
	}

	printf("rebooting...\n");
	if (cpureset_delay > 0)
		delay(cpureset_delay * 1000);
	cpu_reset();
	for(;;) ;
	/*NOTREACHED*/
}

/*
 * XXXfvdl share dumpcode.
 */

/*
 * These variables are needed by /sbin/savecore
 */
uint32_t	dumpmag = 0x8fca0101;	/* magic number */
int 	dumpsize = 0;		/* pages */
long	dumplo = 0; 		/* blocks */

/*
 * cpu_dumpsize: calculate size of machine-dependent kernel core dump headers.
 */
int
cpu_dumpsize(void)
{
	int size;

	size = ALIGN(sizeof(kcore_seg_t)) + ALIGN(sizeof(cpu_kcore_hdr_t)) +
	    ALIGN(mem_cluster_cnt * sizeof(phys_ram_seg_t));
	if (roundup(size, dbtob(1)) != dbtob(1))
		return (-1);

	return (1);
}

/*
 * cpu_dump_mempagecnt: calculate the size of RAM (in pages) to be dumped.
 */
u_long
cpu_dump_mempagecnt(void)
{
	u_long i, n;

	n = 0;
	for (i = 0; i < mem_cluster_cnt; i++)
		n += atop(mem_clusters[i].size);
	return (n);
}

/*
 * cpu_dump: dump the machine-dependent kernel core dump headers.
 */
int
cpu_dump(void)
{
	int (*dump)(dev_t, daddr_t, void *, size_t);
	char buf[dbtob(1)];
	kcore_seg_t *segp;
	cpu_kcore_hdr_t *cpuhdrp;
	phys_ram_seg_t *memsegp;
	const struct bdevsw *bdev;
	int i;

	bdev = bdevsw_lookup(dumpdev);
	if (bdev == NULL)
		return (ENXIO);

	dump = bdev->d_dump;

	memset(buf, 0, sizeof buf);
	segp = (kcore_seg_t *)buf;
	cpuhdrp = (cpu_kcore_hdr_t *)&buf[ALIGN(sizeof(*segp))];
	memsegp = (phys_ram_seg_t *)&buf[ ALIGN(sizeof(*segp)) +
	    ALIGN(sizeof(*cpuhdrp))];

	/*
	 * Generate a segment header.
	 */
	CORE_SETMAGIC(*segp, KCORE_MAGIC, MID_MACHINE, CORE_CPU);
	segp->c_size = dbtob(1) - ALIGN(sizeof(*segp));

	/*
	 * Add the machine-dependent header info.
	 */
	cpuhdrp->ptdpaddr = PDPpaddr;
	cpuhdrp->nmemsegs = mem_cluster_cnt;

	/*
	 * Fill in the memory segment descriptors.
	 */
	for (i = 0; i < mem_cluster_cnt; i++) {
		memsegp[i].start = mem_clusters[i].start;
		memsegp[i].size = mem_clusters[i].size;
	}

	return (dump(dumpdev, dumplo, (void *)buf, dbtob(1)));
}

/*
 * This is called by main to set dumplo and dumpsize.
 * Dumps always skip the first PAGE_SIZE of disk space
 * in case there might be a disk label stored there.
 * If there is extra space, put dump at the end to
 * reduce the chance that swapping trashes it.
 */
void
cpu_dumpconf(void)
{
	const struct bdevsw *bdev;
	int nblks, dumpblks;	/* size of dump area */

	if (dumpdev == NODEV)
		goto bad;
	bdev = bdevsw_lookup(dumpdev);
	if (bdev == NULL) {
		dumpdev = NODEV;
		goto bad;
	}
	if (bdev->d_psize == NULL)
		goto bad;
	nblks = (*bdev->d_psize)(dumpdev);
	if (nblks <= ctod(1))
		goto bad;

	dumpblks = cpu_dumpsize();
	if (dumpblks < 0)
		goto bad;
	dumpblks += ctod(cpu_dump_mempagecnt());

	/* If dump won't fit (incl. room for possible label), punt. */
	if (dumpblks > (nblks - ctod(1)))
		goto bad;

	/* Put dump at end of partition */
	dumplo = nblks - dumpblks;

	/* dumpsize is in page units, and doesn't include headers. */
	dumpsize = cpu_dump_mempagecnt();
	return;

 bad:
	dumpsize = 0;
}

/*
 * Doadump comes here after turning off memory management and
 * getting on the dump stack, either when called above, or by
 * the auto-restart code.
 */
#define BYTES_PER_DUMP  PAGE_SIZE /* must be a multiple of pagesize XXX small */
static vaddr_t dumpspace;

vaddr_t
reserve_dumppages(vaddr_t p)
{

	dumpspace = p;
	return (p + BYTES_PER_DUMP);
}

void
dodumpsys(void)
{
	const struct bdevsw *bdev;
	u_long totalbytesleft, bytes, i, n, memseg;
	u_long maddr;
	int psize;
	daddr_t blkno;
	int (*dump)(dev_t, daddr_t, void *, size_t);
	int error;

	if (dumpdev == NODEV)
		return;
	bdev = bdevsw_lookup(dumpdev);
	if (bdev == NULL || bdev->d_psize == NULL)
		return;

	/*
	 * For dumps during autoconfiguration,
	 * if dump device has already configured...
	 */
	if (dumpsize == 0)
		cpu_dumpconf();
	if (dumplo <= 0 || dumpsize == 0) {
		printf("\ndump to dev %u,%u not possible\n", major(dumpdev),
		    minor(dumpdev));
		return;
	}
	printf("\ndumping to dev %u,%u offset %ld\n", major(dumpdev),
	    minor(dumpdev), dumplo);

	psize = (*bdev->d_psize)(dumpdev);
	printf("dump ");
	if (psize == -1) {
		printf("area unavailable\n");
		return;
	}

	if ((error = cpu_dump()) != 0)
		goto err;

	totalbytesleft = ptoa(cpu_dump_mempagecnt());
	blkno = dumplo + cpu_dumpsize();
	dump = bdev->d_dump;
	error = 0;

	for (memseg = 0; memseg < mem_cluster_cnt; memseg++) {
		maddr = mem_clusters[memseg].start;
		bytes = mem_clusters[memseg].size;

		for (i = 0; i < bytes; i += n, totalbytesleft -= n) {
			/* Print out how many MBs we have left to go. */
			if ((totalbytesleft % (1024*1024)) == 0)
				printf("%ld ", totalbytesleft / (1024 * 1024));

			/* Limit size for next transfer. */
			n = bytes - i;
			if (n > BYTES_PER_DUMP)
				n = BYTES_PER_DUMP;

			(void) pmap_map(dumpspace, maddr, maddr + n,
			    VM_PROT_READ);

			error = (*dump)(dumpdev, blkno, (void *)dumpspace, n);
			if (error)
				goto err;
			maddr += n;
			blkno += btodb(n);		/* XXX? */

#if 0	/* XXX this doesn't work.  grr. */
			/* operator aborting dump? */
			if (sget() != NULL) {
				error = EINTR;
				break;
			}
#endif
		}
	}

 err:
	switch (error) {

	case ENXIO:
		printf("device bad\n");
		break;

	case EFAULT:
		printf("device not ready\n");
		break;

	case EINVAL:
		printf("area improper\n");
		break;

	case EIO:
		printf("i/o error\n");
		break;

	case EINTR:
		printf("aborted from console\n");
		break;

	case 0:
		printf("succeeded\n");
		break;

	default:
		printf("error %d\n", error);
		break;
	}
	printf("\n\n");
	delay(5000000);		/* 5 seconds */
}

/*
 * Clear registers on exec
 */
void
setregs(struct lwp *l, struct exec_package *pack, u_long stack)
{
	struct pcb *pcb = &l->l_addr->u_pcb;
	struct trapframe *tf;

	/* If we were using the FPU, forget about it. */
	if (l->l_addr->u_pcb.pcb_fpcpu != NULL)
		fpusave_lwp(l, false);

#ifdef USER_LDT
	pmap_ldt_cleanup(l);
#endif

	l->l_md.md_flags &= ~MDP_USEDFPU;
	pcb->pcb_flags = 0;
	pcb->pcb_fs = 0;
	pcb->pcb_gs = 0;
	pcb->pcb_savefpu.fp_fxsave.fx_fcw = __NetBSD_NPXCW__;
	pcb->pcb_savefpu.fp_fxsave.fx_mxcsr = __INITIAL_MXCSR__;
	pcb->pcb_savefpu.fp_fxsave.fx_mxcsr_mask = __INITIAL_MXCSR_MASK__;

	l->l_proc->p_flag &= ~PK_32;

	tf = l->l_md.md_regs;
	tf->tf_ds = LSEL(LUDATA_SEL, SEL_UPL);
	tf->tf_es = LSEL(LUDATA_SEL, SEL_UPL);
	tf->tf_fs = LSEL(LUDATA_SEL, SEL_UPL);
	tf->tf_gs = LSEL(LUDATA_SEL, SEL_UPL);
	tf->tf_rdi = 0;
	tf->tf_rsi = 0;
	tf->tf_rbp = 0;
	tf->tf_rbx = (uint64_t)l->l_proc->p_psstr;
	tf->tf_rdx = 0;
	tf->tf_rcx = 0;
	tf->tf_rax = 0;
	tf->tf_rip = pack->ep_entry;
	tf->tf_cs = LSEL(LUCODE_SEL, SEL_UPL);
	tf->tf_rflags = PSL_USERSET;
	tf->tf_rsp = stack;
	tf->tf_ss = LSEL(LUDATA_SEL, SEL_UPL);
}

/*
 * Initialize segments and descriptor tables
 */

#ifdef XEN
struct trap_info *xen_idt;
int xen_idt_idx;
#endif
char *ldtstore;
char *gdtstore;
extern  struct user *proc0paddr;

void
setgate(struct gate_descriptor *gd, void *func, int ist, int type, int dpl, int sel)
{

	kpreempt_disable();
	pmap_changeprot_local(idt_vaddr, VM_PROT_READ|VM_PROT_WRITE);

	gd->gd_looffset = (uint64_t)func & 0xffff;
	gd->gd_selector = sel;
	gd->gd_ist = ist;
	gd->gd_type = type;
	gd->gd_dpl = dpl;
	gd->gd_p = 1;
	gd->gd_hioffset = (uint64_t)func >> 16;
	gd->gd_zero = 0;
	gd->gd_xx1 = 0;
	gd->gd_xx2 = 0;
	gd->gd_xx3 = 0;

	pmap_changeprot_local(idt_vaddr, VM_PROT_READ);
	kpreempt_enable();
}

void
unsetgate( struct gate_descriptor *gd)
{

	kpreempt_disable();
	pmap_changeprot_local(idt_vaddr, VM_PROT_READ|VM_PROT_WRITE);

	memset(gd, 0, sizeof (*gd));

	pmap_changeprot_local(idt_vaddr, VM_PROT_READ);
	kpreempt_enable();
}

void
setregion(struct region_descriptor *rd, void *base, uint16_t limit)
{
	rd->rd_limit = limit;
	rd->rd_base = (uint64_t)base;
}

/*
 * Note that the base and limit fields are ignored in long mode.
 */
void
set_mem_segment(struct mem_segment_descriptor *sd, void *base, size_t limit,
	int type, int dpl, int gran, int def32, int is64)
{
	sd->sd_lolimit = (unsigned)limit;
	sd->sd_lobase = (unsigned long)base;
	sd->sd_type = type;
	sd->sd_dpl = dpl;
	sd->sd_p = 1;
	sd->sd_hilimit = (unsigned)limit >> 16;
	sd->sd_avl = 0;
	sd->sd_long = is64;
	sd->sd_def32 = def32;
	sd->sd_gran = gran;
	sd->sd_hibase = (unsigned long)base >> 24;
}

void
set_sys_segment(struct sys_segment_descriptor *sd, void *base, size_t limit,
	int type, int dpl, int gran)
{
	memset(sd, 0, sizeof *sd);
	sd->sd_lolimit = (unsigned)limit;
	sd->sd_lobase = (uint64_t)base;
	sd->sd_type = type;
	sd->sd_dpl = dpl;
	sd->sd_p = 1;
	sd->sd_hilimit = (unsigned)limit >> 16;
	sd->sd_gran = gran;
	sd->sd_hibase = (uint64_t)base >> 24;
}

void
cpu_init_idt(void)
{
#ifndef XEN
	struct region_descriptor region;

	setregion(&region, idt, NIDT * sizeof(idt[0]) - 1);
	lidt(&region); 
#else
	if (HYPERVISOR_set_trap_table(xen_idt))
		panic("HYPERVISOR_set_trap_table() failed");
#endif
}

#ifndef XEN
void
add_mem_cluster(uint64_t seg_start, uint64_t seg_end, uint32_t type)
{
	extern struct extent *iomem_ex;
	uint64_t io_end, new_physmem;
	int i;

	if (seg_end > 0x100000000000ULL) {
		printf("WARNING: skipping large "
		    "memory map entry: "
		    "0x%"PRIx64"/0x%"PRIx64"/0x%x\n",
                    seg_start,
                    (seg_end - seg_start),
                    type);
		return;
	}

	/*
	 * XXX Chop the last page off the size so that
	 * XXX it can fit in avail_end.
	 */
	if (seg_end == 0x100000000000ULL)
		seg_end -= PAGE_SIZE;

	if (seg_end <= seg_start)
		return;

	for (i = 0; i < mem_cluster_cnt; i++) {
		if ((mem_clusters[i].start == round_page(seg_start))
		    && (mem_clusters[i].size
			== trunc_page(seg_end) - mem_clusters[i].start)) {
#ifdef DEBUG_MEMLOAD
			printf("WARNING: skipping duplicate segment entry\n");
#endif
			return;
		}
	}

	/*
	 * Allocate the physical addresses used by RAM
	 * from the iomem extent map.  This is done before
	 * the addresses are page rounded just to make
	 * sure we get them all.
	 */
	if (seg_start < 0x100000000UL) {
		if (seg_end > 0x100000000UL)
			io_end = 0x100000000UL;
		else
			io_end = seg_end;

		if (extent_alloc_region(iomem_ex, seg_start,
		    io_end - seg_start, EX_NOWAIT)) {
			/* XXX What should we do? */
			printf("WARNING: CAN't ALLOCATE "
			    "MEMORY SEGMENT "
			    "(0x%"PRIx64"/0x%"PRIx64"/0x%x) FROM "
			    "IOMEM EXTENT MAP!\n",
			    seg_start, seg_end - seg_start, type);
			return;
		}
	}

	/*
	 * If it's not free memory, skip it.
	 */
	if (type != BIM_Memory)
		return;

	/* XXX XXX XXX */
 if (mem_cluster_cnt >= VM_PHYSSEG_MAX)
  { printf("mem_cluster_cnt=%d >= VM_PHYSSEG_MAX=%d\n",(int)mem_cluster_cnt,(int)VM_PHYSSEG_MAX);
    panic("init_x86_64: too many memory segments (increase VM_PHYSSEG_MAX)");
  }

#ifdef PHYSMEM_MAX_ADDR
	if (seg_start >= MBTOB(PHYSMEM_MAX_ADDR))
		return;
	if (seg_end > MBTOB(PHYSMEM_MAX_ADDR))
		seg_end = MBTOB(PHYSMEM_MAX_ADDR);
#endif

	seg_start = round_page(seg_start);
	seg_end = trunc_page(seg_end);

	if (seg_start == seg_end)
		return;

	mem_clusters[mem_cluster_cnt].start = seg_start;
	new_physmem = physmem + atop(seg_end - seg_start);

#ifdef PHYSMEM_MAX_SIZE
	if (physmem >= atop(MBTOB(PHYSMEM_MAX_SIZE)))
		return;
	if (new_physmem > atop(MBTOB(PHYSMEM_MAX_SIZE))) {
		seg_end = seg_start + MBTOB(PHYSMEM_MAX_SIZE) - ptoa(physmem);
		new_physmem = atop(MBTOB(PHYSMEM_MAX_SIZE));
	}
#endif

	mem_clusters[mem_cluster_cnt].size = seg_end - seg_start;

	if (avail_end < seg_end)
		avail_end = seg_end;
	physmem = new_physmem;
	mem_cluster_cnt++;
}
#endif

#define	IDTVEC(name)	__CONCAT(X, name)
typedef void (vector)(void);
extern vector IDTVEC(syscall);
extern vector IDTVEC(syscall32);
#if defined(COMPAT_16) || defined(COMPAT_NETBSD32)
extern vector IDTVEC(osyscall);
#endif
#if defined(COMPAT_10) || defined(COMPAT_IBCS2)
extern vector IDTVEC(oosyscall);
#endif
extern vector *IDTVEC(exceptions)[];

#define	KBTOB(x)	((size_t)(x) * 1024UL)
#define	MBTOB(x)	((size_t)(x) * 1024UL * 1024UL)

static void
init_x86_64_msgbuf(void)
{
	/* Message buffer is located at end of core. */
	struct vm_physseg *vps;
	psize_t sz = round_page(MSGBUFSIZE);
	psize_t reqsz = sz;
	int x;
		
 search_again:
	vps = NULL;

	for (x = 0; x < vm_nphysseg; x++) {
		vps = &vm_physmem[x];
		if (ptoa(vps->avail_end) == avail_end)
			break;
	}
	if (x == vm_nphysseg)
		panic("init_x86_64: can't find end of memory");

	/* Shrink so it'll fit in the last segment. */
	if ((vps->avail_end - vps->avail_start) < atop(sz))
		sz = ptoa(vps->avail_end - vps->avail_start);

	vps->avail_end -= atop(sz);
	vps->end -= atop(sz);
            msgbuf_p_seg[msgbuf_p_cnt].sz = sz;
            msgbuf_p_seg[msgbuf_p_cnt++].paddr = ptoa(vps->avail_end);

	/* Remove the last segment if it now has no pages. */
	if (vps->start == vps->end) {
		for (vm_nphysseg--; x < vm_nphysseg; x++)
			vm_physmem[x] = vm_physmem[x + 1];
	}

	/* Now find where the new avail_end is. */
	for (avail_end = 0, x = 0; x < vm_nphysseg; x++)
		if (vm_physmem[x].avail_end > avail_end)
			avail_end = vm_physmem[x].avail_end;
	avail_end = ptoa(avail_end);

	if (sz == reqsz)
		return;

	reqsz -= sz;
	if (msgbuf_p_cnt == VM_PHYSSEG_MAX) {
		/* No more segments available, bail out. */
		printf("WARNING: MSGBUFSIZE (%zu) too large, using %zu.\n",
		    (size_t)MSGBUFSIZE, (size_t)(MSGBUFSIZE - reqsz));
		return;
	}

	sz = reqsz;
	goto search_again;
}

static void
init_x86_64_ksyms(void)
{
#if NKSYMS || defined(DDB) || defined(LKM)
	extern int end;
	extern int *esym;
#ifndef XEN
	struct btinfo_symtab *symtab;
	vaddr_t tssym, tesym;
#endif

#ifdef DDB
	db_machine_init();
#endif

#ifndef XEN
	symtab = lookup_bootinfo(BTINFO_SYMTAB);
	if (symtab) {
		tssym = (vaddr_t)symtab->ssym + KERNBASE;
		tesym = (vaddr_t)symtab->esym + KERNBASE;
		ksyms_init(symtab->nsym, (void *)tssym, (void *)tesym);
	} else
		ksyms_init(*(long *)(void *)&end,
		    ((long *)(void *)&end) + 1, esym);
#else  /* XEN */
	esym = xen_start_info.mod_start ?
	    (void *)xen_start_info.mod_start :
	    (void *)xen_start_info.mfn_list;
	ksyms_init(*(int *)(void *)&end,
	    ((int *)(void *)&end) + 1, esym);
#endif /* XEN */
#endif
}

void
init_x86_64(paddr_t first_avail)
{
	extern void consinit(void);
	struct region_descriptor region;
	struct mem_segment_descriptor *ldt_segp;
	int x;
#ifndef XEN
	int first16q, first4gq, ist;
	extern struct extent *iomem_ex;
	uint64_t seg_start, seg_end;
	uint64_t seg_start1, seg_end1;
#if !defined(REALEXTMEM) && !defined(REALBASEMEM)
	struct btinfo_memmap *bim;
	uint64_t addr, size;
#endif
	cpu_probe(&cpu_info_primary);
#else /* XEN */
	cpu_probe(&cpu_info_primary);
	KASSERT(HYPERVISOR_shared_info != NULL);
	cpu_info_primary.ci_vcpu = &HYPERVISOR_shared_info->vcpu_info[0];

	__PRINTK(("init_x86_64(0x%lx)\n", first_avail));
	first_bt_vaddr = (vaddr_t) (first_avail + KERNBASE + PAGE_SIZE * 2);
	__PRINTK(("first_bt_vaddr 0x%lx\n", first_bt_vaddr));
	/* not on Xen... */
	cpu_feature &= ~(CPUID_PGE|CPUID_PSE|CPUID_MTRR|CPUID_FXSR);
	cpu_feature3 &= ~(CPUID_NOX);
#endif /* XEN */

	cpu_init_msrs(&cpu_info_primary, true);

	lwp0.l_addr = proc0paddr;
#ifdef XEN
	lwp0.l_addr->u_pcb.pcb_cr3 = xen_start_info.pt_base - KERNBASE;
	__PRINTK(("pcb_cr3 0x%lx\n", xen_start_info.pt_base - KERNBASE));
#endif

#if NISA > 0 || NPCI > 0
	x86_bus_space_init();
#endif

	consinit();	/* XXX SHOULD NOT BE DONE HERE */

	/*
	 * Initailize PAGE_SIZE-dependent variables.
	 */
	uvm_setpagesize();

	uvmexp.ncolors = 2;

#ifndef XEN
	/*
	 * Low memory reservations:
	 * Page 0:	BIOS data
	 * Page 1:	BIOS callback (not used yet, for symmetry with i386)
	 * Page 2:	MP bootstrap
	 * Page 3:	ACPI wakeup code
	 * Page 4:	Temporary page table for 0MB-4MB
	 * Page 5:	Temporary page directory
	 * Page 6:	Temporary page map level 3
	 * Page 7:	Temporary page map level 4
	 */
	avail_start = 8 * PAGE_SIZE;
#else	/* XEN */
	/* Parse Xen command line (replace bootinfo */
	xen_parse_cmdline(XEN_PARSE_BOOTFLAGS, NULL);

	/* Determine physical address space */
	avail_start = first_avail;
	avail_end = ptoa(xen_start_info.nr_pages);
	pmap_pa_start = (KERNTEXTOFF - KERNBASE);
	pmap_pa_end = avail_end;
	__PRINTK(("pmap_pa_start 0x%lx avail_start 0x%lx avail_end 0x%lx\n",
	    pmap_pa_start, avail_start, avail_end));
#endif	/* !XEN */

	/*
	 * Call pmap initialization to make new kernel address space.
	 * We must do this before loading pages into the VM system.
	 */
	pmap_bootstrap(VM_MIN_KERNEL_ADDRESS);

	if (avail_start != PAGE_SIZE)
		pmap_prealloc_lowmem_ptps();

#ifndef XEN
#if !defined(REALBASEMEM) && !defined(REALEXTMEM)

	/*
	 * Check to see if we have a memory map from the BIOS (passed
	 * to us by the boot program.
	 */
	bim = lookup_bootinfo(BTINFO_MEMMAP);
	if (bim != NULL && bim->num > 0) {
#ifdef DEBUG_MEMLOAD
		printf("BIOS MEMORY MAP (%d ENTRIES):\n", bim->num);
#endif
		for (x = 0; x < bim->num; x++) {
			addr = bim->entry[x].addr;
			size = bim->entry[x].size;
#ifdef DEBUG_MEMLOAD
			printf("    addr 0x%"PRIx64"  size 0x%"PRIx64
				"  type 0x%x\n",
				addr, size, bim->entry[x].type);
#endif

			/*
			 * If the segment is not memory, skip it.
			 */
			switch (bim->entry[x].type) {
			case BIM_Memory:
			case BIM_ACPI:
			case BIM_NVS:
				break;
			default:
				continue;
			}

			/*
			 * If the segment is smaller than a page, skip it.
			 */
			if (size < NBPG)
				continue;

			seg_start = addr;
			seg_end = addr + size;

			/*
			 *   Avoid Compatibility Holes.
			 * XXX  Holes within memory space that allow access
			 * XXX to be directed to the PC-compatible frame buffer
			 * XXX (0xa0000-0xbffff),to adapter ROM space
			 * XXX (0xc0000-0xdffff), and to system BIOS space
			 * XXX (0xe0000-0xfffff).
			 * XXX  Some laptop(for example,Toshiba Satellite2550X)
			 * XXX report this area and occurred problems,
			 * XXX so we avoid this area.
			 */
			if (seg_start < 0x100000 && seg_end > 0xa0000) {
				printf("WARNING: memory map entry overlaps "
				    "with ``Compatibility Holes'': "
				    "0x%"PRIx64"/0x%"PRIx64"/0x%x\n", seg_start,
				    seg_end - seg_start, bim->entry[x].type);
				add_mem_cluster(seg_start, 0xa0000,
				    bim->entry[x].type);
				add_mem_cluster(0x100000, seg_end,
				    bim->entry[x].type);
			} else
				add_mem_cluster(seg_start, seg_end,
				    bim->entry[x].type);
		}
	}
#endif	/* ! REALBASEMEM && ! REALEXTMEM */

	/*
	 * If the loop above didn't find any valid segment, fall back to
	 * former code.
	 */
	if (mem_cluster_cnt == 0) {
		/*
		 * Allocate the physical addresses used by RAM from the iomem
		 * extent map.  This is done before the addresses are
		 * page rounded just to make sure we get them all.
		 */
		if (extent_alloc_region(iomem_ex, 0, KBTOB(biosbasemem),
		    EX_NOWAIT)) {
			/* XXX What should we do? */
			printf("WARNING: CAN'T ALLOCATE BASE MEMORY FROM "
			    "IOMEM EXTENT MAP!\n");
		}
		mem_clusters[0].start = 0;
		mem_clusters[0].size = trunc_page(KBTOB(biosbasemem));
		physmem += atop(mem_clusters[0].size);
		if (extent_alloc_region(iomem_ex, IOM_END, KBTOB(biosextmem),
		    EX_NOWAIT)) {
			/* XXX What should we do? */
			printf("WARNING: CAN'T ALLOCATE EXTENDED MEMORY FROM "
			    "IOMEM EXTENT MAP!\n");
		}
#if NISADMA > 0
		/*
		 * Some motherboards/BIOSes remap the 384K of RAM that would
		 * normally be covered by the ISA hole to the end of memory
		 * so that it can be used.  However, on a 16M system, this
		 * would cause bounce buffers to be allocated and used.
		 * This is not desirable behaviour, as more than 384K of
		 * bounce buffers might be allocated.  As a work-around,
		 * we round memory down to the nearest 1M boundary if
		 * we're using any isadma devices and the remapped memory
		 * is what puts us over 16M.
		 */
		if (biosextmem > (15*1024) && biosextmem < (16*1024)) {
			char pbuf[9];

			format_bytes(pbuf, sizeof(pbuf),
			    biosextmem - (15*1024));
			printf("Warning: ignoring %s of remapped memory\n",
			    pbuf);
			biosextmem = (15*1024);
		}
#endif
		mem_clusters[1].start = IOM_END;
		mem_clusters[1].size = trunc_page(KBTOB(biosextmem));
		physmem += atop(mem_clusters[1].size);

		mem_cluster_cnt = 2;

		avail_end = IOM_END + trunc_page(KBTOB(biosextmem));
	}

	/*
	 * If we have 16M of RAM or less, just put it all on
	 * the default free list.  Otherwise, put the first
	 * 16M of RAM on a lower priority free list (so that
	 * all of the ISA DMA'able memory won't be eaten up
	 * first-off).
	 */
#define ADDR_16M (16 * 1024 * 1024)
#define ADDR_4G (4ULL * 1024 * 1024 * 1024)

	if (avail_end <= ADDR_16M)
		first16q = VM_FREELIST_DEFAULT;
	else
		first16q = VM_FREELIST_FIRST16;

	if (avail_end <= ADDR_4G)
		first4gq = VM_FREELIST_DEFAULT;
	else
		first4gq = VM_FREELIST_FIRST4G;

	/* Make sure the end of the space used by the kernel is rounded. */
	first_avail = round_page(first_avail);

	kern_end = KERNBASE + first_avail;
	lkm_start = kern_end;
	lkm_end = KERNBASE + NKL2_KIMG_ENTRIES * NBPD_L2;

	/*
	 * Now, load the memory clusters (which have already been
	 * rounded and truncated) into the VM system.
	 *
	 * NOTE: WE ASSUME THAT MEMORY STARTS AT 0 AND THAT THE KERNEL
	 * IS LOADED AT IOM_END (1M).
	 */
	for (x = 0; x < mem_cluster_cnt; x++) {
		seg_start = mem_clusters[x].start;
		seg_end = mem_clusters[x].start + mem_clusters[x].size;
		seg_start1 = 0;
		seg_end1 = 0;

		/*
		 * Skip memory before our available starting point.
		 */
		if (seg_end <= avail_start)
			continue;

		if (avail_start >= seg_start && avail_start < seg_end) {
			if (seg_start != 0)
				panic("init_x86_64: memory doesn't start at 0");
			seg_start = avail_start;
			if (seg_start == seg_end)
				continue;
		}

		/*
		 * If this segment contains the kernel, split it
		 * in two, around the kernel.
		 */
		if (seg_start <= IOM_END && first_avail <= seg_end) {
			seg_start1 = first_avail;
			seg_end1 = seg_end;
			seg_end = IOM_END;
		}

		/* First hunk */
		if (seg_start != seg_end) {
			if (seg_start < ADDR_16M &&
			    first16q != VM_FREELIST_DEFAULT) {
				uint64_t tmp;

				if (seg_end > ADDR_16M)
					tmp = ADDR_16M;
				else
					tmp = seg_end;

				if (tmp != seg_start) {
#ifdef DEBUG_MEMLOAD
					printf("loading first16q 0x%"PRIx64
					    "-0x%"PRIx64" (0x%lx-0x%lx)\n",
					    seg_start, tmp,
					    atop(seg_start), atop(tmp));
#endif
					uvm_page_physload(atop(seg_start),
					    atop(tmp), atop(seg_start),
					    atop(tmp), first16q);
				}
				seg_start = tmp;
			}
			if (seg_start < ADDR_4G &&
			    first4gq != VM_FREELIST_DEFAULT) {
				uint64_t tmp;

				if (seg_end > ADDR_4G)
					tmp = ADDR_4G;
				else
					tmp = seg_end;

				if (tmp != seg_start) {
#ifdef DEBUG_MEMLOAD
					printf("loading first4gq 0x%"PRIx64
					    "-0x%"PRIx64" (0x%lx-0x%lx)\n",
					    seg_start, tmp,
					    atop(seg_start), atop(tmp));
#endif
					uvm_page_physload(atop(seg_start),
					    atop(tmp), atop(seg_start),
					    atop(tmp), first4gq);
				}
				seg_start = tmp;
			}

			if (seg_start != seg_end) {
#ifdef DEBUG_MEMLOAD
				printf("loading default 0x%"PRIx64"-0x%"PRIx64
				    " (0x%lx-0x%lx)\n",
				    seg_start, seg_end,
				    atop(seg_start), atop(seg_end));
#endif
				uvm_page_physload(atop(seg_start),
				    atop(seg_end), atop(seg_start),
				    atop(seg_end), VM_FREELIST_DEFAULT);
			}
		}

		/* Second hunk */
		if (seg_start1 != seg_end1) {
			if (seg_start1 < ADDR_16M &&
			    first16q != VM_FREELIST_DEFAULT) {
				uint64_t tmp;

				if (seg_end1 > ADDR_16M)
					tmp = ADDR_16M;
				else
					tmp = seg_end1;

				if (tmp != seg_start1) {
#ifdef DEBUG_MEMLOAD
					printf("loading first16q 0x%"PRIx64
					    "-0x%"PRIx64" (0x%lx-0x%lx)\n",
					    seg_start1, tmp,
					    atop(seg_start1), atop(tmp));
#endif
					uvm_page_physload(atop(seg_start1),
					    atop(tmp), atop(seg_start1),
					    atop(tmp), first16q);
				}
				seg_start1 = tmp;
			}
			if (seg_start1 < ADDR_4G &&
			    first4gq != VM_FREELIST_DEFAULT) {
				uint64_t tmp;

				if (seg_end1 > ADDR_4G)
					tmp = ADDR_4G;
				else
					tmp = seg_end1;

				if (tmp != seg_start1) {
#ifdef DEBUG_MEMLOAD
					printf("loading first4gq 0x%"PRIx64
					    "-0x%"PRIx64" (0x%lx-0x%lx)\n",
					    seg_start1, tmp,
					    atop(seg_start1), atop(tmp));
#endif
					uvm_page_physload(atop(seg_start1),
					    atop(tmp), atop(seg_start1),
					    atop(tmp), first4gq);
				}
				seg_start1 = tmp;
			}

			if (seg_start1 != seg_end1) {
#ifdef DEBUG_MEMLOAD
				printf("loading default 0x%"PRIx64"-0x%"PRIx64
				    " (0x%lx-0x%lx)\n",
				    seg_start1, seg_end1,
				    atop(seg_start1), atop(seg_end1));
#endif
				uvm_page_physload(atop(seg_start1),
				    atop(seg_end1), atop(seg_start1),
				    atop(seg_end1), VM_FREELIST_DEFAULT);
			}
		}
	}
#else	/* XEN */
	kern_end = KERNBASE + first_avail;
	physmem = xen_start_info.nr_pages;

	uvm_page_physload(atop(avail_start),
		atop(avail_end), atop(avail_start),
		atop(avail_end), VM_FREELIST_DEFAULT);
#endif	/* !XEN */

	init_x86_64_msgbuf();

	pmap_growkernel(VM_MIN_KERNEL_ADDRESS + 32 * 1024 * 1024);

	kpreempt_disable();
	pmap_kenter_pa(idt_vaddr, idt_paddr, VM_PROT_READ|VM_PROT_WRITE);
	pmap_update(pmap_kernel());
	memset((void *)idt_vaddr, 0, PAGE_SIZE);
#ifndef XEN
	pmap_changeprot_local(idt_vaddr, VM_PROT_READ);
#endif
	pmap_kenter_pa(idt_vaddr + PAGE_SIZE, idt_paddr + PAGE_SIZE,
	    VM_PROT_READ|VM_PROT_WRITE);
#ifdef XEN
	/* Steal one more page for LDT */
	pmap_kenter_pa(idt_vaddr + 2 * PAGE_SIZE, idt_paddr + 2 * PAGE_SIZE,
	    VM_PROT_READ|VM_PROT_WRITE);
#endif
	pmap_kenter_pa(lo32_vaddr, lo32_paddr, VM_PROT_READ|VM_PROT_WRITE);
	pmap_update(pmap_kernel());

#ifndef XEN
	idt_init();
	idt = (struct gate_descriptor *)idt_vaddr;
	gdtstore = (char *)(idt + NIDT);
	ldtstore = gdtstore + DYNSEL_START;
#else
	xen_idt = (struct trap_info *)idt_vaddr;
	xen_idt_idx = 0;
	/* Xen wants page aligned GDT/LDT in separated pages */
	ldtstore = (char *) roundup((vaddr_t) (xen_idt + NIDT), PAGE_SIZE);
	gdtstore = (char *) (ldtstore + PAGE_SIZE);
#endif /* XEN */

	/* make gdt gates and memory segments */
	set_mem_segment(GDT_ADDR_MEM(gdtstore, GCODE_SEL), 0,
	    0xfffff, SDT_MEMERA, SEL_KPL, 1, 0, 1);

	set_mem_segment(GDT_ADDR_MEM(gdtstore, GDATA_SEL), 0,
	    0xfffff, SDT_MEMRWA, SEL_KPL, 1, 0, 1);

#ifndef XEN
	set_sys_segment(GDT_ADDR_SYS(gdtstore, GLDT_SEL), ldtstore,
	    LDT_SIZE - 1, SDT_SYSLDT, SEL_KPL, 0);
#endif

	set_mem_segment(GDT_ADDR_MEM(gdtstore, GUCODE_SEL), 0,
	    x86_btop(VM_MAXUSER_ADDRESS) - 1, SDT_MEMERA, SEL_UPL, 1, 0, 1);

	set_mem_segment(GDT_ADDR_MEM(gdtstore, GUDATA_SEL), 0,
	    x86_btop(VM_MAXUSER_ADDRESS) - 1, SDT_MEMRWA, SEL_UPL, 1, 0, 1);

	/* make ldt gates and memory segments */
#if defined(COMPAT_10) || defined(COMPAT_IBCS2)
	setgate((struct gate_descriptor *)(ldtstore + LSYS5CALLS_SEL),
	    &IDTVEC(oosyscall), 0, SDT_SYS386CGT, SEL_UPL,
	    GSEL(GCODE_SEL, SEL_KPL));
#endif
	*(struct mem_segment_descriptor *)(ldtstore + LUCODE_SEL) =
	    *GDT_ADDR_MEM(gdtstore, GUCODE_SEL);
	*(struct mem_segment_descriptor *)(ldtstore + LUDATA_SEL) =
	    *GDT_ADDR_MEM(gdtstore, GUDATA_SEL);

	/*
	 * 32 bit GDT entries.
	 */

	set_mem_segment(GDT_ADDR_MEM(gdtstore, GUCODE32_SEL), 0,
	    x86_btop(VM_MAXUSER_ADDRESS32) - 1, SDT_MEMERA, SEL_UPL, 1, 1, 0);

	set_mem_segment(GDT_ADDR_MEM(gdtstore, GUDATA32_SEL), 0,
	    x86_btop(VM_MAXUSER_ADDRESS32) - 1, SDT_MEMRWA, SEL_UPL, 1, 1, 0);

	/*
	 * 32 bit LDT entries.
	 */
	ldt_segp = (struct mem_segment_descriptor *)(ldtstore + LUCODE32_SEL);
	set_mem_segment(ldt_segp, 0, x86_btop(VM_MAXUSER_ADDRESS32) - 1,
	    SDT_MEMERA, SEL_UPL, 1, 1, 0);
	ldt_segp = (struct mem_segment_descriptor *)(ldtstore + LUDATA32_SEL);
	set_mem_segment(ldt_segp, 0, x86_btop(VM_MAXUSER_ADDRESS32) - 1,
	    SDT_MEMRWA, SEL_UPL, 1, 1, 0);

	/*
	 * Other entries.
	 */
	memcpy((struct gate_descriptor *)(ldtstore + LSOL26CALLS_SEL),
	    (struct gate_descriptor *)(ldtstore + LSYS5CALLS_SEL),
	    sizeof (struct gate_descriptor));
	memcpy((struct gate_descriptor *)(ldtstore + LBSDICALLS_SEL),
	    (struct gate_descriptor *)(ldtstore + LSYS5CALLS_SEL),
	    sizeof (struct gate_descriptor));

	/* exceptions */
	for (x = 0; x < 32; x++) {
#ifndef XEN
		idt_vec_reserve(x);
		switch (x) {
		case 2:	/* NMI */
			ist = 3;
			break;
		case 8:	/* double fault */
			ist = 2;
			break;
		default:
			ist = 0;
			break;
		}
		setgate(&idt[x], IDTVEC(exceptions)[x], ist, SDT_SYS386IGT,
		    (x == 3 || x == 4) ? SEL_UPL : SEL_KPL,
		    GSEL(GCODE_SEL, SEL_KPL));
#else /* XEN */
		xen_idt[xen_idt_idx].vector = x;

		switch (x) {
		case 2:  /* NMI */
		case 18: /* MCA */
			TI_SET_IF(&(xen_idt[xen_idt_idx]), 2);
			break;
		case 3:
		case 4:
			xen_idt[xen_idt_idx].flags = SEL_UPL;
			break;
		default:
			xen_idt[xen_idt_idx].flags = SEL_KPL;
			break;
		}

		xen_idt[xen_idt_idx].cs = GSEL(GCODE_SEL, SEL_KPL);
		xen_idt[xen_idt_idx].address =
		    (unsigned long)IDTVEC(exceptions)[x];
		xen_idt_idx++;
#endif /* XEN */
	}

#if defined(COMPAT_16) || defined(COMPAT_NETBSD32)
	/* new-style interrupt gate for syscalls */
#ifndef XEN
	idt_vec_reserve(128);
	setgate(&idt[128], &IDTVEC(osyscall), 0, SDT_SYS386IGT, SEL_UPL,
	    GSEL(GCODE_SEL, SEL_KPL));
#else
	xen_idt[xen_idt_idx].vector = 128;
	xen_idt[xen_idt_idx].flags = SEL_KPL;
	xen_idt[xen_idt_idx].cs = GSEL(GCODE_SEL, SEL_KPL);
	xen_idt[xen_idt_idx].address =  (unsigned long) &IDTVEC(osyscall);
	xen_idt_idx++;
#endif /* XEN */
#endif
#ifdef XEN
	pmap_changeprot_local(idt_vaddr, VM_PROT_READ);
#endif
	kpreempt_enable();

	setregion(&region, gdtstore, DYNSEL_START - 1);
	lgdt(&region);

#ifdef XEN
	/* Init Xen callbacks and syscall handlers */
	if (HYPERVISOR_set_callbacks(
	    (unsigned long) hypervisor_callback,
	    (unsigned long) failsafe_callback,
	    (unsigned long) Xsyscall))
		panic("HYPERVISOR_set_callbacks() failed");
#endif /* XEN */
	cpu_init_idt();

	init_x86_64_ksyms();

#ifndef XEN
	intr_default_setup();
#else
	events_default_setup();
#endif

	splraise(IPL_HIGH);
	x86_enable_intr();

	x86_init();

#ifdef DDB
	if (boothowto & RB_KDB)
		Debugger();
#endif
#ifdef KGDB
	kgdb_port_init();
	if (boothowto & RB_KDB) {
		kgdb_debug_init = 1;
		kgdb_connect(1);
	}
#endif
}

void
cpu_reset(void)
{
	x86_disable_intr();

#ifdef XEN
	HYPERVISOR_reboot();
#else

	x86_reset();

	/*
	 * Try to cause a triple fault and watchdog reset by making the IDT
	 * invalid and causing a fault.
	 */
	kpreempt_disable();
	pmap_changeprot_local(idt_vaddr, VM_PROT_READ|VM_PROT_WRITE);           
	pmap_changeprot_local(idt_vaddr + PAGE_SIZE,
	    VM_PROT_READ|VM_PROT_WRITE);
	memset((void *)idt, 0, NIDT * sizeof(idt[0]));
	kpreempt_enable();
	breakpoint();

#if 0
	/*
	 * Try to cause a triple fault and watchdog reset by unmapping the
	 * entire address space and doing a TLB flush.
	 */
	memset((void *)PTD, 0, PAGE_SIZE);
	tlbflush(); 
#endif
#endif	/* XEN */

	for (;;);
}

void
cpu_getmcontext(struct lwp *l, mcontext_t *mcp, unsigned int *flags)
{
	const struct trapframe *tf = l->l_md.md_regs;
	__greg_t ras_rip;

	/* Copy general registers member by member */
#define copy_from_tf(reg, REG, idx) mcp->__gregs[_REG_##REG] = tf->tf_##reg;
	_FRAME_GREG(copy_from_tf)
#undef copy_from_tf

	if ((ras_rip = (__greg_t)ras_lookup(l->l_proc,
	    (void *) mcp->__gregs[_REG_RIP])) != -1)
		mcp->__gregs[_REG_RIP] = ras_rip;

	*flags |= _UC_CPU;

	if ((l->l_md.md_flags & MDP_USEDFPU) != 0) {
		if (l->l_addr->u_pcb.pcb_fpcpu)
			fpusave_lwp(l, true);
		memcpy(mcp->__fpregs, &l->l_addr->u_pcb.pcb_savefpu.fp_fxsave,
		    sizeof (mcp->__fpregs));
		*flags |= _UC_FPU;
	}
}

int
cpu_setmcontext(struct lwp *l, const mcontext_t *mcp, unsigned int flags)
{
	struct trapframe *tf = l->l_md.md_regs;
	const __greg_t *gr = mcp->__gregs;
	struct proc *p = l->l_proc;
	int error;
	int err, trapno;
	int64_t rflags;

	if ((flags & _UC_CPU) != 0) {
		error = check_mcontext(l, mcp, tf);
		if (error != 0)
			return error;
		/*
		 * XXX maybe inline this.
		 */
		rflags = tf->tf_rflags;
		err = tf->tf_err;
		trapno = tf->tf_trapno;

		/* Copy general registers member by member */
#define copy_to_tf(reg, REG, idx) tf->tf_##reg = gr[_REG_##REG];
		_FRAME_GREG(copy_to_tf)
#undef copy_to_tf

#ifdef XEN
		/*
		 * Xen has its own way of dealing with %cs and %ss,
		 * reset it to proper values.
		 */
		tf->tf_ss = GSEL(GUDATA_SEL, SEL_UPL);
		tf->tf_cs = GSEL(GUCODE_SEL, SEL_UPL);
#endif
		rflags &= ~PSL_USER;
		tf->tf_rflags = rflags | (gr[_REG_RFLAGS] & PSL_USER);
		tf->tf_err = err;
		tf->tf_trapno = trapno;

		l->l_md.md_flags |= MDP_IRET;
	}

	if (l->l_addr->u_pcb.pcb_fpcpu != NULL)
		fpusave_lwp(l, false);

	if ((flags & _UC_FPU) != 0) {
		memcpy(&l->l_addr->u_pcb.pcb_savefpu.fp_fxsave, mcp->__fpregs,
		    sizeof (mcp->__fpregs));
		l->l_md.md_flags |= MDP_USEDFPU;
	}

	mutex_enter(p->p_lock);
	if (flags & _UC_SETSTACK)
		l->l_sigstk.ss_flags |= SS_ONSTACK;
	if (flags & _UC_CLRSTACK)
		l->l_sigstk.ss_flags &= ~SS_ONSTACK;
	mutex_exit(p->p_lock);

	return 0;
}

int
check_mcontext(struct lwp *l, const mcontext_t *mcp, struct trapframe *tf)
{
	const __greg_t *gr;
	uint16_t sel;
	int error;
	struct pmap *pmap = l->l_proc->p_vmspace->vm_map.pmap;

	gr = mcp->__gregs;

	if (((gr[_REG_RFLAGS] ^ tf->tf_rflags) & PSL_USERSTATIC) != 0)
		return EINVAL;

	if (__predict_false(pmap->pm_ldt != NULL)) {
		error = valid_user_selector(l, gr[_REG_ES], NULL, 0);
		if (error != 0)
			return error;

		error = valid_user_selector(l, gr[_REG_FS], NULL, 0);
		if (error != 0)
			return error;

		error = valid_user_selector(l, gr[_REG_GS], NULL, 0);
		if (error != 0)
			return error;

		if ((gr[_REG_DS] & 0xffff) == 0)
			return EINVAL;
		error = valid_user_selector(l, gr[_REG_DS], NULL, 0);
		if (error != 0)
			return error;

#ifndef XEN
		if ((gr[_REG_SS] & 0xffff) == 0)
			return EINVAL;
		error = valid_user_selector(l, gr[_REG_SS], NULL, 0);
		if (error != 0)
			return error;
#endif
	} else {
		sel = gr[_REG_ES] & 0xffff;
		if (sel != 0 && !VALID_USER_DSEL(sel))
			return EINVAL;

		sel = gr[_REG_FS] & 0xffff;
		if (sel != 0 && !VALID_USER_DSEL(sel))
			return EINVAL;

		sel = gr[_REG_GS] & 0xffff;
		if (sel != 0 && !VALID_USER_DSEL(sel))
			return EINVAL;

		sel = gr[_REG_DS] & 0xffff;
		if (!VALID_USER_DSEL(sel))
			return EINVAL;

#ifndef XEN
		sel = gr[_REG_SS] & 0xffff;
		if (!VALID_USER_DSEL(sel)) 
			return EINVAL;
#endif

	}

#ifndef XEN
	sel = gr[_REG_CS] & 0xffff;
	if (!VALID_USER_CSEL(sel))
		return EINVAL;
#endif

	if (gr[_REG_RIP] >= VM_MAXUSER_ADDRESS)
		return EINVAL;
	return 0;
}

void
cpu_initclocks(void)
{
	(*initclock_func)();
}

int
memseg_baseaddr(struct lwp *l, uint64_t seg, char *ldtp, int llen,
		uint64_t *addr)
{
	int off, len;
	char *dt;
	struct mem_segment_descriptor *sdp;
	struct proc *p = l->l_proc;
	struct pmap *pmap= p->p_vmspace->vm_map.pmap;
	uint64_t base;

	seg &= 0xffff;

	if (seg == 0) {
		if (addr != NULL)
			*addr = 0;
		return 0;
	}

	off = (seg & 0xfff8);
	if (seg & SEL_LDT) {
		if (ldtp != NULL) {
			dt = ldtp;
			len = llen;
		} else if (pmap->pm_ldt != NULL) {
			len = pmap->pm_ldt_len; /* XXX broken */
			dt = (char *)pmap->pm_ldt;
		} else {
			dt = ldtstore;
			len = LDT_SIZE;
		}

		if (off > (len - 8))
			return EINVAL;
	} else {
		if (seg != GUDATA_SEL || seg != GUDATA32_SEL)
			return EINVAL;
	}

	sdp = (struct mem_segment_descriptor *)(dt + off);
	if (sdp->sd_type < SDT_MEMRO || sdp->sd_p == 0)
		return EINVAL;

	base = ((uint64_t)sdp->sd_hibase << 32) | ((uint64_t)sdp->sd_lobase);
	if (sdp->sd_gran == 1)
		base <<= PAGE_SHIFT;

	if (base >= VM_MAXUSER_ADDRESS)
		return EINVAL;

	if (addr == NULL)
		return 0;

	*addr = base;

	return 0;
}

int
valid_user_selector(struct lwp *l, uint64_t seg, char *ldtp, int len)
{
	return memseg_baseaddr(l, seg, ldtp, len, NULL);
}
