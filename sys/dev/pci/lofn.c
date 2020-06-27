/*
 * Driver for the `lofn', the HiFn 6500 RNG and ALU.
 *
 * This really should be called `hifn6500'.  But autoconf really does
 *  not like device names ending with digits (which is understandable
 *  but annoying).  So, instead, we use lofn, which name I got from
 *  OpenBSD (and it is not entirely inappropriate, as this is an
 *  unusually simple device).
 */

#include <sys/bus.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/mutex.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/device.h>
#include <sys/select.h>
#include <sys/endian.h>
#include <sys/condvar.h>
#include <machine/bus.h>
#include <machine/param.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include "lofn-reg.h"
#include "lofn-api.h"

typedef struct softc SOFTC;

/*
 * Our minor number is made up of two fields.  The low three bits are a
 *  "function within device" field; the rest is the unit number.
 *
 * We currently have only three functions defined, one for the RNG, one
 *  for the ALU, and one for human-readable status.  (We could,
 *  currently, get away with only two bits for function number; the
 *  other one is for future-proofing.)
 */
#define LOFN_MINOR(unit,fxn) (((unit)<<3)|((fxn)&7))
#define LOFN_MINOR_UNIT(m) ((m) >> 3)
#define LOFN_MINOR_FXN(m) ((m) & 7)
#define LOFN_FXN_RNG  0
#define LOFN_FXN_ALU  1
#define LOFN_FXN_STAT 7

typedef unsigned long long int ICNT;

/*
 * dev - the device_t.
 *
 * XXX rng_data really should be marked volatile.  The only reason I
 *  don't do that is that bus_space_read_region_4's fourth arg isn't
 *  volatile-qualified(!).  Instead, we add the volatile when we need
 *  to when accessing it.
 *
 * The max useful compute vector size is capped by:
 *	- write each register
 *	- write all available code
 *	- read each register
 *  which takes up 16*(1+1+32+1) + 1+1+32 + 16*(1+1+32+1) words, hence
 *  the definition of LOFN_MAX_COMPUTE.
 *
 * We have multiple ALU state bits because the ALU can be in multiple states:
 *
 *	- Idle (IDLE)
 *	- Grabbed, loading (SETUP)
 *	- Crunching, caller is waiting (BUSY)
 *	- Crunching, caller has given up (DRAIN)
 *	- Done, caller has not yet collected results (DONE)
 */
#define LOFN_MAX_COMPUTE ((16*(1+1+32+1)) + (1+1+32) + (16*(1+1+32+1)))
struct softc {
  device_t dev;
  unsigned int flags;
#define LOF_BROKEN    0x00000001
#define LOF_RNG_OPEN  0x00000002
#define LOF_ALU_OPEN  0x00000004
  kmutex_t sc_mtx;
  unsigned int alu_state;
#define ALU_STATE_IDLE  1
#define ALU_STATE_SETUP 2
#define ALU_STATE_BUSY  3
#define ALU_STATE_DRAIN 4
#define ALU_STATE_DONE  5
  kcondvar_t alu_cv;
  kmutex_t alu_mtx;
  pci_chipset_tag_t pc;
  pcitag_t pt;
  bus_space_tag_t bt;
  bus_space_handle_t bh;
  bus_addr_t ba;
  bus_size_t bs;
  pci_intr_handle_t ih;
  void *ie;
  unsigned int win;
#define LOFN_RNG_BUF_LEN 64
  u_int32_t rng_data[LOFN_RNG_BUF_LEN];
  volatile int rng_wx; // in u_int32_ts
  volatile int rng_rx; // in bytes
  volatile int rng_n; // in bytes
  kcondvar_t rng_cv;
  kmutex_t rng_mtx;
  struct selinfo si;
  volatile ICNT icnt_rng;
  volatile ICNT icnt_alu;
  u_int32_t compute_vec[LOFN_MAX_COMPUTE];
  } ;

extern struct cfdriver lofn_cd;

