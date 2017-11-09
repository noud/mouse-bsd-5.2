/*
 * Driver for the Adlink 7300A digital I/O board.
 *
 * Adlink's documentation is woefully incomplete in some respects.  For
 *  example, they say they use the PLX-9080 for PCI interfacing, but
 *  they don't say anything about what local buses they have set up and
 *  how they can/must be configured.  The 9080 config values herein
 *  were obtained by dumping out the 9080 registers as set up by
 *  Adlink's library routines, cross-referenced with the 9080
 *  documentation, and healthy doses of guessing and experimentation.
 *
 * The hardware is fairly badly crippled.  It does not appear to have
 *  any "how far has this transfer progressed?" operation, in
 *  particular, which limits the APIs we can provide.  Instead, we wire
 *  in yet more knowledge of what this is being used for and detect how
 *  far DMA has progressed by figuring out how much of the buffer has
 *  been overwritten (by filling it with a pattern and trusting that
 *  that pattern will not occur - it certainly shouldn't, since 23 of
 *  the data pins are grounded).
 *
 * We keep a large (DMABUFSIZE below, one megabyte at this writing) DMA
 *  buffer.  We break it up into smaller chunks and configure the 9080
 *  in chaining mode, with interrupts after each chunk is done, so we
 *  can - provided we stay ahead of the data stream - never let it
 *  reach end-of-ring.
 */

#include <sys/bus.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/select.h>
#include <machine/bus.h>
#include <machine/param.h>
#include <dev/pci/pcivar.h>

#include "7300a.h"
#include "7300a-reg.h"

#define DMABUFSIZE 0x1000000
#define DMABUFWORDS (DMABUFSIZE>>2)
#define FILL_PATTERN 0xa5a5a5a5
#define NBUFFRAGS 32

#define CHUNKWORDS (((DMABUFSIZE/NBUFFRAGS)-16)>>2)
#define CHUNKBYTES (CHUNKWORDS << 2)
#define BUFWORDS (CHUNKWORDS * NBUFFRAGS)
#define BUFBYTES (BUFWORDS << 2)
#if CHUNKBYTES >= (1 << 23)
#error "Chunk size too large"
#endif

typedef struct softc SOFTC;

/*
 * dsamp is the index of the next sample to be DMA-filled (which may,
 *  and often will, lag the samples actually filled by the hardware).
 *  rsamp is the index of the next sample to be returned by read().
 *
 * In the DMA interrupt routine, we advance dsamp until it encounters a
 *  not-yet-overwritten sample, or it encounters rsamp.  If the former,
 *  we then advance rsamp if necessary so there are at least two frags'
 *  worth of space between it and dsamp.  If the latter, DMA has not
 *  only overrun the reader but has overrun the interrupt routine; in
 *  this case, we have no idea where it's currently DMAing into, so we
 *  kinda have to reset the hardware and restart from ground zero.
 *
 * Overruns - either hardware-reported or dsamp running into rsamp -
 *  are not reflected to the read() interface, since userland doesn't
 *  care.  They _are_ reported to the console, though, so we can get
 *  some idea how common they are.
 */
struct softc {
  device_t dev;
  unsigned int flags;
#define SCF_BROKEN  0x00000001
#define SCF_OPEN    0x00000002
  pci_chipset_tag_t pc;
  pcitag_t pt;
  bus_dma_tag_t dmat;
  char *dmamem;
  bus_addr_t dmapci;
  bus_dmamap_t dmam;
  bus_space_tag_t lcr_t;
  bus_space_handle_t lcr_h;
  bus_addr_t lcr_a;
  bus_size_t lcr_s;
  bus_space_tag_t regs_t;
  bus_space_handle_t regs_h;
  bus_addr_t regs_a;
  bus_size_t regs_s;
  pci_intr_handle_t intr_h;
  void *intr_est;
  volatile uint32_t *bufbase;
  volatile int rsamp;		// samples
  volatile int dsamp;		// samples
  } ;

extern struct cfdriver adlink7300a_cd;

#define HRINGSIZE 1024
volatile int hring_7300[HRINGSIZE];
volatile int hring_7300_h;
volatile int hring_7300_t;
#define HRING_RECORD() hring_record(__LINE__)

static void hring_init(void)
{
 hring_7300_h = 0;
 hring_7300_t = 0;
}

