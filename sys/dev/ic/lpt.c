/*	$NetBSD: lpt.c,v 1.75 2008/06/10 22:53:08 cegger Exp $	*/

/*
 * Copyright (c) 1993, 1994 Charles M. Hannum.
 * Copyright (c) 1990 William F. Jolitz, TeleMuse
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
 *	This software is a component of "386BSD" developed by
 *	William F. Jolitz, TeleMuse.
 * 4. Neither the name of the developer nor the name "386BSD"
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS A COMPONENT OF 386BSD DEVELOPED BY WILLIAM F. JOLITZ
 * AND IS INTENDED FOR RESEARCH AND EDUCATIONAL PURPOSES ONLY. THIS
 * SOFTWARE SHOULD NOT BE CONSIDERED TO BE A COMMERCIAL PRODUCT.
 * THE DEVELOPER URGES THAT USERS WHO REQUIRE A COMMERCIAL PRODUCT
 * NOT MAKE USE OF THIS WORK.
 *
 * FOR USERS WHO WISH TO UNDERSTAND THE 386BSD SYSTEM DEVELOPED
 * BY WILLIAM F. JOLITZ, WE RECOMMEND THE USER STUDY WRITTEN
 * REFERENCES SUCH AS THE  "PORTING UNIX TO THE 386" SERIES
 * (BEGINNING JANUARY 1991 "DR. DOBBS JOURNAL", USA AND BEGINNING
 * JUNE 1991 "UNIX MAGAZIN", GERMANY) BY WILLIAM F. JOLITZ AND
 * LYNNE GREER JOLITZ, AS WELL AS OTHER BOOKS ON UNIX AND THE
 * ON-LINE 386BSD USER MANUAL BEFORE USE. A BOOK DISCUSSING THE INTERNALS
 * OF 386BSD ENTITLED "386BSD FROM THE INSIDE OUT" WILL BE AVAILABLE LATE 1992.
 *
 * THIS SOFTWARE IS PROVIDED BY THE DEVELOPER ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE DEVELOPER BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Device Driver for AT style parallel printer port
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: lpt.c,v 1.75 2008/06/10 22:53:08 cegger Exp $");

#include <sys/poll.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/syslog.h>
#include <sys/intr.h>
#include <sys/select.h>

#include <sys/bus.h>

#include <dev/ic/lptreg.h>
#include <dev/ic/lptvar.h>
#include <dev/ic/lptioc.h>

#define	TIMEOUT		hz*16	/* wait up to 16 seconds for a ready */
#define	STEP		hz/4

#define	LPTPRI		(PZERO+8)
#define	LPT_BSIZE	1024

#define LPTDEBUG

#ifndef LPTDEBUG
#define LPRINTF(a)
#else
#define LPRINTF(a)	if (lptdebug) printf a
int lptdebug = 0;
#endif

extern struct cfdriver lpt_cd;

#define	LPTUNIT(s)	(minor(s) & 0x1f)
#define	LPTFLAGS(s)	(minor(s) & ~0x1f)

static void	lptsoftintr(void *);

static struct lpt_softc *rawpp = 0;

static callout_t ticker_co;
static volatile int ticker_started = 0;

void
lpt_attach_subr(sc)
	struct lpt_softc *sc;
{
	bus_space_tag_t iot;
	bus_space_handle_t ioh;

	sc->sc_state = 0;

	iot = sc->sc_iot;
	ioh = sc->sc_ioh;

	bus_space_write_1(iot, ioh, lpt_control, LPC_NINIT);

	callout_init(&sc->sc_wakeup_ch, 0);
	sc->sc_sih = softint_establish(SOFTINT_SERIAL, lptsoftintr, sc);

	sc->sc_dev_ok = 1;

 selinit(&sc->rsel);
}

int
lpt_detach_subr(device_t self, int flags)
{
	struct lpt_softc *sc = device_private(self);

 seldestroy(&sc->rsel);
	sc->sc_dev_ok = 0;
	softint_disestablish(sc->sc_sih);
	callout_destroy(&sc->sc_wakeup_ch);
	return 0;
}

