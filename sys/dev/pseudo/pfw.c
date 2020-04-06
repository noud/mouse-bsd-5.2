/*
 * Watcher notify messagse:
 *
 *	key	data and meaning
 *	z	none
 *		all blocking was cleared
 *	d	address
 *		address was deleted
 *	m	address
 *		memory shortage trying to list address
 *	a	address, int=N, N bytes of packet data
 *		address was listed; packet causing listing follows
 *		if listing was from ioctl, packet is zero-length
 *	f	address, int=N, N bytes of packet data
 *		just like a except address was already listed ("freshen")
 *
 * "address" means a 4-byte IP address in network byte order.
 *
 * This driver runs giantlocked, which is why we can get away with
 *  locking against ourselves with spl*().
 */

#include "pfw.h"
#if NPFW > 0

#define PFW_INCLUDE_INCLUDES
#include <dev/pseudo/pfw-vers.h>

#include <dev/pseudo/pfw-kern.h>

#define EXPIRE 86400
#define HOLDDOWN 3600

typedef struct softc SOFTC;
typedef struct ftn FTN;
typedef struct watch WATCH;

#define PFWUNIT(m) ((m) >> 4)
#define PFWKIND(m) ((m) & 15)

#define PFWKIND_IFACE  0
#define PFWKIND_CLEAR  1
#define PFWKIND_SERIAL 2
#define PFWKIND_COUNT  3
#define PFWKIND_LIST   4
#define PFWKIND_WATCH  5
#define PFWKIND_ADD    6
#define PFWKIND_DEL    7
#define PFWKIND_SAVE   8

struct softc {
  int unit;
  struct ifnet *fifp;
  int nftn;
  int aftn;
  FTN **ftnv;
  FTN *ftnr;
  WATCH *watchers;
  } ;

struct watch {
  WATCH *link;
  struct file *f;
  int errored;
  } ;

/*
 * FTNs are kept simultaneously in two data structures: (a) a heap, in
 *  ftnv[] (with aftn the allocated size and nftn the live size), with
 *  the comparison criterion such that the top of the heap has minimum
 *  exp value, and (b) a balanced binary tree, with u, l, and r
 *  pointers, such that it's a binary search tree on the addr values,
 *  with bal representing the imbalance figure (+ve means the right
 *  subtree is deeper, -ve, the left).
 */
struct ftn {
  FTN *u;
  FTN *l;
  FTN *r;
  int bal;
  u_int32_t addr;
  time_t exp;
  time_t upd;
  int hx;
  } ;

static SOFTC pfw_softc[NPFW];
static int maxlive;
static int running;
static int attached;
static unsigned long long int serial;
static WATCHER_TYPE watchproc;
DECLARE_TICKER_HANDLE
DECLARE_INET_PFIL_HEAD
static int noverify = 0;
static WATCH *deadwatch;
static kmutex_t deadwatch_mtx;
static kcondvar_t deadwatch_cv;

/*
 * Rebalance the binary tree after an insertion or deletion.  *up is
 *  the root pointer of the (sub)tree we want to maybe rebalance; uptr
 *  is the correct thing to put in the u field of an element replacing
 *  *up.  The tree must be self-consistent (all u, l, r pointers right,
 *  all bal values correct given the structure).  The input tree must
 *  not be unbalanced by more than 2.
 *
 * Return value is 1 if the result is unbalanced (it will never be
 *  unbalanced by more than 1) or 0 if it is now balanced.
 */
