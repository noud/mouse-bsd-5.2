#include "diskwatch.h"
#if NDISKWATCH > 0

#include <sys/vnode.h>

#include <sys/kmem.h>
#include <sys/poll.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/fcntl.h>
#include <sys/mutex.h>
#include <sys/select.h>
#include <sys/condvar.h>
#include <sys/filedesc.h>
#include <lib/libkern/libkern.h>
#include <miscfs/specfs/specdev.h>

#include "diskwatch-kern.h"

/*
 * Locking is annoying.
 *
 * Normally, I'd use one lock per softc.  But the watchptrs[] array
 *  also needs locking, and the way SOFTCs are chained to support
 *  multiple watchers watching the same unit means that we may need to
 *  lock multiple SOFTCs when clearing a watch.
 *
 * Rather than use complex and subtle locking (and probably get it
 *  wrong), we just punt and use a single lock to protect pretty much
 *  everything we do.
 */

#define RING_SIZE 65536
#define RING_MAXDATA 8192

#define RING_INC(v) (((v)>=RING_SIZE-1)?0:((v)+1))
#define RING_DEC(v) (((v)<1)?RING_SIZE-1:((v)-1))

typedef struct softc SOFTC;
typedef struct pkt PKT;

struct softc {
  int unit;
  unsigned int flags;
#define SF_COPEN 0x00000001
#define SF_DOPEN 0x00000002
#define SF_ISREADY(flags) (((flags)&(SF_COPEN|SF_DOPEN))==(SF_COPEN|SF_DOPEN))
#define SF_SET   0x00000004
#define SF_RATE  0x00000018
#define SF_RATE_OFF 0x00000000
#define SF_RATE_LO  0x00000008
#define SF_RATE_HI  0x00000010
#define SF_NBIO  0x00000020
#define SF_HEAD  0x00000040
  dev_t cdev;
  SOFTC *flink;
  SOFTC *blink;
  PKT *ring;
  int ringh;	/* head */
  int ringt;	/* tail */
  int ringn;	/* packet count */
  int ringd;	/* PKT_DATA packet count */
  unsigned char *blocks;
  struct selinfo rsel;
  kcondvar_t cv;
  int freehand;
  int watchunit;
  unsigned int freeblk[(RING_MAXDATA+31)/32];
  } ;

struct pkt {
  unsigned char type;
#define PKT_FREE 1
#define PKT_DATA 2
#define PKT_NUM  3
#define PKT_STOP 4
  daddr_t blk;
  void *data;
  } ;

#define DW_MINOR_UNIT(min) ((min)>>2)
#define DW_MINOR_KIND(min) ((min)&3)
#define DW_KIND_CTL  0
#define DW_KIND_DATA 1
#define DW_KIND_DBG  2

static SOFTC *softcv;
static int ndiskwatch;
static int diskwatch_verbose;
static SOFTC **watchptrs;
static kmutex_t the_lock;

static void *get_data_block(SOFTC *sc)
{
 int i;
 int n;
 unsigned int t;

 KASSERT(mutex_owned(&the_lock));
 i = sc->freehand;
 n = 0;
 while (sc->freeblk[i] == 0)
  { i ++;
    if (i >= ((RING_MAXDATA+31)>>5)) i = 0;
    if (n++ > ((RING_MAXDATA+31)>>5)+3) panic("diskwatch: no data block!");
  }
 t = sc->freeblk[i];
 n = 0;
 if (! (t & 0xffff))
  { n += 16;
    t >>= 16;
  }
 if (! (t & 0xff))
  { n += 8;
    t >>= 8;
  }
 if (! (t & 0xf))
  { n += 4;
    t >>= 4;
  }
 if (! (t & 0x3))
  { n += 2;
    t >>= 2;
  }
 if (! (t & 0x1))
  { n += 1;
    t >>= 1;
  }
 if (! (t&1)) panic("diskwatch: lost freeblk bit");
 sc->freeblk[i] &= ~(1U << n);
 sc->freehand = i;
 return(sc->blocks+((n+(i<<5))*512));
}

