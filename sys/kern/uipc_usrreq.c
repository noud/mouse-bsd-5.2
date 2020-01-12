/*	$NetBSD: uipc_usrreq.c,v 1.119.4.5 2012/06/03 08:47:28 jdc Exp $	*/

/*-
 * Copyright (c) 1998, 2000, 2004, 2008, 2009 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center, and by Andrew Doran.
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
 * Copyright (c) 1982, 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)uipc_usrreq.c	8.9 (Berkeley) 5/14/95
 */

/*
 * Copyright (c) 1997 Christopher G. Demetriou.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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
 *	@(#)uipc_usrreq.c	8.9 (Berkeley) 5/14/95
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: uipc_usrreq.c,v 1.119.4.5 2012/06/03 08:47:28 jdc Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/filedesc.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/unpcb.h>
#include <sys/un.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mbuf.h>
#include <sys/kauth.h>
#include <sys/kmem.h>
#include <sys/atomic.h>
#include <sys/uidinfo.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <uvm/uvm.h>

/*
 * Unix communications domain.
 *
 * TODO:
 *	RDM
 *	rethink name space problems
 *	need a proper out-of-band
 *
 * Notes on locking:
 *
 * The generic rules noted in uipc_socket2.c apply.  In addition:
 *
 * o We have a global lock, uipc_lock.
 *
 * o All datagram sockets are locked by uipc_lock.
 *
 * o For stream socketpairs, the two endpoints are created sharing the same
 *   independent lock.  Sockets presented to PRU_CONNECT2 must already have
 *   matching locks.
 *
 * o Stream sockets created via socket() start life with their own
 *   independent lock.
 * 
 * o Stream connections to a named endpoint are slightly more complicated.
 *   Sockets that have called listen() have their lock pointer mutated to
 *   the global uipc_lock.  When establishing a connection, the connecting
 *   socket also has its lock mutated to uipc_lock, which matches the head
 *   (listening socket).  We create a new socket for accept() to return, and
 *   that also shares the head's lock.  Until the connection is completely
 *   done on both ends, all three sockets are locked by uipc_lock.  Once the
 *   connection is complete, the association with the head's lock is broken.
 *   The connecting socket and the socket returned from accept() have their
 *   lock pointers mutated away from uipc_lock, and back to the connecting
 *   socket's original, independent lock.  The head continues to be locked
 *   by uipc_lock.
 *
 * o If uipc_lock is determined to be a significant source of contention,
 *   it could easily be hashed out.  It is difficult to simply make it an
 *   independent lock because of visibility / garbage collection issues:
 *   if a socket has been associated with a lock at any point, that lock
 *   must remain valid until the socket is no longer visible in the system.
 *   The lock must not be freed or otherwise destroyed until any sockets
 *   that had referenced it have also been destroyed.
 */
const struct sockaddr_un sun_noname = {
	.sun_len = sizeof(sun_noname),
	.sun_family = AF_LOCAL,
};
ino_t	unp_ino;			/* prototype for fake inode numbers */

struct mbuf *unp_addsockcred(struct lwp *, struct mbuf *);
static void unp_mark(file_t *);
static void unp_scan(struct mbuf *, void (*)(file_t *), int);
static void unp_discard_now(file_t *);
static void unp_discard_later(file_t *);
static void unp_thread(void *);
static void unp_thread_kick(void);
static kmutex_t *uipc_lock;

static kcondvar_t unp_thread_cv;
static lwp_t *unp_thread_lwp;
static SLIST_HEAD(,file) unp_thread_discard;
static int unp_defer;

// Define this to get debugging kernel printfs.
//#define DEBUG_UNP_CTL
static struct vm_map unp_mem_map;
static uint32_t unp_max_mem;
MALLOC_DEFINE(M_SOCKMEM,"sockbits","inflight socket memory");
static kmutex_t unp_mem_mutex;

/*
 * Initialize Unix protocols.
 */
void
uipc_init(void)
{
	int error;

	uipc_lock = mutex_obj_alloc(MUTEX_DEFAULT, IPL_NONE);
	cv_init(&unp_thread_cv, "unpgc");

	error = kthread_create(PRI_NONE, KTHREAD_MPSAFE, NULL, unp_thread,
	    NULL, &unp_thread_lwp, "unpgc");
	if (error != 0)
		panic("uipc_init %d", error);
 unp_max_mem = (1U << 30) / PAGE_SIZE;
 uvm_map_setup(&unp_mem_map,(vaddr_t)PAGE_SIZE,(vaddr_t)(PAGE_SIZE*(unp_max_mem+1)),VM_MAP_PAGEABLE);
 unp_mem_map.pmap = 0;
 mutex_init(&unp_mem_mutex,MUTEX_DEFAULT,IPL_NONE);
}

/*
 * A connection succeeded: disassociate both endpoints from the head's
 * lock, and make them share their own lock.  There is a race here: for
 * a very brief time one endpoint will be locked by a different lock
 * than the other end.  However, since the current thread holds the old
 * lock (the listening socket's lock, the head) access can still only be
 * made to one side of the connection.
 */
static void
unp_setpeerlocks(struct socket *so, struct socket *so2)
{
	struct unpcb *unp;
	kmutex_t *lock;

	KASSERT(solocked2(so, so2));

	/*
	 * Bail out if either end of the socket is not yet fully
	 * connected or accepted.  We only break the lock association
	 * with the head when the pair of sockets stand completely
	 * on their own.
	 */
	if (so->so_head != NULL || so2->so_head != NULL)
		return;

	/*
	 * Drop references to old lock.  A third reference (from the
	 * queue head) must be held as we still hold its lock.  Bonus:
	 * we don't need to worry about garbage collecting the lock.
	 */
	lock = so->so_lock;
	KASSERT(lock == uipc_lock);
	mutex_obj_free(lock);
	mutex_obj_free(lock);

	/*
	 * Grab stream lock from the initiator and share between the two
	 * endpoints.  Issue memory barrier to ensure all modifications
	 * become globally visible before the lock change.  so2 is
	 * assumed not to have a stream lock, because it was created
	 * purely for the server side to accept this connection and
	 * started out life using the domain-wide lock.
	 */
	unp = sotounpcb(so);
	KASSERT(unp->unp_streamlock != NULL);
	KASSERT(sotounpcb(so2)->unp_streamlock == NULL);
	lock = unp->unp_streamlock;
	unp->unp_streamlock = NULL;
	mutex_obj_hold(lock);
	membar_exit();
	/*
	 * possible race if lock is not held - see comment in
	 * uipc_usrreq(PRU_ACCEPT).
	 */
	KASSERT(mutex_owned(lock));
	solockreset(so, lock);
	solockreset(so2, lock);
}

/*
 * Reset a socket's lock back to the domain-wide lock.
 */
static void
unp_resetlock(struct socket *so)
{
	kmutex_t *olock, *nlock;
	struct unpcb *unp;

	KASSERT(solocked(so));

	olock = so->so_lock;
	nlock = uipc_lock;
	if (olock == nlock)
		return;
	unp = sotounpcb(so);
	KASSERT(unp->unp_streamlock == NULL);
	unp->unp_streamlock = olock;
	mutex_obj_hold(nlock);
	mutex_enter(nlock);
	solockreset(so, nlock);
	mutex_exit(olock);
}

static void
unp_free(struct unpcb *unp)
{

	if (unp->unp_addr)
		free(unp->unp_addr, M_SONAME);
	if (unp->unp_streamlock != NULL)
		mutex_obj_free(unp->unp_streamlock);
	free(unp, M_PCB);
}

int
unp_output(struct mbuf *m, struct mbuf *control, struct unpcb *unp,
	struct lwp *l)
{
	struct socket *so2;
	const struct sockaddr_un *sun;

	so2 = unp->unp_conn->unp_socket;

	KASSERT(solocked(so2));

	if (unp->unp_addr)
		sun = unp->unp_addr;
	else
		sun = &sun_noname;
	if (unp->unp_conn->unp_flags & UNP_WANTCRED)
		control = unp_addsockcred(l, control);
	if (sbappendaddr(&so2->so_rcv, (const struct sockaddr *)sun, m,
	    control) == 0) {
		so2->so_rcv.sb_overflowed++;
		unp_dispose(control);
		m_freem(control);
		m_freem(m);
		return (ENOBUFS);
	} else {
		sorwakeup(so2);
		return (0);
	}
}

void
unp_setaddr(struct socket *so, struct mbuf *nam, bool peeraddr)
{
	const struct sockaddr_un *sun;
	struct unpcb *unp;
	bool ext;

	KASSERT(solocked(so));
	unp = sotounpcb(so);
	ext = false;

	for (;;) {
		sun = NULL;
		if (peeraddr) {
			if (unp->unp_conn && unp->unp_conn->unp_addr)
				sun = unp->unp_conn->unp_addr;
		} else {
			if (unp->unp_addr)
				sun = unp->unp_addr;
		}
		if (sun == NULL)
			sun = &sun_noname;
		nam->m_len = sun->sun_len;
		if (nam->m_len > MLEN && !ext) {
			sounlock(so);
			MEXTMALLOC(nam, MAXPATHLEN * 2, M_WAITOK);
			solock(so);
			ext = true;
		} else {
			KASSERT(nam->m_len <= MAXPATHLEN * 2);
			memcpy(mtod(nam, void *), sun, (size_t)nam->m_len);
			break;
		}
	}
}

/*ARGSUSED*/
int
uipc_usrreq(struct socket *so, int req, struct mbuf *m, struct mbuf *nam,
	struct mbuf *control, struct lwp *l)
{
	struct unpcb *unp = sotounpcb(so);
	struct socket *so2;
	struct proc *p;
	u_int newhiwat;
	int error = 0;

	if (req == PRU_CONTROL)
		return (EOPNOTSUPP);

#ifdef DIAGNOSTIC
	if (req != PRU_SEND && req != PRU_SENDOOB && control)
		panic("uipc_usrreq: unexpected control mbuf");
#endif
	p = l ? l->l_proc : NULL;
	if (req != PRU_ATTACH) {
		if (unp == 0) {
			error = EINVAL;
			goto release;
		}
		KASSERT(solocked(so));
	}