static void hring_record(int v)
{
 int s;
 int h;
 int t;

 s = splhigh();
 h = hring_7300_h;
 t = hring_7300_t;
 hring_7300[h] = v;
 h ++;
 if (h >= HRINGSIZE) h = 0;
 hring_7300_h = h;
 if (h == t)
  { t ++;
    if (t >= HRINGSIZE) t = 0;
    hring_7300_t = t;
  }
 splx(s);
}

/*
LOC 00e8	INTCSR, PCI 068
LOC 0100	DMAMODE0, PCI 080
LOC e110	not found
	110	DMADPR0

ch0 done: bit [10] of register LOC 100
ch0 terminal: bit [2] of register LOC e110
either DMA 0: bit [18] of register LOC e8 and bit [17] of register LOC 100
if bit [17] is 0 gen LINTo#; if bit [17] is 1 gen INTA#.
read Interrupt Control/Status Register to determine whether DMA channel interrupt is pending.
Done status bit in that register can determine which kind of interrupt it is.
write 1 to Clear Interrupt bit in DMA Command/Status Register to clear interrupt
*/

static int start_dma(SOFTC *sc)
{
 int s;

// printf("D");
 HRING_RECORD();
 s = splhigh();
 // DMAMODE0 is good from init
 if (sc->dmapci & ~PLX9080_DMADPRx_NEXT) panic("misaligned descriptor");
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMADPR0,
	(sc->dmapci + BUFBYTES) | PLX9080_DMADPRx_SPACE_PCI | PLX9080_DMADPRx_IRQ | PLX9080_DMADPRx_DIR_L2P);
 // Enable DMA completion interrupt.
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR,bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR)|PLX9080_INTCSR_LCL_CH0_IE);
 // Set it going!
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,PLX9080_DMACSRx_ENB);
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,PLX9080_DMACSRx_ENB|PLX9080_DMACSRx_GO);
 // Enable digital input - but mask off "write 1 to do something" bits.
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,
	(bus_space_read_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR)&~AL7300_DI_CSR_OVERRUN)|AL7300_DI_CSR_ENABLE);
 splx(s);
 HRING_RECORD();
 return(0);
}

