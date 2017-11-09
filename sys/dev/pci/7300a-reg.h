#ifndef _7300A_REG_H_2b3cfc94_
#define _7300A_REG_H_2b3cfc94_

/*
 * Bit meanings here come from the 9080 doc (for PLX9080* values) or
 *  the 7300 book (for AL7300* values).
 */

#define AL7300_CONF_LCR 0x14
#define AL7300_CONF_BASE 0x18

// Local Address Space 0 Range Register, for PCI->local
#define PLX9080_LAS0RR 0x00
#define PLX9080_LASxRR_SPACE        0x00000001 // which space?
#define PLX9080_LASxRR_SPACE_MEM    0x00000000 // ...memory
#define PLX9080_LASxRR_SPACE_IO     0x00000001 // ...I/O
#define PLX9080_LASxRR_MEM_LOC      0x00000006 // mem: where?
#define PLX9080_LASxRR_MEM_LOC_32   0x00000000 // mem: 32-bit space
#define PLX9080_LASxRR_MEM_LOC_20   0x00000002 // mem: below 1M
#define PLX9080_LASxRR_MEM_LOC_64   0x00000004 // mem: 64-bit space
#define PLX9080_LASxRR_MEM_LOC_RSVD 0x00000006 // mem: reserved
#define PLX9080_LASxRR_MEM_PREFETCH 0x00000008 // mem: prefetch enable
#define PLX9080_LASxRR_MEM_DECODE   0xfffffff0 // mem: bits to decode
#define PLX9080_LASxRR_IO_MBZ       0x00000002 // I/O: MBZ
#define PLX9080_LASxRR_IO_DECODE    0xfffffffc // I/O: bits to decode
// Local Address Space 0 Local Base Address
#define PLX9080_LAS0BA 0x04
#define PLX9080_LASxBA_ENABLE   0x00000001 // enable PCI access to local
#define PLX9080_LASxBA_MBZ      0x00000002 // reserved
#define PLX9080_LASxBA_MEM_MBZ  0x0000000c // if mem space: MBZ
#define PLX9080_LASxBA_MEM_ADDR 0xfffffff0 // if mem space: remap bits
#define PLX9080_LASxBA_IO_ADDR  0xfffffffc // if I/O space: remap bits
// Local/DMA Arbitration (LB here means "local bus")
#define PLX9080_LARBR 0x08
#define PLX9080_LARBR_LBLAT   0x000000ff // LB latency timer
#define PLX9080_LARBR_LBPAUSE 0x0000ff00 // LB pause timer
#define PLX9080_LARBR_LBLENB  0x00010000 // LB latency timer enable
#define PLX9080_LARBR_LBPENB  0x00020000 // LB pause timer enable
#define PLX9080_LARBR_LBRQENB 0x00040000 // LB BREQ input enable
#define PLX9080_LARBR_DMAPRIO 0x00180000 // DMA channel priority
#define PLX9080_LARBR_DMAPRIO_RR  0x00000000 // "rotational" priority
#define PLX9080_LARBR_DMAPRIO_CH0 0x00080000 // channel 0 has priority
#define PLX9080_LARBR_DMAPRIO_CH1 0x00100000 // channel 1 has priority
#define PLX9080_LARBR_DMAPRIO_RSV 0x00180000 // reserved
#define PLX9080_LARBR_LBDSGU 0x00200000 // LB direct slave give-up-bus
#define PLX9080_LARBR_LLOCK  0x00400000 // Direct slave LLOCKo enable
#define PLX9080_LARBR_PCIREQ 0x00800000 // PCI request mode
#define PLX9080_LARBR_PCI_21 0x01000000 // PCI rev 2.1 mode
#define PLX9080_LARBR_PCIRNW 0x02000000 // PCI read-forbids-write
#define PLX9080_LARBR_PCIWFR 0x04000000 // PCI write-flushes-read
#define PLX9080_LARBR_LBTBRQ 0x08000000 // BREQ gates LB latency timer
#define PLX9080_LARBR_PCIRNF 0x10000000 // PCI read-no-flush
#define PLX9080_LARBR_SUBSYS 0x20000000 // PCI 00 returns subsystem IDs
#define PLX9080_LARBR_RESVD  0xc0000000 // reserved
// Big/Little Endian (set bits make the corresponding things BE)
#define PLX9080_BIGEND 0x0c
#define PLX9080_BIGEND_LCL_CONF 0x00000001 // local config registers
#define PLX9080_BIGEND_DIR_MAST 0x00000002 // direct master accesses
#define PLX9080_BIGEND_DS_LAS0  0x00000004 // direct slave, local AS 0
#define PLX9080_BIGEND_DS_ROM   0x00000008 // direct slave, ROM
#define PLX9080_BIGEND_BYTE_LN  0x00000010 // high byte lanes 8/16 bit
#define PLX9080_BIGEND_DS_LAS1  0x00000020 // direct slave, local AS 1
#define PLX9080_BIGEND_DS_DMA1  0x00000040 // DMA channel 1
#define PLX9080_BIGEND_DS_DMA0  0x00000080 // DMA channel 0
#define PLX9080_BIGEND_RESVD    0xffffff00 // reserved
// Expansion ROM Range Register
#define PLX9080_EROMRR 0x10
#define PLX9080_EROMRR_RESVD   0x000003ff // reserved
#define PLX9080_EROMRR_PCIADDR 0xfffffc00 // PCI bits to decode to ROM
// Expansion ROM Local Base Address
#define PLX9080_EROMBA 0x14
#define PLX9080_EROMBA_BACKOFF   0x0000000f // backoff clocks
#define PLX9080_EROMBA_BREQo_ENB 0x00000010 // enables BREQo
#define PLX9080_EROMBA_BREQo_64  0x00000020 // BREQo timer = 64 clocks
#define PLX9080_EROMBA_RESVD     0x000007c0 // reserved
#define PLX9080_EROMBA_LOCALADDR 0xfffff800 // remap bits
// Local Address Space 0 / Expansion ROM
#define PLX9080_LBRD0 0x18
#define PLX9080_LBRD0_MEM0SIZE 0x00000003 // memory 0 local bus width
#define PLX9080_LBRD0_MEM0SIZE_8   0x00000000 // 8 bits
#define PLX9080_LBRD0_MEM0SIZE_16  0x00000001 // 16 bits
#define PLX9080_LBRD0_MEM0SIZE_32  0x00000002 // 32 bits
#define PLX9080_LBRD0_MEM0SIZE_32x 0x00000003 // 32 bits
#define PLX9080_LBRD0_MEM0WAIT 0x0000003c // memory 0 wait states
#define PLX9080_LBRD0_MEM0WAIT_M 15
#define PLX9080_LBRD0_MEM0WAIT_S 2
#define PLX9080_LBRD0_MS0READY 0x00000040 // memory 0 ready input enb
#define PLX9080_LBRD0_MS0BTERM 0x00000080 // memory 0 BTERM input enb
#define PLX9080_LBRD0_MS0PFDIS 0x00000100 // memory 0 prefetch dis
#define PLX9080_LBRD0_ROMPFDIS 0x00000200 // ROM prefetch dis
#define PLX9080_LBRD0_PFCNTENB 0x00000400 // prefetch count enable
#define PLX9080_LBRD0_PFCOUNT  0x00007800 // prefetch count
#define PLX9080_LBRD0_PFCOUNT_M 15
#define PLX9080_LBRD0_PFCOUNT_S 11
#define PLX9080_LBRD0_RESVD    0x00008000 // reserved
#define PLX9080_LBRD0_ROMSIZE  0x00030000 // ROM local bus width
#define PLX9080_LBRD0_ROMSIZE_8   0x00000000 // 8 bits
#define PLX9080_LBRD0_ROMSIZE_16  0x00010000 // 16 bits
#define PLX9080_LBRD0_ROMSIZE_32  0x00020000 // 32 bits
#define PLX9080_LBRD0_ROMSIZE_32x 0x00030000 // 32 bits
#define PLX9080_LBRD0_ROMWAIT  0x003c0000 // ROM wait states
#define PLX9080_LBRD0_ROMWAIT_M 15
#define PLX9080_LBRD0_ROMWAIT_S 18
#define PLX9080_LBRD0_RDY_ENB  0x00400000 // ROM ready input enb
#define PLX9080_LBRD0_ROMBTERM 0x00800000 // ROM BTERM input enb
#define PLX9080_LBRD0_MS0BURST 0x01000000 // memory 0 burst enb
#define PLX9080_LBRD0_XLONG_LD 0x02000000 // extra long EEPROM load
#define PLX9080_LBRD0_ROMBURST 0x04000000 // ROM burst enb
#define PLX9080_LBRD0_DS_WMODE 0x08000000 // direct slave FIFO full:
#define PLX9080_LBRD0_DS_WMODE_DISC 0x00000000 // disconnect
#define PLX9080_LBRD0_DS_WMODE_TRDY 0x08000000 // deassert TRDY
#define PLX9080_LBRD0_RETRYCLK 0xf0000000 // PCI retry clocks
#define PLX9080_LBRD0_RETRYCLK_M 15
#define PLX9080_LBRD0_RETRYCLK_S 28
// Local Range Register for direct master, local to PCI (mem/IO/cfg)
#define PLX9080_DMRR 0x1c
#define PLX9080_DMRR_RESVD  0x0000ffff // reserved
#define PLX9080_DMRR_DECODE 0xffff0000 // bits to decode
// Local Bus Base Address for direct master, local to PCI memory
#define PLX9080_DMLBAM 0x20
#define PLX9080_DMLBAM_RESVD  0x0000ffff // reserved
#define PLX9080_DMLBAM_DECODE 0xffff0000 // bits to decode
// Local Base Address for direct master, local to PCI IO/cfg
#define PLX9080_DMLBAI 0x24
#define PLX9080_DMLBAI_RESVD  0x0000ffff // reserved
#define PLX9080_DMLBAI_DECODE 0xffff0000 // bits to decode
// PCI remap and config, local to PCI
#define PLX9080_DMPBAM 0x28
#define PLX9080_DMPBAM_MEM_ENB   0x00000001 // local->memory enable
#define PLX9080_DMPBAM_IO_ENB    0x00000002 // local->I/O enable
#define PLX9080_DMPBAM_LLOCK_ENB 0x00000004 // LLOCK input enable
#define PLX9080_DMPBAM_PREFETCH  0x00001008 // prefetch size:
#define PLX9080_DMPBAM_PREFETCH_ALL 0x00000000 // until done
#define PLX9080_DMPBAM_PREFETCH_4   0x00000008 // up to 4 words
#define PLX9080_DMPBAM_PREFETCH_8   0x00001000 // up to 8 words
#define PLX9080_DMPBAM_PREFETCH_16  0x00001008 // up to 16 words
#define PLX9080_DMPBAM_DM_RMODE  0x00000010 // direct master FIFO full:
#define PLX9080_DMPBAM_DM_RMODE_RELEASE 0x00000000 // release PCI
#define PLX9080_DMPBAM_DM_RMODE_IRDY    0x00000010 // deassert IRDY
#define PLX9080_DMPBAM_NEAR_FULL 0x000005e0 // write FIFO DMPAF point
#define PLX9080_DMPBAM_WINV      0x00000200 // write-and-invalidate
#define PLX9080_DMPBAM_NO_PF_4K  0x00000800 // no pf across 4K points
#define PLX9080_DMPBAM_IO_REMAP  0x00002000 // use 0 instead of REMAP
#define PLX9080_DMPBAM_WDELAY    0x00006000 // write delay:
#define PLX9080_DMPBAM_WDELAY_NONE 0x00000000 // start immediately
#define PLX9080_DMPBAM_WDELAY_4    0x00002000 // 4 PCI clocks
#define PLX9080_DMPBAM_WDELAY_8    0x00004000 // 8 PCI clocks
#define PLX9080_DMPBAM_WDELAY_16   0x00006000 // 16 PCI clocks
#define PLX9080_DMPBAM_PCI_ADDR  0xffff0000 // remap bits
// PCI configuration address for direct master to PCI IO/cfg
#define PLX9080_DMCFGA 0x2c
#define PLX9080_DMCFGA_CFG_TYPE 0x000000003 // configuration type:
#define PLX9080_DMCFGA_CFG_TYPE_0 0x000000000 // type 0
#define PLX9080_DMCFGA_CFG_TYPE_1 0x000000001 // type 1
// doc is silent on other values for CFG_TYPE
#define PLX9080_DMCFGA_REGISTER 0x000000fc // register number
#define PLX9080_DMCFGA_REGISTER_M 0x3f
#define PLX9080_DMCFGA_REGISTER_S 2
#define PLX9080_DMCFGA_FXN      0x00000700 // function number
#define PLX9080_DMCFGA_FXN_M 7
#define PLX9080_DMCFGA_FXN_S 8
#define PLX9080_DMCFGA_DEV      0x0000f800 // device number
#define PLX9080_DMCFGA_DEV_M 31
#define PLX9080_DMCFGA_DEV_S 11
#define PLX9080_DMCFGA_BUS      0x00ff0000 // bus number
#define PLX9080_DMCFGA_BUS_M 255
#define PLX9080_DMCFGA_BUS_S 16
#define PLX9080_DMCFGA_RESVD    0x7f000000 // reserved
#define PLX9080_DMCFGA_CFG_ENB  0x80000000 // convert I/O to cfg
// Local Address Space 1 Range Register, for PCI->local
#define PLX9080_LAS1RR 0xf0
// see PLX9080_LASxRR_* above
// Local Address Space 1 Local Base Address
#define PLX9080_LAS1BA 0xf4
// see PLX9080_LASxBA_* above
// Local Address Space 1 (similar to LBRD0 in the low bits)
#define PLX9080_LBRD1 0xf8
#define PLX9080_LBRD1_MEM1SIZE 0x00000003 // memory 1 local bus width
#define PLX9080_LBRD1_MEM1SIZE_8   0x00000000 // 8 bits
#define PLX9080_LBRD1_MEM1SIZE_16  0x00000001 // 16 bits
#define PLX9080_LBRD1_MEM1SIZE_32  0x00000002 // 32 bits
#define PLX9080_LBRD1_MEM1SIZE_32x 0x00000003 // 32 bits
#define PLX9080_LBRD1_MEM1WAIT 0x0000003c // memory 0 wait states
#define PLX9080_LBRD1_MEM1WAIT_M 15
#define PLX9080_LBRD1_MEM1WAIT_S 2
#define PLX9080_LBRD1_MS1READY 0x00000040 // memory 1 ready input enb
#define PLX9080_LBRD1_MS1BTERM 0x00000080 // memory 1 BTERM input enb
#define PLX9080_LBRD1_MS1BURST 0x00000100 // memory 1 burst enb
#define PLX9080_LBRD1_MS1PFDIS 0x00000200 // memory 1 prefetch dis
#define PLX9080_LBRD1_PFCNTENB 0x00000400 // prefetch count enable
#define PLX9080_LBRD1_PFCOUNT  0x00007800 // prefetch count
#define PLX9080_LBRD1_PFCOUNT_M 15
#define PLX9080_LBRD1_PFCOUNT_S 11
#define PLX9080_LBRD1_RESVD    0xffff8000 // reserved
// Mailboxes and doorbells - stuff we have no use for
// 0x40 and 0x44 change depending on whether I2O is enabled
// They can be MBOX0 and MBOX1, or message queue registers.
#define PLX9080_MBOX2 0x48
#define PLX9080_MBOX3 0x4c
#define PLX9080_MBOX4 0x50
#define PLX9080_MBOX5 0x54
#define PLX9080_MBOX6 0x58
#define PLX9080_MBOX7 0x5c
#define PLX9080_P2LDBELL 0x60
#define PLX9080_L2PDBELL 0x64
// Interrupt control/status
#define PLX9080_INTCSR 0x68
#define PLX9080_INTCSR_LSERR_PCIABT 0x00000001 // LSERR on PCI abort
#define PLX9080_INTCSR_LSERR_PCIPER 0x00000002 // LSERR on PCI parity
#define PLX9080_INTCSR_PCI_SERR     0x00000004 // causes PCI SERR
#define PLX9080_INTCSR_RESVD        0x000000f8 // reserved
#define PLX9080_INTCSR_PCI_IE       0x00000100 // PCI int enb
#define PLX9080_INTCSR_PCI_DB_IE    0x00000200 // PCI doorbell int enb
#define PLX9080_INTCSR_PCI_ABT_IE   0x00000400 // PCI abort int enb
#define PLX9080_INTCSR_PCI_LCL_IE   0x00000800 // PCI local int enb
#define PLX9080_INTCSR_RETRY_ABORT  0x00001000 // 256 retries -> abort
#define PLX9080_INTCSR_PCI_DB_IRQ   0x00002000 // PCI doorbell int req
#define PLX9080_INTCSR_PCI_ABT_IRQ  0x00004000 // PCI abort int req
#define PLX9080_INTCSR_PCI_LCL_IRQ  0x00008000 // PCI local int req
#define PLX9080_INTCSR_LCL_IE       0x00010000 // lcl int enb
#define PLX9080_INTCSR_LCL_DB_IE    0x00020000 // lcl doorbell int enb
#define PLX9080_INTCSR_LCL_CH0_IE   0x00040000 // DMA chan 0 int enb
#define PLX9080_INTCSR_LCL_CH1_IE   0x00080000 // DMA chan 1 int enb
#define PLX9080_INTCSR_LCL_DB_IRQ   0x00100000 // lcl doorbell int req
#define PLX9080_INTCSR_LCL_CH0_IRQ  0x00200000 // DMA chan 0 int req
#define PLX9080_INTCSR_LCL_CH1_IRQ  0x00400000 // DMA chan 1 int req
#define PLX9080_INTCSR_BIST_IRQ     0x00800000 // builtin selftest irq
#define PLX9080_INTCSR_DM_ABT       0x01000000 // direct master abort
#define PLX9080_INTCSR_CH0_ABT      0x02000000 // DMA chan 0 abort
#define PLX9080_INTCSR_CH1_ABT      0x04000000 // DMA chan 1 abort
#define PLX9080_INTCSR_RETRY_ABT    0x08000000 // 256-retry abort
#define PLX9080_INTCSR_MBOX0        0x10000000 // PCI wrote to mbox0
#define PLX9080_INTCSR_MBOX1        0x20000000 // PCI wrote to mbox1
#define PLX9080_INTCSR_MBOX2        0x40000000 // PCI wrote to mbox2
#define PLX9080_INTCSR_MBOX3        0x80000000 // PCI wrote to mbox3
// EEPROM, PCI command, user I/O, and init control
#define PLX9080_CNTRL 0x6c
#define PLX9080_CNTRL_DMA_R_CMD 0x0000000f // cmd: DMA reads
#define PLX9080_CNTRL_DMA_R_CMD_M 15
#define PLX9080_CNTRL_DMA_R_CMD_S 0
#define PLX9080_CNTRL_DMA_W_CMD 0x000000f0 // cmd: DMA writes
#define PLX9080_CNTRL_DMA_W_CMD_M 15
#define PLX9080_CNTRL_DMA_W_CMD_S 4
#define PLX9080_CNTRL_DM_R_CMD  0x00000f00 // cmd: direct master reads
#define PLX9080_CNTRL_DM_R_CMD_M 15
#define PLX9080_CNTRL_DM_R_CMD_S 8
#define PLX9080_CNTRL_DM_W_CMD  0x0000f000 // cmd: direct master writes
#define PLX9080_CNTRL_DM_W_CMD_M 15
#define PLX9080_CNTRL_DM_W_CMD_S 12
#define PLX9080_CNTRL_GPIO_O    0x00010000 // GPIO output (USERO)
#define PLX9080_CNTRL_GPIO_I    0x00020000 // GPIO input (USERI)
#define PLX9080_CNTRL_RESVD     0x00fc0000 // reserved
#define PLX9080_CNTRL_EECLOCK   0x01000000 // EEPROM clock pin
#define PLX9080_CNTRL_EECSEL    0x02000000 // EEPROM chip select
#define PLX9080_CNTRL_EEWRITE   0x04000000 // EEPROM write bit
#define PLX9080_CNTRL_EEREAD    0x08000000 // EEPROM read bit
#define PLX9080_CNTRL_EEEXIST   0x10000000 // EEPROM exists
#define PLX9080_CNTRL_RELOAD    0x20000000 // reload PCI cfg <- EEPROM
#define PLX9080_CNTRL_SOFTRESET 0x40000000 // software reset
#define PLX9080_CNTRL_INITDONE  0x80000000 // local init done
// PCI permanent configuration
#define PLX9080_PCIHIDR 0x70
#define PLX9080_PCIHIDR_VENDOR 0x0000ffff // permanent vendor ID, 10b5
#define PLX9080_PCIHIDR_VENDOR_M 0xffff
#define PLX9080_PCIHIDR_VENDOR_S 0
#define PLX9080_PCIHIDR_DEVICE 0xffff0000 // permanent device ID, 9080
#define PLX9080_PCIHIDR_DEVICE_M 0xffff
#define PLX9080_PCIHIDR_DEVICE_S 16
// PCI permanent revision ID
#define PLX9080_PCIHREV 0x74
#define PLX9080_PCIHREV_M 255
#define PLX9080_PCIHREV_S 0
// See the [0x40,0x68) range above
#define PLX9080_MBOX0 0x78
#define PLX9080_MBOX1 0x7c
// DMA channel 0 mode
#define PLX9080_DMAMODE0 0x80
#define PLX9080_DMAMODEx_LB_WIDTH 0x00000003 // local bus width
#define PLX9080_DMAMODEx_LB_WIDTH_8   0x00000000 // 8 bits
#define PLX9080_DMAMODEx_LB_WIDTH_16  0x00000001 // 16 bits
#define PLX9080_DMAMODEx_LB_WIDTH_32  0x00000002 // 32 bits
#define PLX9080_DMAMODEx_LB_WIDTH_32x 0x00000003 // 32 bits
#define PLX9080_DMAMODEx_WAITS    0x0000003c // internal wait states
#define PLX9080_DMAMODEx_WAITS_M 15
#define PLX9080_DMAMODEx_WAITS_S 2
#define PLX9080_DMAMODEx_READY    0x00000040 // READY input enb
#define PLX9080_DMAMODEx_BTERM    0x00000080 // BTERM input enb
#define PLX9080_DMAMODEx_LBURST   0x00000100 // local burst enb
#define PLX9080_DMAMODEx_CHAIN    0x00000200 // DMA chaining enb
#define PLX9080_DMAMODEx_DONE_IE  0x00000400 // DMA-done int enb
#define PLX9080_DMAMODEx_LCLCONST 0x00000800 // local addr constant
#define PLX9080_DMAMODEx_DEMAND   0x00001000 // DMA demand mode
#define PLX9080_DMAMODEx_WINV     0x00002000 // DMA write-&-invalidate
#define PLX9080_DMAMODEx_EOT      0x00004000 // EOT input enb
#define PLX9080_DMAMODEx_DMA_STOP 0x00008000 // DMA stop method:
#define PLX9080_DMAMODEx_DMA_STOP_BLAST 0x00000000 // send BLAST
#define PLX9080_DMAMODEx_DMA_STOP_E_D   0x00008000 // EOT/DREQ
#define PLX9080_DMAMODEx_CLR_CNT  0x00010000 // clear-count mode
#define PLX9080_DMAMODEx_CHx_INT  0x00020000 // DMA intr sel:
#define PLX9080_DMAMODEx_CHx_INT_LCL 0x00000000 // local intr
#define PLX9080_DMAMODEx_CHx_INT_PCI 0x00020000 // CI intr
#define PLX9080_DMAMODEx_RESVD    0xfffc00000 // reserved
// DMA channel 0 PCI address
#define PLX9080_DMAPADR0 0x84
// DMA channel 0 local address
#define PLX9080_DMALADR0 0x88
// DMA channel 0 transfer byte count
#define PLX9080_DMASIZ0  0x8c
#define PLX9080_DMASIZx_MAX 0x007fffff
// DMA channel 0 descriptor pointer
#define PLX9080_DMADPR0  0x90
#define PLX9080_DMADPRx_SPACE 0x00000001 // address space:
#define PLX9080_DMADPRx_SPACE_LCL 0x00000000 // local
#define PLX9080_DMADPRx_SPACE_PCI 0x00000001 // PCI
#define PLX9080_DMADPRx_END   0x00000002 // end-of-chain
#define PLX9080_DMADPRx_IRQ   0x00000004 // interrupt at end
#define PLX9080_DMADPRx_DIR   0x00000008 // transfer direction
#define PLX9080_DMADPRx_DIR_P2L 0x00000000 // PCI->local
#define PLX9080_DMADPRx_DIR_L2P 0x00000008 // local->PCI
#define PLX9080_DMADPRx_NEXT  0xfffffff0 // next descriptor address
// DMA channel 1 mode
#define PLX9080_DMAMODE1 0x94
// See the PLX9080_DMAMODEx_* defines, above
// DMA channel 1 PCI address
#define PLX9080_DMAPADR1 0x98
// DMA channel 1 local address
#define PLX9080_DMALADR1 0x9c
// DMA channel 1 transfer byte count
#define PLX9080_DMASIZ1 0xa0
// See the PLX9080_DMASIZx_* defines, above
// DMA channel 1 descriptor pointer
#define PLX9080_DMADPR1 0xa4
// See the PLX9080_DMADPRx_* defines, above
// DMA channel 0 command/status
#define PLX9080_DMACSR0  0xa8
#define PLX9080_DMACSRx_ENB   0x01 // channel enable
#define PLX9080_DMACSRx_GO    0x02 // start
#define PLX9080_DMACSRx_ABORT 0x04 // abort
#define PLX9080_DMACSRx_IACK  0x08 // clear interrupts
#define PLX9080_DMACSRx_DONE  0x10 // transfer done
#define PLX9080_DMACSRx_RESVD 0xe0
// DMA channel 1 command/status
#define PLX9080_DMACSR1  0xa9
// See the PLX9080_DMACSRx_* defines, above
// DMA arbitration (same as LARBR, at 0x08)
#define PLX9080_DMAARB 0xac
// DMA threshold register
#define PLX9080_DMATHR 0xb0
// The odd-looking ordering makes more sense if you read it as
//	000f	Entries before requesting local bus for writes
//	00f0	Entries before requesting local bus for reads
//	0f00	Entries before requesting PCI bus for writes
//	f000	Entries before requesting PCI bus for reads
// There are shoulds applying to various sums:
//	CxPLAF+1 + CxPLAE+1 should be <= 32
//	CxLPAF+1 + CxLPAE+1 should be <= 32
#define PLX9080_DMATHR_C0PLAF 0x0000000f // Ch0 PCI->local almost full
#define PLX9080_DMATHR_C0PLAF_M 15
#define PLX9080_DMATHR_C0PLAF_S 0
#define PLX9080_DMATHR_C0LPAE 0x000000f0 // Ch0 local->PCI almost empty
#define PLX9080_DMATHR_C0LPAE_M 15
#define PLX9080_DMATHR_C0LPAE_S 4
#define PLX9080_DMATHR_C0LPAF 0x00000f00 // Ch0 local->PCI almost full
#define PLX9080_DMATHR_C0LPAF_M 15
#define PLX9080_DMATHR_C0LPAF_S 8
#define PLX9080_DMATHR_C0PLAE 0x0000f000 // Ch0 local->PCI almost empty
#define PLX9080_DMATHR_C0PLAE_M 15
#define PLX9080_DMATHR_C0PLAE_S 12
#define PLX9080_DMATHR_C1PLAF 0x000f0000 // Ch1 PCI->local almost full
#define PLX9080_DMATHR_C1PLAF_M 15
#define PLX9080_DMATHR_C1PLAF_S 16
#define PLX9080_DMATHR_C1LPAE 0x00f00000 // Ch1 local->PCI almost empty
#define PLX9080_DMATHR_C1LPAE_M 15
#define PLX9080_DMATHR_C1LPAE_S 20
#define PLX9080_DMATHR_C1LPAF 0x0f000000 // Ch1 local->PCI almost full
#define PLX9080_DMATHR_C1LPAF_M 15
#define PLX9080_DMATHR_C1LPAF_S 24
#define PLX9080_DMATHR_C1PLAE 0xf0000000 // Ch1 local->PCI almost empty
#define PLX9080_DMATHR_C1PLAE_M 15
#define PLX9080_DMATHR_C1PLAE_S 28