static int rebalance(FTN **up, FTN *uptr)
{
 FTN *u;
 FTN *f;
 FTN *b;
 FTN *c;

 u = *up;
 if (uptr != u->u) printf("pfw rebalance: uptr wrong\n");
 switch (u->bal)
  { case 0:
       return(0);
       break;
    case -1: case 1:
       return(1);
       break;
    case -2:
       if (u->l->bal <= 0)
	{ /*
	   * Pivot:
	   *
	   *					u (-2)
	   *			A (-1)				B
	   *		C		f		       / \
	   *	       / \	       / \		      /   \
	   *	      /   \           /   \		     -------
	   *	     /     \	     -------
	   *	    ---------
	   * becomes
	   *				A (0)
	   *		C				u (0)
	   *	       / \			f		B
	   *	      /   \		       / \	       / \
	   *	     /     \		      /   \	      /   \
	   *	    ---------		     -------	     -------
	   *
	   * and
	   *
	   *					u (-2)
	   *			A (0)				B
	   *		C		f		       / \
	   *	       / \	       / \		      /   \
	   *	      /   \	      /   \		     -------
	   *	     /     \	     /     \
	   *	    ---------	    ---------
	   * becomes
	   *				A (1)
	   *		C				u (-1)
	   *	       / \			f		B
	   *	      /   \		       / \	       / \
	   *	     /     \		      /   \	      /   \
	   *	    ---------		     /     \	     -------
	   *				    ---------
	   */
	  u->bal = -1 - u->l->bal;
	  u->l->bal ++;
	  *up = u->l;
	  u->l->u = uptr;
	  f = u->l->r;
	  u->l->r = u;
	  u->u = u->l;
	  u->l = f;
	  if (f) f->u = u;
	  if (u->bal) return(1);
	}
       else if (u->l->bal > 0)
	{ /*
	   * Pivot:
	   *
	   *					u (-2)
	   *			A (1)				B
	   *		C		f (?)		       / \
	   *	       / \	   b	     c		      /   \
	   *	      /   \	  / \	    / \		     /     \
	   *	     /     \	 /   \	   /   \	    ---------
	   *	    ---------	----  \	  ----  \
	   *			    ----      ----
	   * becomes
	   *				    f (0)
	   *		    A (-1,0)			    u (0,1)
	   *	    C		    b		    c		    D
	   *	   / \		   / \		   / \		   / \
	   *	  /   \		  /   \		  /   \		  /   \
	   *	 /     \	 ----  \	 ----  \	 /     \
	   *    ---------	     ----	     ----	---------
	   */
	  f = u->l->r;
	  b = f->l;
	  c = f->r;
	  *up = f;
	  f->u = uptr;
	  f->l = u->l;
	  f->l->u = f;
	  f->r = u;
	  u->u = f;
	  f->l->r = b;
	  if (b) b->u = f->l;
	  u->l = c;
	  if (c) c->u = u;
	  f->l->bal = (f->bal > 0) ? -1 : 0;
	  f->r->bal = (f->bal < 0) ?  1 : 0;
	  f->bal = 0;
	}
       return(0);
       break;
    case 2:
       /* See case -2, above, for diagrams, but reverse left/right */
       if (u->r->bal >= 0)
	{ u->bal = 1 - u->r->bal;
	  u->r->bal --;
	  *up = u->r;
	  u->r->u = uptr;
	  f = u->r->l;
	  u->r->l = u;
	  u->u = u->r;
	  u->r = f;
	  if (f) f->u = u;
	  if (u->bal) return(1);
	}
       else if (u->r->bal < 0)
	{ f = u->r->l;
	  b = f->r;
	  c = f->l;
	  *up = f;
	  f->u = uptr;
	  f->r = u->r;
	  f->r->u = f;
	  f->l = u;
	  u->u = f;
	  f->r->l = b;
	  if (b) b->u = f->r;
	  u->r = c;
	  if (c) c->u = u;
	  f->r->bal = (f->bal < 0) ?  1 : 0;
	  f->l->bal = (f->bal > 0) ? -1 : 0;
	  f->bal = 0;
	}
       return(0);
       break;
  }
 panic("pfw: impossible balance");
}

/*
 * Insert f into the binary tree whose root pointer is *up.  uptr is
 *  the correct thing to put in the u field of an element replacing
 *  *up.
 *
 * Return value is:
 *
 *	INSERT_DROP
 *		The element is a duplicate and should be dropped; the
 *		binary tree has not been modified.
 *
 *	INSERT_SAME
 *		The element has been absorbed without changing the
 *		depth of the tree.
 *
 *	INSERT_DEEPEN
 *		The element has been inserted; the resulting tree is
 *		one level deeper than before.
 */
#define INSERT_DROP   1
#define INSERT_SAME   2
#define INSERT_DEEPEN 3
static int insert(FTN *f, FTN **up, FTN *uptr)
{
 FTN *u;

 u = *up;
 if (! u)
  { *up = f;
    f->u = uptr;
    return(INSERT_DEEPEN);
  }
 if (f->addr < u->addr)
  { switch (insert(f,&u->l,u))
     { default:
	  panic("impossible < sub-insert");
	  break;
       case INSERT_DROP:
	  return(INSERT_DROP);
	  break;
       case INSERT_SAME:
	  return(INSERT_SAME);
	  break;
       case INSERT_DEEPEN:
	  u->bal --;
	  return(rebalance(up,uptr)?INSERT_DEEPEN:INSERT_SAME);
	  break;
     }
  }
 else if (f->addr > u->addr)
  { switch (insert(f,&u->r,u))
     { default:
	  panic("impossible > sub-insert");
	  break;
       case INSERT_DROP:
	  return(INSERT_DROP);
	  break;
       case INSERT_SAME:
	  return(INSERT_SAME);
	  break;
       case INSERT_DEEPEN:
	  u->bal ++;
	  return(rebalance(up,uptr)?INSERT_DEEPEN:INSERT_SAME);
	  break;
     }
  }
 else
  { /* This can happen if, eg, the add device is used to
       add an address that's already present. */
    return(INSERT_DROP);
  }
}

/*
 * Search for the correct place and insert f, rebalancing as necessary.
 *  Return value is as for insert(), above.
 */
static int search_insert(FTN *f, FTN **rootp)
{
 f->l = 0;
 f->r = 0;
 f->bal = 0;
 return(insert(f,rootp,0));
}

/*
 * Delete f from the binary tree, rebalancing as necessary.  Blindly
 *  assumes that (a) f is already part of the tree and (b) the tree is
 *  self-consistent.
 */