	switch (req) {

	case PRU_ATTACH:
		if (unp != 0) {
			error = EISCONN;
			break;
		}
		error = unp_attach(so);
		break;

	case PRU_DETACH:
		unp_detach(unp);
		break;

	case PRU_BIND:
		KASSERT(l != NULL);
		error = unp_bind(so, nam, l);
		break;

	case PRU_LISTEN:
		/*
		 * If the socket can accept a connection, it must be
		 * locked by uipc_lock.
		 */
		unp_resetlock(so);
		if (unp->unp_vnode == 0)
			error = EINVAL;
		break;

	case PRU_CONNECT:
		KASSERT(l != NULL);
		error = unp_connect(so, nam, l);
		break;

	case PRU_CONNECT2:
		error = unp_connect2(so, (struct socket *)nam, PRU_CONNECT2);
		break;

	case PRU_DISCONNECT:
		unp_disconnect(unp);
		break;

	case PRU_ACCEPT:
		KASSERT(so->so_lock == uipc_lock);
		/*
		 * Mark the initiating STREAM socket as connected *ONLY*
		 * after it's been accepted.  This prevents a client from
		 * overrunning a server and receiving ECONNREFUSED.
		 */
		if (unp->unp_conn == NULL)
			break;
		so2 = unp->unp_conn->unp_socket;
		if (so2->so_state & SS_ISCONNECTING) {
			KASSERT(solocked2(so, so->so_head));
			KASSERT(solocked2(so2, so->so_head));
			soisconnected(so2);
		}
		/*
		 * If the connection is fully established, break the
		 * association with uipc_lock and give the connected
		 * pair a seperate lock to share.
		 * There is a race here: sotounpcb(so2)->unp_streamlock
		 * is not locked, so when changing so2->so_lock
		 * another thread can grab it while so->so_lock is still
		 * pointing to the (locked) uipc_lock.
		 * this should be harmless, exept that this makes
		 * solocked2() and solocked() unreliable.
		 * Another problem is that unp_setaddr() expects the
		 * the socket locked. Grabing sotounpcb(so2)->unp_streamlock
		 * fixes both issues.
		 */
		mutex_enter(sotounpcb(so2)->unp_streamlock);
		unp_setpeerlocks(so2, so);
		/*
		 * Only now return peer's address, as we may need to
		 * block in order to allocate memory.
		 *
		 * XXX Minor race: connection can be broken while
		 * lock is dropped in unp_setaddr().  We will return
		 * error == 0 and sun_noname as the peer address.
		 */
		unp_setaddr(so, nam, true);
		/* so_lock now points to unp_streamlock */
		mutex_exit(so2->so_lock);
		break;

	case PRU_SHUTDOWN:
		socantsendmore(so);
		unp_shutdown(unp);
		break;

	case PRU_RCVD:
		switch (so->so_type) {

		case SOCK_DGRAM:
			panic("uipc 1");
			/*NOTREACHED*/

		case SOCK_SEQPACKET: /* FALLTHROUGH */
		case SOCK_STREAM:
#define	rcv (&so->so_rcv)
#define snd (&so2->so_snd)
			if (unp->unp_conn == 0)
				break;
			so2 = unp->unp_conn->unp_socket;
			KASSERT(solocked2(so, so2));
			/*
			 * Adjust backpressure on sender
			 * and wakeup any waiting to write.
			 */
			snd->sb_mbmax += unp->unp_mbcnt - rcv->sb_mbcnt;
			unp->unp_mbcnt = rcv->sb_mbcnt;
			newhiwat = snd->sb_hiwat + unp->unp_cc - rcv->sb_cc;
			(void)chgsbsize(so2->so_uidinfo,
			    &snd->sb_hiwat, newhiwat, RLIM_INFINITY);
			unp->unp_cc = rcv->sb_cc;
			sowwakeup(so2);
#undef snd
#undef rcv
			break;

		default:
			panic("uipc 2");
		}
		break;

	case PRU_SEND:
		/*
		 * Note: unp_internalize() is responsible for
		 *  preventing forged SCM_CREDS and the like; it must
		 *  allow only messages userland is allowed to send.
		 */
		if (control) {
			sounlock(so);
			error = unp_internalize(&control);
			solock(so);
			if (error != 0) {
				m_freem(control);
				m_freem(m);
				break;
			}
		}
		switch (so->so_type) {

		case SOCK_DGRAM: {
			KASSERT(so->so_lock == uipc_lock);
			if (nam) {
				if ((so->so_state & SS_ISCONNECTED) != 0)
					error = EISCONN;
				else {
					/*
					 * Note: once connected, the
					 * socket's lock must not be
					 * dropped until we have sent
					 * the message and disconnected.
					 * This is necessary to prevent
					 * intervening control ops, like
					 * another connection.
					 */
					error = unp_connect(so, nam, l);
				}
			} else {
				if ((so->so_state & SS_ISCONNECTED) == 0)
					error = ENOTCONN;
			}
			if (error) {
				unp_dispose(control);
				m_freem(control);
				m_freem(m);
				break;
			}
			KASSERT(p != NULL);
			error = unp_output(m, control, unp, l);
			if (nam)
				unp_disconnect(unp);
			break;
		}

		case SOCK_SEQPACKET: /* FALLTHROUGH */
		case SOCK_STREAM:
#define	rcv (&so2->so_rcv)
#define	snd (&so->so_snd)
			if (unp->unp_conn == NULL) {
				error = ENOTCONN;
				break;
			}
			so2 = unp->unp_conn->unp_socket;
			KASSERT(solocked2(so, so2));
			if (unp->unp_conn->unp_flags & UNP_WANTCRED) {
				/*
				 * Credentials are passed only once on
				 * SOCK_STREAM and SOCK_SEQPACKET.
				 */
				unp->unp_conn->unp_flags &= ~UNP_WANTCRED;
				control = unp_addsockcred(l, control);
			}
			/*
			 * Send to paired receive port, and then reduce
			 * send buffer hiwater marks to maintain backpressure.
			 * Wake up readers.
			 */
			if (control) {
				if (sbappendcontrol(rcv, m, control) != 0)
					control = NULL;
			} else {
				switch(so->so_type) {
				case SOCK_SEQPACKET:
					sbappendrecord(rcv, m);
					break;
				case SOCK_STREAM:
					sbappend(rcv, m);
					break;
				default:
					panic("uipc_usrreq");
					break;
				}
			}
			snd->sb_mbmax -=
			    rcv->sb_mbcnt - unp->unp_conn->unp_mbcnt;
			unp->unp_conn->unp_mbcnt = rcv->sb_mbcnt;
			newhiwat = snd->sb_hiwat -
			    (rcv->sb_cc - unp->unp_conn->unp_cc);
			(void)chgsbsize(so->so_uidinfo,
			    &snd->sb_hiwat, newhiwat, RLIM_INFINITY);
			unp->unp_conn->unp_cc = rcv->sb_cc;
			sorwakeup(so2);
#undef snd
#undef rcv
			if (control != NULL) {
				unp_dispose(control);
				m_freem(control);
			}
			break;

		default:
			panic("uipc 4");
		}
		break;

	case PRU_ABORT:
		(void)unp_drop(unp, ECONNABORTED);

		KASSERT(so->so_head == NULL);
#ifdef DIAGNOSTIC
		if (so->so_pcb == 0)
			panic("uipc 5: drop killed pcb");
#endif
		unp_detach(unp);
		break;

	case PRU_SENSE:
		((struct stat *) m)->st_blksize = so->so_snd.sb_hiwat;
		switch (so->so_type) {
		case SOCK_SEQPACKET: /* FALLTHROUGH */
		case SOCK_STREAM:
			if (unp->unp_conn == 0) 
				break;

			so2 = unp->unp_conn->unp_socket;
			KASSERT(solocked2(so, so2));
			((struct stat *) m)->st_blksize += so2->so_rcv.sb_cc;
			break;
		default:
			break;
		}
		((struct stat *) m)->st_dev = NODEV;
		if (unp->unp_ino == 0)
			unp->unp_ino = unp_ino++;
		((struct stat *) m)->st_atimespec =
		    ((struct stat *) m)->st_mtimespec =
		    ((struct stat *) m)->st_ctimespec = unp->unp_ctime;
		((struct stat *) m)->st_ino = unp->unp_ino;
		return (0);

	case PRU_RCVOOB:
		error = EOPNOTSUPP;
		break;

	case PRU_SENDOOB:
		m_freem(control);
		m_freem(m);
		error = EOPNOTSUPP;
		break;

	case PRU_SOCKADDR:
		unp_setaddr(so, nam, false);
		break;

	case PRU_PEERADDR:
		unp_setaddr(so, nam, true);
		break;

	default:
		panic("piusrreq");
	}

release:
	return (error);
}

/*
 * Unix domain socket option processing.
 */
int
uipc_ctloutput(int op, struct socket *so, struct sockopt *sopt)
{
	struct unpcb *unp = sotounpcb(so);
	int optval = 0, error = 0;

	KASSERT(solocked(so));

	if (sopt->sopt_level != 0) {
		error = ENOPROTOOPT;
	} else switch (op) {

	case PRCO_SETOPT:
		switch (sopt->sopt_name) {
		case LOCAL_CREDS:
		case LOCAL_CONNWAIT:
			error = sockopt_getint(sopt, &optval);
			if (error)
				break;
			switch (sopt->sopt_name) {
#define	OPTSET(bit) \
	if (optval) \
		unp->unp_flags |= (bit); \
	else \
		unp->unp_flags &= ~(bit);

			case LOCAL_CREDS:
				OPTSET(UNP_WANTCRED);
				break;
			case LOCAL_CONNWAIT:
				OPTSET(UNP_CONNWAIT);
				break;
			}
			break;
#undef OPTSET

		default:
			error = ENOPROTOOPT;
			break;
		}
		break;

	case PRCO_GETOPT:
		sounlock(so);
		switch (sopt->sopt_name) {
		case LOCAL_PEEREID:
			if (unp->unp_flags & UNP_EIDSVALID) {
				error = sockopt_set(sopt,
				    &unp->unp_connid, sizeof(unp->unp_connid));
			} else {
				error = EINVAL;
			}
			break;
		case LOCAL_CREDS:
#define	OPTBIT(bit)	(unp->unp_flags & (bit) ? 1 : 0)

			optval = OPTBIT(UNP_WANTCRED);
			error = sockopt_setint(sopt, optval);
			break;
#undef OPTBIT

		default:
			error = ENOPROTOOPT;
			break;
		}
		solock(so);
		break;
	}
	return (error);
}

/*
 * Both send and receive buffers are allocated PIPSIZ bytes of buffering
 * for stream sockets, although the total for sender and receiver is
 * actually only PIPSIZ.
 * Datagram sockets really use the sendspace as the maximum datagram size,
 * and don't really want to reserve the sendspace.  Their recvspace should
 * be large enough for at least one max-size datagram plus address.
 */
#define	PIPSIZ	4096
u_long	unpst_sendspace = PIPSIZ;
u_long	unpst_recvspace = PIPSIZ;
u_long	unpdg_sendspace = 2*1024;	/* really max datagram size */
u_long	unpdg_recvspace = 4*1024;

u_int	unp_rights;			/* files in flight */
u_int	unp_rights_ratio = 2;		/* limit, fraction of maxfiles */