static void put_data_block(SOFTC *sc, void *blk)
{
 unsigned char *bp;
 int i;

 KASSERT(mutex_owned(&the_lock));
 bp = blk;
 if ( (bp < sc->blocks) ||
      (bp >= sc->blocks+(RING_MAXDATA*512)) ||
      ((i=(bp-sc->blocks)) & 511) ) panic("diskwatch: putting bogus data block");
 i >>= 9;
 sc->freeblk[i>>5] |= 1U << (i & 31);
}

static void drop_tail_packet(SOFTC *sc)
{
 KASSERT(mutex_owned(&the_lock));
 if (sc->ringn < 1) panic("drop_tail_packet: ring empty");
 switch (sc->ring[sc->ringt].type)
  { case PKT_FREE:
       panic("drop_tail_packet: dropping free packet");
       break;
    case PKT_DATA:
       if (sc->ringd < 1) panic("drop_tail_packet: dropping nonexistent data");
       put_data_block(sc,sc->ring[sc->ringt].data);
       sc->ringd --;
       break;
    case PKT_NUM:
    case PKT_STOP:
       break;
    default:
       panic("drop_tail_packet: dropping bad-type packet");
       break;
  }
 sc->ring[sc->ringt].type = PKT_FREE;
 sc->ringt = RING_INC(sc->ringt);
 sc->ringn --;
}

static int check_ring(SOFTC *sc)
{
 KASSERT(mutex_owned(&the_lock));
 if (SF_ISREADY(sc->flags) && !sc->ring)
  { int i;
    sc->ring = kmem_alloc(RING_SIZE*sizeof(PKT),KM_SLEEP);
    if (! sc->ring) return(1);
    sc->ringh = 0;
    sc->ringt = 0;
    sc->ringn = 0;
    sc->ringd = 0;
    sc->blocks = kmem_alloc(RING_MAXDATA*512,KM_SLEEP);
    if (! sc->blocks)
     { kmem_free(sc->ring,RING_SIZE*sizeof(PKT));
       return(1);
     }
    for (i=(RING_MAXDATA/32)-1;i>=0;i--) sc->freeblk[i] = 0xffffffff;
#if RING_MAXDATA % 32
    sc->freeblk[RING_MAXDATA/32] = (1U << (RING_MAXDATA%32)) - 1;
#endif
    sc->freehand = 0;
  }
 else if (!SF_ISREADY(sc->flags) && sc->ring)
  { while (sc->ringn) drop_tail_packet(sc);
    kmem_free(sc->ring,RING_SIZE*sizeof(PKT));
    kmem_free(sc->blocks,RING_MAXDATA*512);
    sc->ring = 0;
  }
 return(0);
}

static void clr_device(SOFTC *sc)
{
 KASSERT(mutex_owned(&the_lock));
 if (! (sc->flags & SF_SET)) panic("impossible clr_device");
 if (sc->flink) sc->flink->blink = sc->blink;
 if (sc->blink) sc->blink->flink = sc->flink;
 if (sc->flags & SF_HEAD)
  { if (sc->flink)
     { sc->flink->flags |= SF_HEAD;
       sc->flink->watchunit = sc->watchunit;
       watchptrs[sc->watchunit] = sc->flink;
     }
    else
     { cdev_ioctl(sc->cdev,DWIOCCLR,(void *)&sc->watchunit,FWRITE,0);
       watchptrs[sc->watchunit] = 0;
     }
  }
 sc->flags &= ~(SF_SET|SF_HEAD);
}