static ICNT lofn_read_icnt(volatile ICNT *p)
{
 ICNT a;
 ICNT b;

 b = *p;
 while (1)
  { a = b;
    b = *p;
    if (a == b) return(a);
  }
}

static void lofn_rng_start(SOFTC *sc)
{
 sc->rng_rx = 0;
 sc->rng_wx = 0;
 sc->rng_n = 0;
 bus_space_write_4(sc->bt,sc->bh,LOFN_CFG_2,bus_space_read_4(sc->bt,sc->bh,LOFN_CFG_2)|LOFN_CFG_2_RNG_ENB);
 bus_space_write_4(sc->bt,sc->bh,LOFN_INTENB,bus_space_read_4(sc->bt,sc->bh,LOFN_INTENB)|LOFN_INTENB_RNG);
}

static void lofn_alu_start(SOFTC *sc)
{
 bus_space_write_4(sc->bt,sc->bh,LOFN_CFG_2,bus_space_read_4(sc->bt,sc->bh,LOFN_CFG_2)|LOFN_CFG_2_ALU_ENB);
}

static void lofn_rng_stop(SOFTC *sc)
{
 bus_space_write_4(sc->bt,sc->bh,LOFN_CFG_2,bus_space_read_4(sc->bt,sc->bh,LOFN_CFG_2)&~LOFN_CFG_2_RNG_ENB);
 bus_space_write_4(sc->bt,sc->bh,LOFN_INTENB,bus_space_read_4(sc->bt,sc->bh,LOFN_INTENB)&~LOFN_INTENB_RNG);
}

static void lofn_alu_stop(SOFTC *sc)
{
 bus_space_write_4(sc->bt,sc->bh,LOFN_CFG_2,bus_space_read_4(sc->bt,sc->bh,LOFN_CFG_2)&~LOFN_CFG_2_ALU_ENB);
 bus_space_write_4(sc->bt,sc->bh,LOFN_INTENB,bus_space_read_4(sc->bt,sc->bh,LOFN_INTENB)&~LOFN_INTENB_DONE);
}

static int lofn_intr(void *scv)
{
 SOFTC *sc;
 unsigned int stat;
 unsigned int v;
 int ours;
 int i;
 int n;
 int x;

 sc = scv;
 ours = 0;
 stat = bus_space_read_4(sc->bt,sc->bh,LOFN_STATUS);
 if (stat & LOFN_STATUS_RNG_READY)
  { sc->icnt_rng ++;
    v = bus_space_read_4(sc->bt,sc->bh,LOFN_STATUS);
    if (v & LOFN_STATUS_RNG_UF)
     { ours = 1;
       printf("%s: RNG underflow (disabling)\n",&sc->dev->dv_xname[0]);
       lofn_rng_stop(sc);
     }
    else if (v & LOFN_STATUS_RNG_READY)
     { ours = 1;
       mutex_enter(&sc->rng_mtx);
       n = sc->rng_n;
       x = sc->rng_wx;
       /*
	* The magic number 8 here is the number of words of randomness
	*  guaranteed to be present in the FIFO.
	*/
       for (i=8;i>0;i--)
	{ v = bus_space_read_4(sc->bt,sc->bh,LOFN_RNG_FIFO);
	  if (n+4 <= LOFN_RNG_BUF_LEN)
	   { ((volatile u_int32_t *)&sc->rng_data[0])[x] = v;
	     x ++;
	     if (x >= LOFN_RNG_BUF_LEN) x = 0;
	     n += 4;
	   }
	}
       sc->rng_n = n;
       sc->rng_wx = x;
       cv_broadcast(&sc->rng_cv);
       mutex_exit(&sc->rng_mtx);
       selnotify(&sc->si,0,0);
     }
  }
 if (stat & LOFN_STATUS_DONE)
  { sc->icnt_alu ++;
    mutex_enter(&sc->alu_mtx);
    switch (sc->alu_state)
     { case ALU_STATE_BUSY:
	  sc->alu_state = ALU_STATE_DONE;
	  break;
       case ALU_STATE_DRAIN:
	  sc->alu_state = ALU_STATE_IDLE;
	  break;
       default:
	  break;
     }
    /*
     * Someday should provide a way to queue stuff, so we can shift
     *	from one op to another here, instead of disabling the
     *	interrupt, shifting to non-interrupt code, and shifting back.
     */
    bus_space_write_4(sc->bt,sc->bh,LOFN_INTENB,bus_space_read_4(sc->bt,sc->bh,LOFN_INTENB)&~LOFN_INTENB_DONE);
    cv_broadcast(&sc->alu_cv);
    mutex_exit(&sc->alu_mtx);
  }
 return(ours);
}