static void adlink7300a_reset(SOFTC *sc)
{
 uint32_t r;

 HRING_RECORD();
 /*
  * Initialize 9080 registers.
  */
 /* Tell the 9080 to soft reset */
 r = bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_CNTRL);
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_CNTRL,r|PLX9080_CNTRL_SOFTRESET);
 DELAY(10); // is this actually needed?
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_CNTRL,r&~PLX9080_CNTRL_SOFTRESET);
 DELAY(10); // is this actually needed?
 // LAS0RR, Local Address Space 0 Range Register for PCI to Local
 // We know, here, that the 7300 occupies 0x40 bytes of space.
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_LAS0RR,(-(uint32_t)0x40)|PLX9080_LASxRR_SPACE_IO);
 // LAS0BA, Local Address Space 0 Local Base Address (Remap) Register
 // Apparently, local address 0 is the right thing here.
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_LAS0BA,PLX9080_LASxBA_ENABLE|0);
 // The on-reset values are suitable for:
 //	MARBR, Mode/Arbitration Register
 //	BIGEND, Big/Little Endian Descriptor Register
 //	EROMRR, Expansion ROM Range Register
 //	EROMBA, Expansion ROM Local Base Address Register
 // LBRD0, Local Address Space 0/Expansion ROM Bus Region Descriptor
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_LBRD0,
	PLX9080_LBRD0_MEM0SIZE_32x |
	(0 << PLX9080_LBRD0_MEM0WAIT_S) |
	PLX9080_LBRD0_MS0READY |
	PLX9080_LBRD0_MS0BTERM |
	PLX9080_LBRD0_MS0PFDIS |
	PLX9080_LBRD0_ROMPFDIS |
	PLX9080_LBRD0_ROMSIZE_8 |
	(0 << PLX9080_LBRD0_ROMWAIT_S) |
	PLX9080_LBRD0_MS0BURST |
	PLX9080_LBRD0_XLONG_LD |
	PLX9080_LBRD0_DS_WMODE_TRDY |
	(4 << PLX9080_LBRD0_RETRYCLK_S) );
 // The on-reset values are suitable for:
 //	DMRR, Local Range Register for Direct Master to PCI
 //	DMLBAM, Local Bus Base Address Register for Direct Master to PCI Memory
 //	DMLBAI, Local Base Address Register for Direct Master to PCI IO/CFG
 //	DMPBAM, PCI Base Address (Remap) Register for Direct Master to PCI Memory
 //	DMCFGA, PCI Configuration Address Register for Direct Master to PCI IO/CFG
 //	LAS1RR, Local Address Space 1 Ranger Register for PCI to Local Bus
 //	LAS1BA, Local Address Space 1 Local Base Address (Remap) Register
 //	LBRD1, Local Address Space 1 Bus Region Descriptor Register
 //	MBOX0, Mailbox Register 0
 //	MBOX1, Mailbox Register 1
 //	MBOX2, Mailbox Register 2
 //	MBOX3, Mailbox Register 3
 //	MBOX4, Mailbox Register 4
 //	MBOX5, Mailbox Register 5
 //	MBOX6, Mailbox Register 6
 //	MBOX7, Mailbox Register 7
 //	P2LDBELL, PCI to Local Doorbell Register
 //	L2PDBELL, Local to PCI Doorbell Register
 // INTCSR, interrupt control/status
 // I also see PLX9080_INTCSR_LCL_CH0_IE set, but we don't want any DMA
 // interrupts yet 'cause we're not doing DMA yet.
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR,PLX9080_INTCSR_PCI_LCL_IE|PLX9080_INTCSR_PCI_IE);
 // CNTRL, EEPROM/PCI/user/init control
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_CNTRL,
	(12 << PLX9080_CNTRL_DMA_R_CMD_S) |
	(7 << PLX9080_CNTRL_DMA_W_CMD_S) |
	(6 << PLX9080_CNTRL_DM_R_CMD_S) |
	(7 << PLX9080_CNTRL_DM_W_CMD_S) |
	PLX9080_CNTRL_INITDONE);
 // The on-reset values are suitable for:
 //	PCIHIDR, PCI Permanent Configuration ID Register
 //	PCIHREV, PCI Permanent Revision ID Register
 // DMACSR[01], DMA Channel [01] Command/Status Register
 // We set these now, out of numerical order, to disable the channels
 // before we start meddling with their configuration.  Clear their
 // interrupts just in case.
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,0);
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,PLX9080_DMACSRx_IACK);
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR1,0);
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR1,PLX9080_DMACSRx_IACK);
 // DMAMODE0, DMA Channel 0 Mode Register
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMAMODE0,
	PLX9080_DMAMODEx_LB_WIDTH_32 |
	(0 << PLX9080_DMAMODEx_WAITS_S) |
	PLX9080_DMAMODEx_READY |
	PLX9080_DMAMODEx_BTERM |
	PLX9080_DMAMODEx_LBURST |
	PLX9080_DMAMODEx_CHAIN |
	PLX9080_DMAMODEx_DONE_IE |
	PLX9080_DMAMODEx_LCLCONST |
	PLX9080_DMAMODEx_DEMAND |
	PLX9080_DMAMODEx_DMA_STOP_BLAST |
	PLX9080_DMAMODEx_CHx_INT_PCI);
 // We don't set these now since we aren't doing DMA yet.
 //	DMAPADR0, DMA Channel 0 PCI Address Register
 //	DMALADR0, DMA Channel 0 Local Address Register
 //	DMASIZ0, DMA Channel 0 Transfer Size (Bytes) Register
 //	DMADPR0, DMA Channel 0 Descriptor Pointer Register
 // DMAMODE1, DMA Channel 1 Mode Register
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMAMODE1,
	PLX9080_DMAMODEx_LB_WIDTH_16 |
	(0 << PLX9080_DMAMODEx_WAITS_S) |
	PLX9080_DMAMODEx_BTERM |
	PLX9080_DMAMODEx_LBURST |
	PLX9080_DMAMODEx_DONE_IE |
	PLX9080_DMAMODEx_LCLCONST |
	PLX9080_DMAMODEx_DEMAND |
	PLX9080_DMAMODEx_DMA_STOP_BLAST |
	PLX9080_DMAMODEx_CHx_INT_PCI);
 // We don't set these now since we aren't doing DMA yet.
 //	DMAPADR1, DMA Channel 1 PCI Address Register
 //	DMALADR1, DMA Channel 1 Local Address Register
 //	DMASIZ1, DMA Channel 1 Transfer Size (Bytes) Register
 //	DMADPR1, DMA Channel 1 Descriptor Pointer Register
 // We set these above, out of numerical order; see above for why.
 //	DMACSR0, DMA Channel 0 Command/Status Register
 //	DMACSR1, DMA Channel 1 Command/Status Register
 // DMAARB, DMA Arbitration Register - same register as MARBR, above.
 // DMATHR, DMA Threshold Register
 // I suspect some of these never get used because we don't output.
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMATHR,
	( 7 << PLX9080_DMATHR_C0PLAF_S) |
	( 7 << PLX9080_DMATHR_C0PLAE_S) |
	( 4 << PLX9080_DMATHR_C0LPAE_S) |
	(10 << PLX9080_DMATHR_C0LPAF_S) |
	( 4 << PLX9080_DMATHR_C1PLAF_S) |
	( 4 << PLX9080_DMATHR_C1PLAE_S) |
	( 4 << PLX9080_DMATHR_C1LPAE_S) |
	( 4 << PLX9080_DMATHR_C1LPAF_S) );
 // Is this actually necessary?
 pci_conf_write(sc->pc,sc->pt,PCI_BHLC_REG,pci_conf_read(sc->pc,sc->pt,PCI_BHLC_REG)|(PCI_LATTIMER_MASK<<PCI_LATTIMER_SHIFT));
 HRING_RECORD();
}