#define D2A_RING_SIZE 1048576
#define D2A_RING_ADV(x) (((x) ? : D2A_RING_SIZE) - 1)
static volatile unsigned char d2a_b[D2A_RING_SIZE];
static volatile unsigned int d2a_h;
static volatile unsigned int d2a_t;
static volatile int d2a_enb;
static kmutex_t d2a_mtx;

static void ticker(void *arg __attribute__((__unused__)))
{
#if 1
 int b;
 unsigned int h;
 unsigned int t;

 if (rawpp)
  { mutex_enter(&d2a_mtx);
    if (d2a_enb)
     { h = d2a_h;
       t = d2a_t;
       if (h == t)
	{ b = -1;
	}
       else
	{ b = d2a_b[t];
	  t = D2A_RING_ADV(t);
	  d2a_t = t;
	}
     }
    else
     { d2a_t = d2a_h;
       b = -1;
     }
    if (b >= 0) bus_space_write_1(rawpp->sc_iot,rawpp->sc_ioh,lpt_data,b);
    mutex_exit(&d2a_mtx);
//    if (b >= 0) printf("S%02x",b); else printf("S-");
  }
#else
 static unsigned char v = 0;

 if (rawpp)
  { bus_space_write_1(rawpp->sc_iot,rawpp->sc_ioh,lpt_data,v);
    v ++;
  }
#endif
 callout_schedule(&ticker_co,1);
}

static void d2a_enqueue(unsigned char c)
{
 unsigned int h;

 mutex_enter(&d2a_mtx);
 h = d2a_h;
 d2a_b[h] = c;
 d2a_h = D2A_RING_ADV(h);
 mutex_exit(&d2a_mtx);
// printf("Q%02x",c);
}

static void d2a_set_enable(int enb)
{
 mutex_enter(&d2a_mtx);
 if (rawpp)
  { unsigned char ctl;
    ctl = bus_space_read_1(rawpp->sc_iot,rawpp->sc_ioh,lpt_control);
    if (enb)
     { ctl &= ~LPC_INPUT;
       bus_space_write_1(rawpp->sc_iot,rawpp->sc_ioh,lpt_control,ctl);
       bus_space_write_1(rawpp->sc_iot,rawpp->sc_ioh,lpt_data,127);
       ctl &= ~LPC_NINIT;
     }
    else
     { ctl |= LPC_NINIT;
     }
    bus_space_write_1(rawpp->sc_iot,rawpp->sc_ioh,lpt_control,ctl);
  }
 if (enb)
  { d2a_enb = 1;
  }
 else
  { d2a_enb = 0;
    d2a_h = 0;
    d2a_t = 0;
  }
 mutex_exit(&d2a_mtx);
// printf(enb?"E":"D");
}

/*
 * We assume we cannot collide with ourselves.  Because of how we're
 *  called, this amounts to saying that lptopen() is single-threaded.
 *  Currently, this is safe, because this driver runs giantlocked.
 */
static void maybe_start_ticker(void)
{
 if (ticker_started) return;
 callout_init(&ticker_co,0);
 callout_reset(&ticker_co,1,&ticker,0);
 d2a_enb = 0;
 d2a_h = 0;
 d2a_t = 0;
 mutex_init(&d2a_mtx,MUTEX_DEFAULT,IPL_HIGH);
 ticker_started = 1;
}

/*
 * Reset the printer, then wait until it's selected and not busy.
 *
 * The manipulations of sc_state and flags have the potential to
 *  collide unpleasantly with other such, here and elsewhere.  We don't
 *  worry about that because this driver runs giantlocked.  This will
 *  need fixing if/when we want to make this thing MPSAFE.
 */
