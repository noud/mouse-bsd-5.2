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

#define ADLINK7300A_CONF_LCR 0x14
#define ADLINK7300A_CONF_BASE 0x18

#define PLX9080_CNTRL    0x6c
#define PLX9080_CNTRL_SOFTRESET 0x40000000
#define PLX9080_LAS0RR   0x00
#define PLX9080_LAS0BA   0x04
#define PLX9080_LBRD0    0x18
#define PLX9080_INTCSR   0x68
#define PLX9080_DMACSR0  0xa8
#define PLX9080_DMACSR1  0xa9
#define PLX9080_DMAMODE0 0x80
#define PLX9080_DMAPADR0 0x84
#define PLX9080_DMALADR0 0x88
#define PLX9080_DMASIZ0  0x8c
#define PLX9080_DMADPR0  0x90
#define PLX9080_DMAMODE1 0x94
#define PLX9080_DMATHR   0xb0

#define AL7300_DI_CSR       0x00
#define AL7300_DO_CSR       0x04
#define AL7300_AUX_DIO      0x08
#define AL7300_INT_CSR      0x0c
#define AL7300_DI_FIFO      0x10
#define AL7300_DO_FIFO      0x14
#define AL7300_FIFO_CR      0x18
#define AL7300_POL_CTRL     0x1c
#define AL7300_8254_COUNT0  0x20
#define AL7300_8254_COUNT1  0x24
#define AL7300_8254_COUNT2  0x28
#define AL7300_8254_CONTROL 0x2c

#define DMABUFSIZE 0x100000

typedef struct softc SOFTC;

struct softc {
  device_t dev;
  unsigned int flags;
#define SCF_BROKEN 0x00000001
#define SCF_OPEN   0x00000002
#define SCF_SET_UP 0x00000004
  enum adlink7300a_dma_status dmastat;
  struct adlink7300a_dma dmareq;
  struct lwp *dmawho;
  pci_chipset_tag_t pc;
  pcitag_t pt;
  bus_dma_tag_t dmat;
  char *dmabuf;
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
  struct selinfo si;
  } ;

extern struct cfdriver adlink7300a_cd;

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
 volatile SOFTC *sc;
 unsigned int v;

 sc = scv;
 v = bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR);
 /*
  * 0x00200000 is DMA channel 0 interrupt; 0x00008000 is local bus
  *  interrupt.  I'm not sure what local bus interrupts correspond to,
  *  but I see the local bus interrupt enable set, so they presumably
  *  mean something.  However, I have no idea what, so, if we get such
  *  an interrupt, we just turn off the interrupt enable and let it go
  *  at that.
  */
 if (v & 0x00208000)
  { if (v & 0x00200000)
     { //aprint_normal_dev(sc->dev,"DMA interrupt\n");
       // 0x08 is write-1-to-clear for the channel DMA-done interrupt.
       // Also clears 0x10, the Done bit
       bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,0x08);
       v = bus_space_read_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR);
       // 0x400 is DI_OVER (FIFO overrun occurred during input)
       if (v & 0x400)
	{ // DI_OVER is the only write-1-to-clear bit, so...
	  aprint_normal_dev(sc->dev,"input overrun\n");
	  bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,v);
	}
       switch (sc->dmastat)
	{ case ADLINK7300A_DMA_PENDING:
	     sc->dmastat = ADLINK7300A_DMA_DONE;
	     break;
	  case ADLINK7300A_DMA_FLUSH:
	     bus_dmamap_sync(sc->dmat,sc->dmam,0,sc->dmareq.count*4,BUS_DMASYNC_POSTREAD);
	     bus_dmamap_unload(sc->dmat,sc->dmam);
	     sc->dmastat = ADLINK7300A_DMA_NONE;
	     break;
	  case ADLINK7300A_DMA_NONE:
	     aprint_normal_dev(sc->dev,"warning: DMA interrupt with no DMA in progress\n");
	     break;
	  case ADLINK7300A_DMA_DONE:
	     aprint_normal_dev(sc->dev,"warning: DMA interrupt after DMA done\n");
	     break;
	  default:
	     panic("adlink7300a: impossible DMA state %d\n",(int)sc->dmastat);
	     break;
	}
       // XXX what's the right way to de-volatile here?
       selnotify(&((SOFTC *)scv)->si,0,0);
     }
    if (v & 0x00008000)
     { aprint_normal_dev(sc->dev,"local bus interrupt\n");
       // 0x800 is PCI local interrupt enable.
       bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR,bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR)&~0x800);
     }
    return(1);
  }
 return(0);
}