static int set_device(SOFTC *sc, dev_t dev)
{
 int e;
 int i;
 SOFTC *sc2;
 int w;

 KASSERT(mutex_owned(&the_lock));
 if (sc->flags & SF_SET) clr_device(sc);
 sc->cdev = dev;
 sc->flags = (sc->flags & ~(SF_RATE|SF_HEAD)) | SF_SET;
 for (i=ndiskwatch-1;i>=0;i--)
  { sc2 = &softcv[i];
    if ((sc2 != sc) && (sc2->flags & SF_SET) && (sc2->cdev == dev))
     { sc->flink = sc2->flink;
       sc->blink = sc2;
       sc2->flink = sc;
       if (sc->flink) sc->flink->blink = sc;
       return(0);
     }
  }
 for (w=ndiskwatch-1;(w>=0)&&watchptrs[w];w--) ;
 if (w < 0) panic("diskwatch set_device: no free watchptr\n");
 watchptrs[w] = sc;
 sc->watchunit = w;
 sc->flags |= SF_HEAD;
 sc->flink = 0;
 sc->blink = 0;
 e = cdev_ioctl(dev,DWIOCSET,(void *)&w,FWRITE,0);
 if (e)
  { sc->flags &= ~(SF_SET|SF_HEAD);
    watchptrs[w] = 0;
    return(e);
  }
 return(0);
}

static unsigned int num(const char *s)
{
 unsigned int n;

 n = 0;
 while ((*s >= '0') && (*s <= '9')) n = (n * 10) + (*s++ - '0');
 return(n);
}

static void kick_softc(SOFTC *sc)
{
 KASSERT(mutex_owned(&the_lock));
 selnotify(&sc->rsel,0,0);
 cv_broadcast(&sc->cv);
}

static PKT *queue_pkt(SOFTC *sc)
{
 PKT *p;

 KASSERT(mutex_owned(&the_lock));
 if (sc->ringn >= RING_SIZE) panic("diskwatch: queue_pkt: ring overflow");
 p = &sc->ring[sc->ringh];
 sc->ringh = RING_INC(sc->ringh);
 sc->ringn ++;
 kick_softc(sc);
 return(p);
}

static void queue_stop(SOFTC *sc)
{
 PKT *p;

 KASSERT(mutex_owned(&the_lock));
 p = queue_pkt(sc);
 p->type = PKT_STOP;
}

static void queue_nums(SOFTC *sc, daddr_t blk, int nblks)
{
 PKT *p;

 KASSERT(mutex_owned(&the_lock));
 for (;nblks>0;nblks--)
  { p = queue_pkt(sc);
    p->type = PKT_NUM;
    p->blk = blk;
    blk ++;
  }
}

static void queue_data(SOFTC *sc, daddr_t blk, int nblks, char *data)
{
 PKT *p;
 void *buf;

 KASSERT(mutex_owned(&the_lock));
 for (;nblks>0;nblks--)
  { buf = get_data_block(sc);
    p = queue_pkt(sc);
    if (buf)
     { p->type = PKT_DATA;
       p->data = buf;
       bcopy(data,buf,512);
       sc->ringd ++;
     }
    else
     { p->type = PKT_NUM;
     }
    p->blk = blk;
    blk ++;
    data += 512;
  }
}

static void discard_ring_data(SOFTC *sc)
{
 int i;
 int h;

 KASSERT(mutex_owned(&the_lock));
 h = sc->ringt;
 for (i=sc->ringn;i>0;i--)
  { if (sc->ring[h].type == PKT_DATA)
     { sc->ring[h].type = PKT_NUM;
       put_data_block(sc,sc->ring[h].data);
       sc->ringd --;
     }
    h = RING_INC(h);
  }
 if (sc->ringd) panic("discard_ring_data: ringd=%d",sc->ringd);
}

void diskwatchattach(int n)
{
 int i;

 ndiskwatch = n;
 softcv = kmem_alloc(n*sizeof(SOFTC),KM_SLEEP);
 if (! softcv)
  { printf("diskwatch: no memory for softcs\n");
    return;
  }
 watchptrs = kmem_alloc(n*sizeof(SOFTC *),KM_SLEEP);
 if (! watchptrs)
  { printf("diskwatch: no memory for watch pointers\n");
    kmem_free(softcv,n*sizeof(SOFTC));
    return;
  }
 for (i=n-1;i>=0;i--)
  { softcv[i].flags = 0;
    softcv[i].unit = i;
    softcv[i].ring = 0;
    selinit(&softcv[i].rsel);
    cv_init(&softcv[i].cv,"dwread");
    watchptrs[i] = 0;
  }
 mutex_init(&the_lock,MUTEX_DEFAULT,IPL_NONE);
}