static int lofn_match(device_t parent, cfdata_t cf, void *aux)
{
 struct pci_attach_args *pa;

 pa = aux;
 if ( (PCI_VENDOR(pa->pa_id) != PCI_VENDOR_HIFN) ||
      (PCI_PRODUCT(pa->pa_id) != PCI_PRODUCT_HIFN_6500) ) return(0);
 return(1);
}

static void lofn_attach(device_t parent, device_t self, void *aux)
{
 SOFTC *sc;
 struct pci_attach_args *pa;
 pcireg_t r;
 char devinfo[256];

 sc = device_private(self);
 pa = aux;
 sc->dev = self;
 sc->pc = pa->pa_pc;
 sc->pt = pa->pa_tag;
 pci_devinfo(pa->pa_id,pa->pa_class,0,&devinfo[0],sizeof(devinfo));
 aprint_normal(": %s (rev. 0x%02x)\n",&devinfo[0],PCI_REVISION(pa->pa_class));
 r = pci_conf_read(pa->pa_pc,pa->pa_tag,PCI_COMMAND_STATUS_REG);
 if (! (r & PCI_COMMAND_MEM_ENABLE))
  { aprint_error_dev(sc->dev,"BIOS didn't enable memory space\n");
    sc->flags = LOF_BROKEN;
    return;
  }
 r = pci_conf_read(pa->pa_pc,pa->pa_tag,PCI_CLASS_REG);
 if (PCI_CLASS(r) != PCI_CLASS_PROCESSOR)
  { aprint_error_dev(sc->dev,"class isn't `processor'\n");
    sc->flags = LOF_BROKEN;
    return;
  }
 if (PCI_SUBCLASS(r) != PCI_SUBCLASS_PROCESSOR_COPROC)
  { aprint_error_dev(sc->dev,"subclass isn't `coprocessor'\n");
    sc->flags = LOF_BROKEN;
    return;
  }
 if (pci_mapreg_map(pa,LOFN_BAR,PCI_MAPREG_TYPE_MEM,0,&sc->bt,&sc->bh,&sc->ba,&sc->bs))
  { aprint_normal_dev(sc->dev,"can't map\n");
    sc->flags = LOF_BROKEN;
    return;
  }
#if !defined(BYTE_ORDER)
#error "BYTE_ORDER not defined"
#elif BYTE_ORDER == BIG_ENDIAN
 sc->win = LOFN_WIN_BE_LE;
#elif BYTE_ORDER == LITTLE_ENDIAN
 sc->win = LOFN_WIN_LE_LE;
#else
#error "BYTE_ORDER neither BIG_ENDIAN nor LITTLE_ENDIAN"
#endif
 bus_space_write_4(sc->bt,sc->bh,LOFN_CFG_1,bus_space_read_4(sc->bt,sc->bh,LOFN_CFG_1));
 if (pci_intr_map(pa,&sc->ih))
  { aprint_error_dev(sc->dev,"can't map interrupt\n");
    sc->flags = LOF_BROKEN;
    return;
  }
 aprint_normal_dev(sc->dev,"interrupting at %s\n",pci_intr_string(sc->pc,sc->ih));
 sc->ie = pci_intr_establish(sc->pc,sc->ih,IPL_BIO,&lofn_intr,sc);
 sc->flags = 0;
 sc->icnt_rng = 0;
 sc->icnt_alu = 0;
 mutex_init(&sc->sc_mtx,MUTEX_DEFAULT,IPL_BIO);
 mutex_init(&sc->rng_mtx,MUTEX_DEFAULT,IPL_BIO);
 cv_init(&sc->rng_cv,"lofn-rng");
 mutex_init(&sc->alu_mtx,MUTEX_DEFAULT,IPL_BIO);
 cv_init(&sc->alu_cv,"lofn-alu");
 selinit(&sc->si);
 sc->alu_state = ALU_STATE_IDLE;
 /*
  * This is the slowest generation speed available, and presumably the
  *  best randomness.  It's still fast enough for the expected uses.
  *  For a 100MHz part, 32M cycles per sample gives
  *  100e6 cycles/second / 32<<20 cycles/sample, or 2.98+ samples per
  *  second, so 8 samples takes about 2.684 seconds.  If 50MHz, of
  *  course, it's half that speed.
  *
  * I'm told OpenBSD used 0x700 as the value here.  That is 512K cycles
  *  per sample, for 190.735- samples/second.
  */
 bus_space_write_4(sc->bt,sc->bh,LOFN_RNGCFG,0);
}