static void adlink7300a_setup(SOFTC *sc)
{
 HRING_RECORD();
 // DMAMODE0 is correct as set up by _reset()
 /*
  * Set almost-empty and almost-full values.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_FIFO_CR,
	(8 << AL7300_FIFO_CR_PA_S) | (8 << AL7300_FIFO_CR_PB_S));
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_FIFO_CR,
	(128 << AL7300_FIFO_CR_PA_S) | (128 << AL7300_FIFO_CR_PB_S));
 /*
  * The table calls this register POL_CTRL; the text calls it
  *  POL_CNTRL.  All the _NEG bits are called _NEG in the table and
  *  _NEQ in the text, too.  Adlink _really_ needs to proofread their
  *  manual.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_POL_CTRL,
	AL7300_POL_CTRL_DI_REQ_RISING |
	AL7300_POL_CTRL_DI_ACK_RISING |
	AL7300_POL_CTRL_DI_TRIG_RISING |
	AL7300_POL_CTRL_DO_REQ_RISING |
	AL7300_POL_CTRL_DO_ACK_RISING |
	AL7300_POL_CTRL_DO_TRIG_RISING );
 /*
  * Clear D[IO]_UNDER (the diagram shows DO_UNDER, the text says
  *  DI_UNDER), set DO_FIFO_CLR, clear DO_EN, terminator off.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DO_CSR,
	AL7300_DO_CSR_CLK_TIMER1 |
	AL7300_DO_CSR_B_TERM_OFF |
	AL7300_DO_CSR_CLR_FIFO |
	AL7300_DO_CSR_UNDERRUN );
 /*
  * Clear DI_OVER, set DI_FIFO_CLR and DI_EN, terminator off, start
  *  sampling now, no handshaking, external clock, 32-bit width.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,
	AL7300_DI_CSR_DI_32 |
	AL7300_DI_CSR_CLK_EXT |
	AL7300_DI_CSR_A_TERM_OFF |
	AL7300_DI_CSR_ENABLE |
	AL7300_DI_CSR_CLR_FIFO |
	AL7300_DI_CSR_OVERRUN );
 HRING_RECORD();
}

static void fill_buffer(volatile uint32_t *buf, int nwords)
{
 for (;nwords>0;nwords--) *buf++ = FILL_PATTERN;
}

/*
 * Advance dsamp until it either (a) finds an unmodified value or (b)
 *  runs into rsamp.  If the former, advance rsamp if necessary and
 *  return 0; if the latter, return 1 without bothering to update the
 *  SOFTC (since it'll be thrown out anyway).
 */
static int dma_catchup(SOFTC *sc)
{
 int ds;
 int rs;
 int s;
 uint32_t val;
 int room;

 s = splhigh();
 ds = sc->dsamp;
 rs = sc->rsamp;
 splx(s);
 while (1)
  { bus_dmamap_sync(sc->dmat,sc->dmam,ds*4,4,BUS_DMASYNC_POSTREAD);
    val = sc->bufbase[ds];
    if (val == FILL_PATTERN)
     { sc->dsamp = ds;
       if (rs == ds) return(0);
       room = (rs > ds) ? rs - ds : (rs + DMABUFWORDS - ds);
       if (room < 2*CHUNKWORDS)
	{ rs = ds + (2 * CHUNKWORDS);
	  if (rs >= BUFWORDS) rs -= BUFWORDS;
	  sc->rsamp = rs;
	  // XXX Report this to the console?
	}
       return(0);
     }
    ds ++;
    if (ds >= BUFWORDS) ds = 0;
    if (ds == rs) return(1);
  }
}

static void flush_fifo(SOFTC *sc)
{
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,bus_space_read_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR)|AL7300_DI_CSR_CLR_FIFO);
}

