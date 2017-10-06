#ifndef _7300A_H_e60a54c3_
#define _7300A_H_e60a54c3_

#include <sys/ioctl.h>

/*
 * NONE = DMA not in progress, DMA map unloaded.
 *
 * PENDING = DMA in progress, still ongoing.
 *
 * DONE = DMA request done, userland not yet notified.
 *
 * FLUSH = closing with request in progress, flushing
 */
enum adlink7300a_dma_status {
  ADLINK7300A_DMA_NONE = 1,
  ADLINK7300A_DMA_PENDING,
  ADLINK7300A_DMA_DONE,
  ADLINK7300A_DMA_FLUSH,
  } ;

struct adlink7300a_dma {
  unsigned int *buf;
  int count;
  unsigned int flags;
#define ADLINK7300A_DMAF_FLUSH 0x00000001
  } ;

#define ADLINK7300A_DO_AUX     _IOW('7',0,char)
#define ADLINK7300A_DI_AUX     _IOR('7',1,char)
#define ADLINK7300A_DMA_START  _IOW('7',2,struct adlink7300a_dma)
#define ADLINK7300A_DMA_STATUS _IOR('7',3,enum adlink7300a_dma_status)

#endif