/*
 * All bit meanings here come from the 9080 doc.
 */
static void adlink7300a_reset(volatile SOFTC *sc)
{
 uint32_t r;

 /*
  * Initialize 9080 registers.
  */
 /* Tell the 9080 to soft reset */
 /*
  * CNTRL, CoNTRoL of various things.
  *
  *	Bits		R/W	Meaning
  *	80000000	RW	Local init done.
  *	40000000	RW	PCI adapter soft reset.
  *	20000000	RW	Reload local config from EEPROM.
  *	10000000	R	1 iff serial EEPROM present.
  *	08000000	R	Serial EEPROM read data bit.
  *	04000000	RW	Serial EEPROM write data bit.
  *	02000000	RW	Serial EEPROM chip select.
  *	01000000	RW	Serial EEPROM clock.
  *	00fc0000	R	Reserved.
  *	00020000	R	USERI pin input value.
  *	00010000	RW	USERO pin output value.
  *	0000f000	RW	PCI memory write command code.
  *	00000f00	RW	PCI memory read command code.
  *	000000f0	RW	PCI DMA write command code.
  *	0000000f	RW	PCI DMA read command code.
  */
 r = bus_space_read_4(sc->lcr_t,sc->lcr_h,0x6c);
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_CNTRL,r|PLX9080_CNTRL_SOFTRESET);
 DELAY(10); // is this actually needed?
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_CNTRL,r&~PLX9080_CNTRL_SOFTRESET);
 DELAY(10); // is this actually needed?
 /*
  * LAS0RR, Local Address Space 0 Range Register for PCI to Local
  *
  * 0xffffffc1 means: occupy 0x40 bytes (decode the 0xffffffc0 address
  *  bits), 0x08 bit set means prefetchable, 0x00 bit set means I/O
  *  space (as opposed to memory space), and, in I/O space, 0x04 bit is
  *  part of decoding mask and 0x02 bit "must be set to 0".  The 7300
  *  doc describes only 0x30 bytes of registers, but, because the
  *  paradigm is to specify what address bits to decode, the space
  *  occupied must be a power of two.  (The 9080 doc says that the
  *  range "must be a power of 2", meaning that the bits-to-decode must
  *  be a single contiguous set of bits at the high end of the value.
  *  It doesn't give any indication what might happen if not.)
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_LAS0RR,0xffffffc1);
 /*
  * LAS0BA, Local Address Space 0 Local Base Address (Remap) Register
  *
  * The 0x01 bit set enables access, the 0x02 bit is reserved, and the
  *  rest of the bits replace the PCI address bits, meaning we're
  *  accessing local addresses based at zero.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_LAS0BA,0x00000001);
 /*
  * MARBR, Mode/Arbitration Register
  * BIGEND, Big/Little Endian Descriptor Register
  * EROMRR, Expansion ROM Range Register
  * EROMBA, Expansion ROM Local Base Address Register
  *
  * The on-reset values are suitable for these.
  */
 /*
  * LBRD0, Local Address Space 0/Expansion ROM Bus Region Descriptor
  *  Register
  *
  *	Bits		Meaning
  *	f0000000	Specify PCI target retry delay.
  *	08000000	When FIFO full, disconnect (if set to 0) or
  *			  deassert TRDY# (if set to 1).
  *	04000000	ROM space burst enable (on local bus).
  *	02000000	1 means load subsystem ID and local address
  *			  space 1 registers from EEPROM; 0, don't.
  *	01000000	Memory space 0 burst enable (on local bus).
  *	00800000	ROM space BTERM# input enable.
  *	00400000	ROM space Ready input enable.
  *	003c0000	ROM space internal wait state count.
  *	00030000	ROM space local bus width.
  *	00008000	Reserved.
  *	00007800	Prefetch count during memory reads.
  *	00000400	Prefetch count enable.
  *	00000200	ROM space prefetch disable.
  *	00000100	Memory space 0 prefetch disable.
  *	00000080	Memory space 0 BTERM# input enable.
  *	00000040	Memory space 0 Ready input enable.
  *	0000003c	Memory space 0 internal wait states.
  *	00000003	Memory space 0 local bus width.
  *
  *  Bus widths are specified as 00 = 8 bit, 01 = 16 bit, 10 or 11 = 32
  *  bit.
  *
  * So, 4b0003c3 means: retry delay 4, FIFO full deassert TRDY#, ROM
  *  space burst disabled, load subsystem ID and local address space 1
  *  from EEPROM, memory space 0 local bursting enabled, ROM space
  *  BTERM# and Ready inputs disabled with no wait states and 8-bit bus
  *  width, memory prefetch continues until terminted by PCI bus, ROM
  *  space and memory space 0 prefetching disabled, memory space 0
  *  BTERM# and Ready inputs enabled with no wait states and 32-bit bus
  *  width.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_LBRD0,0x4b0003c3);
 /*
  * DMRR, Local Range Register for Direct Master to PCI
  * DMLBAM, Local Bus Base Address Register for Direct Master to PCI
  *  Memory
  * DMLBAI, Local Base Address Register for Direct Master to PCI IO/CFG
  * DMPBAM, PCI Base Address (Remap) Register for Direct Master to PCI
  *  Memory
  * DMCFGA, PCI Configuration Address Register for Direct Master to PCI
  *  IO/CFG
  * LAS1RR, Local Address Space 1 Ranger Register for PCI to Local Bus
  * LAS1BA, Local Address Space 1 Local Base Address (Remap) Register
  * LBRD1, Local Address Space 1 Bus Region Descriptor Register
  *
  * MBOX0, Mailbox Register 0
  * MBOX1, Mailbox Register 1
  * MBOX2, Mailbox Register 2
  * MBOX3, Mailbox Register 3
  * MBOX4, Mailbox Register 4
  * MBOX5, Mailbox Register 5
  * MBOX6, Mailbox Register 6
  * MBOX7, Mailbox Register 7
  * P2LDBELL, PCI to Local Doorbell Register
  * L2PDBELL, Local to PCI Doorbell Register
  *
  * The on-reset values are suitable for these.
  */
 /*
  * Interrupt control/status.
  *
  *	Bits		R/W	Meaning
  *	80000000	R	PCI wrote data to mailbox #3.
  *	40000000	R	PCI wrote data to mailbox #2.
  *	20000000	R	PCI wrote data to mailbox #1.
  *	10000000	R	PCI wrote data to mailbox #0.
  *	0f000000	R	Abort status indication (see doc).
  *	00800000	R	BIST interrupt active.
  *	00400000	R	DMA channel 1 interrupt active.
  *	00200000	R	DMA channel 0 interrupt active.
  *	00100000	R	Local doorbell interrupt active.
  *	00080000	RW	Local DMA channel 1 interrupt enable.
  *	00040000	RW	Local DMA channel 0 interrupt enable.
  *	00020000	RW	Local doorbell interrupt enable.
  *	00010000	RW	Local interrupt output enable.
  *	00008000	R	Local interrupt (LINTi#) is active.
  *	00004000	R	PCI abort interrupt is active.
  *	00002000	R	PCI doorbell interrupt is active.
  *	00001000	RW	Retry abort enable ("diagnostic only").
  *	00000800	RW	PCI local interrupt enable.
  *	00000400	RW	PCI abort interrupt enable.
  *	00000200	RW	PCI doorbell interrupt enable.
  *	00000100	RW	PCI interrupt enable.
  *	000000f0	R	Reserved.
  *	00000008	RW	Mailbox interrupt enable.
  *	00000004	RW	Generate PCI SERR#.
  *	00000002	RW	Enable local LSERR# on PCI parity
  *				  error.
  *	00000001	RW	Enable local bus LSERR# on PCI target
  *				  abort or master abort.
  *
  * So, 00000900 means: disable most stuff, enable PCI local
  *  interrupts, meaning that a local bus interrupt produces a PCI
  *  interrupt.  I also see 00040000 turned on, but we don't want any
  *  DMA interrupts yet 'cause we're not doing DMA yet.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR,0x00000900);
 /*
  * See above for the bits in CNTRL.  9000767c means (ignoring
  *  read-only bits): local init done, both writes use command 7,
  *  direct master reads use command 6 (do we ever do direct master
  *  reads?), DMA reads use command c.  I'm not sure what these
  *  "command"s are; presumably if I knew more about PCI they would
  *  make sense to me.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_CNTRL,0x9000767c);
 /*
  * PCIHIDR, PCI Permanent Configuration ID Register
  * PCIHREV, PCI Permanent Revision ID Register
  *
  * The on-reset values are suitable for these.
  */
 /*
  * DMACSR0, DMA Channel 0 Command/Status Register
  * DMACSR1, DMA Channel 1 Command/Status Register
  *
  *	Bits	R/W	Meaning
  *	e0	R	Reserved.
  *	10	R	Channel 0/1 transfer done.
  *	08	W/C	Writing 1 clears channel 0/1 interrupts.
  *	04	W/S	Writing 1 aborts transfer in progress.
  *	02	W/S	Writing 1 starts transfer.
  *	01	RW	Channel 0/1 enable.
  *
  * We set these now, out of numerical order, to disable the channels
  *  before we start meddling with their configuration.
  */
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,0x00);
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR1,0x00);
 /*
  * DMAMODE0, DMA Channel 0 Mode Register
  *
  *	Bits		R/W	Meaning
  *	fffc0000	R	Reserved.
  *	00020000	RW	Channel 0 interrupt select (0 for local
  *				  bus interrupt, 1 for PCI interrupt).
  *	00010000	RW	Clear count mode.
  *	00008000	RW	DMA stop data mode.  0 sends BLAST; 1
  *				  asserts EOT or negates DREQ[1:0].
  *	00004000	RW	DMA EOT input pin enable.
  *	00002000	RW	Write-and-invalidate mode for DMA.
  *	00001000	RW	Demand mode.
  *	00000800	RW	Local addressing mode.
  *	00000400	RW	Done interrupt enable.
  *	00000200	RW	Chaining enable.
  *	00000100	RW	Local-bus bursting enable.
  *	00000080	RW	BTERM# input enable.
  *	00000040	RW	Ready input enable.
  *	0000003c	RW	Internal wait states.
  *	00000003	RW	Local bus width (see LBRD0 doc above).
  *
  *  00021dc2 means: PCI interrupts, no clear count, BLAST, no EOT
  *  input, no write-and-invalidate, demand mode, local-bus address is
  *  constant, done interrupts enabled, no chaining, local bursting
  *  enabled, BTERM# and Ready inputs enabled, no internal wait states,
  *  32-bit local bus width.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMAMODE0,0x00021dc2);
 /*
  * DMAPADR0, DMA Channel 0 PCI Address Register
  * DMALADR0, DMA Channel 0 Local Address Register
  * DMASIZ0, DMA Channel 0 Transfer Size (Bytes) Register
  * DMADPR0, DMA Channel 0 Descriptor Pointer Register
  *
  * These we don't set now since we aren't doing DMA now.
  */
 /*
  * DMAMODE1, DMA Channel 1 mode Register.
  *
  * Bits are just like DMAMODE0, above, except that of course they
  *  apply to channel 1 instead of channel 0.
  *
  *  00021d81 means: PCI interrupts, no clear count, BLAST, no EOT
  *  input, no write-and-invalidate, demand mode, local-bus address is
  *  constant, done interrupts enabled, no chaining, local bursting
  *  eabled, BTERM# input enabled, Ready input disabled, no internal
  *  wait states, 16-bit local bus width.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMAMODE1,0x00021d81);
 /*
  * DMAPADR1, DMA Channel 1 PCI Address Register
  * DMALADR1, DMA Channel 1 Local Address Register
  * DMASIZ1, DMA Channel 1 Transfer Size (Bytes) Register
  * DMADPR1, DMA Channel 1 Descriptor Pointer Register
  *
  * These we don't set now since we aren't doing DMA now.
  */
 /*
  * DMACSR0, DMA Channel 0 Command/Status Register
  * DMACSR1, DMA Channel 1 Command/Status Register
  *
  * These are set, above, out of numerical order.
  */
 /*
  * DMAARB, DMA Arbitration Register - same as MARBR, above.
  */
 /*
  * DMATHR, DMA Threshold Register
  *
  *	Bits		R/W	Meaning
  *	f0000000	RW	Channel 1 PCI->local almost empty.
  *	0f000000	RW	Channel 1 local->PCI almost full.
  *	00f00000	RW	Channel 1 local->PCI almost empty.
  *	000f0000	RW	Channel 1 PCI->local almost full.
  *	0000f000	RW	Channel 0 PCI->local almost empty.
  *	00000f00	RW	Channel 0 local->PCI almost full.
  *	000000f0	RW	Channel 0 local->PCI almost empty.
  *	0000000f	RW	Channel 0 PCI->local almost full.
  *
  *  Each nybble is "Number of <x> entries (divided by two, minus one)
  *  in FIFO" before requesting the relevant bus in the relevant
  *  direction.  For example, "PCI->local almost full" is the number of
  *  entries before requesting the local bus for writes.  The peculiar
  *  ordering makes more sense if you look at it as
  *
  *	f000	Entries before requesting PCI for reads.
  *	0f00	Entries before requesting PCI for writes.
  *	00f0	Entries before requesting local bus for reads.
  *	000f	Entries before requesting local bus for writes.
  *
  *  There are "should" criteria applying to sums of various fields;
  *  see the doc.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMATHR,0x44447a47);
 /*
  * Is this actually necessary?
  */
 pci_conf_write(sc->pc,sc->pt,PCI_BHLC_REG,pci_conf_read(sc->pc,sc->pt,PCI_BHLC_REG)|(PCI_LATTIMER_MASK<<PCI_LATTIMER_SHIFT));
}