static int lptopen(dev_t dev, int flag, int mode, struct lwp *l)
{
	unsigned int flags = LPTFLAGS(dev);
	struct lpt_softc *sc;
	bus_space_tag_t iot;
	bus_space_handle_t ioh;
	u_char control;
	int error;
	int spin;

	sc = device_lookup_private(&lpt_cd, LPTUNIT(dev));
	if (!sc || !sc->sc_dev_ok)
		return ENXIO;

 // Can't combine raw with any other qualifiers.
 if ((flags & LPT_RAWPP) && (flags & (LPT_AUTOLF|LPT_NOPRIME|LPT_NOINTR)))
  { return(EINVAL);
  }
 // Can't open if already open and existing open and new open disagree on rawness.
 if ( (sc->sc_state & LPT_OPEN) &&
      ( ((flags & LPT_RAWPP) && !(sc->sc_state & LPT_RAW)) ||
	(!(flags & LPT_RAWPP) && (sc->sc_state & LPT_RAW)) ) )
  { return(EBUSY);
  }
 // For raw opens, no further tests apply.
 if (flags & LPT_RAWPP)
  { if (! (sc->sc_state & LPT_OPEN))
     { sc->sc_state = LPT_OPEN | LPT_RAW;
       sc->laststat = LPT_NO_STAT;
     }
    maybe_start_ticker();
    rawpp = sc;
    return(0);
  }

#if 0	/* XXX what to do? */
	if (sc->sc_irq == IRQUNK && (flags & LPT_NOINTR) == 0)
		return ENXIO;
#endif

#ifdef DIAGNOSTIC
	if (sc->sc_state)
		aprint_verbose_dev(sc->sc_dev, "stat=0x%x not zero\n",
		    sc->sc_state);
#endif

	if (sc->sc_state)
		return EBUSY;

	sc->sc_state = LPT_INIT | (flags & LPT_RAWPP);
	sc->sc_flags = flags;
	LPRINTF(("%s: open: flags=0x%x\n", device_xname(sc->sc_dev),
	    (unsigned)flags));
	iot = sc->sc_iot;
	ioh = sc->sc_ioh;

	if ((flags & LPT_NOPRIME) == 0) {
		/* assert INIT for 100 usec to start up printer */
		bus_space_write_1(iot, ioh, lpt_control, LPC_SELECT);
		delay(100);
	}

	control = LPC_SELECT | LPC_NINIT;
	bus_space_write_1(iot, ioh, lpt_control, control);

	/* wait till ready (printer running diagnostics) */
	for (spin = 0; NOT_READY_ERR(); spin += STEP) {
		if (spin >= TIMEOUT) {
			sc->sc_state = 0;
			return EBUSY;
		}

		/* wait 1/4 second, give up if we get a signal */
		error = tsleep((void *)sc, LPTPRI | PCATCH, "lptopen", STEP);
		if (error != EWOULDBLOCK) {
			sc->sc_state = 0;
			return error;
		}
	}

	if ((flags & LPT_NOINTR) == 0)
		control |= LPC_IENABLE;
	if (flags & LPT_AUTOLF)
		control |= LPC_AUTOLF;
	sc->sc_control = control;
	bus_space_write_1(iot, ioh, lpt_control, control);

	sc->sc_inbuf = malloc(LPT_BSIZE, M_DEVBUF, M_WAITOK);
	sc->sc_count = 0;
	sc->sc_state = LPT_OPEN;

	if ((sc->sc_flags & LPT_NOINTR) == 0)
		lptwakeup(sc);

	LPRINTF(("%s: opened\n", device_xname(sc->sc_dev)));
	return 0;
}

int
lptnotready(status, sc)
	u_char status;
	struct lpt_softc *sc;
{
	u_char new;

	status = (status ^ LPS_INVERT) & LPS_MASK;
	new = status & ~sc->sc_laststatus;
	sc->sc_laststatus = status;

	if (sc->sc_state & LPT_OPEN) {
		if (new & LPS_SELECT)
			log(LOG_NOTICE,
			    "%s: offline\n", device_xname(sc->sc_dev));
		else if (new & LPS_NOPAPER)
			log(LOG_NOTICE,
			    "%s: out of paper\n", device_xname(sc->sc_dev));
		else if (new & LPS_NERR)
			log(LOG_NOTICE,
			    "%s: output error\n", device_xname(sc->sc_dev));
	}

	return status;
}

void
lptwakeup(arg)
	void *arg;
{
	struct lpt_softc *sc = arg;
	int s;

	s = spllpt();
	lptintr(sc);
	splx(s);

	callout_reset(&sc->sc_wakeup_ch, STEP, lptwakeup, sc);
}

/*
 * Close the device, and free the local line buffer.
 *
 * callout_halt() is undocumented; I learnt about it from tech-kern.
 */