CFATTACH_DECL_NEW(lofn,sizeof(SOFTC),&lofn_match,&lofn_attach,0,0);

static int lofn_open(dev_t dev, int flags, int mode, struct lwp *l)
{
 int u;
 int f;
 SOFTC *sc;
 int e;

 u = minor(dev);
 f = LOFN_MINOR_FXN(u);
 u = LOFN_MINOR_UNIT(u);
 sc = device_lookup_private(&lofn_cd,u);
 if (! sc) return(ENXIO);
 if (sc->flags & LOF_BROKEN) return(ENXIO);
 e = 0;
 mutex_enter(&sc->sc_mtx);
 switch (f)
  { case LOFN_FXN_RNG:
       if (! (sc->flags & LOF_RNG_OPEN))
	{ sc->flags |= LOF_RNG_OPEN;
	  lofn_rng_start(sc);
	}
       break;
    case LOFN_FXN_ALU:
       if (! (sc->flags & LOF_ALU_OPEN))
	{ sc->flags |= LOF_ALU_OPEN;
	  lofn_alu_start(sc);
	}
       break;
    case LOFN_FXN_STAT:
       break;
    default:
       e = ENXIO;
       break;
  }
 mutex_exit(&sc->sc_mtx);
 return(e);
}

static int lofn_close(dev_t dev, int flags, int mode, struct lwp *l)
{
 int u;
 int f;
 SOFTC *sc;

 u = minor(dev);
 f = LOFN_MINOR_FXN(u);
 u = LOFN_MINOR_UNIT(u);
 sc = device_lookup_private(&lofn_cd,u);
 if (! sc) panic("closing nonexistent lofn %d",u);
 if (sc->flags & LOF_BROKEN) panic("closing broken lofn %d",u);
 mutex_enter(&sc->sc_mtx);
 switch (f)
  { case LOFN_FXN_RNG:
       sc->flags &= ~LOF_RNG_OPEN;
       lofn_rng_stop(sc);
       break;
    case LOFN_FXN_ALU:
       sc->flags &= ~LOF_ALU_OPEN;
       lofn_alu_stop(sc);
       break;
    case LOFN_FXN_STAT:
       break;
    default:
       panic("closing impossible lofn fxn %d",f);
       break;
  }
 mutex_exit(&sc->sc_mtx);
 return(0);
}

