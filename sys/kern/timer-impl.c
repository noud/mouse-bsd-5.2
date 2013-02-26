#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mutex.h>
#include <sys/domain.h>
#include <sys/callout.h>
#include <sys/socketvar.h>
#include <sys/timersock.h>

/*
 * Locking is rudimentary: take tsock_gblmtx when doing anything that
 *  calls for locking.
 *
 * This is because I've been unable to think of a way to do it any
 *  better than this.  Because of the heap manipulations that happen in
 *  the callout, it must be global-locked.  But it has to global-lock
 *  before it can lock any individual PCB's lock, so individual locks
 *  must be nested within the global lock.  This then means that the
 *  only manipulations that could benefit from fine-grained locks are
 *  ones that don't take the global lock at all, and, because most
 *  manipulations end up potentially touching the heap, there basically
 *  aren't any of those.
 *
 * The original used spl locking, so global-locking everything here
 *  amounts to much the same thing, only MP-friendlier in that other
 *  CPUs can carry on with non-timer-socket stuff while we're locked.
 *
 * So individual PCBs have no locks of their own; everything is locked
 *  with tsock_gblmtx.
 */

/*
 * Goddammit.  When will NeetBSD stop preempting perfectly reasonable
 *  names with each new release!?  4.0.1 stole min, now 5.2 is stealing
 *  timer_tick and assorted other timer_* names.  So all the names are
 *  now tsock_* instead of timer_*.  Who knows when _that_ will get
 *  stolen too....
 */

typedef struct timerpcb PCB;

struct timerpcb {
  struct socket *socket;
  struct itimerval itv;
  int flags;
#define TPF_RUNNING  0x00000001
#define TPF_NOWRITES 0x00000002
  int hx;
  int pending;
  struct mbuf *emerg;
  } ;

volatile int timerdebug = 0;

static kmutex_t tsock_gblmtx;
static volatile int ticking;
static volatile int nrunning;
static PCB ** volatile heap;
static volatile int heapa;
static struct callout tsock_callout;

/* Why they put the comparison out at the end I'll never figure out. */
#define tvcmp(a,c,b) timercmp(a,b,c)

static void heap_verify(void)
{
#ifdef DEBUG
 int i;
 int n;
 PCB **h;

 KDASSERT(mutex_owned(&tsock_gblmtx));
 n = nrunning;
 h = heap;
 if (n < 0) panic("heap_verify nrunning<0");
 if (n > heapa) panic("heap_verify nrunning>heapa");
 for (i=0;i<n;i++)
  { PCB *t;
    t = h[i];
    if (t->hx != i) panic("heap_verifyy index wrong (heap[%d]->hx = %d)",i,t->hx);
    if (i > 0)
     { int u;
       u = (i-1) >> 1;
       if (tvcmp(&h[i]->itv.it_value,<,&h[u]->itv.it_value))
	{ panic("heap_verify: heap invariant violated: [%d]=%lu.%06lu, [%d]=%lu.%06lu\n",i,(unsigned long int)h[i]->itv.it_value.tv_sec,(unsigned long int)h[i]->itv.it_value.tv_usec,u,(unsigned long int)h[u]->itv.it_value.tv_sec,(unsigned long int)h[u]->itv.it_value.tv_usec);
	}
     }
  }
#endif
}

static void heap_bubble_up(int x, PCB *t)
{
 int u;
 PCB **h;

 KASSERT(mutex_owned(&tsock_gblmtx));
 h = heap;
 while (x > 0)
  { u = (x-1) >> 1;
    if (tvcmp(&t->itv.it_value,>=,&h[u]->itv.it_value)) break;
    (h[x]=h[u])->hx = x;
    x = u;
  }
 h[x] = t;
 t->hx = x;
}

/* we need to bubble either up or down, not both, but telling which
   involves an extra test, roughly equivalent to doing both anyway. */