static void search_delete(FTN *f, FTN **rootp)
{
 FTN *u;
 FTN *l;
 FTN *r;
 FTN **up;
 int dr;
 FTN *s;

 u = f->u;
 l = f->l;
 r = f->r;
 up = u ? (u->l == f) ? &u->l : &u->r : rootp;
 dr = u ? (u->l == f) ? 1 : -1 : 0;
 if (! f->r)
  { if (! f->l)
     { *up = 0;
     }
    else
     { f->l->u = u;
       *up = f->l;
     }
  }
 else if (! f->l)
  { f->r->u = u;
    *up = f->r;
  }
 else if (! f->r->l)
  { f->r->l = f->l;
    f->l->u = f->r;
    f->r->u = u;
    *up = f->r;
    u = f->r;
    u->bal = f->bal;
    dr = -1;
  }
 else
  { s = f->r;
    while (s->l) s = s->l;
    s->u->l = s->r;
    if (s->r) s->r->u = s->u;
    s->l = f->l;
    f->l->u = s;
    s->r = f->r;
    f->r->u = s;
    s->bal = f->bal;
    f = s->u;
    s->u = u;
    *up = s;
    u = f;
    dr = 1;
  }
 if (u)
  { u->bal += dr;
    while <"delrebal"> (1)
     { switch (u->bal)
	{ case 0:
	     if (u->u)
	      { u->u->bal += (u == u->u->l) ? 1 : -1;
		u = u->u;
		continue;
	      }
	     break <"delrebal">;
	     break;
	  case -1:
	  case 1:
	     break <"delrebal">;
	     break;
	  case -2:
	  case 2:
	      { FTN *v;
		v = u->u;
		if (v)
		 { int ob;
		   ob = v->bal;
		   v->bal += (u == v->l) ? 1 : -1;
		   if (rebalance((u==v->l)?&v->l:&v->r,v))
		    { v->bal = ob;
		      break <"delrebal">;
		    }
		   u = v;
		   continue;
		 }
		rebalance(rootp,0);
		break <"delrebal">;
	      }
	     break;
	  default:
	     panic("pfw: impossible delete rebalance");
	     break;
	}
     }
  }
}

/*
 * Look up the FTN for an address; return nil if not found.
 */
static FTN *search_find(u_int32_t a, FTN *root)
{
 FTN *f;

 f = root;
 while (1)
  { if (! f) return(0);
    if (f->addr == a) return(f);
    f = (a < f->addr) ? f->l : f->r;
  }
}

static void addr_to_buf(u_int32_t addr, unsigned char *buf)
{
 buf[0] = (addr >> 24) & 0xff;
 buf[1] = (addr >> 16) & 0xff;
 buf[2] = (addr >>  8) & 0xff;
 buf[3] = (addr      ) & 0xff;
}

static __inline__ u_int32_t nw__cvt_uint32t(u_int32_t v) { return(v); }
static __inline__ struct mbuf *nw__cvt_mbuf_p(struct mbuf *v) { return(v); }
static __inline__ int nw__cvt_int(int v) { return(v); }

static void notify_watchers(SOFTC *sc, char flgc, ...)
#define NW__END  1
#define NW_END NW__END
#define NW__ADDR 2
#define NW_ADDR(a) NW__ADDR, nw__cvt_uint32t((a))
#define NW__MBUF 3
#define NW_MBUF(m) NW__MBUF, nw__cvt_mbuf_p((m))
#define NW__INT  4
#define NW_INT(i) NW__INT, nw__cvt_int((i))
{
 WATCH **wp;
 WATCH *w;
 struct socket *so;
 struct mbuf *m0;

 wp = &sc->watchers;
 while <"nextwatcher"> ((w = *wp))
  { do <"closethis">
     { so = w->f->f_data;
       solock(so);
       if ((so->so_state & (SS_ISCONNECTED|SS_CANTSENDMORE)) != SS_ISCONNECTED) break <"closethis">;
       do <"keepthis">
	{ if (so->so_snd.sb_mbcnt > 64)
	   { if (w->errored) break <"keepthis">;
	     MGETHDR(m0,M_NOWAIT,MT_DATA);
	     if (m0 == 0) break <"keepthis">;
	     m0->m_pkthdr.len = 1;
	     mtod(m0,char *)[0] = 'e';
	   }
	  else
	   { va_list ap;
	     int key;
	     struct mbuf *m;
	     struct mbuf **tail;
	     MGETHDR(m0,M_DONTWAIT,MT_DATA);
	     if (m0 == 0) break <"keepthis">;
	     m0->m_len = 1;
	     m0->m_pkthdr.len = 1;
	     *mtod(m0,unsigned char *) = flgc;
	     tail = &m0->m_next;
	     va_start(ap,flgc);
	     while <"argloop"> (1)
	      { key = va_arg(ap,int);
		switch (key)
		 { case NW__END:
		      *tail = 0;
		      break <"argloop">;
		   case NW__ADDR:
		       { u_int32_t a;
			 a = va_arg(ap,u_int32_t);
			 MGET(m,M_DONTWAIT,MT_DATA);
			 m->m_len = 4;
			 addr_to_buf(a,mtod(m,unsigned char *));
			 *tail = m;
			 tail = &m->m_next;
		       }
		      break;
		   case NW__MBUF:
		      m = va_arg(ap,struct mbuf *);
		      if (m)
		       { m = m_dup(m,0,M_COPYALL,M_DONTWAIT);
			 if (m == 0)
			  { m_freem(m0);
			    break <"keepthis">;
			  }
			 *tail = m;
			 while (m->m_next)
			  { m0->m_pkthdr.len += m->m_len;
			    m = m->m_next;
			  }
			 m0->m_pkthdr.len += m->m_len;
			 tail = &m->m_next;
		       }
		      break;
		   case NW__INT:
		       { int v;
			 v = va_arg(ap,int);
			 MGET(m,M_DONTWAIT,MT_DATA);
			 if (m == 0)
			  { m_freem(m0);
			    break <"keepthis">;
			  }
			 m->m_len = sizeof(int);
			 *mtod(m,int *) = v;
			 *tail = m;
			 tail = &m->m_next;
			 m0->m_pkthdr.len += sizeof(int);
		       }
		      break;
		   default:
		      panic("notify-watchers: key %d",key);
		      break;
		 }
	      }
	     va_end(ap);
	     w->errored = 0;
	   }
	  m0->m_pkthdr.rcvif = 0;
	  if ((*so->so_proto->pr_usrreq)(so,PRU_SEND,m0,0,0,USRREQ_ARG(watchproc))) break <"closethis">;
	} while (0);
       sounlock(so);
       wp = &w->link;
       continue <"nextwatcher">;
     } while (0);
    FILE_USE(w->f);
    soshutdown(so,SHUT_RDWR);
    sounlock(so);
    *wp = w->link;
    mutex_enter(&deadwatch_mtx);
    w->link = deadwatch;
    deadwatch = w;
    cv_broadcast(&deadwatch_cv);
    mutex_exit(&deadwatch_mtx);
    continue <"nextwatcher">;
  }
}