static void adlink7300a_setup(volatile SOFTC *sc)
{
 /*
  * Set DMA mode.  All we do here is set 32-bit bus width.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMAMODE0,(bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_DMAMODE0)&~3U)|2);
 /*
  * Set almost-empty and almost-full values.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_FIFO_CR,0x00080008);
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_FIFO_CR,0x00800080);
 /*
  * The table calls this register POL_CTRL; the text calls it
  *  POL_CNTRL.  All the _NEG bits are called _NEG in the table and
  *  _NEQ in the text, too.  Adlink _really_ needs to proofread their
  *  manual.
  *
  * 0 configures everything to be rising edge active, which is what we
  *  want here.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_POL_CTRL,0);
 /*
  * Clear D[IO]_UNDER (the diagram shows DO_UNDER, the text says
  *  DI_UNDER), set DO_FIFO_CLR, clear DO_EN, terminator off.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DO_CSR,0x00000640);
 /*
  * clear DI_OVER, set DI_FIFO_CLR and DI_EN, terminator off, start
  *  sampling now, no handshaking, external clock, 32-bit width.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,0x00000747);
}

static int start_dma(volatile SOFTC *sc, const struct adlink7300a_dma *req, struct lwp *l)
{
 int e;

 if ((req->count < 1) || (req->count > DMABUFSIZE/4)) return(EMSGSIZE); // needs translation
 switch (sc->dmastat)
  { default:
       panic("impossible dmastat %d",(int)sc->dmastat);
       break;
    case ADLINK7300A_DMA_PENDING:
    case ADLINK7300A_DMA_DONE:
    case ADLINK7300A_DMA_FLUSH:
       break;
    case ADLINK7300A_DMA_NONE:
       e = bus_dmamap_load(sc->dmat,sc->dmam,sc->dmabuf,req->count*4,0,BUS_DMA_WAITOK|BUS_DMA_READ);
       if (e)
	{ printf("adlink7300a: DMA map load failed %d\n",e);
	  return(EIO);
	}
       break;
  }
 sc->dmareq = *req;
 bus_dmamap_sync(sc->dmat,sc->dmam,0,req->count*4,BUS_DMASYNC_PREREAD);
 /* 0x200 is DI_FIFO_CLR */
 if (req->flags & ADLINK7300A_DMAF_FLUSH) bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,bus_space_read_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR)|0x200);
 /* DMAMODE0 is good from init */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMAPADR0,sc->dmam->dm_segs->ds_addr);
 /*
  * Why 0x10 here?  That's what I find there, and, given the value in
  *  DMAMODE0, this is held constant....
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMALADR0,0x10);
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMASIZ0,req->count*4);
 /*
  * 0xb is (0x8) local->PCI, (0x4) no interrupt, (0x2) no chain, (0x1)
  *  the descriptor is in PCI space (except we're not chaining, so
  *  `descriptor' is not a useful term).
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_DMADPR0,0xb);
 /*
  * Enable DMA completion interrupt.  0x00040000 is DMA 0 enable.
  */
 bus_space_write_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR,bus_space_read_4(sc->lcr_t,sc->lcr_h,PLX9080_INTCSR)|0x00040000);
 /*
  * Set it going!  0x01 = enable, 0x02 = start.
  */
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,1);
 bus_space_write_1(sc->lcr_t,sc->lcr_h,PLX9080_DMACSR0,3);
 /*
  * 0x100 is DI_EN.  0x3ff masks off status bits which are "write 1 to
  *  do something" but we don't want to do anything to now.
  */
 bus_space_write_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR,(bus_space_read_4(sc->regs_t,sc->regs_h,AL7300_DI_CSR)&0x3ff)|0x100);
 sc->dmastat = ADLINK7300A_DMA_PENDING;
 return(0);
}