static int lptclose(dev_t dev, int flag, int mode,
    struct lwp *l)
{
	struct lpt_softc *sc =
	    device_lookup_private(&lpt_cd, LPTUNIT(dev));
	bus_space_tag_t iot = sc->sc_iot;
	bus_space_handle_t ioh = sc->sc_ioh;

 // Not much to do on last close of a raw open.
 if (sc->sc_state & LPT_RAW)
  { if (rawpp == sc) rawpp = 0;
    sc->sc_state = 0;
    return(0);
  }

	if (sc->sc_count)
		(void) lptpushbytes(sc);

	if ((sc->sc_flags & LPT_NOINTR) == 0)
		callout_stop(&sc->sc_wakeup_ch);

	bus_space_write_1(iot, ioh, lpt_control, LPC_NINIT);
	sc->sc_state = 0;
	bus_space_write_1(iot, ioh, lpt_control, LPC_NINIT);
	free(sc->sc_inbuf, M_DEVBUF);

	LPRINTF(("%s: closed\n", device_xname(sc->sc_dev)));
	return 0;
}

int
lptpushbytes(sc)
	struct lpt_softc *sc;
{
	bus_space_tag_t iot = sc->sc_iot;
	bus_space_handle_t ioh = sc->sc_ioh;
	int error;

	if (sc->sc_flags & LPT_NOINTR) {
		int spin, tic;
		u_char control = sc->sc_control;

		while (sc->sc_count > 0) {
			spin = 0;
			while (NOT_READY()) {
				if (++spin < sc->sc_spinmax)
					continue;
				tic = 0;
				/* adapt busy-wait algorithm */
				sc->sc_spinmax++;
				while (NOT_READY_ERR()) {
					/* exponential backoff */
					tic = tic + tic + 1;
					if (tic > TIMEOUT)
						tic = TIMEOUT;
					error = tsleep((void *)sc,
					    LPTPRI | PCATCH, "lptpsh", tic);
					if (error != EWOULDBLOCK)
						return error;
				}
				break;
			}

			bus_space_write_1(iot, ioh, lpt_data, *sc->sc_cp++);
			DELAY(1);
			bus_space_write_1(iot, ioh, lpt_control,
			    control | LPC_STROBE);
			DELAY(1);
			sc->sc_count--;
			bus_space_write_1(iot, ioh, lpt_control, control);
			DELAY(1);

			/* adapt busy-wait algorithm */
			if (spin*2 + 16 < sc->sc_spinmax)
				sc->sc_spinmax--;
		}
	} else {
		int s;

		while (sc->sc_count > 0) {
			/* if the printer is ready for a char, give it one */
			if ((sc->sc_state & LPT_OBUSY) == 0) {
				LPRINTF(("%s: write %lu\n",
				    device_xname(sc->sc_dev),
				    (u_long)sc->sc_count));
				s = spllpt();
				(void) lptintr(sc);
				splx(s);
			}
			error = tsleep((void *)sc, LPTPRI | PCATCH,
			    "lptwrite2", 0);
			if (error)
				return error;
		}
	}
	return 0;
}

/*
 * No reading.
 *
 * ENODEV isn't really right here, but it's what the old code did for
 *  reads (it used (dev_type_read((*)))enodev).
 */
static int lptread(dev_t dev, struct uio *uio, int flags)
{
 (void)dev;
 (void)uio;
 (void)flags;
 return(ENODEV);
}

/*
 * Flexible generic I/O.
 *
 * XXX Should we be using ioctl_copyin and ioctl_copyout?  My
 *  impression is that they matter iff the ioctl driving this is called
 *  from within the kernel; for now, I'm willing to ignore that
 *  possibility.  (Doing that would mean passing the flag value from
 *  ioctl down to us.)
 */