static void ftn_clear(SOFTC *sc)
{
 int s;
 int n;
 FTN **v;

 s = splnet();
 n = sc->nftn;
 v = sc->ftnv;
 sc->nftn = 0;
 sc->aftn = 0;
 sc->ftnv = 0;
 sc->ftnr = 0;
 notify_watchers(sc,'z',NW_END);
 splx(s);
 for (n--;n>=0;n--) free(v[n],M_DEVBUF);
 if (v) free(v,M_DEVBUF);
}

static void heap_down(int n, FTN **v, FTN *f)
{
 int x;
 int l;
 int r;
 int s;

 x = f->hx;
 while (1)
  { l = x + x + 1;
    r = l + 1;
    if ((l < n) && (v[l]->exp < f->exp))
     { if ((r < n) && (v[r]->exp < f->exp))
	{ s = (v[l]->exp < v[r]->exp) ? l : r;
	}
       else
	{ s = l;
	}
     }
    else
     { if ((r < n) && (v[r]->exp < f->exp))
	{ s = r;
	}
       else
	{ break;
	}
     }
    v[s]->hx = x;
    v[x] = v[s];
    x = s;
  }
 v[x] = f;
 f->hx = x;
}

static int verify_walk(FTN **pp, FTN *f, FTN *u, int n, FTN **v)
{
 int ld;
 int rd;

 if (f == 0) return(0);
 if ((f->hx < 0) || (f->hx >= n)) panic("pfw verify_walk: hx %d",f->hx);
 if (f != v[f->hx]) panic("pfw verify_walk: %p != v[%d]=%p",(void *)f,f->hx,(void *)v[f->hx]);
 if (f->u != u) panic("pfw verify_walk: %p->u=%p != %p",(void *)f,(void *)f->u,(void *)u);
 ld = verify_walk(pp,f->l,f,n,v);
 if (*pp && ((*pp)->addr >= f->addr)) panic("pfw verify_walk: %p->addr (%08lx) >= %p->addr (%08lx)",(void *)*pp,(unsigned long int)(*pp)->addr,(void *)f,(unsigned long int)f->addr);
 *pp = f;
 rd = verify_walk(pp,f->r,f,n,v);
 if ( ((ld == rd+1) && (f->bal == -1)) ||
      ((ld == rd) && (f->bal == 0)) ||
      ((ld == rd-1) && (f->bal == 1)) ) return(((ld>rd)?ld:rd)+1);
 panic("pfw verify_walk: %p ld=%d rd=%d bal=%d",(void *)f,ld,rd,f->bal);
}

static void verify(SOFTC *sc)
{
 int s;
 int i;
 FTN *f;
 FTN *p;

 if (noverify) return;
 s = splnet();
 for (i=0;i<sc->nftn;i++)
  { f = sc->ftnv[i];
    if (! f) panic("pfw verify: !ftnv[%d<%d]",i,sc->nftn);
    if (f->hx != i) panic("pfw verify: ftnv[%d] hx %d",i,f->hx);
    if (i && (f->exp < sc->ftnv[(f->hx-1)>>1]->exp))
     { panic("pfw verify: ftnv[%d]->exp (%lu) < ftnv[%d]->exp (%lu)",
		f->hx,(unsigned long int)f->exp,
		(f->hx-1)>>1,(unsigned long int)sc->ftnv[(f->hx-1)>>1]->exp);
     }
  }
 p = 0;
 verify_walk(&p,sc->ftnr,0,sc->nftn,sc->ftnv);
 splx(s);
}