static void dma_status(volatile SOFTC *sc, enum adlink7300a_dma_status *stp)
{
 enum adlink7300a_dma_status stat;

 stat = sc->dmastat;
 switch (stat)
  { default:
       aprint_normal_dev(sc->dev,"DMA status unknown (%d)\n",(int)stat);
       stat = ADLINK7300A_DMA_NONE;
       /* fall through */
    case ADLINK7300A_DMA_NONE:
    case ADLINK7300A_DMA_PENDING:
       *stp = sc->dmastat;
    case ADLINK7300A_DMA_DONE:
       *stp = sc->dmastat;
       bus_dmamap_sync(sc->dmat,sc->dmam,0,sc->dmareq.count*4,BUS_DMASYNC_POSTREAD);
       bus_dmamap_unload(sc->dmat,sc->dmam);
       copyout(sc->dmabuf,sc->dmareq.buf,sc->dmareq.count*4);
       *stp = ADLINK7300A_DMA_DONE;
       sc->dmastat = ADLINK7300A_DMA_NONE;
       break;
    case ADLINK7300A_DMA_FLUSH:
       *stp = ADLINK7300A_DMA_PENDING;
       break;
  }
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
 if (pci_mapreg_map(pa,ADLINK7300A_CONF_LCR,PCI_MAPREG_TYPE_IO,0,&sc->lcr_t,&sc->lcr_h,&sc->lcr_a,&sc->lcr_s))
  { aprint_error_dev(sc->dev,"can't map LCR registers\n");
    sc->flags = SCF_BROKEN;
    return;
  }
 if (pci_mapreg_map(pa,ADLINK7300A_CONF_BASE,PCI_MAPREG_TYPE_IO,0,&sc->regs_t,&sc->regs_h,&sc->regs_a,&sc->regs_s))
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
 e = bus_dmamem_alloc(sc->dmat,DMABUFSIZE,1,0x40000000,&dmaseg,1,&ns,BUS_DMA_WAITOK|BUS_DMA_STREAMING);
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
 e = bus_dmamem_map(sc->dmat,&dmaseg,1,DMABUFSIZE,&dmamapped,BUS_DMA_WAITOK);
 if (e)
  { aprint_error_dev(sc->dev,"can't map DMA memory (%d)\n",e);
    sc->flags = SCF_BROKEN;
    return;
  }
 sc->dmabuf = dmamapped;
 e = bus_dmamap_create(sc->dmat,DMABUFSIZE,1,DMABUFSIZE,0,BUS_DMA_WAITOK,&sc->dmam);
 if (e)
  { aprint_error_dev(sc->dev,"can't create DMA map (%d)\n",e);
    sc->flags = SCF_BROKEN;
    return;
  }
 sc->flags = 0;
 selinit(&sc->si);
}