int
unp_attach(struct socket *so)
{
	struct unpcb *unp;
	int error;

	switch (so->so_type) {
	case SOCK_SEQPACKET: /* FALLTHROUGH */
	case SOCK_STREAM:
		if (so->so_lock == NULL) {
			/* 
			 * XXX Assuming that no socket locks are held,
			 * as this call may sleep.
			 */
			so->so_lock = mutex_obj_alloc(MUTEX_DEFAULT, IPL_NONE);
			solock(so);
		}
		if (so->so_snd.sb_hiwat == 0 || so->so_rcv.sb_hiwat == 0) {
			error = soreserve(so, unpst_sendspace, unpst_recvspace);
			if (error != 0)
				return (error);
		}
		break;

	case SOCK_DGRAM:
		if (so->so_lock == NULL) {
			mutex_obj_hold(uipc_lock);
			so->so_lock = uipc_lock;
			solock(so);
		}
		if (so->so_snd.sb_hiwat == 0 || so->so_rcv.sb_hiwat == 0) {
			error = soreserve(so, unpdg_sendspace, unpdg_recvspace);
			if (error != 0)
				return (error);
		}
		break;

	default:
		panic("unp_attach");
	}
	KASSERT(solocked(so));
	unp = malloc(sizeof(*unp), M_PCB, M_NOWAIT);
	if (unp == NULL)
		return (ENOBUFS);
	memset((void *)unp, 0, sizeof(*unp));
	unp->unp_socket = so;
	so->so_pcb = unp;
	nanotime(&unp->unp_ctime);
	return (0);
}

void
unp_detach(struct unpcb *unp)
{
	struct socket *so;
	vnode_t *vp;

	so = unp->unp_socket;

 retry:
	if ((vp = unp->unp_vnode) != NULL) {
		sounlock(so);
		/* Acquire v_interlock to protect against unp_connect(). */
		/* XXXAD racy */
		mutex_enter(&vp->v_interlock);
		vp->v_socket = NULL;
		vrelel(vp, 0);
		solock(so);
		unp->unp_vnode = NULL;
	}
	if (unp->unp_conn)
		unp_disconnect(unp);
	while (unp->unp_refs) {
		KASSERT(solocked2(so, unp->unp_refs->unp_socket));
		if (unp_drop(unp->unp_refs, ECONNRESET)) {
			solock(so);
			goto retry;
		}
	}
	soisdisconnected(so);
	so->so_pcb = NULL;
	if (unp_rights) {
		/*
		 * Normally the receive buffer is flushed later, in sofree,
		 * but if our receive buffer holds references to files that
		 * are now garbage, we will enqueue those file references to
		 * the garbage collector and kick it into action.
		 */
		sorflush(so);
		unp_free(unp);
		unp_thread_kick();
	} else
		unp_free(unp);
}

int
unp_bind(struct socket *so, struct mbuf *nam, struct lwp *l)
{
	struct sockaddr_un *sun;
	struct unpcb *unp;
	vnode_t *vp;
	struct vattr vattr;
	size_t addrlen;
	int error;
	struct nameidata nd;
	proc_t *p;

	unp = sotounpcb(so);
	if (unp->unp_vnode != NULL)
		return (EINVAL);
	if ((unp->unp_flags & UNP_BUSY) != 0) {
		/*
		 * EALREADY may not be strictly accurate, but since this
		 * is a major application error it's hardly a big deal.
		 */
		return (EALREADY);
	}
	unp->unp_flags |= UNP_BUSY;
	sounlock(so);

	/*
	 * Allocate the new sockaddr.  We have to allocate one
	 * extra byte so that we can ensure that the pathname
	 * is nul-terminated.
	 */
	p = l->l_proc;
	addrlen = nam->m_len + 1;
	sun = malloc(addrlen, M_SONAME, M_WAITOK);
	m_copydata(nam, 0, nam->m_len, (void *)sun);
	*(((char *)sun) + nam->m_len) = '\0';

	NDINIT(&nd, CREATE, FOLLOW | LOCKPARENT | TRYEMULROOT, UIO_SYSSPACE,
	    sun->sun_path);

/* SHOULD BE ABLE TO ADOPT EXISTING AND wakeup() ALA FIFO's */
	if ((error = namei(&nd)) != 0)
		goto bad;
	vp = nd.ni_vp;
	if (vp != NULL) {
		VOP_ABORTOP(nd.ni_dvp, &nd.ni_cnd);
		if (nd.ni_dvp == vp)
			vrele(nd.ni_dvp);
		else
			vput(nd.ni_dvp);
		vrele(vp);
		error = EADDRINUSE;
		goto bad;
	}
	VATTR_NULL(&vattr);
	vattr.va_type = VSOCK;
	vattr.va_mode = ACCESSPERMS & ~(p->p_cwdi->cwdi_cmask);
	error = VOP_CREATE(nd.ni_dvp, &nd.ni_vp, &nd.ni_cnd, &vattr);
	if (error)
		goto bad;
	vp = nd.ni_vp;
	solock(so);
	vp->v_socket = unp->unp_socket;
	unp->unp_vnode = vp;
	unp->unp_addrlen = addrlen;
	unp->unp_addr = sun;
	unp->unp_connid.unp_pid = p->p_pid;
	unp->unp_connid.unp_euid = kauth_cred_geteuid(l->l_cred);
	unp->unp_connid.unp_egid = kauth_cred_getegid(l->l_cred);
	unp->unp_flags |= UNP_EIDSBIND;
	VOP_UNLOCK(vp, 0);
	unp->unp_flags &= ~UNP_BUSY;
	return (0);

 bad:
	free(sun, M_SONAME);
	solock(so);
	unp->unp_flags &= ~UNP_BUSY;
	return (error);
}

int
unp_connect(struct socket *so, struct mbuf *nam, struct lwp *l)
{
	struct sockaddr_un *sun;
	vnode_t *vp;
	struct socket *so2, *so3;
	struct unpcb *unp, *unp2, *unp3;
	size_t addrlen;
	int error;
	struct nameidata nd;

	unp = sotounpcb(so);
	if ((unp->unp_flags & UNP_BUSY) != 0) {
		/*
		 * EALREADY may not be strictly accurate, but since this
		 * is a major application error it's hardly a big deal.
		 */
		return (EALREADY);
	}
	unp->unp_flags |= UNP_BUSY;
	sounlock(so);

	/*
	 * Allocate a temporary sockaddr.  We have to allocate one extra
	 * byte so that we can ensure that the pathname is nul-terminated.
	 * When we establish the connection, we copy the other PCB's
	 * sockaddr to our own.
	 */
	addrlen = nam->m_len + 1;
	sun = malloc(addrlen, M_SONAME, M_WAITOK);
	m_copydata(nam, 0, nam->m_len, (void *)sun);
	*(((char *)sun) + nam->m_len) = '\0';

	NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF | TRYEMULROOT, UIO_SYSSPACE,
	    sun->sun_path);

	if ((error = namei(&nd)) != 0)
		goto bad2;
	vp = nd.ni_vp;
	if (vp->v_type != VSOCK) {
		error = ENOTSOCK;
		goto bad;
	}
	if ((error = VOP_ACCESS(vp, VWRITE, l->l_cred)) != 0)
		goto bad;
	/* Acquire v_interlock to protect against unp_detach(). */
	mutex_enter(&vp->v_interlock);
	so2 = vp->v_socket;
	if (so2 == NULL) {
		mutex_exit(&vp->v_interlock);
		error = ECONNREFUSED;
		goto bad;
	}
	if (so->so_type != so2->so_type) {
		mutex_exit(&vp->v_interlock);
		error = EPROTOTYPE;
		goto bad;
	}
	solock(so);
	unp_resetlock(so);
	mutex_exit(&vp->v_interlock);
	if ((so->so_proto->pr_flags & PR_CONNREQUIRED) != 0) {
		/*
		 * This may seem somewhat fragile but is OK: if we can
		 * see SO_ACCEPTCONN set on the endpoint, then it must
		 * be locked by the domain-wide uipc_lock.
		 */
		KASSERT((so->so_options & SO_ACCEPTCONN) == 0 ||
		    so2->so_lock == uipc_lock);
		if ((so2->so_options & SO_ACCEPTCONN) == 0 ||
		    (so3 = sonewconn(so2, 0)) == 0) {
			error = ECONNREFUSED;
			sounlock(so);
			goto bad;
		}
		unp2 = sotounpcb(so2);
		unp3 = sotounpcb(so3);
		if (unp2->unp_addr) {
			unp3->unp_addr = malloc(unp2->unp_addrlen,
			    M_SONAME, M_WAITOK);
			memcpy(unp3->unp_addr, unp2->unp_addr,
			    unp2->unp_addrlen);
			unp3->unp_addrlen = unp2->unp_addrlen;
		}
		unp3->unp_flags = unp2->unp_flags;
		unp3->unp_connid.unp_pid = l->l_proc->p_pid;
		unp3->unp_connid.unp_euid = kauth_cred_geteuid(l->l_cred);
		unp3->unp_connid.unp_egid = kauth_cred_getegid(l->l_cred);
		unp3->unp_flags |= UNP_EIDSVALID;
		if (unp2->unp_flags & UNP_EIDSBIND) {
			unp->unp_connid = unp2->unp_connid;
			unp->unp_flags |= UNP_EIDSVALID;
		}
		so2 = so3;
	}
	error = unp_connect2(so, so2, PRU_CONNECT);
	sounlock(so);
 bad:
	vput(vp);
 bad2:
	free(sun, M_SONAME);
	solock(so);
	unp->unp_flags &= ~UNP_BUSY;
	return (error);
}

int
unp_connect2(struct socket *so, struct socket *so2, int req)
{
	struct unpcb *unp = sotounpcb(so);
	struct unpcb *unp2;

	if (so2->so_type != so->so_type)
		return (EPROTOTYPE);

	/*
	 * All three sockets involved must be locked by same lock:
	 *
	 * local endpoint (so)
	 * remote endpoint (so2)
	 * queue head (so->so_head, only if PR_CONNREQUIRED)
	 */
	KASSERT(solocked2(so, so2));
	if (so->so_head != NULL) {
		KASSERT(so->so_lock == uipc_lock);
		KASSERT(solocked2(so, so->so_head));
	}

	unp2 = sotounpcb(so2);
	unp->unp_conn = unp2;
	switch (so->so_type) {

	case SOCK_DGRAM:
		unp->unp_nextref = unp2->unp_refs;
		unp2->unp_refs = unp;
		soisconnected(so);
		break;

	case SOCK_SEQPACKET: /* FALLTHROUGH */
	case SOCK_STREAM:
		unp2->unp_conn = unp;
		if (req == PRU_CONNECT &&
		    ((unp->unp_flags | unp2->unp_flags) & UNP_CONNWAIT))
			soisconnecting(so);
		else
			soisconnected(so);
		soisconnected(so2);
		/*
		 * If the connection is fully established, break the
		 * association with uipc_lock and give the connected
		 * pair a seperate lock to share.  For CONNECT2, we
		 * require that the locks already match (the sockets
		 * are created that way).
		 */
		if (req == PRU_CONNECT)
			unp_setpeerlocks(so, so2);
		break;

	default:
		panic("unp_connect2");
	}
	return (0);
}