static void rebuild_heap(SOFTC *sc)
{
 int i;

 if (sc->nftn < 1) return;
 for (i=(sc->nftn-1)/2;i>=0;i--) heap_down(sc->nftn,sc->ftnv,sc->ftnv[i]);
 verify(sc);
}

static void ftn_freshen(SOFTC *sc, FTN *f)
{
 f->exp = MONO_TIME_SEC + EXPIRE;
 f->upd = MONO_TIME_SEC + HOLDDOWN;
 heap_down(sc->nftn,sc->ftnv,f);
 serial ++;
 verify(sc);
}

static void del_ftn(SOFTC *sc, FTN *f)
{
 int x;

 search_delete(f,&sc->ftnr);
 serial ++;
 x = f->hx;
 free(f,M_DEVBUF);
 sc->nftn --;
 if ((sc->nftn > 0) && (x < sc->nftn))
  { f = sc->ftnv[sc->nftn];
    f->hx = x;
    heap_down(sc->nftn,sc->ftnv,f);
  }
 verify(sc);
}

static void ticker(void *arg __attribute__((__unused__)))
{
 int i;
 SOFTC *sc;
 int s;
 FTN *f;
 int wanttick;

 wanttick = 0;
 s = splnet();
 for (i=NPFW-1;i>=0;i--)
  { sc = &pfw_softc[i];
    while (sc->nftn > 0)
     { f = sc->ftnv[0];
       if (f->exp > MONO_TIME_SEC) break;
       notify_watchers(sc,'d',NW_ADDR(f->addr),NW_END);
       del_ftn(sc,f);
     }
    if (sc->nftn > 0) wanttick = 1;
  }
 if (wanttick) RESET_TICKER(); else running = 0;
 splx(s);
}

static int total_mbuf_len(struct mbuf *m)
{
 int len;

 for (len=0;m;m=m->m_next) len += m->m_len;
 return(len);
}

static FTN *add_block(SOFTC *sc, u_int32_t addr, struct mbuf *pkt)
{
 FTN *f;

 f = malloc(sizeof(FTN),M_DEVBUF,M_NOWAIT);
 if (f == 0)
  { notify_watchers(sc,'m',NW_ADDR(addr),NW_END);
    return(0);
  }
 if (sc->nftn >= sc->aftn)
  { FTN **nv;
    int a;
    a = sc->aftn + 64;
    nv = malloc(a*sizeof(FTN *),M_DEVBUF,M_NOWAIT);
    if (nv == 0)
     { free(f,M_DEVBUF);
       notify_watchers(sc,'m',NW_ADDR(addr),NW_END);
       return(0);
     }
    bcopy(sc->ftnv,nv,sc->nftn*sizeof(FTN *));
    if (sc->ftnv) free(sc->ftnv,M_DEVBUF);
    sc->ftnv = nv;
    sc->aftn = a;
  }
 f->addr = addr;
 if (search_insert(f,&sc->ftnr) == INSERT_DROP)
  { free(f,M_DEVBUF);
    f = search_find(addr,sc->ftnr);
    if (! f) panic("can't find duplicate");
    ftn_freshen(sc,f);
    notify_watchers(sc,'f',NW_ADDR(addr),NW_INT(total_mbuf_len(pkt)),NW_MBUF(pkt),NW_END);
    return(f);
  }
 /* we know the new FTN belongs at the bottom of the heap */
 f->hx = sc->nftn++;
 ftn_freshen(sc,f); /* also stores into ftnv[], and calls verify() */
 notify_watchers(sc,'a',NW_ADDR(f->addr),NW_INT(total_mbuf_len(pkt)),NW_MBUF(pkt),NW_END);
 if (! running)
  { RESET_TICKER();
    running = 1;
  }
 return(f);
}

static void del_block(SOFTC *sc, u_int32_t addr)
{
 FTN *f;

 f = search_find(addr,sc->ftnr);
 if (f == 0) return;
 notify_watchers(sc,'d',NW_ADDR(f->addr),NW_END);
 del_ftn(sc,f);
}