static int lofn_read(dev_t dev, struct uio *uio, int flags)
{
 int u;
 int f;
 SOFTC *sc;
 int e;

 u = minor(dev);
 f = LOFN_MINOR_FXN(u);
 u = LOFN_MINOR_UNIT(u);
 sc = device_lookup_private(&lofn_cd,u);
 if (! sc) panic("reading nonexistent lofn %d",u);
 if (sc->flags & LOF_BROKEN) panic("reading broken lofn %d",u);
 e = 0;
 switch (f)
  { case LOFN_FXN_RNG:
       mutex_enter(&sc->rng_mtx);
       while (uio->uio_resid > 0)
	{ int i;
	  int n;
	  int nb;
	  n = sc->rng_n;
	  if (n < 1)
	   { if (flags & IO_NDELAY)
	      { e = EWOULDBLOCK;
		break;
	      }
	     else
	      { e = cv_wait_sig(&sc->rng_cv,&sc->rng_mtx);
		if (e) break;
		continue;
	      }
	   }
	  i = sc->rng_rx;
	  nb = n;
	  if (nb > uio->uio_resid)
	   { nb = uio->uio_resid;
	   }
	  if (i+nb > 4*LOFN_RNG_BUF_LEN)
	   { nb = (4 * LOFN_RNG_BUF_LEN) - i;
	   }
	  e = uiomove(((char *)&sc->rng_data[0])+i,nb,uio);
	  if (! e)
	   { i += nb;
	     n -= nb;
	   }
	  sc->rng_rx = i;
	  sc->rng_n = n;
	  if (e) break;
	}
       mutex_exit(&sc->rng_mtx);
       break;
    case LOFN_FXN_ALU:
       e = EIO;
       break;
    case LOFN_FXN_STAT:
	{ ICNT ir;
	  ICNT ia;
	  int l;
	  char dbuf[256];
	  if (uio->uio_offset) return(0);
	  ir = lofn_read_icnt(&sc->icnt_rng);
	  ia = lofn_read_icnt(&sc->icnt_alu);
	  l = sprintf(&dbuf[0],"icnt: rng %llu, alu %llu\n",
		(unsigned long long int)ir,
		(unsigned long long int)ia);
	  if (l > uio->uio_resid) l = uio->uio_resid;
	  e = uiomove(&dbuf[0],l,uio);
	}
       break;
  }
 return(e);
}

static int lofn_write(dev_t dev, struct uio *uio, int flags)
{
 (void)dev;
 (void)uio;
 (void)flags;
 return(EIO);
}

/*
 * alu_mtx exists to be the mutex for alu_cv.  alu_cv is used for two
 *  different things: (1) it's used to wait for the ALU to become
 *  available when someone else is using it and (2) it's used to wait
 *  for ALU operation to complete when we're using it.
 */