static int lpt_doio(struct lpt_softc *sc, struct lpt_io *io)
{
 char iostate;
 unsigned char ctl;
 unsigned char cbuf[256];
 int nc;
 unsigned char wbuf[256];
 int nw;
 unsigned char rbuf[256];
 int nr;
 int e;
 int s;
 int i;

 iostate = '?';
 nc = io->clen;
 if (nc > 256)
  { printf("%s: nc = %d is too large\n",__func__,nc);
    return(EMSGSIZE); // needs translation
  }
 e = copyin(io->ctl,&cbuf[0],nc);
 if (e) return(e);
 nw = 0;
 nr = 0;
 for (i=0;i<nc;i++)
  { switch (cbuf[i])
     { case LPTCTL_SETDO:
	  iostate = 'o';
	  break;
       case LPTCTL_SETDI:
	  iostate = 'i';
	  break;
       case LPTCTL_WDATA:
	  if (iostate != 'o')
	   { printf("%s: WRITE without SET_O\n",__func__);
	     return(EIO); // needs translation
	   }
	  nw ++;
	  break;
       case LPTCTL_RDATA:
	  if (iostate != 'i')
	   { printf("%s: READ without SET_I\n",__func__);
	     return(EIO); // needs translation
	   }
	  nr ++;
	  break;
       case LPTCTL_WCTL:
	  nw ++;
	  break;
       case LPTCTL_RSTAT:
	  nr ++;
	  break;
       default:
	   { printf("%s: bad ctl %02x\n",__func__,(unsigned char)cbuf[i]);
	     return(EIO); // needs translation
	   }
	  break;
     }
  }
 if ((nw > io->wlen) || (nr > io->rlen))
  { printf("%s: nw %d wlen %u, nr %d rlen %u\n",__func__,nw,io->wlen,nr,io->rlen);
    return(EMSGSIZE); // needs translation
  }
 if (nw > 0)
  { e = copyin(io->wdata,&wbuf[0],nw);
    if (e) return(e);
  }
 /*
  * We really do mean splhigh() here, even on MP machines.  The point
  *  is not to lock against other processors, but to ensure we don't
  *  take an interrupt on this CPU during this sequence.
  *
  * Locking against other processors fiddling the hardware is something
  *  we ignore.  We can get away with this for now because this driver
  *  runs giantlocked; if/when this is to be made MPSAFE, this will
  *  need attention.
  */
 s = splhigh();
 ctl = bus_space_read_1(sc->sc_iot,sc->sc_ioh,lpt_control);
 nw = 0;
 nr = 0;
 for (i=0;i<nc;i++)
  { switch (cbuf[i])
     { case LPTCTL_SETDO:
	  ctl &= ~LPC_INPUT;
	  bus_space_write_1(sc->sc_iot,sc->sc_ioh,lpt_control,ctl);
	  break;
       case LPTCTL_SETDI:
	  ctl |= LPC_INPUT;
	  bus_space_write_1(sc->sc_iot,sc->sc_ioh,lpt_control,ctl);
	  break;
       case LPTCTL_WDATA:
	  bus_space_write_1(sc->sc_iot,sc->sc_ioh,lpt_data,wbuf[nw++]);
	  break;
       case LPTCTL_RDATA:
	  rbuf[nr++] = bus_space_read_1(sc->sc_iot,sc->sc_ioh,lpt_data);
	  break;
       case LPTCTL_WCTL:
	  ctl = (ctl & ~0x0f) | (wbuf[nw++] & 0x0f);
	  bus_space_write_1(sc->sc_iot,sc->sc_ioh,lpt_control,ctl);
	  break;
       case LPTCTL_RSTAT:
	  rbuf[nr++] = bus_space_read_1(sc->sc_iot,sc->sc_ioh,lpt_status) & 0xf8;
	  break;
     }
  }
 splx(s);
 if (nr > 0)
  { e = copyout(&rbuf[0],io->rdata,nr);
    if (e) return(e);
  }
 io->wlen = nw;
 io->rlen = nr;
 return(0);
}

/*
 * Copy a line from user space to a local buffer, then call putc to get the
 * chars moved to the output queue.
 */