static int pfw_hook(PFW_HOOK_ARGS)
{
 struct tcphdr *th;
 struct udphdr *uh;
 FTN *ftn;
 int s;
 int i;
 SOFTC *sc;
 int rv;
 PFW_HOOK_LOCALS

 KERNEL_LOCK(1,0);
 s = splnet();
 rv = 0;
 for (i=maxlive;i>=0;i--)
  { sc = &pfw_softc[i];
    if (ifp != sc->fifp) continue;
    ftn = search_find(ntohl(ip->ip_src.s_addr),sc->ftnr);
    if (ftn)
     { if (MONO_TIME_SEC > ftn->upd)
	{ ftn_freshen(sc,ftn);
	  notify_watchers(sc,'f',NW_ADDR(ftn->addr),NW_INT(total_mbuf_len(*m)),NW_MBUF(*m),NW_END);
	}
       rv = 1;
       continue;
     }
    if (ip->ip_v == 4)
     { if ((ip->ip_off & IP_OFFMASK) == 0)
	{ switch (ip->ip_p)
	   { case IPPROTO_TCP:
		*m = m_pullup(*m,hlen+sizeof(struct tcphdr));
		if (! *m)
		 { rv = 1;
		   continue;
		 }
		th = (struct tcphdr *) (mtod(*m,char *) + hlen);
		switch (ntohs(th->th_dport))
		 { case 137: /* netbios-ns */
		   case 138: /* netbios-dgm */
		   case 139: /* netbios-ssn */
		   case 445: /* microsoft-ds */
		   case 623: /* Intel ATM backdoor */
		   case 664: /* Intel ATM backdoor */
		   case 16992: /* Intel ATM backdoor */
		   case 16993: /* Intel ATM backdoor */
		   case 16994: /* Intel ATM backdoor */
		   case 16995: /* Intel ATM backdoor */
		      add_block(sc,ntohl(ip->ip_src.s_addr),*m);
		      rv = 1;
		      continue;
		      break;
		 }
		break;
	     case IPPROTO_UDP:
		*m = m_pullup(*m,hlen+sizeof(struct udphdr));
		if (! *m)
		 { rv = 1;
		   continue;
		 }
		uh = (struct udphdr *) (mtod(*m,char *) + hlen);
		switch (ntohs(uh->uh_dport))
		 { case 137:
		   case 138:
		   case 139:
		      add_block(sc,ntohl(ip->ip_src.s_addr),*m);
		      rv = 1;
		      continue;
		      break;
		 }
		break;
	   }
	}
       switch (ntohl(ip->ip_dst.s_addr))
	{ case 0x627c3d58:
	  case 0x627c3d5f:
	     add_block(sc,ntohl(ip->ip_src.s_addr),*m);
	     rv = 1;
	     continue;
	     break;
#if 0
	  case 0xd82e0509:
	     if (ip->ip_p == IPPROTO_TCP)
	      { *m = m_pullup(*m,hlen+sizeof(struct tcphdr));
		if (! *m)
		 { rv = 1;
		   continue;
		 }
		th = (struct tcphdr *) (mtod(*m,char *) + hlen);
		if (ntohs(th->th_dport) == 25)
		 { add_block(sc,ntohl(ip->ip_src.s_addr),*m);
		   rv = 1;
		   continue;
		 }
	      }
	     break;
#endif
	}
     }
  }
 if (rv && *m) m_freem(*m);
 splx(s);
 KERNEL_UNLOCK_ONE(0);
 return(rv);
}

/*
 * Actual attachment is delayed until first open so we know inetsw and
 *  ip_protox are set up.  This is ugly, since it means pfwopen() can
 *  race against itself testing and setting attached.  We don't worry
 *  about this because we are not MPSAFE and thus the giantlock
 *  prevents that from happening.  This really should be fixed right.
 */
void pfwattach(int arg __attribute__((__unused__)))
{
 attached = 0;
 ATTACH_TIME_EARLY();
}

DEVSW_SCLASS int pfwopen(dev_t dev, int flag, int mode, PROCTYPE p)
{
 unsigned int u;
 SOFTC *sc;
 int i;

 u = PFWUNIT(minor(dev));
 if (u >= NPFW) return(ENXIO);
 if (! attached)
  { ATTACH_TIME_LATE();
    for (i=NPFW-1;i>=0;i--)
     { sc = &pfw_softc[i];
       sc->unit = i;
       sc->fifp = 0;
       sc->nftn = 0;
       sc->aftn = 0;
       sc->ftnv = 0;
       sc->ftnr = 0;
       sc->watchers = 0;
     }
    running = 0;
    serial = 0;
    watchproc = 0;
    deadwatch = 0;
    mutex_init(&deadwatch_mtx,MUTEX_DEFAULT,IPL_VM);
    cv_init(&deadwatch_cv,"pfwdead");
    SETUP_PFW_HOOK();
    TICKER_SETUP();
    attached = 1;
    maxlive = -1;
  }
 return(0);
}

DEVSW_SCLASS int pfwclose(dev_t dev, int flag, int mode, PROCTYPE p)
{
 return(0);
}