CFATTACH_DECL_NEW(adlink7300a,sizeof(SOFTC),adlink7300a_match,adlink7300a_attach,0,0);

static int adlink7300a_open(dev_t dev, int flags, int mode, struct lwp *l)
{
 int u;
 volatile SOFTC *sc;
 int s;

 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc)
  { printf("adlink7300a_open, unit %d: nonexistent\n",u);
    return(ENXIO);
  }
 if (sc->flags & SCF_BROKEN)
  { printf("adlink7300a_open, unit %d: broken\n",u);
    return(ENXIO);
  }
 s = splhigh();
 if (sc->flags & SCF_OPEN)
  { splx(s);
    printf("adlink7300a_open, unit %d: already open\n",u);
    return(0);
  }
 if (! (sc->flags & SCF_SET_UP))
  { adlink7300a_setup(sc);
    sc->flags |= SCF_SET_UP;
    sc->dmastat = ADLINK7300A_DMA_NONE;
  }
 sc->flags |= SCF_OPEN;
 splx(s);
 printf("adlink7300a_open, unit %d: succeeded\n",u);
 return(0);
}

static int adlink7300a_close(dev_t dev, int flags, int mode, struct lwp *l)
{
 int u;
 SOFTC *sc;
 int s;

 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc) panic("closing nonexistent 7300\n");
 if (sc->flags & SCF_BROKEN) panic("closing broken 7300\n");
 s = splhigh();
 if (sc->dmastat == ADLINK7300A_DMA_PENDING) sc->dmastat = ADLINK7300A_DMA_FLUSH;
 sc->flags &= ~SCF_OPEN;
 splx(s);
 printf("adlink7300a_close, unit %d\n",u);
 return(0);
}