static void restart_dma(SOFTC *sc)
{
// printf("R");
 HRING_RECORD();
 adlink7300a_setup(sc);
 HRING_RECORD();
 flush_fifo(sc);
 HRING_RECORD();
 fill_buffer((void *)sc->dmamem,BUFWORDS);
 HRING_RECORD();
 bus_dmamap_sync(sc->dmat,sc->dmam,0,DMABUFSIZE,BUS_DMASYNC_PREWRITE);
 HRING_RECORD();
 sc->rsamp = 0;
 sc->dsamp = 0;
 start_dma(sc);
 HRING_RECORD();
}

/*
 * pci_intr(9) does not describe the semantics of the return value.
 *  Going under the hood, it appears a return value of 1 means the
 *  routine is claiming the interrupt and 0 that it is disclaiming it.
 *  I do not know what the semantics of other values are; even this
 *  much is a guess.  We _do_ seem to get called sometimes when
 *  unrelated interrupts occur (since PCI has only four interrupt
 *  lines, interrupt conflicts are inevitable on any PCI bus with more
 *  than four interrupt-generating devices).
 */
static int adlink7300a_intr(void *scv)
{
 SOFTC *sc;
 unsigned int icsr;
 unsigned int v;

 HRING_RECORD();
 sc = scv;
 icsr = bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR);
 /*
  * I'm not sure what local bus interrupts correspond to, but I see the
  *  local bus interrupt enable set, so they presumably mean something.
  *  However, I have no idea what, so, if we get such an interrupt, we
  *  just turn off the interrupt enable and let it go at that (and log
  *  it, so we can find out it happened).
  */
 if (icsr & (PLX9080_INTCSR_LCL_CH0_IRQ|PLX9080_INTCSR_PCI_LCL_IRQ))
  { if (icsr & PLX9080_INTCSR_LCL_CH0_IRQ)
     { //aprint_normal_dev(sc->dev,"DMA interrupt\n");
       HRING_RECORD();
       // ACK the interrupt - and clear the DONE bit, if set.
       bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,PLX9080_DMACSRx_ENB|PLX9080_DMACSRx_IACK);
       v = bus_space_read_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR);
       if (v & AL7300_DI_CSR_OVERRUN)
	{ //report_overrun(sc);
	  // AL7300_DI_CSR_OVERRUN is the only write-1-to-clear bit...
	  bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,v);
	}
       if (dma_catchup(sc)) restart_dma(sc);
     }
    if (icsr & PLX9080_INTCSR_PCI_LCL_IRQ)
     { HRING_RECORD();
       aprint_normal_dev(sc->dev,"local bus interrupt\n");
       bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR,bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR)&~PLX9080_INTCSR_PCI_LCL_IE);
     }
    return(1);
  }
 HRING_RECORD();
 return(0);
}

static void adlink7300a_flush(SOFTC *sc)
{
 int s;
 int rs;
 int ds;
 int loops;
 uint32_t val;

 HRING_RECORD();
 s = splhigh();
 rs = sc->rsamp;
 ds = sc->dsamp;
 while (rs != ds)
  { sc->bufbase[rs] = FILL_PATTERN;
    bus_dmamap_sync(sc->dmat,sc->dmam,rs<<2,4,BUS_DMASYNC_PREWRITE);
    rs ++;
    if (rs >= BUFWORDS) rs = 0;
  }
 loops = BUFWORDS + 1;
 while (1)
  { bus_dmamap_sync(sc->dmat,sc->dmam,rs<<2,4,BUS_DMASYNC_POSTREAD);
    val = sc->bufbase[rs];
    if (val == FILL_PATTERN) break;
    sc->bufbase[rs] = FILL_PATTERN;
    rs ++;
    if (rs >= BUFWORDS) rs = 0;
    if (loops-- < 1)
     { HRING_RECORD();
       restart_dma(sc);
       splx(s);
       HRING_RECORD();
       return;
     }
  }
 sc->rsamp = rs;
 sc->dsamp = rs;
 splx(s);
 HRING_RECORD();
}