DEVSW_SCLASS int pfwread(dev_t dev, struct uio *uio, int ioflag)
{
 unsigned int unit;
 unsigned int kind;
 SOFTC *sc;
 off_t o;
 int owi;
 int e;
 int s;
 char *d;
 char t;
 int l;
 unsigned int val_ui;
 unsigned long long int val_ulli;

 unit = minor(dev);
 kind = PFWKIND(unit);
 unit = PFWUNIT(unit);
 if (unit >= NPFW) return(ENXIO);
 sc = &pfw_softc[unit];
 if (uio->uio_offset < 0) return(EINVAL);
 switch (kind)
  { case PFWKIND_IFACE:
       if (sc->fifp)
	{ d = &sc->fifp->if_xname[0];
	  l = strlen(d);
	}
       else
	{ t = '-';
	  d = &t;
	  l = 1;
	}
       break;
    case PFWKIND_SERIAL:
       s = splnet();
       val_ulli = serial;
       splx(s);
       d = (void *) &val_ulli;
       l = sizeof(val_ulli);
       break;
    case PFWKIND_COUNT:
       s = splnet();
       val_ui = sc->nftn;
       splx(s);
       d = (void *) &val_ui;
       l = sizeof(val_ui);
       break;
    case PFWKIND_LIST:
       while (uio->uio_resid)
	{ u_int32_t val_32;
	  unsigned char buf[16];
	  o = uio->uio_offset / 4;
	  owi = uio->uio_offset % 4;
	  if (o < 0) return(EINVAL);
	  s = splnet();
	  if (o >= sc->nftn)
	   { splx(s);
	     break;
	   }
	  val_32 = sc->ftnv[o]->addr;
	  splx(s);
	  buf[0] = (val_32 >> 24) & 0xff;
	  buf[1] = (val_32 >> 16) & 0xff;
	  buf[2] = (val_32 >>  8) & 0xff;
	  buf[3] = (val_32      ) & 0xff;
	  e = uiomove(owi+(char *)&buf[0],4-owi,uio);
	  if (e) return(e);
	}
       return(0);
       break;
    case PFWKIND_SAVE:
       if (uio->uio_resid < 1) return(0);
       if ((uio->uio_resid == 4) && (uio->uio_offset == 0))
	{ unsigned int val;
	  unsigned char buf[4];
	  s = splnet();
	  val = sc->nftn;
	  splx(s);
	  buf[0] = (val >> 24) & 0xff;
	  buf[1] = (val >> 16) & 0xff;
	  buf[2] = (val >>  8) & 0xff;
	  buf[3] = (val      ) & 0xff;
	  return(uiomove((char *)&buf[0],4,uio));
	}
       if (uio->uio_offset == 4)
	{ int n;
	  unsigned char buf[16];
	  int i;
	  u_int32_t v_a;
	  u_int32_t v_t;
	  n = uio->uio_resid / 16;
	  e = 0;
	  s = splnet();
	  if (n < sc->nftn)
	   { splx(s);
	     return(EMSGSIZE);
	   }
	  for (i=0;i<n;i++)
	   { v_a = sc->ftnv[i]->addr;
	     v_t = MONO_TIME_SEC - (sc->ftnv[i]->exp - EXPIRE);
	     buf[0] = (v_a >> 24) & 0xff;
	     buf[1] = (v_a >> 16) & 0xff;
	     buf[2] = (v_a >>  8) & 0xff;
	     buf[3] = (v_a      ) & 0xff;
	     buf[4] = (v_t >> 24) & 0xff;
	     buf[5] = (v_t >> 16) & 0xff;
	     buf[6] = (v_t >>  8) & 0xff;
	     buf[7] = (v_t      ) & 0xff;
	     buf[8] = 0;
	     buf[9] = 0;
	     buf[10] = 0;
	     buf[11] = 0;
	     buf[12] = 0;
	     buf[13] = 0;
	     buf[14] = 0;
	     buf[15] = 0;
	     e = uiomove((char *)&buf[0],16,uio);
	     if (e) break;
	   }
	  splx(s);
	  return(e);
	}
       return(EINVAL);
    default:
       return(0);
       break;
  }
 if (uio->uio_offset >= l) return(0);
 return(uiomove(d+uio->uio_offset,l-uio->uio_offset,uio));
 return(0);
}

static void watchproc_main(void *arg __attribute__((__unused__)))
{
 WATCH *list;
 WATCH *w;

 mutex_enter(&deadwatch_mtx);
 while (1)
  { list = deadwatch;
    if (list == 0)
     { cv_wait(&deadwatch_cv,&deadwatch_mtx);
       continue;
     }
    deadwatch = 0;
    mutex_exit(&deadwatch_mtx);
    while (list)
     { w = list;
       list = w->link;
       CLOSEF(w->f);
       free(w,M_DEVBUF);
     }
    mutex_enter(&deadwatch_mtx);
  }
}

/* curproc/curlwp is an implicit additional argument to addwatch() */
static int addwatch(SOFTC *sc, int fd)
{
 struct file *fp;
 int e;
 struct socket *so;
 int s;
 WATCH *w;

 s = splnet();
 if (watchproc == 0)
  { e = CREATE_WATCHER(&watchproc_main,0,&watchproc);
    if (e)
     { splx(s);
       return(e);
     }
  }
 splx(s);
 GETSOCK(fd,e,fp);
 if (e) return(e);
 if (fp->f_type != DTYPE_SOCKET)
  { FILE_UNUSE(fd);
    return(ENOTSOCK);
  }
 so = fp->f_data;
 if (so->so_proto->pr_type != SOCK_STREAM)
  { FILE_UNUSE(fd);
    return(ESOCKTNOSUPPORT);
  }
 if (! (so->so_state & SS_ISCONNECTED))
  { FILE_UNUSE(fd);
    return(ENOTCONN);
  }
 fp->f_count ++;
 FDRELEASE(fd);
 w = malloc(sizeof(WATCH),M_DEVBUF,M_WAITOK);
 w->f = fp;
 w->errored = 0;
 s = splnet();
 w->link = sc->watchers;
 sc->watchers = w;
 splx(s);
 return(0);
}