static int lofn_compute(SOFTC *sc, struct lwp *l, const struct lofn_compute *comp)
{
 int e;
 int x;
 int phase;
 int n;
 int i;
 u_int32_t v;

 if (sizeof(unsigned int) != sizeof(u_int32_t))
  { printf("sizeof(unsigned int) != sizeof(u_int32_t)\n");
    return(EIO);
  }
 KASSERT(mutex_owned(&sc->sc_mtx));
 if (comp->len < 1) return(EINVAL);
 if (comp->len > LOFN_MAX_COMPUTE) return(E2BIG); // needs translation?
 mutex_exit(&sc->sc_mtx);
 mutex_enter(&sc->alu_mtx);
 while (1)
  { if (sc->alu_state == ALU_STATE_IDLE) break;
    e = cv_wait_sig(&sc->alu_cv,&sc->alu_mtx);
    if (e)
     { mutex_exit(&sc->alu_mtx);
       mutex_enter(&sc->sc_mtx);
       return(e);
     }
  }
 sc->alu_state = ALU_STATE_SETUP;
 mutex_exit(&sc->alu_mtx);
 v = bus_space_read_4(sc->bt,sc->bh,LOFN_STATUS);
 if (! (v & LOFN_STATUS_DONE)) panic("lofn_compute: idle ALU isn't done");
 do <"done">
  { do <"inval">
     { e = copyin(comp->vec,&sc->compute_vec[0],comp->len*sizeof(u_int32_t));
       if (e) break <"done">;
       x = 0;
       phase = 0;
       while (1)
	{ if (x == comp->len) break;
	  if (x > comp->len) panic("lofn_compute: verifier overrun (%d > %d)",x,comp->len);
	  switch (sc->compute_vec[x])
	   { default:
		break <"inval">;
	     case LOFN_COMPUTE_SETREG:
		if (phase != 0)
		 { printf("lofn_compute: SETREG at [%d] is late\n",x);
		   break <"inval">;
		 }
		if (x+35 > comp->len)
		 { printf("lofn_compute: SETREG at [%d] overruns buffer size %d\n",x,comp->len);
		   break <"inval">;
		 }
		v = sc->compute_vec[x+1];
		if (v > 15)
		 { printf("lofn_compute: SETREG at [%d] uses bad register %lu\n",x,(unsigned long int)v);
		   break <"inval">;
		 }
		i = v;
		v = sc->compute_vec[x+34];
		if (v > 1024)
		 { printf("lofn_compute: SETREG at [%d] has bad length %lu\n",x,(unsigned long int)v);
		   break <"inval">;
		 }
		bus_space_write_region_4(sc->bt,sc->bh,sc->win+LOFN_BIGNUM(i,0),&sc->compute_vec[x+2],32);
		bus_space_write_4(sc->bt,sc->bh,LOFN_LENGTH(i),v);
		x += 35;
		break;
	     case LOFN_COMPUTE_CODE:
		if (phase != 0)
		 { printf("lofn_compute: CODE at [%d] is extra\n",x);
		   break <"inval">;
		 }
		if (x+1 > comp->len)
		 { printf("lofn_compute: CODE at [%d] is at end of buffer size %d\n",x,comp->len);
		   break <"inval">;
		 }
		v = sc->compute_vec[x+1];
		if ((v < 1) || (v > 32))
		 { printf("lofn_compute: CODE at [%d] has bad count %lu\n",x,(unsigned long int)v);
		   break <"inval">;
		 }
		n = v;
		if (x+2+n > comp->len)
		 { printf("lofn_compute: CODE at [%d] overruns buffer size %d\n",x,comp->len);
		   break <"inval">;
		 }
		for (i=0;i<n;i++)
		 { v = sc->compute_vec[x+2+i];
		   if (v & LOFN_INST_LAST)
		    { if (i != n-1)
		       { printf("lofn_compute: CODE at [%d] instr %d is LAST but not last\n",x,i);
			 break <"inval">;
		       }
		    }
		   else
		    { if (i == n-1)
		       { printf("lofn_compute: CODE at [%d] instr %d is last but not LAST\n",x,i);
			 break <"inval">;
		       }
		    }
		   switch ((v >> LOFN_INST_OPC_S) & LOFN_INST_OPC_M)
		    { case LOFN_INST_OPC_MODEXP:
		      case LOFN_INST_OPC_MODMUL:
		      case LOFN_INST_OPC_MODRED:
		      case LOFN_INST_OPC_MODADD:
		      case LOFN_INST_OPC_MODSUB:
		      case LOFN_INST_OPC_ADD:
		      case LOFN_INST_OPC_SUB:
		      case LOFN_INST_OPC_ADD_C:
		      case LOFN_INST_OPC_SUB_B:
		      case LOFN_INST_OPC_2K_MUL:
		      case LOFN_INST_OPC_INC:
		      case LOFN_INST_OPC_DEC:
			 if (((v >> LOFN_INST_RD_M) & LOFN_INST_RD_S) > 15)
			  { printf("lofn_compute: CODE at [%d] instr %d has invalid Rd %d\n",x,i,(int)((v>>LOFN_INST_RD_M)&LOFN_INST_RD_S));
			    break <"inval">;
			  }
			 if (((v >> LOFN_INST_RA_M) & LOFN_INST_RA_S) > 15)
			  { printf("lofn_compute: CODE at [%d] instr %d has invalid Ra %d\n",x,i,(int)((v>>LOFN_INST_RA_M)&LOFN_INST_RA_S));
			    break <"inval">;
			  }
			 if (((v >> LOFN_INST_RB_M) & LOFN_INST_RB_S) > 15)
			  { printf("lofn_compute: CODE at [%d] instr %d has invalid Rb %d\n",x,i,(int)((v>>LOFN_INST_RB_M)&LOFN_INST_RB_S));
			    break <"inval">;
			  }
			 if (((v >> LOFN_INST_RM_M) & LOFN_INST_RM_S) > 15)
			  { printf("lofn_compute: CODE at [%d] instr %d has invalid Rm %d\n",x,i,(int)((v>>LOFN_INST_RM_M)&LOFN_INST_RM_S));
			    break <"inval">;
			  }
			 if (v & LOFN_INST_RESV_RRRR)
			  { printf("lofn_compute: CODE at [%d] instr %d has reserved bits %08x\n",x,i,(unsigned int)(v&LOFN_INST_RESV_RRRR));
			    break <"inval">;
			  }
			 break;
		      case LOFN_INST_OPC_SHR:
		      case LOFN_INST_OPC_SHL:
		      case LOFN_INST_OPC_SETLEN:
			 if (((v >> LOFN_INST_RD_M) & LOFN_INST_RD_S) > 15)
			  { printf("lofn_compute: CODE at [%d] instr %d has invalid Rd %d\n",x,i,(int)((v>>LOFN_INST_RD_M)&LOFN_INST_RD_S));
			    break <"inval">;
			  }
			 if (((v >> LOFN_INST_RA_M) & LOFN_INST_RA_S) > 15)
			  { printf("lofn_compute: CODE at [%d] instr %d has invalid Ra %d\n",x,i,(int)((v>>LOFN_INST_RA_M)&LOFN_INST_RA_S));
			    break <"inval">;
			  }
			 if (v & LOFN_INST_RESV_RRV)
			  { printf("lofn_compute: CODE at [%d] instr %d has reserved bits %08x\n",x,i,(unsigned int)(v&LOFN_INST_RESV_RRV));
			    break <"inval">;
			  }
			 break;
		      default:
			 printf("lofn_compute: CODE at [%d] instr %d has bad opcode %02x\n",x,i,(unsigned int)((v>>LOFN_INST_OPC_S)&LOFN_INST_OPC_M));
			 break <"inval">;
			 break;
		    }
		 }
		bus_space_write_region_4(sc->bt,sc->bh,LOFN_CODE(0),&sc->compute_vec[x+2],n);
		x += 2 + n;
		mutex_enter(&sc->alu_mtx);
		bus_space_write_4(sc->bt,sc->bh,LOFN_COMMAND,0);
		bus_space_write_4(sc->bt,sc->bh,LOFN_INTENB,bus_space_read_4(sc->bt,sc->bh,LOFN_INTENB)|LOFN_INTENB_DONE);
		sc->alu_state = ALU_STATE_BUSY;
		do
		 { cv_wait_sig(&sc->alu_cv,&sc->alu_mtx);
		   if (e)
		    { sc->alu_state = ALU_STATE_DRAIN;
		      mutex_exit(&sc->alu_mtx);
		      break <"done">;
		    }
		 } while (sc->alu_state != ALU_STATE_DONE);
		mutex_exit(&sc->alu_mtx);
		phase = 1;
		break;
	     case LOFN_COMPUTE_GETREG:
		if (phase != 1)
		 { printf("lofn_compute: GETREG at [%d] is early\n",x);
		   e = EINVAL;
		   break <"done">;
		 }
		if (x+35 > comp->len)
		 { printf("lofn_compute: GETREG at [%d] overruns buffer size %d\n",x,comp->len);
		   e = EINVAL;
		   break <"done">;
		 }
		v = sc->compute_vec[x+1];
		if (v > 15)
		 { printf("lofn_compute: GETREG at [%d] uses bad register %lu\n",x,(unsigned long int)v);
		   e = EINVAL;
		   break <"done">;
		 }
		bus_space_read_region_4(sc->bt,sc->bh,sc->win+LOFN_BIGNUM(v,0),&sc->compute_vec[x+2],32);
		sc->compute_vec[x+34] = bus_space_read_4(sc->bt,sc->bh,LOFN_LENGTH(v));
		e = copyout(&sc->compute_vec[x+2],comp->vec+x+2,33*sizeof(u_int32_t));
		if (e)
		 { printf("lofn_compute: GETREG at [%d] copyout failed %d\n",x,e);
		   break <"done">;
		 }
		x += 35;
		break;
	   }
	}
       e = 0;
       break <"done">;
     } while (0);
    e = EINVAL;
    break <"done">;
  } while (0);
 mutex_enter(&sc->alu_mtx);
 switch (sc->alu_state)
  { case ALU_STATE_IDLE:
       panic("impossible idle ALU");
       break;
    case ALU_STATE_SETUP:
       sc->alu_state = ALU_STATE_IDLE;
       break;
    case ALU_STATE_BUSY:
       panic("impossible busy ALU");
       break;
    case ALU_STATE_DRAIN:
       break;
    case ALU_STATE_DONE:
       sc->alu_state = ALU_STATE_IDLE;
       break;
    default:
       panic("impossible ALU state %u",sc->alu_state);
       break;
  }
 mutex_exit(&sc->alu_mtx);
 mutex_enter(&sc->sc_mtx);
 return(e);
}