void
unp_disconnect(struct unpcb *unp)
{
	struct unpcb *unp2 = unp->unp_conn;
	struct socket *so;

	if (unp2 == 0)
		return;
	unp->unp_conn = 0;
	so = unp->unp_socket;
	switch (so->so_type) {
	case SOCK_DGRAM:
		if (unp2->unp_refs == unp)
			unp2->unp_refs = unp->unp_nextref;
		else {
			unp2 = unp2->unp_refs;
			for (;;) {
				KASSERT(solocked2(so, unp2->unp_socket));
				if (unp2 == 0)
					panic("unp_disconnect");
				if (unp2->unp_nextref == unp)
					break;
				unp2 = unp2->unp_nextref;
			}
			unp2->unp_nextref = unp->unp_nextref;
		}
		unp->unp_nextref = 0;
		so->so_state &= ~SS_ISCONNECTED;
		break;

	case SOCK_SEQPACKET: /* FALLTHROUGH */
	case SOCK_STREAM:
		KASSERT(solocked2(so, unp2->unp_socket));
		soisdisconnected(so);
		unp2->unp_conn = 0;
		soisdisconnected(unp2->unp_socket);
		break;
	}
}

#ifdef notdef
unp_abort(struct unpcb *unp)
{
	unp_detach(unp);
}
#endif

void
unp_shutdown(struct unpcb *unp)
{
	struct socket *so;

	switch(unp->unp_socket->so_type) {
	case SOCK_SEQPACKET: /* FALLTHROUGH */
	case SOCK_STREAM:
		if (unp->unp_conn && (so = unp->unp_conn->unp_socket))
			socantrcvmore(so);
		break;
	default:
		break;
	}
}

bool
unp_drop(struct unpcb *unp, int errno)
{
	struct socket *so = unp->unp_socket;

	KASSERT(solocked(so));

	so->so_error = errno;
	unp_disconnect(unp);
	if (so->so_head) {
		so->so_pcb = NULL;
		/* sofree() drops the socket lock */
		sofree(so);
		unp_free(unp);
		return true;
	}
	return false;
}

#ifdef notdef
unp_drain(void)
{

}
#endif

static void dump_mbuf_chain(struct mbuf *, const char *) __attribute__((__used__));
static void dump_mbuf_chain(struct mbuf *m, const char *what)
{
 int i;
 const char *sep;
 const unsigned char *dp;
 int t;

 printf("%s:",what);
 sep = "";
 t = 0;
 for (;m;m=m->m_next)
  { printf("%s",sep);
    sep = " |";
    dp = (void *)mtod(m,char *);
    for (i=0;i<m->m_len;i++)
     { printf(" %02x",*dp++);
       if (t++ > 128)
	{ printf("...\n");
	  return;
	}
     }
  }
 printf("\n");
}

// State for unp_externalize.
typedef struct extern_state EXTERN_STATE;
struct extern_state {
  struct lwp *l;
  int err;
  int nfds;
  file_t *files[SCM_RIGHTS_MAX];
  int fds[SCM_RIGHTS_MAX];
  int nmem;
  INFLIGHT_MEMORY im[SCM_MEMORY_MAX];
  struct socket_memory sm[SCM_MEMORY_MAX];
  struct mbuf *imb;
  char *ibuf;
  int ioff;
  char *ebuf;
  int eoff;
  } ;

static int externalize_gather_rights(EXTERN_STATE *s)
{
 struct cmsghdr *cmh;
 struct proc *p;
 int nfds;
 file_t *fp;
 file_t **rp;
 int i;

 cmh = (void *)(s->ibuf+s->ioff);
 p = s->l->l_proc;
 if (cmh->cmsg_type != SCM_RIGHTS) panic("impossible externalize_gather_rights");
 nfds = (cmh->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr))) / sizeof(file_t *);
 rp = (file_t **)(s->ibuf+s->ioff+CMSG_ALIGN(sizeof(struct cmsghdr)));
 rw_enter(&p->p_cwdi->cwdi_lock,RW_READER);
 for (i=0;i<nfds;i++)
  { fp = rp[i];
    if (p->p_cwdi->cwdi_rdir)
     { /*
	* If we are in a chrooted directory, and someone wants to pass
	*  us a directory, make sure it's inside the subtree we're
	*  allowed to access.  (Otherwise, it's too easy for a
	*  subverted chrooted process to escape the chroot.)
	*/
       if (fp->f_type == DTYPE_VNODE)
	{ vnode_t *vp = (vnode_t *)fp->f_data;
	  if ( (vp->v_type == VDIR) &&
	       !vn_isunder(vp,p->p_cwdi->cwdi_rdir,s->l) )
	   { rw_exit(&p->p_cwdi->cwdi_lock);
	     return(EPERM);
	   }
	}
     }
    s->files[s->nfds++] = fp;
  }
 rw_exit(&p->p_cwdi->cwdi_lock);
 return(0);
}

static int externalize_gather_memory(EXTERN_STATE *s)
{
 struct cmsghdr *cmh;
 int nmb;
 int i;
 int o;
 INFLIGHT_MEMORY im;

 cmh = (void *)(s->ibuf+s->ioff);
 if (cmh->cmsg_type != SCM_MEMORY) panic("impossible externalize_gather_memory");
 nmb = (cmh->cmsg_len - sizeof(struct cmsghdr)) / sizeof(INFLIGHT_MEMORY);
 o = s->ioff + sizeof(struct cmsghdr);
 im.u.im.npages = 0;
 im.u.im.pgv = 0;
 for (i=0;i<nmb;i++) bcopy(s->ibuf+o,&s->im[s->nmem++],sizeof(INFLIGHT_MEMORY));
 return(0);
}

static int externalize_copy(EXTERN_STATE *s)
{
 struct cmsghdr *cmh;
 int n;

 cmh = (void *)(s->ibuf+s->ioff);
 bcopy(s->ibuf+s->ioff,s->ebuf+s->eoff,cmh->cmsg_len);
 n = CMSG_ALIGN(cmh->cmsg_len) - cmh->cmsg_len;
 if (n) bzero(s->ebuf+s->eoff+cmh->cmsg_len,n);
 s->eoff += CMSG_ALIGN(cmh->cmsg_len);
 return(0);
}

/*
 * XXX If ints have padding bits, we can leak kernel memory (stack
 *  trash or whatever) through them.  But I don't really see any way
 *  around that.
 */
static int externalize_process_rights(EXTERN_STATE *s)
{
 struct cmsghdr cmh;
 struct proc *p;
 char *ep;
 int i;
 int e;
 file_t *fp;

 if (s->nfds < 1) return(0);
 p = s->l->l_proc;
 bzero(&cmh,sizeof(cmh));
 cmh.cmsg_len = CMSG_LEN(s->nfds*sizeof(int));
 cmh.cmsg_level = SOL_SOCKET;
 cmh.cmsg_type = SCM_RIGHTS;
 ep = s->ebuf + s->eoff;
 i = sizeof(struct cmsghdr);
 bcopy(&cmh,ep,i);
 if (i != CMSG_SPACE(0)) bzero(ep+i,CMSG_SPACE(0)-i);
 ep += CMSG_SPACE(0);
 while <"retry"> (1)
  { for (i=s->nfds-1;i>=0;i--)
     { e = fd_alloc(p,0,&s->fds[i]);
       if (e)
	{ for (i--;i>=0;i--) fd_abort(p,0,s->fds[i]);
	  if (e == ENOSPC)
	   { fd_tryexpand(p);
	     continue <"retry">;
	   }
	  // This is the historical error, some code may expect it
	  return(EMSGSIZE);
	}
     }
    break;
  }
 i = s->nfds * sizeof(int);
 bcopy(&s->fds[0],ep,i);
 if (i != CMSG_ALIGN(i)) bzero(ep+i,CMSG_ALIGN(i)-i);
 s->eoff += CMSG_SPACE(i);
 for (i=s->nfds-1;i>=0;i--)
  { fp = s->files[i];
    atomic_dec_uint(&unp_rights);
    fd_affix(p,fp,s->fds[i]);
    mutex_enter(&fp->f_lock);
    fp->f_msgcount --;
    mutex_exit(&fp->f_lock);
    closef(fp);
  }
 return(0);
}

static void externalize_backout_rights(EXTERN_STATE *s)
{
 int i;

 for (i=s->nfds-1;i>=0;i--)
  { // We've already closef()ed the message's reference.
    fd_abort(s->l->l_proc,0,s->fds[i]);
  }
}

static void externalize_backout_memory(EXTERN_STATE *s, struct socket_memory *sm, int nsm)
{
 struct vm_map *map;

 map = &s->l->l_proc->p_vmspace->vm_map;
 for (;nsm>0;nsm--,sm++) uvm_unmap(map,(vaddr_t)sm->base,sm->size);
}

static int externalize_process_memory(EXTERN_STATE *s)
{
 struct cmsghdr cmh;
 char *ep;
 int i;
 vaddr_t dstva;
 struct vm_map *dstmap;
 vaddr_t pgva;
 INFLIGHT_MEMORY *im;
 int j;
 int j0;
 int e;

 if (s->nmem < 1) return(0);
 bzero(&cmh,sizeof(cmh));
 cmh.cmsg_len = sizeof(struct cmsghdr) + (s->nmem * sizeof(struct socket_memory));
 cmh.cmsg_level = SOL_SOCKET;
 cmh.cmsg_type = SCM_MEMORY;
 ep = s->ebuf + s->eoff;
 bzero(ep,CMSG_SPACE(s->nfds*sizeof(int)));
 bcopy(&cmh,ep,sizeof(struct cmsghdr));
 ep += sizeof(struct cmsghdr);
 dstmap = &s->l->l_proc->p_vmspace->vm_map;
 for (i=s->nmem-1;i>=0;i--)
  { im = &s->im[i];
    dstva = (dstmap->flags & VM_MAP_TOPDOWN) ? vm_map_max(dstmap) : vm_map_min(dstmap);
    if (! uvm_map_reserve(dstmap,im->u.im.npages*PAGE_SIZE,0,0,&dstva,0))
     {
#ifdef DEBUG_UNP_CTL
       printf("externalize_process_memory: uvm_map_reserve failed\n");
#endif
       if (i < s->nmem-1) externalize_backout_memory(s,&s->sm[i+1],s->nmem-1-i);
       return(ENOMEM);
     }
#ifdef DEBUG_UNP_CTL
    printf("externalize_process_memory: pages");
    for (j=0;j<im->u.im.npages;j++) printf(" %p",(void *)im->u.im.pgv[j]);
    printf("\n");
#endif
    j0 = -1;
    for (j=0;j<im->u.im.npages;j++)
     { if (j0 < 0)
	{ j0 = j;
	  continue;
	}
       if (im->u.im.pgv[j] == im->u.im.pgv[j-1]+PAGE_SIZE) continue;
       pgva = dstva + (j0 * PAGE_SIZE);
       e = uvm_map_extract(&unp_mem_map,im->u.im.pgv[j0],(j-j0)*PAGE_SIZE,dstmap,&pgva,UVM_EXTRACT_RESERVED);
       if (e)
	{
#ifdef DEBUG_UNP_CTL
	  printf("externalize_process_memory: uvm_map_extract 1 (%d@%p [%d]) failed\n",j-j0,(void *)im->u.im.pgv[j0],j0);
#endif
	  if (j0 > 0) uvm_unmap(dstmap,dstva,j0*PAGE_SIZE);
	  if (i < s->nmem-1) externalize_backout_memory(s,&s->sm[i+1],s->nmem-1-i);
	  return(e);
	}
       j0 = -1;
     }
    if (j0 >= 0)
     { pgva = dstva + (j0 * PAGE_SIZE);
       e = uvm_map_extract(&unp_mem_map,im->u.im.pgv[j0],(j-j0)*PAGE_SIZE,dstmap,&pgva,UVM_EXTRACT_RESERVED);
       if (e)
	{
#ifdef DEBUG_UNP_CTL
	  printf("externalize_process_memory: uvm_map_extract 2 (%d@%p [%d]) failed\n",j-j0,(void *)im->u.im.pgv[j0],j0);
#endif
	  if (j0 > 0) uvm_unmap(dstmap,dstva,j0*PAGE_SIZE);
	  if (i < s->nmem-1) externalize_backout_memory(s,&s->sm[i+1],s->nmem-1-i);
	  return(e);
	}
     }
    s->sm[i].base = (void *)dstva;
    s->sm[i].size = im->u.im.npages * PAGE_SIZE;
  }
 bcopy(&s->sm[0],ep,s->nmem*sizeof(struct socket_memory));
 i = s->nmem * sizeof(struct socket_memory);
 if (CMSG_SPACE(i) != CMSG_LEN(i)) bzero(ep+i,CMSG_SPACE(i)-CMSG_LEN(i));
 s->eoff += CMSG_SPACE(i);
 free_inflight_memory((void *)&s->im[0],s->nmem);
 return(0);
}