static int lptwrite(dev_t dev, struct uio *uio, int flags)
{
	struct lpt_softc *sc =
	    device_lookup_private(&lpt_cd, LPTUNIT(dev));
	size_t n;
	int error = 0;

 if (sc->sc_state & LPT_RAW) return(EOPNOTSUPP);
	while ((n = min(LPT_BSIZE, uio->uio_resid)) != 0) {
		uiomove(sc->sc_cp = sc->sc_inbuf, n, uio);
		sc->sc_count = n;
		error = lptpushbytes(sc);
		if (error) {
			/*
			 * Return accurate residual if interrupted or timed
			 * out.
			 */
			uio->uio_resid += sc->sc_count;
			sc->sc_count = 0;
			return error;
		}
	}
	return 0;
}

/*
 * Handle printer interrupts which occur when the printer is ready to accept
 * another char.
 */
int lptintr(arg)
	void *arg;
{
	struct lpt_softc *sc = arg;
	bus_space_tag_t iot = sc->sc_iot;
	bus_space_handle_t ioh = sc->sc_ioh;

#if 0
	if ((sc->sc_state & LPT_OPEN) == 0)
		return 0;
#endif

	/* is printer online and ready for output */
	if (NOT_READY() && NOT_READY_ERR())
		return 0;

	if (sc->sc_count) {
		u_char control = sc->sc_control;
		/* send char */
		bus_space_write_1(iot, ioh, lpt_data, *sc->sc_cp++);
		DELAY(1);
		bus_space_write_1(iot, ioh, lpt_control, control | LPC_STROBE);
		DELAY(1);
		sc->sc_count--;
		bus_space_write_1(iot, ioh, lpt_control, control);
		DELAY(1);
		sc->sc_state |= LPT_OBUSY;
	} else
		sc->sc_state &= ~LPT_OBUSY;

	if (sc->sc_count == 0) {
		/* none, wake up the top half to get more */
		softint_schedule(sc->sc_sih);
	}

	return 1;
}

static void lptsoftintr(void *cookie)
{

	wakeup(cookie);
}

/*
 * ENODEV if not open raw is arguably wrong, but it's what the old
 *  driver did.
 *
 * The manipulations of sc_state here have the potential to collide
 *  with others doing likewise.  We don't worry about it because this
 *  driver runs giantlocked.  If/when we want to make this MPSAFE, this
 *  will need fixing.
 *
 * We cv_broadcast when turning NBIO on, but not off, because turning
 *  it on calls for waking up anyone sleeping, but when NBIO is on,
 *  there is never anyone sleeping and thus nobody to wake up.  (If
 *  NBIO was already set the way it's being "changed" to here, nothing
 *  needs doing in either case, but the cv_broadcast is harmless.)
 */
static int lptioctl(dev_t dev, u_long cmd, void *data,
    int flag, struct lwp *l)
{
 struct lpt_softc *sc;

 sc = device_lookup_private(&lpt_cd,LPTUNIT(dev));
 if (! sc) panic("lptioctl: nonexistent lpt");
 if ((sc->sc_state & (LPT_OPEN|LPT_RAW)) != (LPT_OPEN|LPT_RAW)) return(ENODEV);
 switch (cmd)
  { case FIONBIO:
       return(0);
       break;
    case LPTIOC_IO:
       return(lpt_doio(sc,data));
       break;
    case LPTIOC_ENABLE_D2A:
       d2a_set_enable(1);
       break;
    case LPTIOC_SEND_D2A:
       if (d2a_enb) d2a_enqueue(*(const unsigned char *)data);
       break;
    case LPTIOC_DISABLE_D2A:
       d2a_set_enable(0);
       break;
  }
 return(EINVAL);
}

static int lptpoll(dev_t dev, int events, struct lwp *l)
{
 struct lpt_softc *sc;
 volatile struct lpt_softc *vsc;
 int rev;

 sc = device_lookup_private(&lpt_cd,LPTUNIT(dev));
 if (! sc) panic("lptpoll: nonexistent lpt");
 vsc = sc;
 if ((sc->sc_state & (LPT_OPEN|LPT_RAW)) != (LPT_OPEN|LPT_RAW)) return(seltrue(dev,events,l));
 rev = events & (POLLOUT | POLLWRNORM | POLLIN | POLLRDNORM);
 return(rev);
}

const struct cdevsw lpt_cdevsw = {
	lptopen, lptclose, lptread, lptwrite, lptioctl,
	nostop, notty, lptpoll, nommap, nokqfilter, D_OTHER,
};