static int diskwatchopen(dev_t dev, int flag, int mode, struct lwp *p)
{
 unsigned int u;
 unsigned int k;
 SOFTC *sc;
 unsigned int prevflags;
 int rv;

 u = minor(dev);
 k = DW_MINOR_KIND(u);
 u = DW_MINOR_UNIT(u);
 if (u >= ndiskwatch) return(ENXIO);
 sc = &softcv[u];
 mutex_enter(&the_lock);
 rv = 0;
 do <"done">
  { prevflags = sc->flags;
    switch (k)
     { case DW_KIND_CTL:
	  if (diskwatch_verbose) printf("diskwatchopen: %d/%d CTL\n",u,k);
	  sc->flags |= SF_COPEN;
	  break;
       case DW_KIND_DATA:
	  if (diskwatch_verbose) printf("diskwatchopen: %d/%d DATA\n",u,k);
	  sc->flags |= SF_DOPEN;
	  break;
       case DW_KIND_DBG:
	  if (diskwatch_verbose) printf("diskwatchopen: %d/%d DBG\n",u,k);
	  break <"done">;
       default:
	  if (diskwatch_verbose) printf("diskwatchopen: %d/%d bad kind\n",u,k);
	  rv = ENXIO;
	  break <"done">;
     }
    if (check_ring(sc))
     { sc->flags = prevflags;
       if (diskwatch_verbose) printf("diskwatchopen: %d/%d check_ring failed\n",u,k);
       rv = ENOBUFS;
       break <"done">;
     }
    if (diskwatch_verbose) printf("diskwatchopen: %d/%d succeeded\n",u,k);
  } while (0);
 mutex_exit(&the_lock);
 return(rv);
}

static int diskwatchclose(dev_t dev, int flag, int mode, struct lwp *p)
{
 unsigned int u;
 unsigned int k;
 SOFTC *sc;
 int rv;

 u = minor(dev);
 k = DW_MINOR_KIND(u);
 u = DW_MINOR_UNIT(u);
 if (u >= ndiskwatch) panic("closing diskwatch #%d",u);
 sc = &softcv[u];
 mutex_enter(&the_lock);
 rv = 0;
 do <"done">
  { switch (k)
     { case DW_KIND_CTL:
	  if (diskwatch_verbose) printf("diskwatchclose: %d/%d CTL\n",u,k);
	  sc->flags &= ~SF_COPEN;
	  break;
       case DW_KIND_DATA:
	  if (diskwatch_verbose) printf("diskwatchclose: %d/%d DATA\n",u,k);
	  sc->flags &= ~SF_DOPEN;
	  break;
       case DW_KIND_DBG:
	  if (diskwatch_verbose) printf("diskwatchclose: %d/%d DBG\n",u,k);
	  break <"done">;
       default:
	  if (diskwatch_verbose) printf("diskwatchclose: %d/%d bad kind\n",u,k);
	  panic("closing diskwatch kind %d",k);
	  break;
     }
    if (sc->flags & SF_SET) clr_device(sc);
    sc->flags &= ~SF_RATE;
    check_ring(sc);
    kick_softc(sc);
    if (diskwatch_verbose) printf("diskwatchclose: %d/%d succeeded\n",u,k);
  } while (0);
 mutex_exit(&the_lock);
 return(0);
}