static void externalize_finish(EXTERN_STATE *s)
{
 bcopy(s->ebuf,s->ibuf,s->eoff);
 s->imb->m_len = s->eoff;
}

#ifdef DEBUG_UNP_CTL
static void dump_holding_map(const char *when)
{
 struct vm_map *m;
 struct vm_map_entry *e;

 printf("holding map at %s:\n",when);
 m = &unp_mem_map;
 printf("  flags %08x nentries %d size %llu\n",m->flags,m->nentries,(unsigned long long int)m->size);
 vm_map_lock_read(m);
 for (e=m->header.next;e!=&m->header;e=e->next)
  { printf("  %p:",(void *)e);
    if (e->etype & UVM_ET_OBJ) printf(" +OBJ"); else printf(" -obj");
    if (e->etype & UVM_ET_SUBMAP) printf(" +SUBMAP"); else printf(" -submap");
    if (e->etype & UVM_ET_COPYONWRITE) printf(" +COW"); else printf(" -cow");
    if (e->etype & UVM_ET_NEEDSCOPY) printf(" +NC"); else printf(" -nc");
    printf("\n  %c%c%c %c%c%c %p..%p\n",
	(e->protection & VM_PROT_READ) ? 'r' : '-',
	(e->protection & VM_PROT_WRITE) ? 'w' : '-',
	(e->protection & VM_PROT_EXECUTE) ? 'x' : '-',
	(e->max_protection & VM_PROT_READ) ? 'r' : '-',
	(e->max_protection & VM_PROT_WRITE) ? 'w' : '-',
	(e->max_protection & VM_PROT_EXECUTE) ? 'x' : '-',
	(void *)e->start, (void *)e->end);
  }
 vm_map_unlock_read(m);
}
#endif

/*
 * Convert a control message from internal form to external form.  l is
 *  the receiving lwp (the one into which we should, for example, drop
 *  fds created for SCM_RIGHTS messages).  This is the converse of
 *  unp_internalize().
 *
 * Notably, this processes a single mbuf, not an mbuf chain.  It is
 *  expected to do any of the actions of unp_dispose that need doing.
 *
 * There is a bit of an API botch here.  Our caller assumes that we
 *  will overwrite the contents of rights and adjust its length.  This
 *  means the external form cannot be longer than the internal form!
 *  In particular, this means that file_t * must be no smaller than
 *  int, and struct socket_memory must be no smaller than
 *  INFLIGHT_MEMORY.  We check these.
 *
 * If we really have to return more data, we can do it by chaining
 *  another mbuf off the argument one.  But it's not clear whether
 *  that's part of the API or an accident of our caller's
 *  implementation....
 *
 * If we return an error, we are still expected to dispose of rights.
 */
int unp_externalize(struct mbuf *rights, struct lwp *l)
{
 int i;
 EXTERN_STATE s;
 struct cmsghdr cmh;
 struct mbuf *m;

 if ( (sizeof(file_t *) < sizeof(int)) ||
      (sizeof(INFLIGHT_MEMORY) < sizeof(struct socket_memory)) )
  { panic("unp_externalize size assumptions wrong");
  }
 if (! l)
  { /*
     * Walk the whole chain, instead of returning as soon as we find
     *	something making us return 1, to verify the assumptions made by
     *	the code below.
     */
    s.ioff = 0;
    i = 0;
    while (s.ioff < rights->m_len)
     { if (s.ioff+sizeof(struct cmsghdr) > rights->m_len)
	{ i = 0;
#ifdef DEBUG_UNP_CTL
	  printf("unp_externalize: off %d + msghdr %d > m_len %d\n",s.ioff,(int)sizeof(struct cmsghdr),(int)rights->m_len);
#endif
	  break;
	}
       bcopy(mtod(rights,char *)+s.ioff,&cmh,sizeof(struct cmsghdr));
       if (s.ioff+cmh.cmsg_len > rights->m_len)
	{ i = 0;
#ifdef DEBUG_UNP_CTL
	  printf("unp_externalize: off %d + cmsg_len %d > m_len %d\n",s.ioff,(int)cmh.cmsg_len,(int)rights->m_len);
#endif
	  break;
	}
       switch (cmh.cmsg_type)
	{ case SCM_RIGHTS:
	  case SCM_MEMORY:
	     i = 1;
	     break;
	}
       s.ioff += CMSG_ALIGN(cmh.cmsg_len);
     }
    return(i);
  }
#ifdef DEBUG_UNP_CTL
 dump_mbuf_chain(rights,"unp_externalize entry");
 dump_holding_map("unp_externalize entry");
#endif
 m = m_get(M_WAIT,MT_CONTROL);
 MEXTMALLOC(m,rights->m_len,M_WAIT);
 m->m_len = rights->m_len;
 s.l = l;
 s.err = 0;
 s.nfds = 0;
 s.nmem = 0;
 s.imb = rights;
 s.ibuf = mtod(rights,char *);
 s.ioff = 0;
 s.ebuf = mtod(m,char *);
 s.eoff = 0;
 while (s.ioff < rights->m_len)
  { // overruns tested above; we return 0 if any, so shouldn't be here
#ifdef DEBUG_UNP_CTL
    printf("unp_externalize: %d:",s.ioff);
#endif
    if (s.ioff+sizeof(struct cmsghdr) > rights->m_len) panic("impossible: hdr overrun");
    bcopy(s.ibuf+s.ioff,&cmh,sizeof(struct cmsghdr));
    if (s.ioff+cmh.cmsg_len > rights->m_len) panic("impossible: cmsg overrun");
#ifdef DEBUG_UNP_CTL
    printf(" len %d:",(int)cmh.cmsg_len);
#endif
    switch (cmh.cmsg_type)
     { case SCM_RIGHTS:
#ifdef DEBUG_UNP_CTL
	  printf(" rights\n");
#endif
	  s.err = externalize_gather_rights(&s);
#ifdef DEBUG_UNP_CTL
	  printf("unp_externalize: after rights, err %d eoff now %d\n",s.err,s.eoff);
#endif
	  break;
       case SCM_MEMORY:
#ifdef DEBUG_UNP_CTL
	  printf(" memory\n");
#endif
	  s.err = externalize_gather_memory(&s);

#ifdef DEBUG_UNP_CTL
	  printf("unp_externalize: after memory, err %d eoff now %d\n",s.err,s.eoff);
#endif
	  break;
       default:
#ifdef DEBUG_UNP_CTL
	  printf(" other (%d)\n",(int)cmh.cmsg_type);
#endif
	  s.err = externalize_copy(&s);
#ifdef DEBUG_UNP_CTL
	  printf("unp_externalize: after other, err %d eoff now %d\n",s.err,s.eoff);
#endif
	  break;
     }
    if (s.err) break;
    s.ioff += CMSG_ALIGN(cmh.cmsg_len);
  }
#ifdef DEBUG_UNP_CTL
 printf("unp_externalize: nfds %d nmem %d eoff %d\n",s.nfds,s.nmem,s.eoff);
#endif
 if (s.err)
  {
#ifdef DEBUG_UNP_CTL
    printf("unp_externalize: s.err %d, discarding fds\n",s.err);
#endif
    for (i=s.nfds-1;i>=0;i--) unp_discard_later(s.files[i]);
  }
 else
  {
#ifdef DEBUG_UNP_CTL
    printf("unp_externalize: calling externalize_process_rights\n");
#endif
    s.err = externalize_process_rights(&s);
#ifdef DEBUG_UNP_CTL
    printf("unp_externalize: processed rights, err %d eoff %d\n",s.err,s.eoff);
#endif
  }
 if (s.err)
  {
#ifdef DEBUG_UNP_CTL
    printf("unp_externalize: s.err %d, freeing inflight memory\n",s.err);
#endif
    free_inflight_memory((void *)&s.im[0],s.nmem);
  }
 else
  {
#ifdef DEBUG_UNP_CTL
    printf("unp_externalize: calling externalize_process_memory\n");
#endif
    s.err = externalize_process_memory(&s);
#ifdef DEBUG_UNP_CTL
    printf("unp_externalize: processed memory, err %d eoff %d\n",s.err,s.eoff);
#endif
    if (s.err)
     {
#ifdef DEBUG_UNP_CTL
       printf("unp_externalize: s.err %d, backing out rights\n",s.err);
#endif
       externalize_backout_rights(&s);
     }
  }
 if (s.eoff > rights->m_len) panic("unp_externalize: message grew (%d > %d)",s.eoff,(int)rights->m_len);
 if (! s.err)
  { m->m_len = s.eoff;
#ifdef DEBUG_UNP_CTL
    dump_mbuf_chain(m,"unp_externalize exit");
#endif
  }
 if (! s.err) externalize_finish(&s);
 m_free(m);
#ifdef DEBUG_UNP_CTL
 dump_holding_map("unp_externalize exit");
#endif
 return(s.err);
}

// State for unp_internalize
typedef struct intern_state INTERN_STATE;
struct intern_state {
  int nfds;
  int nmem;
  struct mbuf **newctlt;
  } ;

static void internalize_release_fds(void *fpv, int n)
{
 file_t *fp;

 fp = fpv;
 for (;n>0;n--,fp++) unp_discard_now(fp);
}