static int lofn_ioctl(dev_t dev, u_long ioc, void *addr, int flag, struct lwp *l)
{
 int u;
 int f;
 SOFTC *sc;
 int e;

 u = minor(dev);
 f = LOFN_MINOR_FXN(u);
 u = LOFN_MINOR_UNIT(u);
 sc = device_lookup_private(&lofn_cd,u);
 if (! sc) return(ENXIO);
 if (sc->flags & LOF_BROKEN) return(ENXIO);
 e = 0;
 mutex_enter(&sc->sc_mtx);
 switch (f)
  { case LOFN_FXN_RNG:
       switch (ioc)
	{ case LOFN_IOC_INTRCOUNT:
	     *(unsigned long long int *)addr = lofn_read_icnt(&sc->icnt_rng);
	     break;
	  default:
	     e = ENOTTY;
	     break;
	}
       break;
    case LOFN_FXN_ALU:
       switch (ioc)
	{ case LOFN_IOC_INTRCOUNT:
	     *(unsigned long long int *)addr = lofn_read_icnt(&sc->icnt_alu);
	     break;
	  case LOFN_IOC_COMPUTE:
	     e = lofn_compute(sc,l,(struct lofn_compute *)addr);
	     break;
	  default:
	     e = ENOTTY;
	     break;
	}
       break;
    case LOFN_FXN_STAT:
       e = ENOTTY;
       break;
    default:
       panic("ioctl on impossible lofn fxn %d",f);
       break;
  }
 mutex_exit(&sc->sc_mtx);
 return(e);
}