static int diskwatchread(dev_t dev, struct uio *uio, int ioflag)
{
 unsigned int u;
 unsigned int k;
 SOFTC *sc;
 int e;
 int done;

 u = minor(dev);
 k = DW_MINOR_KIND(u);
 u = DW_MINOR_UNIT(u);
 if (u >= ndiskwatch) panic("reading diskwatch #%d",u);
 sc = &softcv[u];
 mutex_enter(&the_lock);
 e = 0;
 do <"done">
  { switch (k)
     { case DW_KIND_CTL:
	  if (diskwatch_verbose) printf("diskwatchread: %d/%d CTL\n",u,k);
	   { char c;
	     if (! SF_ISREADY(sc->flags))
	      { if (diskwatch_verbose) printf("diskwatchread: %d/%d EIO (!ready)\n",u,k);
		e = EIO;
		break <"done">;
	      }
	     switch (sc->flags & SF_RATE)
	      { case SF_RATE_OFF: c = '0'; break;
		case SF_RATE_LO:  c = '1'; break;
		case SF_RATE_HI:  c = '2'; break;
		default: panic("diskwatchread: bad rate"); break;
	      }
	     if (diskwatch_verbose) printf("diskwatchread: %d/%d returning %c\n",u,k,c);
	     e = uiomove(&c,1,uio);
	     break <"done">;
	   }
	  break;
       case DW_KIND_DATA:
	  if (diskwatch_verbose) printf("diskwatchread: %d/%d DATA\n",u,k);
	  if (! SF_ISREADY(sc->flags))
	   { if (diskwatch_verbose) printf("diskwatchread: %d/%d EIO (!ready)\n",u,k);
	     e = EIO;
	     break <"done">;
	   }
	  while (sc->ringn == 0)
	   { if (sc->flags & SF_NBIO)
	      { if (diskwatch_verbose) printf("diskwatchread: %d/%d EWOULDBLOCK (nbio)\n",u,k);
		e = EWOULDBLOCK;
		break <"done">;
	      }
	     e = cv_wait_sig(&sc->cv,&the_lock);
	     if (e)
	      { if (diskwatch_verbose) printf("diskwatchread: %d/%d %d (cv_wait_sig)\n",u,k,e);
		break <"done">;
	      }
	   }
	  done = 0;
	  while (sc->ringn > 0)
	   { PKT *p;
	     int us;
	     unsigned long long int hdr;
	     p = &sc->ring[sc->ringt];
	     switch (p->type)
	      { case PKT_DATA:
		   if (diskwatch_verbose) printf("diskwatchread: %d/%d data\n",u,k);
		   us = sizeof(hdr) + 512;
		   break;
		case PKT_NUM:
		   if (diskwatch_verbose) printf("diskwatchread: %d/%d num\n",u,k);
		   us = sizeof(hdr);
		   break;
		case PKT_STOP:
		   if (diskwatch_verbose) printf("diskwatchread: %d/%d stop\n",u,k);
		   us = sizeof(hdr);
		   break;
		default:
		   if (diskwatch_verbose) printf("diskwatchread: %d/%d type %d\n",u,k,p->type);
		   panic("reading impossible packet type");
		   break;
	      }
	     if (uio->uio_resid < us)
	      { if (diskwatch_verbose) printf("diskwatchread: %d/%d %s\n",u,k,done?"success":"EMSGSIZE");
		if (done < 1) e = EMSGSIZE;
		break <"done">;
	      }
	     switch (p->type)
	      { case PKT_DATA:
		   hdr = 0x0000000000000000ULL | p->blk;
		   break;
		case PKT_NUM:
		   hdr = 0x0100000000000000ULL | p->blk;
		   break;
		case PKT_STOP:
		   hdr = 0x0200000000000000ULL | p->blk;
		   break;
		default:
		   panic("reading impossible packet type");
		   break;
	      }
	     e = uiomove(&hdr,sizeof(hdr),uio);
	     if (e)
	      { if (diskwatch_verbose) printf("diskwatchread: %d/%d %d (uiomove)\n",u,k,e);
		break <"done">;
	      }
	     if (p->type == PKT_DATA)
	      { e = uiomove(p->data,512,uio);
		if (e)
		 { if (diskwatch_verbose) printf("diskwatchread: %d/%d %d (uiomove data)\n",u,k,e);
		   break <"done">;
		 }
	      }
	     drop_tail_packet(sc);
	     done ++;
	   }
	  break <"done">;
       case DW_KIND_DBG:
	  if (diskwatch_verbose) printf("diskwatchread: %d/%d DBG\n",u,k);
	   { char buf[256];
	     if (uio->uio_offset) break <"done">;
	     sprintf(&buf[0],"f=%08x c=%d,%d ",sc->flags,major(sc->cdev),minor(sc->cdev));
	     if (sc->flags & SF_HEAD) sprintf(&buf[strlen(&buf[0])],"wu=%d ",sc->watchunit);
	     if (sc->ring)
	      { sprintf(&buf[strlen(&buf[0])],"ring h=%d t=%d n=%d d=%d\n",sc->ringh,sc->ringt,sc->ringn,sc->ringd);
	      }
	     else
	      { sprintf(&buf[strlen(&buf[0])],"ring none (h=%d t=%d n=%d d=%d)\n",sc->ringh,sc->ringt,sc->ringn,sc->ringd);
	      }
	     if (diskwatch_verbose) printf("diskwatchread: %d/%d dbg: %s",u,k,&buf[0]);
	     e = uiomove(&buf[0],strlen(&buf[0]),uio);
	     break <"done">;
	   }
	  break;
       default:
	  if (diskwatch_verbose) printf("diskwatchread: %d/%d bad kind\n",u,k);
	  panic("reading diskwatch kind %d",k);
	  break;
     }
  } while (0);
 mutex_exit(&the_lock);
 return(e);
}