// Pretty much the old unp_internalize, atrocious style and all.
static int internalize_rights(struct cmsghdr *cm, INTERN_STATE *s)
{
 filedesc_t *fdescp;
 struct cmsghdr *newcm;
 file_t **rp;
 file_t **files;
 file_t *fp;
 int i;
 int fd;
 int *fdp;
 int nfds;
 int error;
 u_int maxmsg;
 struct mbuf *m;

 /*
  * Without this, gcc complains that newcm "may be used uninitialized
  *  in this function".  Apparently it's too stupid to realize that the
  *  only ways to get to out: before setting newcm involve setting
  *  errno nonzero.  I'm tempted to shut off the warning, but it looks
  *  easier to waste a few cycles here. :-þ
  */
 newcm = 0;
 fdescp = curlwp->l_fd;
 /*
  * Verify that the file descriptors are valid, and acquire a reference
  *  to each.
  */
 nfds = (cm->cmsg_len - CMSG_ALIGN(sizeof(*cm))) / sizeof(int);
 if (s->nfds+nfds > SCM_RIGHTS_MAX) return(EMSGSIZE);
 s->nfds += nfds;
 fdp = (int *)CMSG_DATA(cm);
 maxmsg = maxfiles / unp_rights_ratio;
 for (i=0;i<nfds;i++)
  { fd = *fdp++;
    if (atomic_inc_uint_nv(&unp_rights) > maxmsg)
     { atomic_dec_uint(&unp_rights);
       nfds = i;
       error = EAGAIN;
       goto out;
     }
    if ( !(fp = fd_getfile(fd)) ||
	 (fp->f_type == DTYPE_KQUEUE) )
     { if (fp) fd_putfile(fd);
       atomic_dec_uint(&unp_rights);
       nfds = i;
       error = EBADF;
       goto out;
     }
  }
 /* Allocate new space and copy header into it. */
 newcm = malloc(CMSG_SPACE(nfds*sizeof(file_t *)),M_MBUF,M_WAITOK);
 if (! newcm)
  { error = E2BIG;
    goto out;
  }
 files = (file_t **)CMSG_DATA(newcm);
 /*
  * Transform the file descriptors into file_t pointers.
  */
 fdp = (int *)CMSG_DATA(cm);
 rp = files;
 for (i=0;i<nfds;i++)
  { fp = fdescp->fd_ofiles[*fdp++]->ff_file;
    KASSERT(fp);
    mutex_enter(&fp->f_lock);
    *rp++ = fp;
    fp->f_count ++;
    fp->f_msgcount ++;
    mutex_exit(&fp->f_lock);
  }
 error = 0;
out:
 /* Release descriptor references. */
 fdp = (int *)CMSG_DATA(cm);
 for (i=0;i<nfds;i++)
  { fd_putfile(*fdp++);
    if (error) atomic_dec_uint(&unp_rights);
  }
 if (! error)
  { m = m_get(M_WAIT,MT_CONTROL);
    MEXTADD(m,newcm,CMSG_SPACE(nfds*sizeof(file_t *)),M_MBUF,0,0);
    newcm->cmsg_len = CMSG_LEN(nfds*sizeof(file_t *));
    newcm->cmsg_level = SOL_SOCKET;
    newcm->cmsg_type = SCM_RIGHTS;
    m->m_len = CMSG_SPACE(nfds*sizeof(file_t *));
#ifdef DEBUG_UNP_CTL
    printf("internalize_rights: appending %p (%d)\n",(void *)m,m->m_len);
#endif
    *s->newctlt = m;
    s->newctlt = &m->m_next;
  }
 return(error);
}

void free_cmsg_rights(char *fvp, int nfp)
{
 file_t **fpp;

 fpp = (void *)fvp;
 for (;nfp>0;nfp--,fpp++)
  { unp_discard_later(*fpp);
    *fpp = 0;
  }
}

void free_inflight_memory(char *imp, int nim)
{
 INFLIGHT_MEMORY im;
 int i;

#ifdef DEBUG_UNP_CTL
 printf("free_inflight_memory:");
#endif
 for (;nim>0;nim--,imp+=sizeof(INFLIGHT_MEMORY))
  { bcopy(imp,&im,sizeof(INFLIGHT_MEMORY));
#ifdef DEBUG_UNP_CTL
    printf(" <%u@%p:",im.u.im.npages,(void *)im.u.im.pgv);
    for (i=0;i<im.u.im.npages;i++) printf(" %p",(void *)im.u.im.pgv[i]);
    printf(">");
#endif
    if (im.u.im.npages < 1) continue;
    for (i=im.u.im.npages-1;i>=0;i--) uvm_unmap(&unp_mem_map,im.u.im.pgv[i],im.u.im.pgv[i]+PAGE_SIZE);
    free(im.u.im.pgv,M_SOCKMEM);
    im.u.im.npages = 0;
    im.u.im.pgv = 0;
    bcopy(&im,imp,sizeof(INFLIGHT_MEMORY));
  }
#ifdef DEBUG_UNP_CTL
 printf("\n");
#endif
}

static int internalize_memory(struct cmsghdr *cm, INTERN_STATE *s)
{
 int nmb;
 struct cmsghdr nmh;
 struct socket_memory sm;
 INFLIGHT_MEMORY im;
 char *newbuf;
 int nbsp;
 struct mbuf *m;
 int i;
 int totsiz;
 int j;
 int npgs;
 vaddr_t *pxv;
 int k;
 vaddr_t mapped;
 int e;

 nmb = (cm->cmsg_len - sizeof(struct cmsghdr)) / sizeof(struct socket_memory);
 if (s->nmem+nmb > SCM_MEMORY_MAX) return(EMSGSIZE);
 s->nmem += nmb;
 if (cm->cmsg_len != sizeof(struct cmsghdr) + (nmb * sizeof(struct socket_memory)))
  { printf("internalize_memory: partial struct socket_memory in message: cmsg_len %d != %d + (%d * %d)\n",
	(int)cm->cmsg_len, (int)sizeof(struct cmsghdr), nmb, (int)sizeof(struct socket_memory) );
    return(EINVAL);
  }
 nbsp = sizeof(struct cmsghdr) + (nmb * sizeof(INFLIGHT_MEMORY));
 newbuf = malloc(CMSG_ALIGN(nbsp),M_MBUF,M_WAITOK);
 do <"err">
  { // Validate and compute total size
    totsiz = 0;
    for (i=nmb-1;i>=0;i--)
     { bcopy(((char *)(cm+1))+(i*sizeof(struct socket_memory)),&sm,sizeof(struct socket_memory));
       if ( (((vaddr_t)sm.base) & PAGE_MASK) ||
	    (sm.size & PAGE_MASK) )
	{ e = EINVAL;
	  break <"err">;
	}
       totsiz += sm.size;
     }
    totsiz /= PAGE_SIZE;
    do <"err">
     { // There's enough space.  Find pages and save them.
       for (i=nmb-1;i>=0;i--)
	{ bcopy(((char *)(cm+1))+(i*sizeof(struct socket_memory)),&sm,sizeof(struct socket_memory));
	  npgs = sm.size / PAGE_SIZE;
	  MALLOC(pxv,vaddr_t *,npgs*sizeof(vaddr_t),M_SOCKMEM,M_WAITOK);
	  mutex_enter(&unp_mem_mutex);
	  e = 0;
	  // We really want something more like unwind-protect, here
	  do <"err">
	   { /*
	      * Try to grab it all as one chunk.  If we succeed, win.
	      *	If not, break it down and do each page separately.
	      *	Because we checked unp_mem, above, we_should never run
	      *	out of space, so any error from the second attempt is a
	      *	real error.  On error, just uvm_unmap what we mapped in
	      *	unp_mem_map and return the error.
	      */
	     e = uvm_map_extract( &curlwp->l_proc->p_vmspace->vm_map, (vaddr_t)(char *)sm.base, sm.size,
				  &unp_mem_map, &mapped, 0 );
	     if (e == ENOMEM)
	      { for <"pages"> (j=(sm.size/PAGE_SIZE)-1;j>=0;j--)
		 { e = uvm_map_extract( &curlwp->l_proc->p_vmspace->vm_map, (vaddr_t)(((char *)sm.base)+(j*PAGE_SIZE)), PAGE_SIZE,
					&unp_mem_map, &mapped, 0 );
		   if (e)
		    { for (k=(sm.size/PAGE_SIZE)-1;k>j;k--) uvm_unmap(&unp_mem_map,pxv[k],pxv[k]+PAGE_SIZE);
		      break <"err">;
		    }
		   pxv[j] = mapped;
		 }
	      }
	     else
	      { for (j=(sm.size/PAGE_SIZE)-1;j>=0;j--)
		 { pxv[j] = mapped + (j * PAGE_SIZE);
		 }
	      }
	     im.u.im.npages = npgs;
	     im.u.im.pgv = pxv;
	     pxv = 0;
	     bcopy(&im,newbuf+sizeof(struct cmsghdr)+(i*sizeof(INFLIGHT_MEMORY)),sizeof(INFLIGHT_MEMORY));
	   } while (0);
	  mutex_exit(&unp_mem_mutex);
	  if (pxv) FREE(pxv,M_SOCKMEM);
	  if (e) break <"err">;
	}
       nmh.cmsg_len = sizeof(struct cmsghdr) + (nmb*sizeof(INFLIGHT_MEMORY));
       nmh.cmsg_level = SOL_SOCKET;
       nmh.cmsg_type = SCM_MEMORY;
       bcopy(&nmh,newbuf,sizeof(struct cmsghdr));
       m = m_get(M_WAIT,MT_CONTROL);
       MEXTADD(m,newbuf,nbsp,M_MBUF,0,0);
       m->m_len = CMSG_ALIGN(nbsp);
#ifdef DEBUG_UNP_CTL
       printf("internalize_memory: appending %p (%d)\n",(void *)m,m->m_len);
#endif
       *s->newctlt = m;
       s->newctlt = &m->m_next;
       return(0);
     } while (0);
    free_inflight_memory(newbuf+sizeof(struct cmsghdr)+(i*sizeof(INFLIGHT_MEMORY)),nmb-1-i);
  } while (0);
 free(newbuf,M_WAITOK);
 return(e);
}

static void internalize_backout_rights(struct mbuf *m)
{
 struct cmsghdr *cmh;

 if (m->m_len < sizeof(struct cmsghdr)) panic("internalize_backout_rights: impossible size");
 cmh = mtod(m,struct cmsghdr *);
 if (cmh->cmsg_type != SCM_RIGHTS) panic("internalize_backout_rights: impossible type");
 internalize_release_fds(CMSG_DATA(cmh),(cmh->cmsg_len-CMSG_ALIGN(sizeof(struct cmsghdr)))/sizeof(int));
}

static void internalize_backout_memory(struct mbuf *m)
{
 char *dp;
 struct cmsghdr cmh;

 if (m->m_len < sizeof(struct cmsghdr)) panic("internalize_backout_rights: impossible size");
 dp = mtod(m,char *);
 bcopy(dp,&cmh,sizeof(struct cmsghdr));
 if (cmh.cmsg_type != SCM_MEMORY) panic("internalize_backout_memory: impossible type");
#ifdef DIAGNOSTIC
  { int i;
    i = (cmh.cmsg_len - sizeof(struct cmsghdr)) % sizeof(INFLIGHT_MEMORY);
    if (i)
     { printf("internalize_backout_memory: leftover bytes in message: (%d-%d) %% %d = %d\n",
	(int)cmh.cmsg_len,(int)sizeof(struct cmsghdr),(int)sizeof(INFLIGHT_MEMORY),i);
     }
  }
#endif
 free_inflight_memory(dp+sizeof(struct cmsghdr),(cmh.cmsg_len-sizeof(struct cmsghdr))/sizeof(INFLIGHT_MEMORY));
}

