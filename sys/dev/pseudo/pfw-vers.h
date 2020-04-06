#ifndef _PFW_VERS_H_35f83eb8_
#define _PFW_VERS_H_35f83eb8_

/*
 * We have to include this very early, to get __NetBSD_Version__.
 *
 * This also hides a lot of include-file bugs; I don't entirely
 *  understand _why_ including <sys/param.h> hides so many of them when
 *  they're clearly not missing parameters - for example, uvm/uvm.h,
 *  through the chain uvm/uvm_extern.h -> uvm/uvm_param.h ->
 *  machine/vmparam.h, uses struct simplelock without making sure it's
 *  defined first.  Since <sys/param.h> doesn't define it, I don't
 *  understand why including it first hides this bug, but it does.
 *
 * This is a mixed blessing.  It means we don't have to fix (or paper
 *  over) a lot of stuff - but it also means we lose out on a chance to
 *  find (and fix properly) that same lot of stuff.  Since we have to
 *  include it anyway to get __NetBSD_Version__, we make a virtue of
 *  necessity and declare we consider the former the greater. :)
 */
#include <sys/param.h>

/*
 * Unfortunately there is no way for us to create a #define which pfw.c
 *  can use to get the appropriate list of include files.  So, instead,
 *  there's a flag (PFW_INCLUDE_INCLUDES) which can be defined before
 *  including this; when that's done, it includes the appropriate
 *  include files, based on a flag define set up the first time through
 *  (which may be the inclusion which has PFW_INCLUDE_INCLUDES set).
 *  PFW_INCLUDE_INCLUDES is #undeffed after the include files are
 *  brought in.
 *
 * This is a bit ugly, in that it separates the includes from the code,
 *  but I consider the alternative - flag macros here which pfw.c uses
 *  to include the correct files - even worse; it means that porting to
 *  a new OS version requires creating a new flag macro visible to
 *  pfw.c (the current scheme requires creating one anyway, but it's
 *  private to this file), and clutters pfw.c (which I'd like to keep
 *  relatively uncluttered) with lots of "extra" #include lines.
 *
 * However, C's preprocessor design pretty much compels something of
 *  the sort.  This choice means that this file can't have multiple
 *  inclusions streamlined by (eg) gcc's recognition and optimization
 *  of the #ifndef/#define/.../#endif idiom, but I think that counts as
 *  optimizing machine costs (a little unnecessary processing, which
 *  doesn't happen much anyway since we don't include this file very
 *  often) at the expense of human costs (dealing with cluttered code).
 *  (The apparent idempotency guard at the top of this file actually
 *  protects only part of the file; its matching #endif is the one just
 *  before "#ifdef PFW_INCLUDE_INCLUDES", not the one at the end of the
 *  file.)
 */

#define PFW_VERSION_NOT_SUPPORTED

#ifndef __NetBSD_Version__
#error "No __NetBSD_Version__ - this version not supported"
#endif

#if __NetBSD_Version__ == 502000000
/* 5.2 */
#undef PFW_VERSION_NOT_SUPPORTED
#define PFW_VERSION_5_2
#define MONO_TIME_SEC time_second
#define DECLARE_TICKER_HANDLE \
	static struct callout ticker_h;
#define ATTACH_TIME_EARLY() do { callout_init(&ticker_h,0); } while (0)
#define ATTACH_TIME_LATE() do ; while (0)
#define TICKER_SETUP() callout_setfunc(&ticker_h,&ticker,0)
#define RESET_TICKER() callout_schedule(&ticker_h,hz)
#define DEVICE_ROUTINE_DECLS /* nothing */
#define DEVSW_SCLASS static
#define DEFINE_CDEVSW \
	const struct cdevsw pfw_cdevsw				\
	 = { &pfwopen, &pfwclose, &pfwread, &pfwwrite,		\
	     &pfwioctl, nostop, notty, &pfwpoll, nommap,	\
	     nokqfilter, D_OTHER };