static int diskwatchwrite(dev_t dev, struct uio *uio, int ioflag)
{
 unsigned int u;
 unsigned int k;
 SOFTC *sc;
 int e;

 u = minor(dev);
 k = DW_MINOR_KIND(u);
 u = DW_MINOR_UNIT(u);
 if (u >= ndiskwatch) panic("writing diskwatch #%d",u);
 sc = &softcv[u];
 mutex_enter(&the_lock);
 e = 0;
 do <"done">
  { switch (k)
     { case DW_KIND_CTL:
	  if (diskwatch_verbose) printf("diskwatchwrite: %d/%d CTL\n",u,k);
	   { char cmd;
	     if (! SF_ISREADY(sc->flags))
	      { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d EIO (!ready)\n",u,k);
		e = EIO;
		break <"done">;
	      }
	     e = uiomove(&cmd,1,uio);
	     if (e)
	      { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d %d (uiomove cmd)\n",u,k,e);
		break <"done">;
	      }
	     switch (cmd)
	      { default:
		   if (diskwatch_verbose) printf("diskwatchwrite: %d/%d EIO (bad cmd)\n",u,k);
		   e = EIO;
		   break <"done">;
		   break;
		case 'd':
		    { char buf[64];
		      int n;
		      n = uio->uio_resid;
		      if (n >= sizeof(buf)) n = sizeof(buf) - 1;
		      e = uiomove(&buf[0],n,uio);
		      if (e)
		       { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d %d (uiomove d key)\n",u,k,e);
			 break <"done">;
		       }
		      buf[n] = '\0';
		      switch (buf[0])
		       { case 'f':
			     { int fd;
			       file_t *fp;
			       fd = num(&buf[1]);
			       fp = fd_getfile(fd);
			       if (! fp)
				{ if (diskwatch_verbose) printf("diskwatchwrite: %d/%d EBADF (fd_getfile failed)\n",u,k);
				  e = EBADF;
				}
			       else
				{ if (fp->f_type != DTYPE_VNODE)
				   { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d EFTYPE (is %d)\n",u,k,fp->f_type);
				     e = EFTYPE;
				   }
				  else
				   { /* Wish we could use spec_ioctl here */
				     struct vnode *vp;
				     dev_t dv;
				     vp = fp->f_data;
				     /* this code stolen from spec_ioctl */
				     dv = NODEV;
				     mutex_enter(&vp->v_interlock);
				     if (!(vp->v_iflag & VI_XLOCK) && vp->v_specnode) dv = vp->v_rdev;
				     mutex_exit(&vp->v_interlock);
				     if (dev == NODEV)
				      { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d ENXIO (vnode changing or not special)\n",u,k);
					e = ENXIO;
				      }
				     else if (vp->v_type != VCHR)
				      { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d EFTYPE (not VCHR)\n",u,k);
					e = EFTYPE;
				      }
				     else
				      { e = set_device(sc,dv);
					if (diskwatch_verbose) if (e) printf("diskwatchwrite: %d/%d %d (set_device)\n",u,k,e);
				      }
				   }
				  fd_putfile(fd);
				}
			       if (diskwatch_verbose) printf("diskwatchwrite: %d/%d returning %d\n",u,k,e);
			       break <"done">;
			     }
			    break;
			 default:
			    if (diskwatch_verbose) printf("diskwatchwrite: %d/%d EIO (bad d key)\n",u,k);
			    e = EIO;
			    break <"done">;
		       }
		    }
		   break;
		case '0':
		    { unsigned int r;
		      r = SF_RATE_OFF;
		      if (0)
		       {
		case '1':
			 r = SF_RATE_LO;
		       }
		      if (0)
		       {
		case '2':
			 r = SF_RATE_HI;
		       }
		      if (diskwatch_verbose) printf("diskwatchwrite: %d/%d rate %c\n",u,k,cmd);
		      if (! (sc->flags & SF_SET))
		       { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d EIO (!set)\n",u,k);
			 e = EIO;
			 break <"done">;
		       }
		      sc->flags = (sc->flags & ~SF_RATE) | r;
		    }
		   break;
	      }
	   }
	  break;
       case DW_KIND_DATA:
	  if (diskwatch_verbose) printf("diskwatchwrite: %d/%d DATA\n",u,k);
	  e = EIO;
	  break <"done">;
       case DW_KIND_DBG:
	  if (diskwatch_verbose) printf("diskwatchwrite: %d/%d DBG\n",u,k);
	   { char buf;
	     if (uio->uio_resid < 1) break <"done">;
	     e = uiomove(&buf,1,uio);
	     if (e)
	      { if (diskwatch_verbose) printf("diskwatchwrite: %d/%d %d (uiomove d key)\n",u,k,e);
		break <"done">;
	      }
	     diskwatch_verbose = buf;
	     break <"done">;
	   }
	  break;
       default:
	  if (diskwatch_verbose) printf("diskwatchwrite: %d/%d bad kind\n",u,k);
	  panic("writing diskwatch kind %d",k);
	  break;
     }
    if (diskwatch_verbose) printf("diskwatchwrite: %d/%d success\n",u,k);
  } while (0);
 mutex_exit(&the_lock);
 return(e);
}