/*
 * We can get away with assuming only one message per mbuf only because
 *  that's what internalize_rights() et al, which created this mbuf
 *  chain, generate.
 */
static void internalize_backout(struct mbuf *m)
{
 struct mbuf *m2;
 struct cmsghdr cmh;

 while (m)
  { if (m->m_len >= sizeof(struct cmsghdr))
     { bcopy(mtod(m,char *),&cmh,sizeof(struct cmsghdr));
       switch (cmh.cmsg_type)
	{ case SCM_RIGHTS:
	     internalize_backout_rights(m);
	     break;
	  case SCM_MEMORY:
	     internalize_backout_memory(m);
	     break;
	  default:
	     printf("internalize_backout: impossible type %d\n",(int)cmh.cmsg_type);
	     break;
	}
     }
    MFREE(m,m2);
    m = m2;
  }
}

/*
 * Convert a control message buffer from external (userland) form to
 *  internal (kernel) form.
 *
 * For SCM_RIGHTS messages, this means converting from an array of int
 *  to an array of file_t *.  For SCM_MEMORY messages, this means
 *  converting from an array of struct socket_memory to an array of
 *  struct inflight_memory *.
 *
 * The control message buffer must all be in a single mbuf.
 *
 * There is a severe API design bug here.  The SCM_RIGHTS API silently
 *  changed.  The traditional way had the buffer containing a bunch of
 *  ints, packed tightly after the struct cmsghdr.  But then someone
 *  took the CMSG_* macros, invented for, near as I can tell, all the
 *  control crap IPv6 wants to fling around, and gratuitously inflicted
 *  them on SCM_RIGHTS messages.  If sizeof(int) is not a power of two,
 *  or if it's smaller than __cmsg_alignbytes+1, the new ABI differs
 *  from the old, thus breaking formerly working code.  What's more,
 *  the CMSG_* macros are a broken API; they don't support any coding
 *  style except overlaying structs onto the buffer, which means the
 *  buffer must be suitably aligned for struct cmsghdr; worse, it must
 *  be aligned on a __cmsg_alignbytes() boundary - see CMSG_ALIGN
 *  (presumably the implementation makes sure that __cmsg_alignbytes
 *  alignment is suitable for struct cmsghdr).  There also is no way in
 *  the API to compute the data pointer except as a function of the
 *  struct cmsghdr pointer, making it impossible to operate portably by
 *  copying the struct cmsghdr out of the buffer, because computing the
 *  data pointer may then generate a pointer past the end of an object.
 *  (We kludge around this here by going under the hood and knowing how
 *  the internals of the APU work.)
 *
 * Unfortunately, there is too much code in the tree that uses
 *  SCM_RIGHTS the broken ("new") way for us to just fix this.  And we
 *  can't just define SCM_FIXEDRIGHTS or some such, because the kernel,
 *  not userland, chooses the control message type during recvmsg() (if
 *  we generate SCM_FIXEDRIGHTS, code that expects SCM_RIGHTS won't
 *  know what to do; if we generate SCM_RIGHTS, code will expect it to
 *  use the CMSG_* crap).
 *
 * So we continue to do SCM_RIGHTS the broken way.  But at least we can
 *  make sure SCM_MEMORY is done right, though we do have to CMSG_ALIGN
 *  the SCM_MEMORY message as a whole, so that an SCM_RIGHTS after it
 *  can use the same API as an SCM_RIGHTS before it.
 */
int unp_internalize(struct mbuf **controlp)
{
 struct mbuf *ctlm;
 struct cmsghdr *cm;
 struct mbuf *newctl;
 int off;
 int pad;
 int e;
 INTERN_STATE s;

 ctlm = *controlp;
#ifdef DEBUG_UNP_CTL
 dump_mbuf_chain(ctlm,"unp_internalize entry");
 dump_holding_map("unp_internalize entry");
#endif
 off = 0;
 pad = 0;
 s.nfds = 0;
 s.nmem = 0;
 s.newctlt = &newctl;
 while <"done"> (1)
  {
#ifdef DEBUG_UNP_CTL
    printf("unp_internalize: at %d\n",off);
#endif
    if (off > ctlm->m_len) panic("unp_internalize: impossible overrun (%d > %d)\n",off,(int)ctlm->m_len);
    off += pad;
    if (off != CMSG_ALIGN(off)) panic("unp_internalize: misaligned\n");
    if (off >= ctlm->m_len)
     {
#ifdef DEBUG_UNP_CTL
       printf("unp_internalize: loop done\n");
#endif
       e = 0;
       break;
     }
    if (off+sizeof(struct cmsghdr) > ctlm->m_len)
     {
#ifdef DEBUG_UNP_CTL
       printf("unp_internalize: msg overruns buffer 1 (%d+%d > %d)\n",
		(int)off, (int)sizeof(struct cmsghdr), (int)ctlm->m_len);
#endif
       e = EINVAL;
       break;
     }
    cm = (void *)(mtod(ctlm,char *) + off);
    if (off+cm->cmsg_len > ctlm->m_len)
     {
#ifdef DEBUG_UNP_CTL
       printf("unp_internalize: msg overruns buffer 2 (%d+%d > %d)\n",
		(int)off, (int)CMSG_LEN(cm->cmsg_len), (int)ctlm->m_len);
#endif
       e = EINVAL;
       break;
     }
    if ( (cm->cmsg_level != SOL_SOCKET) ||
	 (off+cm->cmsg_len > ctlm->m_len) )
     { e = EINVAL;
       break;
     }
    switch (cm->cmsg_type)
     { case SCM_RIGHTS:
	  if (cm->cmsg_len < CMSG_ALIGN(sizeof(struct cmsghdr)))
	   { e = EINVAL;
	     break <"done">;
	   }
#ifdef DEBUG_UNP_CTL
	  printf("unp_internalize: rights\n");
#endif
	  e = internalize_rights(cm,&s);
	  if (e) break <"done">;
	  break;
       case SCM_MEMORY:
#ifdef DEBUG_UNP_CTL
	  printf("unp_internalize: memory\n");
#endif
	  e = internalize_memory(cm,&s);
	  if (e) break <"done">;
	  break;
       default:
#ifdef DEBUG_UNP_CTL
	  printf("unp_internalize: unsupported type %d\n",(int)cm->cmsg_type);
#endif
	  e = EINVAL;
	  break <"done">;
     }
    off += cm->cmsg_len;
    pad = CMSG_ALIGN(cm->cmsg_len) - cm->cmsg_len;
  }
 *s.newctlt = 0;
#ifdef DEBUG_UNP_CTL
 printf("unp_internalize: error %d, new chain",e);
  { struct mbuf *m;
    for (m=newctl;m;m=m->m_next) printf(" %p",(void *)m);
  }
 printf("\n");
#endif
 if (e)
  { internalize_backout(newctl);
  }
 else
  { struct mbuf *m;
    struct mbuf *t;
    if (! newctl) panic("unp_internalize: success but no new ctl mbuf");
    off = 0;
    for (ctlm=newctl;ctlm;ctlm=ctlm->m_next) off += ctlm->m_len;
    m = m_get(M_WAIT,MT_CONTROL);
    if (! m)
     { // Is this possible with M_WAIT/M_WAITOK?
       internalize_backout(newctl);
       e = ENOMEM;
     }
    else
     { if (off > MLEN)
	{ char *data;
	  data = malloc(off,M_MBUF,M_WAITOK);
	  MEXTADD(m,data,off,M_MBUF,0,0);
	}
       m->m_len = off;
       off = 0;
       for (t=newctl;t;t=t->m_next)
	{ bcopy(mtod(t,char *),mtod(m,char *)+off,t->m_len);
	  off += t->m_len;
	}
       if (off != m->m_len) panic("unp_internalize: off %d m_len %d\n",off,(int)m->m_len);
       m_freem(newctl);
       newctl = m;
     }
  }
 if (! e)
  { m_freem(*controlp);
    *controlp = newctl;
#ifdef DEBUG_UNP_CTL
    dump_mbuf_chain(newctl,"unp_internalize exit");
#endif
  }
#ifdef DEBUG_UNP_CTL
 dump_holding_map("unp_internalize exit");
#endif
 return(e);
}

struct mbuf *
unp_addsockcred(struct lwp *l, struct mbuf *control)
{
	struct cmsghdr *cmp;
	struct sockcred *sc;
	struct mbuf *m, *n;
	int len, space, i;

	len = CMSG_LEN(SOCKCREDSIZE(kauth_cred_ngroups(l->l_cred)));
	space = CMSG_SPACE(SOCKCREDSIZE(kauth_cred_ngroups(l->l_cred)));

	m = m_get(M_WAIT, MT_CONTROL);
	if (space > MLEN) {
		if (space > MCLBYTES)
			MEXTMALLOC(m, space, M_WAITOK);
		else
			m_clget(m, M_WAIT);
		if ((m->m_flags & M_EXT) == 0) {
			m_free(m);
			return (control);
		}
	}

	m->m_len = space;
	m->m_next = NULL;
	cmp = mtod(m, struct cmsghdr *);
	sc = (struct sockcred *)CMSG_DATA(cmp);
	cmp->cmsg_len = len;
	cmp->cmsg_level = SOL_SOCKET;
	cmp->cmsg_type = SCM_CREDS;
	sc->sc_uid = kauth_cred_getuid(l->l_cred);
	sc->sc_euid = kauth_cred_geteuid(l->l_cred);
	sc->sc_gid = kauth_cred_getgid(l->l_cred);
	sc->sc_egid = kauth_cred_getegid(l->l_cred);
	sc->sc_ngroups = kauth_cred_ngroups(l->l_cred);
	for (i = 0; i < sc->sc_ngroups; i++)
		sc->sc_groups[i] = kauth_cred_group(l->l_cred, i);

	/*
	 * If a control message already exists, append us to the end.
	 */
	if (control != NULL) {
		for (n = control; n->m_next != NULL; n = n->m_next)
			;
		n->m_next = m;
	} else
		control = m;

	return (control);
}

/*
 * Do a mark-sweep GC of files in the system, to free up any which are
 * caught in flight to an about-to-be-closed socket.  Additionally,
 * process deferred file closures.
 */