#define DECLARE_INET_PFIL_HEAD extern struct pfil_head inet_pfil_hook;
#define SETUP_PFW_HOOK() pfil_add_hook(&pfw_hook,0,PFIL_IN,&inet_pfil_hook)
#define PFW_HOOK_ARGS \
	void *arg __attribute__((__unused__)),	\
	struct mbuf **m,			\
	struct ifnet *ifp,			\
	int dir __attribute__((__unused__))
#define PFW_HOOK_LOCALS \
	struct ip *ip = mtod(*m,struct ip *);				\
	int hlen = ip->ip_hl << 2;
#define USRREQ_ARG(p) (p)
#define FDRELEASE(fd) fd_close((fd))
#define PROCTYPE struct lwp *
#define FILE_USE(x) do ; while (0)
#define FILE_UNUSE(x) fd_putfile((x))
#define CLOSEF(fp) closef((fp))
#define WATCHER_TYPE struct lwp *
#define CREATE_WATCHER(fn,arg,pp) kthread_create(PRI_NONE,0,0,(fn),(arg),(pp),"pfw-watcher")
#define GETSOCK(fd,e,fp) ((e) = (((fp) = fd_getfile((fd))) ? 0 : EBADF))
#define CADDR_T void *
#endif

#if __NetBSD_Version__ == 400000003
/* 4.0.1 */
/*
 * The description of __NetBSD_Version__ in <sys/param.h> leads me to
 *  expect it to be 400000100, not 400000003.  I don't know why the
 *  discrepancy, but 400000003 is what it's set to in 4.0.1.
 */
#undef PFW_VERSION_NOT_SUPPORTED
#define PFW_VERSION_4_0_1
#define MONO_TIME_SEC time_second
#define DECLARE_TICKER_HANDLE \
	static struct callout ticker_h = CALLOUT_INITIALIZER;
#define ATTACH_TIME_EARLY() do ; while (0)
#define ATTACH_TIME_LATE() do ; while (0)
#define TICKER_SETUP() callout_setfunc(&ticker_h,&ticker,0)
#define RESET_TICKER() callout_schedule(&ticker_h,hz)
#define DEVICE_ROUTINE_DECLS /* nothing */
#define DEVSW_SCLASS static
#define DEFINE_CDEVSW \
	const struct cdevsw pfw_cdevsw				\
	 = { &pfwopen, &pfwclose, &pfwread,			\
	     &pfwwrite, &pfwioctl, nostop, notty,		\
	     &pfwpoll, nommap, nokqfilter, D_OTHER };
#define DECLARE_INET_PFIL_HEAD extern struct pfil_head inet_pfil_hook;
#define SETUP_PFW_HOOK() pfil_add_hook(&pfw_hook,0,PFIL_IN,&inet_pfil_hook)
#define PFW_HOOK_ARGS \
	void *arg __attribute__((__unused__)),	\
	struct mbuf **m,			\
	struct ifnet *ifp,			\
	int dir __attribute__((__unused__))
#define PFW_HOOK_LOCALS \
	struct ip *ip = mtod(*m,struct ip *);				\
	int hlen = ip->ip_hl << 2;
#define USRREQ_ARG(p) LIST_FIRST(&(p)->p_lwps)
#define FDRELEASE(fd) fdrelease(curlwp,(fd))
#define PROCTYPE struct lwp *
#define CLOSEF(fp) closef(fp,0)
#define WATCHER_TYPE struct proc *
#define CREATE_WATCHER(fn,arg,pp) kthread_create1((fn),(arg),(pp),"pfw-watcher")
#define GETSOCK(fd,e,fp) e = getsock(curproc->p_fd,fd,&fp)
#endif