/*
 * 10b5 and 9080 are the generic PLX-9080 values, so we check the
 *  subsystem ID as well to avoid false positives.
 */
static int adlink7300a_match(device_t parent, cfdata_t cf, void *aux)
{
 struct pci_attach_args *pa;
 pcireg_t r;

 pa = aux;
 if ( (PCI_VENDOR(pa->pa_id) != 0x10b5) ||
      (PCI_PRODUCT(pa->pa_id) != 0x9080) ) return(0);
 r = pci_conf_read(pa->pa_pc,pa->pa_tag,PCI_SUBSYS_ID_REG);
 return(r==0x7300144a);
}

static void setup_buf(SOFTC *sc)
{
 int i;
 volatile uint32_t *desc;
 volatile char *descbase;

 /*
  * Each frag requires a 16-byte descriptor, so the data space
  *  available in each one is (DMABUFSIZE/NBUFFRAGS)-16 bytes.  We put
  *  all the descriptors at the end of the buffer.
  *
  * XXX This code assumes each descriptor is an integral number of
  *  samples long.  Since both the descriptor size and the sample size
  *  are fixed by the hardware (16 and 4), this is probably safe and
  *  doesn't need a #if/#error/#endif test.
  */
 descbase = (void *) (sc->dmamem + BUFBYTES);
 sc->bufbase = (void *) sc->dmamem;
 for (i=NBUFFRAGS-1;i>=0;i--)
  { desc = (volatile void *) (descbase + (16 * i));
    // Starting DMA address
    desc[0] = sc->dmapci + (CHUNKWORDS * 4 * i);
    // Why 0x10?  Because that's what I found in DMALADR0, and,
    // given DMAMODE0, it's constant....
    desc[1] = 0x10;
    // This value is in bytes
    desc[2] = CHUNKBYTES;
    // Not set: PLX9080_DMADPRx_END, end-of-chain
    desc[3] = (sc->dmapci + BUFBYTES + (((i+1)%NBUFFRAGS) * 16) ) |
		PLX9080_DMADPRx_DIR_L2P |
		PLX9080_DMADPRx_IRQ |
		PLX9080_DMADPRx_SPACE_PCI;
  }
}

static void adlink7300a_attach(device_t parent, device_t self, void *aux)
{
 SOFTC *sc;
 struct pci_attach_args *pa;
 pcireg_t r;
 int e;
 int ns;
 bus_dma_segment_t dmaseg;
 void *dmamapped;
 char devinfo[256];

 hring_init();
 HRING_RECORD();
 sc = device_private(self);
 pa = aux;
 sc->dev = self;
 sc->pc = pa->pa_pc;
 sc->pt = pa->pa_tag;
 sc->dmat = pci_dma64_available(pa) ? pa->pa_dmat64 : pa->pa_dmat;
 pci_devinfo(pa->pa_id,pa->pa_class,0,&devinfo[0],sizeof(devinfo));
 aprint_normal(": %s (rev. 0x%02x)\n",&devinfo[0],PCI_REVISION(pa->pa_class));
 if (PCI_REVISION(pa->pa_class) != 0x0b)
  { aprint_normal_dev(sc->dev,"unsupported revision\n");
    sc->flags = SCF_BROKEN;
    return;
  }
 if (pci_mapreg_map(pa,AL7300_CONF_LCR,PCI_MAPREG_TYPE_IO,0,&sc->lcr_t,&sc->lcr_h,&sc->lcr_a,&sc->lcr_s))
  { aprint_error_dev(sc->dev,"can't map LCR registers\n");
    sc->flags = SCF_BROKEN;
    return;
  }
 if (pci_mapreg_map(pa,AL7300_CONF_BASE,PCI_MAPREG_TYPE_IO,0,&sc->regs_t,&sc->regs_h,&sc->regs_a,&sc->regs_s))
  { aprint_error_dev(sc->dev,"can't map board registers\n");
    sc->flags = SCF_BROKEN;
    return;
  }
 r = pci_conf_read(sc->pc,sc->pt,PCI_COMMAND_STATUS_REG);
 if (! (r & PCI_COMMAND_MASTER_ENABLE))
  { aprint_error_dev(sc->dev,"BIOS didn't enable bus mastering\n");
    sc->flags = SCF_BROKEN;
    return;
  }
 adlink7300a_reset(sc);
 if (pci_intr_map(pa,&sc->intr_h))
  { aprint_error_dev(sc->dev,"can't map interrupt\n");
    sc->flags = SCF_BROKEN;
    return;
  }
 aprint_normal_dev(sc->dev,"interrupting at %s\n",pci_intr_string(sc->pc,sc->intr_h));
 sc->intr_est = pci_intr_establish(sc->pc,sc->intr_h,IPL_BIO,&adlink7300a_intr,sc);
 e = bus_dmamem_alloc(sc->dmat,DMABUFSIZE,4,0x40000000,&dmaseg,1,&ns,BUS_DMA_WAITOK|BUS_DMA_STREAMING);
 if (e)
  { aprint_error_dev(sc->dev,"can't allocate DMA memory (%d)\n",e);
    sc->flags = SCF_BROKEN;
    return;
  }
 if (ns != 1)
  { aprint_error_dev(sc->dev,"bus_dmamem_alloc returned #-segs %d\n",ns);
    sc->flags = SCF_BROKEN;
    return;
  }
 e = bus_dmamem_map(sc->dmat,&dmaseg,1,DMABUFSIZE,&dmamapped,BUS_DMA_WAITOK|BUS_DMA_COHERENT);
 if (e)
  { aprint_error_dev(sc->dev,"can't map DMA memory (%d)\n",e);
    sc->flags = SCF_BROKEN;
    return;
  }
 sc->dmamem = dmamapped;
 e = bus_dmamap_create(sc->dmat,DMABUFSIZE,1,DMABUFSIZE,0,BUS_DMA_WAITOK,&sc->dmam);
 if (e)
  { aprint_error_dev(sc->dev,"can't create DMA map (%d)\n",e);
    sc->flags = SCF_BROKEN;
    return;
  }
 e = bus_dmamap_load(sc->dmat,sc->dmam,sc->dmamem,DMABUFSIZE,0,BUS_DMA_WAITOK);
 if (e)
  { aprint_error_dev(sc->dev,"can't create DMA map (%d)\n",e);
    sc->flags = SCF_BROKEN;
    return;
  }
 sc->dmapci = sc->dmam->dm_segs->ds_addr;
 setup_buf(sc);
 sc->flags = 0;
 HRING_RECORD();
}