static int diskwatchioctl(dev_t dev, u_long cmd, void *data, int flag, struct lwp *p)
{
 unsigned int u;
 unsigned int k;
 SOFTC *sc;
 int e;

 u = minor(dev);
 k = DW_MINOR_KIND(u);
 u = DW_MINOR_UNIT(u);
 if (u >= ndiskwatch) panic("ioctling diskwatch #%d",u);
 sc = &softcv[u];
 mutex_enter(&the_lock);
 switch (k)
  { case DW_KIND_DATA:
       if (diskwatch_verbose) printf("diskwatchioctl: %d/%d DATA\n",u,k);
       switch (cmd)
	{ case FIONBIO:
	     if (diskwatch_verbose) printf("diskwatchioctl: %d/%d FIONBIO %s\n",u,k,(*(int *)data)?"on":"off");
	     if (*(int *)data) sc->flags |= SF_NBIO; else sc->flags &= ~SF_NBIO;
	     e = 0;
	     break;
	  case FIOASYNC:
	     /* annoying bug in fcntl(F_SETFL) on devices in some versions:
		the device must support both FIONBIO and FIOASYNC ioctls,
		even if the corresponding fcntl bits are never set, or
		nonblocking mode is gratuitously turned off! */
	     e = (*(int *)data) ? EOPNOTSUPP : 0;
	     break;
	  default:
	     if (diskwatch_verbose) printf("diskwatchioctl: %d/%d ENOTTY cmd %08lx\n",u,k,(unsigned long int)cmd);
	     e = ENOTTY;
	     break;
	}
       break;
    default:
       if (diskwatch_verbose) printf("diskwatchioctl: %d/%d ENOTTY bad kind\n",u,k);
       e = ENOTTY;
       break;
  }
 mutex_exit(&the_lock);
 return(e);
}