static void reset_maxlive(void)
{
 int i;

 for (i=NPFW-1;i>=0;i--)
  { if (pfw_softc[i].fifp)
     { maxlive = i;
       return;
     }
  }
 maxlive = -1;
}

DEVSW_SCLASS int pfwwrite(dev_t dev, struct uio *uio, int ioflag)
{
 unsigned int unit;
 unsigned int kind;
 SOFTC *sc;
 int e;
 int s;

 unit = minor(dev);
 kind = PFWKIND(unit);
 unit = PFWUNIT(unit);
 if (unit >= NPFW) return(ENXIO);
 sc = &pfw_softc[unit];
 switch (kind)
  { case PFWKIND_IFACE:
	{ char xnbuf[64];
	  int l;
	  struct ifnet *i;
	  /* if (uio->uio_resid < 0) return(EINVAL); unsigned never <0! */
	  if (uio->uio_resid > sizeof(xnbuf)-1) return(EMSGSIZE);
	  l = uio->uio_resid;
	  e = uiomove(&xnbuf[0],l,uio);
	  if (e) return(e);
	  xnbuf[l] = '\0';
	  if ((l == 1) && (xnbuf[0] == '-'))
	   { sc->fifp = 0;
	   }
	  else
	   { i = ifunit(&xnbuf[0]);
	     if (i == 0) return(ENXIO);
	     sc->fifp = i;
	   }
	  reset_maxlive();
	}
       break;
    case PFWKIND_CLEAR:
       uio->uio_resid = 0;
       ftn_clear(sc);
       break;
    case PFWKIND_WATCH:
	{ int fd;
	  switch (uio->uio_resid)
	   { case 1:
		 { unsigned char c;
		   e = uiomove(&c,1,uio);
		   fd = c;
		 }
		break;
	     case sizeof(int):
		e = uiomove(&fd,sizeof(int),uio);
		break;
	     default:
		return(EINVAL);
		break;
	   }
	  if (e) return(e);
	  return(addwatch(sc,fd));
	}
       break;
    case PFWKIND_ADD:
	{ unsigned char a[4];
	  u_int32_t v;
	  FTN *f;
	  if (uio->uio_resid != 4) return(EINVAL);
	  e = uiomove(&a[0],4,uio);
	  if (e) return(e);
	  s = splnet();
	  v = (a[0]*0x1000000) + (a[1]*0x10000) + (a[2]*0x100) + a[3];
	  f = search_find(v,sc->ftnr);
	  if (f)
	   { ftn_freshen(sc,f);
	     notify_watchers(sc,'f',NW_ADDR(v),NW_INT(0),NW_END);
	   }
	  else
	   { add_block(sc,v,0);
	   }
	  splx(s);
	}
       break;
    case PFWKIND_DEL:
	{ unsigned char a[4];
	  if (uio->uio_resid != 4) return(EINVAL);
	  e = uiomove(&a[0],4,uio);
	  if (e) return(e);
	  s = splnet();
	  del_block(sc,(a[0]*0x1000000)+(a[1]*0x10000)+(a[2]*0x100)+a[3]);
	  splx(s);
	}
       break;
    case PFWKIND_SAVE:
	{ int n;
	  int i;
	  unsigned char buf[16];
	  u_int32_t val_a;
	  u_int32_t val_t;
	  time_t t;
	  FTN *f;
	  noverify = 1;
	  if (uio->uio_offset) return(EINVAL);
	  if (uio->uio_resid % 16) return(EINVAL);
	  n = uio->uio_resid / 16;
	  t = MONO_TIME_SEC;
	  ftn_clear(sc);
	  e = 0;
	  s = splnet();
	  for (i=n;i>0;i--)
	   { e = uiomove(&buf[0],16,uio);
	     if (e) break;
	     val_a = (buf[0] * 0x01000000) | (buf[1] * 0x010000) | (buf[2] * 0x0100) | buf[3];
	     val_t = (buf[4] * 0x01000000) | (buf[5] * 0x010000) | (buf[6] * 0x0100) | buf[7];
	     if (EXPIRE > val_t)
	      { f = add_block(sc,val_a,0);
		if (! f)
		 { e = ENOMEM;
		   break;
		 }
		f->exp = t + EXPIRE - val_t;
		f->upd = t + HOLDDOWN - val_t;
	      }
	   }
	  /*
	   * add_block() puts the new FTNs into the heap, but we have
	   *  to rebuild it anyway, because the expiration times ave
	   *  been changed from the ones they were put into the heap
	   *  under.  The `heap' probably is not a heap at the
	   *  moment....
	   */
	  noverify = 0;
	  rebuild_heap(sc);
	  splx(s);
	  notify_watchers(sc,'r',NW_END);
	  return(e);
	}
    default:
       return(EPERM);
       break;
  }
 return(0);
}

DEVSW_SCLASS int pfwioctl(dev_t dev, u_long cmd, CADDR_T data, int flag, PROCTYPE p)
{
 return(ENOTTY);
}

DEVSW_SCLASS int pfwpoll(dev_t dev, int events, PROCTYPE p)
{
 unsigned int unit;

 unit = minor(dev);
 return(events&(POLLIN|POLLRDNORM));
}

DEFINE_CDEVSW

#endif