static void
unp_gc(file_t *dp)
{
	extern	struct domain unixdomain;
	file_t *fp, *np;
	struct socket *so, *so1;
	u_int i, old, new;
	bool didwork;

	KASSERT(curlwp == unp_thread_lwp);
	KASSERT(mutex_owned(&filelist_lock));

	/*
	 * First, process deferred file closures.
	 */
	while (!SLIST_EMPTY(&unp_thread_discard)) {
		fp = SLIST_FIRST(&unp_thread_discard);
		KASSERT(fp->f_unpcount > 0);
		KASSERT(fp->f_count > 0);
		KASSERT(fp->f_msgcount > 0);
		KASSERT(fp->f_count >= fp->f_unpcount);
		KASSERT(fp->f_count >= fp->f_msgcount);
		KASSERT(fp->f_msgcount >= fp->f_unpcount);
		SLIST_REMOVE_HEAD(&unp_thread_discard, f_unplist);
		i = fp->f_unpcount;
		fp->f_unpcount = 0;
		mutex_exit(&filelist_lock);
		for (; i != 0; i--) {
			unp_discard_now(fp);
		}
		mutex_enter(&filelist_lock);
	}

	/*
	 * Clear mark bits.  Ensure that we don't consider new files
	 * entering the file table during this loop (they will not have
	 * FSCAN set).
	 */
	unp_defer = 0;
	LIST_FOREACH(fp, &filehead, f_list) {
		for (old = fp->f_flag;; old = new) {
			new = atomic_cas_uint(&fp->f_flag, old,
			    (old | FSCAN) & ~(FMARK|FDEFER));
			if (__predict_true(old == new)) {
				break;
			}
		}
	}

	/*
	 * Iterate over the set of sockets, marking ones believed (based on
	 * refcount) to be referenced from a process, and marking for rescan
	 * sockets which are queued on a socket.  Recan continues descending
	 * and searching for sockets referenced by sockets (FDEFER), until
	 * there are no more socket->socket references to be discovered.
	 */
	do {
		didwork = false;
		for (fp = LIST_FIRST(&filehead); fp != NULL; fp = np) {
			KASSERT(mutex_owned(&filelist_lock));
			np = LIST_NEXT(fp, f_list);
			mutex_enter(&fp->f_lock);
			if ((fp->f_flag & FDEFER) != 0) {
				atomic_and_uint(&fp->f_flag, ~FDEFER);
				unp_defer--;
				KASSERT(fp->f_count != 0);
			} else {
				if (fp->f_count == 0 ||
				    (fp->f_flag & FMARK) != 0 ||
				    fp->f_count == fp->f_msgcount ||
				    fp->f_unpcount != 0) {
					mutex_exit(&fp->f_lock);
					continue;
				}
			}
			atomic_or_uint(&fp->f_flag, FMARK);

			if (fp->f_type != DTYPE_SOCKET ||
			    (so = fp->f_data) == NULL ||
			    so->so_proto->pr_domain != &unixdomain ||
			    (so->so_proto->pr_flags & PR_RIGHTS) == 0) {
				mutex_exit(&fp->f_lock);
				continue;
			}

			/* Gain file ref, mark our position, and unlock. */
			didwork = true;
			LIST_INSERT_AFTER(fp, dp, f_list);
			fp->f_count++;
			mutex_exit(&fp->f_lock);
			mutex_exit(&filelist_lock);

			/*
			 * Mark files referenced from sockets queued on the
			 * accept queue as well.
			 */
			solock(so);
			unp_scan(so->so_rcv.sb_mb, &unp_mark, 0);
			if ((so->so_options & SO_ACCEPTCONN) != 0) {
				TAILQ_FOREACH(so1, &so->so_q0, so_qe) {
					unp_scan(so1->so_rcv.sb_mb, &unp_mark, 0);
				}
				TAILQ_FOREACH(so1, &so->so_q, so_qe) {
					unp_scan(so1->so_rcv.sb_mb, &unp_mark, 0);
				}
			}
			sounlock(so);

			/* Re-lock and restart from where we left off. */
			closef(fp);
			mutex_enter(&filelist_lock);
			np = LIST_NEXT(dp, f_list);
			LIST_REMOVE(dp, f_list);
		}
		/*
		 * Bail early if we did nothing in the loop above.  Could
		 * happen because of concurrent activity causing unp_defer
		 * to get out of sync.
		 */
	} while (unp_defer != 0 && didwork);

	/*
	 * Sweep pass.
	 *
	 * We grab an extra reference to each of the files that are
	 * not otherwise accessible and then free the rights that are
	 * stored in messages on them.
	 */
	for (fp = LIST_FIRST(&filehead); fp != NULL; fp = np) {
		KASSERT(mutex_owned(&filelist_lock));
		np = LIST_NEXT(fp, f_list);
		mutex_enter(&fp->f_lock);

		/*
		 * Ignore non-sockets.
		 * Ignore dead sockets, or sockets with pending close.
		 * Ignore sockets obviously referenced elsewhere. 
		 * Ignore sockets marked as referenced by our scan.
		 * Ignore new sockets that did not exist during the scan.
		 */
		if (fp->f_type != DTYPE_SOCKET ||
		    fp->f_count == 0 || fp->f_unpcount != 0 ||
		    fp->f_count != fp->f_msgcount ||
		    (fp->f_flag & (FMARK | FSCAN)) != FSCAN) {
			mutex_exit(&fp->f_lock);
			continue;
		}

		/* Gain file ref, mark our position, and unlock. */
		LIST_INSERT_AFTER(fp, dp, f_list);
		fp->f_count++;
		mutex_exit(&fp->f_lock);
		mutex_exit(&filelist_lock);

		/*
		 * Flush all data from the socket's receive buffer.
		 * This will cause files referenced only by the
		 * socket to be queued for close.
		 */
		so = fp->f_data;
		solock(so);
		sorflush(so);
		sounlock(so);

		/* Re-lock and restart from where we left off. */
		closef(fp);
		mutex_enter(&filelist_lock);
		np = LIST_NEXT(dp, f_list);
		LIST_REMOVE(dp, f_list);
	}
}

/*
 * Garbage collector thread.  While SCM_RIGHTS messages are in transit,
 * wake once per second to garbage collect.  Run continually while we
 * have deferred closes to process.
 */
static void
unp_thread(void *cookie)
{
	file_t *dp;

	/* Allocate a dummy file for our scans. */
	if ((dp = fgetdummy()) == NULL) {
		panic("unp_thread");
	}

	mutex_enter(&filelist_lock);
	for (;;) {
		KASSERT(mutex_owned(&filelist_lock));
		if (SLIST_EMPTY(&unp_thread_discard)) {
			if (unp_rights != 0) {
				(void)cv_timedwait(&unp_thread_cv,
				    &filelist_lock, hz);
			} else {
				cv_wait(&unp_thread_cv, &filelist_lock);
			}
		}
		unp_gc(dp);
	}
	/* NOTREACHED */
}

/*
 * Kick the garbage collector into action if there is something for
 * it to process.
 */
static void
unp_thread_kick(void)
{

	if (!SLIST_EMPTY(&unp_thread_discard) || unp_rights != 0) {
		mutex_enter(&filelist_lock);
		cv_signal(&unp_thread_cv);
		mutex_exit(&filelist_lock);
	}
}

static void unp_dispose_msg(void *sv, struct cmsghdr *mh)
{
 (void)sv;
 if (mh->cmsg_level != SOL_SOCKET) return;
 switch (mh->cmsg_type)
  { case SCM_RIGHTS:
       // already handled by unp_scan - see unp_dispose()
       break;
    case SCM_MEMORY:
       free_inflight_memory((void *)(mh+1),(mh->cmsg_len-sizeof(struct cmsghdr))/sizeof(INFLIGHT_MEMORY));
       break;
  }
}

/*
 * Deal with anything that needs dealing with before the mbuf chain is
 *  freed.
 *
 * We call unp_scan to handle file descriptors in SCM_RIGHTS messages,
 *  then use walk_control_msgs with unp_dispose_*() (above) to handle
 *  others (at this writing, only SCM_MEMORY).
 */
void unp_dispose(struct mbuf *m)
{
 if (! m) return;
 unp_scan(m,&unp_discard_later,1);
 walk_control_msgs(m,0,&unp_dispose_msg,0);
}

typedef struct usstate USSTATE;
struct usstate {
  int clear;
  void (*op)(file_t *);
  } ;

static void unp_scan_msg(void *sv, struct cmsghdr *mh)
{
 USSTATE *s;
 int nfds;
 file_t **fpp;
 file_t *fp;
 int i;

 s = sv;
 if (mh->cmsg_level != SOL_SOCKET) return;
 if (mh->cmsg_type != SCM_RIGHTS) return;
#ifdef DIAGNOSTIC
 i = (mh->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr))) % sizeof(file_t *);
 if (i) printf("unp_scan: partial pointer in SCM_RIGHTS message ((%d-%d) %% %d = %d)\n",
	(int)mh->cmsg_len,(int)CMSG_ALIGN(sizeof(struct cmsghdr)),(int)sizeof(file_t *),i);
#endif
 nfds = (mh->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr))) / sizeof(file_t *);
 fpp = (void *)CMSG_DATA(mh);
 for (i=0;i<nfds;i++)
  { fp = fpp[i];
    if (s->clear) fpp[i] = 0;
    (*s->op)(fp);
  }
}

/*
 * Scan an mbuf chain, calling op for each fd in each SCM_RIGHTS
 *  control message in the chain.  This doesn't do anything with other
 *  SCM_ messages; the calls in unp_gc are just about files, and the
 *  only other call, in unp_dispose, handles non-SCM_RIGHTS messages
 *  separately.
 *
 * Freeing the mbuf chain, when appropriate, must be done elsewhere.
 */
static void unp_scan(struct mbuf *m0, void (*op)(file_t *), int clear)
{
 USSTATE s;

 s.clear = clear;
 s.op = op;
 walk_control_msgs(m0,0,&unp_scan_msg,&s);
}

void
unp_mark(file_t *fp)
{

	if (fp == NULL)
		return;

	/* If we're already deferred, don't screw up the defer count */
	mutex_enter(&fp->f_lock);
	if (fp->f_flag & (FMARK | FDEFER)) {
		mutex_exit(&fp->f_lock);
		return;
	}

	/*
	 * Minimize the number of deferrals...  Sockets are the only type of
	 * file which can hold references to another file, so just mark
	 * other files, and defer unmarked sockets for the next pass.
	 */
	if (fp->f_type == DTYPE_SOCKET) {
		unp_defer++;
		KASSERT(fp->f_count != 0);
		atomic_or_uint(&fp->f_flag, FDEFER);
	} else {
		atomic_or_uint(&fp->f_flag, FMARK);
	}
	mutex_exit(&fp->f_lock);
}

static void
unp_discard_now(file_t *fp)
{

	if (fp == NULL)
		return;

	KASSERT(fp->f_count > 0);
	KASSERT(fp->f_msgcount > 0);

	mutex_enter(&fp->f_lock);
	fp->f_msgcount--;
	mutex_exit(&fp->f_lock);
	atomic_dec_uint(&unp_rights);
	(void)closef(fp);
}

static void
unp_discard_later(file_t *fp)
{

	if (fp == NULL)
		return;

	KASSERT(fp->f_count > 0);
	KASSERT(fp->f_msgcount > 0);

	mutex_enter(&filelist_lock);
	if (fp->f_unpcount++ == 0) {
		SLIST_INSERT_HEAD(&unp_thread_discard, fp, f_unplist);
	}
	mutex_exit(&filelist_lock);
}