CFATTACH_DECL_NEW(adlink7300a,sizeof(SOFTC),adlink7300a_match,adlink7300a_attach,0,0);

static int adlink7300a_open(dev_t dev, int flags, int mode, struct lwp *l)
{
 int u;
 SOFTC *sc;
 int s;

 HRING_RECORD();
 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc)
  { HRING_RECORD();
    //printf("adlink7300a_open, unit %d: nonexistent\n",u);
    return(ENXIO);
  }
 if (sc->flags & SCF_BROKEN)
  { HRING_RECORD();
    //printf("adlink7300a_open, unit %d: broken\n",u);
    return(ENXIO);
  }
 s = splhigh();
 if (sc->flags & SCF_OPEN)
  { splx(s);
    HRING_RECORD();
    //printf("adlink7300a_open, unit %d: already open\n",u);
    return(0);
  }
 HRING_RECORD();
 restart_dma(sc);
 sc->flags |= SCF_OPEN;
 splx(s);
 //printf("adlink7300a_open, unit %d: succeeded\n",u);
 return(0);
}

static int adlink7300a_close(dev_t dev, int flags, int mode, struct lwp *l)
{
 int u;
 SOFTC *sc;
 int s;

 HRING_RECORD();
 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc) panic("closing nonexistent 7300\n");
 if (sc->flags & SCF_BROKEN) panic("closing broken 7300\n");
 s = splhigh();
 adlink7300a_reset(sc);
 sc->flags &= ~SCF_OPEN;
 splx(s);
 //printf("adlink7300a_close, unit %d\n",u);
 HRING_RECORD();
 return(0);
}

/*
 * Advance rsamp until either (a) we satisfy the read or (b) we run
 *  into a FILL_PATTERN sample.  If we run into dsamp first, push dsamp
 *  forward too.  Reset memory to FILL_PATTERN as we go.
 *
 * XXX Should we sync(PREWRITE) only once (or twice if we wrap) after
 *  the loop, isntead of every time around?  PREWRITE involves an
 *  mfence, which is relatively expensive.  We actually care about it
 *  for only that one piece of memory, but there's no "mfence for just
 *  this little piece of memory".
 */