static void heap_remove(PCB *t)
{
 int x;
 int l;
 int r;
 int s;
 PCB *xpcb;
 int n;
 PCB **h;

 KASSERT(mutex_owned(&tsock_gblmtx));
 heap_verify();
 n = --nrunning;
 if (n < 0) panic("timer heap_remove");
 if (n < 1) return;
 h = heap;
 x = t->hx;
 xpcb = h[n];
 while (1)
  { l = x + x + 1;
    r = l + 1;
    if ((l < n) && tvcmp(&h[l]->itv.it_value,<,&xpcb->itv.it_value))
     { if ((r < n) && tvcmp(&h[r]->itv.it_value,<,&xpcb->itv.it_value))
	{ if (tvcmp(&h[l]->itv.it_value,<,&h[r]->itv.it_value))
	   { s = l;
	   }
	  else
	   { s = r;
	   }
	}
       else
	{ s = l;
	}
     }
    else
     { if ((r < n) && tvcmp(&h[r]->itv.it_value,<,&xpcb->itv.it_value))
	{ s = r;
	}
       else
	{ heap_bubble_up(x,xpcb);
	  heap_verify();
	  return;
	}
     }
    (h[x]=h[s])->hx = x;
    x = s;
  }
}

static void heap_insert(PCB *pcb)
{
 int n;

 KASSERT(mutex_owned(&tsock_gblmtx));
 heap_verify();
 n = nrunning;
 if (n >= heapa)
  { int a;
    a = n + 8;
    heapa = a;
    heap = realloc(heap,a*sizeof(PCB *),M_TEMP,M_WAITOK);
  }
 nrunning = n + 1;
 heap_bubble_up(n,pcb);
 heap_verify();
}

static int tsock_gentick(PCB *t)
{
 struct mbuf *m;
 struct timersock_event ev;
 int rv;

 KASSERT(mutex_owned(&tsock_gblmtx));
 if (! t->emerg) return(0);
 solock(t->socket);
 if (sbspace(&t->socket->so_rcv) < sizeof(struct timersock_event))
  { ((volatile PCB *)t)->pending ++;
    if (timerdebug) printf("gentick %p no space\n",(void *)t);
    rv = 0;
  }
 else
  { bzero(&ev,sizeof(ev));
    MGET(m,M_NOWAIT,MT_DATA);
    if (m == 0)
     { m = ((volatile PCB *)t)->emerg;
       ((volatile PCB *)t)->emerg = 0;
       ev.tse_type = TS_EV_ERROR;
       ev.tse_err = ENOBUFS;
       if (timerdebug) printf("gentick %p used emerg\n",(void *)t);
       rv = 0;
     }
    else
     { ev.tse_type = TS_EV_TICK;
       microtime(&ev.tse_tv);
       if (timerdebug) printf("gentick %p normal\n",(void *)t);
       rv = 1;
     }
    *mtod(m,struct timersock_event *) = ev;
    m->m_len = sizeof(struct timersock_event);
    sbappend(&t->socket->so_rcv,m);
    sorwakeup(t->socket);
  }
 sounlock(t->socket);
 return(rv);
}

static void tsock_tick(void *arg __attribute__((__unused__)))
{
 PCB *t;
 struct timeval now;

 mutex_enter(&tsock_gblmtx);
 microtime(&now);
 while ((nrunning > 0) && tvcmp(&(t=heap[0])->itv.it_value,<=,&now))
  { if (timerdebug) printf("tsock_tick %p fired\n",(void *)t);
    heap_remove(t);
    if (timerisset(&t->itv.it_interval))
     { do
	{ tsock_gentick(t);
	  timeradd(&t->itv.it_value,&t->itv.it_interval,&t->itv.it_value);
	} while (tvcmp(&t->itv.it_value,<=,&now));
       heap_insert(t);
       if (timerdebug) printf("tsock_tick %p reinstalled %lu.%06lu\n",(void *)t,t->itv.it_value.tv_sec,t->itv.it_value.tv_usec);
     }
    else
     { tsock_gentick(t);
       timerclear(&t->itv.it_value);
       t->flags &= ~TPF_RUNNING;
       if (timerdebug) printf("tsock_tick %p done\n",(void *)t);
     }
  }
 if (nrunning > 0)
  { callout_schedule(&tsock_callout,1);
  }
 else
  { ticking = 0;
    if (timerdebug) printf("tsock_tick stopping ticker\n");
  }
 mutex_exit(&tsock_gblmtx);
}