#if __NetBSD_Version__ == 104200000
/* 1.4T */
#undef PFW_VERSION_NOT_SUPPORTED
#define PFW_VERSION_1_4T
#define MONO_TIME_SEC mono_time.tv_sec
#define DECLARE_TICKER_HANDLE /* nothing */
#define ATTACH_TIME_EARLY() do ; while (0)
#define ATTACH_TIME_LATE() do ; while (0)
#define TICKER_SETUP() do { } while (0)
#define RESET_TICKER() timeout(&ticker,0,hz)
#define DEVICE_ROUTINE_DECLS \
	extern int pfwopen(dev_t, int, int, struct proc *);	\
	extern int pfwclose(dev_t, int, int, struct proc *);	\
	extern int pfwread(dev_t, struct uio *, int);		\
	extern int pfwwrite(dev_t, struct uio *, int);		\
	extern int pfwioctl(dev_t, u_long, caddr_t, int, struct proc *);\
	extern int pfwpoll(dev_t, int, struct proc *);
#define DEVSW_SCLASS /* nothing */
#define DEFINE_CDEVSW /* nothing */
#define DECLARE_INET_PFIL_HEAD /* nothing */
#define SETUP_PFW_HOOK() pfil_add_hook(&pfw_hook,PFIL_IN,&inetsw[ip_protox[IPPROTO_IP]])
#define PFW_HOOK_ARGS \
	void *data,				\
	int hlen,				\
	struct ifnet *ifp,			\
	int dir __attribute__((__unused__)),	\
	struct mbuf **m
#define PFW_HOOK_LOCALS \
	struct ip *ip = data;
#define USRREQ_ARG(p) p
#define FDRELEASE(fd) fdrelease(curproc,(fd))
#define PROCTYPE struct proc *
#define CLOSEF(fp) closef(fp,0)
#define WATCHER_TYPE struct proc *
#define CREATE_WATCHER(fn,arg,pp) kthread_create1((fn),(arg),(pp),"pfw-watcher")
#define GETSOCK(fd,e,fp) e = getsock(curproc->p_fd,fd,&fp)
#endif

#ifdef PFW_VERSION_NOT_SUPPORTED
#error "This NetBSD version not supported"
#endif

#endif

#ifdef PFW_INCLUDE_INCLUDES

#ifdef PFW_VERSION_5_2
#undef PFW_INCLUDE_INCLUDES
#include <net/if.h>
#include <sys/mbuf.h>
#include <sys/poll.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <net/pfil.h>
#include <sys/systm.h>
#include <sys/mutex.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/condvar.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/filedesc.h>
#include <sys/socketvar.h>
#include <machine/stdarg.h>
#endif

#ifdef PFW_VERSION_4_0_1
#undef PFW_INCLUDE_INCLUDES
#include <net/if.h>
#include <sys/file.h>
#include <sys/mbuf.h>
#include <sys/proc.h>
#include <sys/poll.h>
#include <sys/conf.h>
#include <net/pfil.h>
#include <sys/malloc.h>
#include <netinet/ip.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/kthread.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/protosw.h>
#include <sys/filedesc.h>
#include <sys/socketvar.h>
#include <machine/stdarg.h>
#endif

#ifdef PFW_VERSION_1_4T
#undef PFW_INCLUDE_INCLUDES
#include <net/if.h>
#include <sys/file.h>
#include <net/pfil.h>
#include <sys/mbuf.h>
#include <sys/poll.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <netinet/ip.h>
#include <sys/protosw.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/kthread.h>
#include <sys/filedesc.h>
#include <sys/socketvar.h>
#include <machine/stdarg.h>
#endif

#ifdef PFW_INCLUDE_INCLUDES
#error "missing includes block - PFW_INCLUDE_INCLUDES survived"
#endif

/*
 * Even though it goes with #ifdef PFW_INCLUDE_INCLUDES rather than the
 *  apparent idempotency guard at the top of the file, make sure the
 *  file ends this way to keep wraphfile happy.
 */

#endif