static int adlink7300a_read(dev_t dev, struct uio *uio, int flags)
{
 int u;
 SOFTC *sc;
 int s;
 int rs;
 int ds;
 uint32_t val;
 uint32_t v[256];
 int nv;
 int maxn;

 HRING_RECORD();
 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc) panic("read on nonexistent 7300");
 if (sc->flags & SCF_BROKEN) panic("read on broken 7300\n");
#if FILL_PATTERN == 0
 val = 1;
#else
 val = 0;
#endif
 while (1)
  { maxn = uio->uio_resid >> 2;
    if (maxn < 1)
     { //printf("B");
       break;
     }
    if (maxn > sizeof(v)/sizeof(v[0])) maxn = sizeof(v) / sizeof(v[0]);
    s = splhigh();
    rs = sc->rsamp;
    ds = sc->dsamp;
    if (maxn > BUFWORDS-rs) maxn = BUFWORDS - rs;
    nv = 0;
    while (nv < maxn)
     { bus_dmamap_sync(sc->dmat,sc->dmam,rs<<2,4,BUS_DMASYNC_POSTREAD);
       val = sc->bufbase[rs];
       if (val == FILL_PATTERN) break;
       sc->bufbase[rs] = FILL_PATTERN;
       bus_dmamap_sync(sc->dmat,sc->dmam,rs<<2,4,BUS_DMASYNC_PREWRITE);
       v[nv++] = val;
       if (rs == ds) ds ++;
       rs ++;
     }
    if (rs >= BUFWORDS) rs = 0;
    if (ds >= BUFWORDS) ds = 0;
    sc->rsamp = rs;
    sc->dsamp = ds;
    splx(s);
    if (nv > 0) uiomove(&v[0],nv<<2,uio);
    if (val == FILL_PATTERN)
     { //printf("F");
       break;
     }
  }
 return(0);
}

static int adlink7300a_write(dev_t dev, struct uio *uio, int flags)
{
 printf("adlink7300a_write: writing not supported\n");
 return(EOPNOTSUPP);
}

static int adlink7300a_ioctl(dev_t dev, u_long ioc, void *addr, int flag, struct lwp *l)
{
 int u;
 SOFTC *sc;
 int s;

 HRING_RECORD();
 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc) panic("ioctl on nonexistent 7300\n");
 if (sc->flags & SCF_BROKEN) panic("ioctl on broken 7300\n");
 switch (ioc)
  { case ADLINK7300A_DO_AUX:
       HRING_RECORD();
	{ unsigned char v;
	  v = *(unsigned char *)addr;
	  //aprint_normal_dev(sc->dev,"DO_AUX %d%d%d%d\n",(v>>3)&1,(v>>2)&1,(v>>1)&1,v&1);
	  s = splhigh();
	  bus_space_write_1(sc->regs_t,sc->regs_h,AL7300_AUX_DIO,v);
	  splx(s);
	}
       break;
    case ADLINK7300A_DI_AUX:
       HRING_RECORD();
	{ unsigned char v;
	  s = splhigh();
	  v = (bus_space_read_1(sc->regs_t,sc->regs_h,AL7300_AUX_DIO) >> 4) & 15;
	  splx(s);
	  //aprint_normal_dev(sc->dev,"DI_AUX %d%d%d%d\n",(v>>3)&1,(v>>2)&1,(v>>1)&1,v&1);
	  *(unsigned char *)addr = v;
	}
       break;
    case ADLINK7300A_FLUSH:
       adlink7300a_flush(sc);
       break;
    default:
       HRING_RECORD();
       return(EINVAL);
       break;
  }
 HRING_RECORD();
 return(0);
}

static int adlink7300a_poll(dev_t dev, int events, struct lwp *l)
{
 int u;
 SOFTC *sc;

 HRING_RECORD();
 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc) panic("poll on nonexistent 7300\n");
 if (sc->flags & SCF_BROKEN) panic("poll on broken 7300\n");
 aprint_normal_dev(sc->dev,"poll\n");
 return(events);
}

static paddr_t adlink7300a_mmap(dev_t dev, off_t off, int prot)
{
 printf("adlink7300a_mmap\n");
 return(-(paddr_t)1);
}

const struct cdevsw adlink7300a_cdevsw
 = { &adlink7300a_open,
     &adlink7300a_close,
     &adlink7300a_read,
     &adlink7300a_write,
     &adlink7300a_ioctl,
     0,
     0,
     &adlink7300a_poll,
     &adlink7300a_mmap,
     &nokqfilter,
     D_OTHER };