static void tsock_halt(PCB *pcb)
{
 KASSERT(mutex_owned(&tsock_gblmtx));
 if (pcb->flags & TPF_RUNNING)
  { pcb->flags &= ~TPF_RUNNING;
    heap_remove(pcb);
  }
}

static void tsock_readsome(PCB *pcb)
{
 struct mbuf *m;

 mutex_enter(&tsock_gblmtx);
 while <"catchup"> (1)
  { if (pcb->emerg == 0)
     { MGET(m,M_WAIT,MT_DATA);
       ((volatile PCB *)pcb)->emerg = m;
     }
    if (pcb->pending)
     { while (pcb->pending)
	{ pcb->pending --;
	  if (! tsock_gentick(pcb)) break <"catchup">;
	}
       continue;
     }
    break;
  }
 mutex_exit(&tsock_gblmtx);
}

static void tsock_start(PCB *pcb)
{
 KASSERT(mutex_owned(&tsock_gblmtx));
 if (timerdebug) printf("tsock_start %p heap %d/%d\n",(void *)pcb,nrunning,heapa);
 heap_insert(pcb);
 pcb->flags |= TPF_RUNNING;
 if (! ticking)
  { if (timerdebug) printf("tsock_start %p starting ticker\n",(void *)pcb);
    callout_schedule(&tsock_callout,1);
    ticking = 1;
  }
 else
  { if (timerdebug) printf("tsock_start %p ticker already running\n",(void *)pcb);
  }
}

static int tsock_set(PCB *pcb, struct mbuf *m)
{
 struct itimerval itv;
 int e;

 m = m_pullup(m,sizeof(struct itimerval));
 if (m == 0) return(EINVAL);
 itv = *mtod(m,struct itimerval *);
 m_freem(m);
 e = itimerfix(&itv.it_value);
 if (! e) e = itimerfix(&itv.it_interval);
 if (! e)
  { mutex_enter(&tsock_gblmtx);
    if (pcb->flags & TPF_RUNNING) tsock_halt(pcb);
    if (timerisset(&itv.it_value))
     { struct timeval now;
       microtime(&now);
       timeradd(&now,&itv.it_value,&pcb->itv.it_value);
       pcb->itv.it_interval = itv.it_interval;
       if (timerdebug) printf("tsock_set %p start %lu.%06lu+%lu.%06lu=%lu.%06lu\n",(void *)pcb,now.tv_sec,now.tv_usec,itv.it_value.tv_sec,itv.it_value.tv_usec,pcb->itv.it_value.tv_sec,pcb->itv.it_value.tv_usec);
       tsock_start(pcb);
     }
    else
     { pcb->itv = itv;
       if (timerdebug) printf("tsock_set %p unset\n",(void *)pcb);
     }
    mutex_exit(&tsock_gblmtx);
  }
 return(e);
}

static int tsock_ctloutput(int op, struct socket *so, struct sockopt *opt)
{
 KASSERT(solocked(so));
 return(ENOPROTOOPT);
}

static int tsock_usrreq(
	struct socket *so,
	int req,
	struct mbuf *m,
	struct mbuf *nam,
	struct mbuf *control,
	struct lwp *p	)
{
 PCB *pcb;
 int e;