// Input CSR
#define AL7300_DI_CSR       0x00
#define AL7300_DI_CSR_DI_32      0x00000001 // input port is 32-bit
#define AL7300_DI_CSR_CLK        0x00000006 // input clock source:
#define AL7300_DI_CSR_CLK_TIMER0 0x00000000 // timer0
#define AL7300_DI_CSR_CLK_20MHz  0x00000002 // 20MHz
#define AL7300_DI_CSR_CLK_10MHz  0x00000004 // 10MHz
#define AL7300_DI_CSR_CLK_EXT    0x00000006 // external (DI_REQ pin)
#define AL7300_DI_CSR_HSHAKE     0x00000008 // enable REQ/ACK handshake
#define AL7300_DI_CSR_WAIT_TRIG  0x00000020 // wait for DITRIG to start
#define AL7300_DI_CSR_A_TERM_OFF 0x00000040 // portA termination off
#define AL7300_DI_CSR_ENABLE     0x00000100 // digital input enable
#define AL7300_DI_CSR_CLR_FIFO   0x00000200 // clear FIFO
#define AL7300_DI_CSR_OVERRUN    0x00000400 // overrun occurred
#define AL7300_DI_CSR_FIFO_FULL  0x00000800 // FIFO is full
#define AL7300_DI_CSR_FIFO_EMPTY 0x00001000 // FIFO is empty
// doc is silent on 0x0000e000 bits
// doc documents 0xffff0000 bits as don't-care
#define AL7300_DI_CSR_DI_MBZ        0x00000080
#define AL7300_DI_CSR_DI_MBO        0x00000010
// Output CSR
#define AL7300_DO_CSR       0x04
#define AL7300_DO_CSR_DO_32      0x00000001 // output port is 32-bit
#define AL7300_DO_CSR_CLK        0x00000006 // clock source:
#define AL7300_DO_CSR_CLK_TIMER1 0x00000000 // timer1
#define AL7300_DO_CSR_CLK_20MHz  0x00000002 // 20MHz
#define AL7300_DO_CSR_CLK_10MHz  0x00000004 // 10MHz
#define AL7300_DO_CSR_CLK_REQACK 0x00000006 // REQ/ACK handshake
#define AL7300_DO_CSR_DELAY_NAE  0x00000008 // wait for !almost-empty
#define AL7300_DO_CSR_PATGEN     0x00000010 // recirculate data
#define AL7300_DO_CSR_WAIT_TRIG  0x00000020 // wait for DOTRIG
#define AL7300_DO_CSR_B_TERM_OFF 0x00000040 // portB termination off
#define AL7300_DO_CSR_STOP_TRIG  0x00000080 // stop PATGEN when !DOTRIG
#define AL7300_DO_CSR_ENABLE     0x00000100 // digital output enable
#define AL7300_DO_CSR_CLR_FIFO   0x00000200 // clear FIFO
#define AL7300_DO_CSR_UNDERRUN   0x00000400 // underrun occurred
#define AL7300_DO_CSR_FIFO_FULL  0x00000800 // FIFO is full
#define AL7300_DO_CSR_FIFO_EMPTY 0x00001000 // FIFO is empty
#define AL7300_DO_CSR_BURST_HS   0x00002000 // burst handshaking
// doc is silent on 0x0000c000 bits
// doc documents 0xffff0000 bits as don't-care
// Aux I/O: four bits of rudimentary GPIO in each direction.
#define AL7300_AUX_DIO      0x08
#define AL7300_AUX_DIO_O 0x0f // Output bits, read-write
#define AL7300_AUX_DIO_O_M 15
#define AL7300_AUX_DIO_O_S 0
#define AL7300_AUX_DIO_I 0xf0 // Input bits, read-only
#define AL7300_AUX_DIO_I_M 15
#define AL7300_AUX_DIO_I_S 4
// Interrupt CSR
// The name of the input pin for these interrupts is rather confused.
// The diagram shows AUXDIO0_EN for the enable bit and AUXIO_INT for
// the interrupt-requested bit.  The text calls the former AUXDI_EN and
// says AUXDI0 when describing it; it calls the latter AUXDI0_INT and
// says AUXDI when describing it.  I'm calling it AUXDI here.
#define AL7300_INT_CSR      0x0c
#define AL7300_INT_CSR_AUX_IE  0x00000001 // AUXDI int enb
#define AL7300_INT_CSR_T2_IE   0x00000002 // timer2 int enb
#define AL7300_INT_CSR_AUX_IRQ 0x00000004 // AUXDI int req
#define AL7300_INT_CSR_T2_IRQ  0x00000008 // timer2 int req
// The diagram documents bits 0x00000030 as "Reserved" and is silent on
// the semantics of the 0x000000c0 bits.  It documents the 0xffffff00
// bits as "Don't Care".  The text is silent on all of 0xfffffff0.
// Input FIFO direct access
#define AL7300_DI_FIFO      0x10
// DI_FIFO is documented as RW, but with the note that "write operation
// should be avoided in normal operation".  There is no hint what, if
// anything, writes to this register do.  If the input FIFO is 8 or 16
// bits wide, 1/2 or 3/4 of this register is meaningless.
// Output FIFO direct access
#define AL7300_DO_FIFO      0x14
// DO_FIFO is documented as RW, but with the note that "read operation
// should be avoided in normal operation".  There is no hint what, if
// anything, reads from this register do.  If the output FIFO is 8 or
// 16 bits wide, 1/2 or 3/4 of this register is meaningless.
// FIFO threshold control
#define AL7300_FIFO_CR      0x18
// This is a little confused.  The diagram shows each field here as
// being 16 bits wide according to the "Bits" column's text, but as
// only 8 bits wide according to the heading (which shows only 8 bit
// positions).  I'm assuming it's actually 16 bits per field.
// Each half must be written twice to set the almost-empty and
// almost-full values - "Programmable almost empty threshold first".
#define AL7300_FIFO_CR_PB 0x0000ffff
#define AL7300_FIFO_CR_PB_M 0xffff
#define AL7300_FIFO_CR_PB_S 0
#define AL7300_FIFO_CR_PA 0xffff0000
#define AL7300_FIFO_CR_PA_M 0xffff
#define AL7300_FIFO_CR_PA_S 16
// Control signal polarity register
#define AL7300_POL_CTRL     0x1c
#define AL7300_POL_CTRL_DI_REQ  0x00000001
#define AL7300_POL_CTRL_DI_REQ_RISING   0x00000000
#define AL7300_POL_CTRL_DI_REQ_FALLING  0x00000001
#define AL7300_POL_CTRL_DI_ACK  0x00000002
#define AL7300_POL_CTRL_DI_ACK_RISING   0x00000000
#define AL7300_POL_CTRL_DI_ACK_FALLING  0x00000002
#define AL7300_POL_CTRL_DI_TRIG 0x00000004
#define AL7300_POL_CTRL_DI_TRIG_RISING  0x00000000
#define AL7300_POL_CTRL_DI_TRIG_FALLING 0x00000004
#define AL7300_POL_CTRL_DO_REQ  0x00000008
#define AL7300_POL_CTRL_DO_REQ_RISING  0x00000000
#define AL7300_POL_CTRL_DO_REQ_FALLING 0x00000008
#define AL7300_POL_CTRL_DO_ACK  0x00000010
#define AL7300_POL_CTRL_DO_ACK_RISING  0x00000000
#define AL7300_POL_CTRL_DO_ACK_FALLING 0x00000010
#define AL7300_POL_CTRL_DO_TRIG 0x00000020
#define AL7300_POL_CTRL_DO_TRIG_RISING  0x00000000
#define AL7300_POL_CTRL_DO_TRIG_FALLING 0x00000020
// The doc is silent on the 0x000000c0 bits
// The doc documents the 0xffffff00 bits a don't-care
// Counter config registers - we don't mess with these
#define AL7300_8254_COUNT0  0x20
#define AL7300_8254_COUNT1  0x24
#define AL7300_8254_COUNT2  0x28
#define AL7300_8254_CONTROL 0x2c

#endif