static int lofn_poll(dev_t dev, int events, struct lwp *l)
{
 int u;
 int f;
 SOFTC *sc;
 int rev;

 u = minor(dev);
 f = LOFN_MINOR_FXN(u);
 u = LOFN_MINOR_UNIT(u);
 sc = device_lookup_private(&lofn_cd,u);
 if (! sc) return(ENXIO);
 if (sc->flags & LOF_BROKEN) return(ENXIO);
 rev = events & (POLLOUT | POLLWRNORM);
 if (events & (POLLIN | POLLRDNORM))
  { mutex_enter(&sc->rng_mtx);
    if (sc->rng_n > 0)
     { rev |= events & (POLLIN | POLLRDNORM);
     }
    else if (rev == 0)
     { selrecord(l,&sc->si);
     }
    mutex_exit(&sc->rng_mtx);
  }
 return(rev);
}

static paddr_t lofn_mmap(dev_t dev, off_t off, int prot)
{
 return(-(paddr_t)1);
}

const struct cdevsw lofn_cdevsw
 = { &lofn_open,
     &lofn_close,
     &lofn_read,
     &lofn_write,
     &lofn_ioctl,
     0,
     0,
     &lofn_poll,
     &lofn_mmap,
     &nokqfilter,
     D_OTHER };