 pcb = (void *) so->so_pcb;
 if (req != PRU_ATTACH)
  { if (! pcb) return(EINVAL);
    KASSERT(solocked(so));
  }
 switch (req)
  { case PRU_ATTACH:
       if (pcb) return(EISCONN);
       if (! so->so_lock)
	{ so->so_lock = mutex_obj_alloc(MUTEX_DEFAULT,IPL_NONE);
	  solock(so);
	}
       if ((so->so_snd.sb_hiwat == 0) || (so->so_rcv.sb_hiwat == 0))
	{ e = soreserve(so,256,8192);
	  if (e) return(e);
	}
       KASSERT(solocked(so));
       pcb = malloc(sizeof(PCB),M_PCB,M_NOWAIT);
       if (pcb == 0) return(ENOBUFS);
       pcb->socket = so;
       pcb->flags = 0;
       pcb->pending = 0;
       pcb->emerg = m_get(M_WAIT,MT_DATA);
       so->so_pcb = pcb;
       soisconnected(so);
       if (timerdebug) printf("timer PRU_ATTACH, socket %p, pcb %p\n",(void *)so,(void *)pcb);
       break;
    case PRU_DETACH:
       if (timerdebug) printf("timer PRU_DETACH, pcb %p\n",(void *)pcb);
       mutex_enter(&tsock_gblmtx);
       if (pcb->flags & TPF_RUNNING) tsock_halt(pcb);
       mutex_exit(&tsock_gblmtx);
       if (pcb->emerg) m_freem(pcb->emerg);
       free(pcb,M_PCB);
       so->so_pcb = 0;
       break;
    case PRU_CONTROL:
	{ unsigned long int cmd;
	  void *data;
	  if (pcb == 0) return(ENXIO);
	  cmd = (unsigned long int) m;
	  data = nam;
	  printf("timer PRU_CONTROL, pcb %p cmd %08lx data %p\n",(void *)pcb,cmd,data);
	  switch (cmd)
	   { case _IOW('~',0,int):
		timerdebug = *(int *)data;
		break;
	     case _IOR('~',1,int):
		*(int *)data = timerdebug;
		break;
	     default:
		return(ENOTTY);
		break;
	   }
	  return(0);
	}
       break;
    case PRU_BIND:
    case PRU_LISTEN:
    case PRU_CONNECT:
    case PRU_CONNECT2:
    case PRU_ACCEPT:
       return(EOPNOTSUPP);
       break;
    case PRU_DISCONNECT:
       soisdisconnected(so);
       break;
    case PRU_SHUTDOWN:
       socantsendmore(so);
       pcb->flags |= TPF_NOWRITES;
       break;
    case PRU_RCVD:
       tsock_readsome(pcb);
       break;
    case PRU_SEND:
       if (control)
	{ m_freem(control);
	  m_freem(m);
	  return(EOPNOTSUPP);
	}
       if (nam)
	{ m_freem(m);
	  return(EINVAL);
	}
       return(tsock_set(pcb,m));
       break;
    case PRU_ABORT:
       panic("tsock_usrreq PRU_ABORT");
       break;
    case PRU_SENSE:
	{ struct stat *stp;
	  static struct timespec zts = { 0, 0 };
	  stp = (void *) m;
	  stp->st_blksize = so->so_rcv.sb_hiwat;
	  stp->st_dev = NODEV;
	  stp->st_atimespec = zts;
	  stp->st_mtimespec = zts;
	  stp->st_ctimespec = zts;
	  stp->st_ino = 0;
	}
       break;
    case PRU_RCVOOB:
    case PRU_SOCKADDR:
    case PRU_PEERADDR:
       return(EOPNOTSUPP);
       break;
    case PRU_SENDOOB:
       m_freem(control);
       m_freem(m);
       return(EOPNOTSUPP);
       break;
    default:
       panic("tsock_usrreq");
       break;
  }
 return(0);
}

static void tsock_init(void)
{
 mutex_init(&tsock_gblmtx,MUTEX_DEFAULT,IPL_NONE);
 ticking = 0;
 nrunning = 0;
 heap = 0;
 heapa = 0;
 callout_init(&tsock_callout,0);
 callout_setfunc(&tsock_callout,&tsock_tick,0);
}

DOMAIN_DEFINE(timerdomain);

static struct protosw timersw[]
 = { { .pr_type = SOCK_STREAM,
       .pr_domain = &timerdomain,
       .pr_flags = PR_ATOMIC|PR_WANTRCVD,
       .pr_ctloutput = &tsock_ctloutput,
       .pr_usrreq = &tsock_usrreq } };

struct domain timerdomain
 = { .dom_init = &tsock_init,
     .dom_family = PF_TIMER,
     .dom_name = "timers",
     .dom_protosw = &timersw[0],
     .dom_protoswNPROTOSW = &timersw[sizeof(timersw)/sizeof(timersw[0])] };