static int diskwatchpoll(dev_t dev, int events, struct lwp *p)
{
 unsigned int u;
 unsigned int k;
 SOFTC *sc;
 int revents;

 u = minor(dev);
 k = DW_MINOR_KIND(u);
 u = DW_MINOR_UNIT(u);
 if (u >= ndiskwatch) panic("polling diskwatch #%d",u);
 sc = &softcv[u];
 mutex_enter(&the_lock);
 if (! SF_ISREADY(sc->flags))
  { mutex_exit(&the_lock);
    return(events);
  }
 switch (k)
  { case DW_KIND_CTL:
    case DW_KIND_DBG:
       mutex_exit(&the_lock);
       return(events);
       break;
    case DW_KIND_DATA:
       revents = events & (POLLOUT|POLLWRNORM);
       if (sc->ringn > 0)
	{ revents |= events & (POLLIN|POLLRDNORM);
	}
       else
	{ selrecord(p,&sc->rsel);
	}
       break;
    default:
       panic("polling diskwatch kind %d",k);
       break;
  }
 mutex_exit(&the_lock);
 return(revents);
}

void diskwatch_watch(int u, struct buf *bp)
{
 SOFTC *sc;
 int nblks;
 int loops;

 if ((u < 0) || (u >= ndiskwatch)) panic("diskwatch_watch: impossible unit %d",u);
 mutex_enter(&the_lock);
 sc = watchptrs[u];
 if (! sc) panic("diskwatch_watch lost watchptr");
 if (! (sc->flags & SF_SET)) panic("diskwatch_watch unset softc");
 if (sc->watchunit != u) panic("diskwatch_watch unit wrong");
 loops = ndiskwatch + 1;
 while (sc)
  { if (! SF_ISREADY(sc->flags)) panic("diskwatch_watch: unready unit %d",u);
    if (bp->b_bcount & 511) panic("diskwatch_watch: unaligned size %lu",(unsigned long int)bp->b_bcount);
    nblks = bp->b_bcount >> 9;
    switch (sc->flags & SF_RATE)
     { case SF_RATE_OFF:
	  break;
       case SF_RATE_HI:
	  if (sc->ringd+nblks > RING_MAXDATA)
	   { discard_ring_data(sc);
	     sc->flags = (sc->flags & ~SF_RATE) | SF_RATE_LO;
       case SF_RATE_LO:
	     if (sc->ringn+nblks >= RING_SIZE)
	      { sc->flags = (sc->flags & ~SF_RATE) | SF_RATE_OFF;
		if (sc->ringn < RING_SIZE) queue_stop(sc);
	      }
	     else
	      { queue_nums(sc,bp->b_blkno,nblks);
	      }
	   }
	  else
	   { queue_data(sc,bp->b_blkno,nblks,bp->b_data);
	   }
	  kick_softc(sc);
	  break;
       default:
	  panic("diskwatch_watch: impossible rate");
	  break;
     }
    sc = sc->flink;
    if (loops-- < 1) panic("loopy diskwatch unit list");
  }
 mutex_exit(&the_lock);
}

void diskwatch_detach(int u)
{
 SOFTC *sc;

 if ((u < 0) || (u >= ndiskwatch)) panic("diskwatch_detach: impossible unit %d",u);
 sc = &softcv[u];
 mutex_enter(&the_lock);
 if (sc->flags & SF_SET) clr_device(sc);
 kick_softc(sc);
 mutex_exit(&the_lock);
}

const struct cdevsw diskwatch_cdevsw
 = { &diskwatchopen, &diskwatchclose, &diskwatchread,
     &diskwatchwrite, &diskwatchioctl, nostop, notty,
     &diskwatchpoll, nommap, nokqfilter, D_OTHER|D_MPSAFE };

#endif