static int adlink7300a_read(dev_t dev, struct uio *uio, int flags)
{
 printf("adlink7300a_read: reading not supported\n");
 return(EOPNOTSUPP);
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
 int e;

 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc) panic("ioctl on nonexistent 7300\n");
 if (sc->flags & SCF_BROKEN) panic("ioctl on broken 7300\n");
 switch (ioc)
  { case ADLINK7300A_DO_AUX:
	{ unsigned char v;
	  v = *(unsigned char *)addr;
	  //aprint_normal_dev(sc->dev,"DO_AUX %d%d%d%d\n",(v>>3)&1,(v>>2)&1,(v>>1)&1,v&1);
	  s = splhigh();
	  bus_space_write_1(sc->regs_t,sc->regs_h,AL7300_AUX_DIO,v);
	  splx(s);
	}
       break;
    case ADLINK7300A_DI_AUX:
	{ unsigned char v;
	  s = splhigh();
	  v = (bus_space_read_1(sc->regs_t,sc->regs_h,AL7300_AUX_DIO) >> 4) & 15;
	  splx(s);
	  //aprint_normal_dev(sc->dev,"DI_AUX %d%d%d%d\n",(v>>3)&1,(v>>2)&1,(v>>1)&1,v&1);
	  *(unsigned char *)addr = v;
	}
       break;
    case ADLINK7300A_DMA_START:
       //aprint_normal_dev(sc->dev,"DMA_START buf %p count %d flags %x\n",(void *)((const struct adlink7300a_dma *)addr)->buf,((const struct adlink7300a_dma *)addr)->count,((const struct adlink7300a_dma *)addr)->flags);
       s = splhigh();
       e = start_dma(sc,(const struct adlink7300a_dma *)addr,l);
       splx(s);
       return(e);
       break;
    case ADLINK7300A_DMA_STATUS:
       s = splhigh();
       dma_status(sc,(enum adlink7300a_dma_status *)addr);
       splx(s);
       break;
    default:
       return(EINVAL);
       break;
  }
 return(0);
}

static int adlink7300a_poll(dev_t dev, int events, struct lwp *l)
{
 int u;
 SOFTC *sc;
 int s;

 u = minor(dev);
 sc = device_lookup_private(&adlink7300a_cd,u);
 if (! sc) panic("poll on nonexistent 7300\n");
 if (sc->flags & SCF_BROKEN) panic("poll on broken 7300\n");
 aprint_normal_dev(sc->dev,"poll\n");
 s = splhigh();
 switch (sc->dmastat)
  { default:
       panic("impossible dmastat");
       break;
    case ADLINK7300A_DMA_NONE:
    case ADLINK7300A_DMA_PENDING:
       selrecord(l,&sc->si);
       splx(s);
       return(0);
       break;
    case ADLINK7300A_DMA_DONE:
       splx(s);
       return(events);
       break;
  }
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
