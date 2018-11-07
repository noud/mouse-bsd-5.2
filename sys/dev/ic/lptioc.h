#ifndef _LPTIOC_H_622015bb_
#define _LPTIOC_H_622015bb_

#include <sys/ioccom.h>

struct lpt_io {
  unsigned char *ctl;
#define LPTCTL_SETDO 0x01
#define LPTCTL_SETDI 0x02
#define LPTCTL_WDATA 0x03
#define LPTCTL_RDATA 0x04
#define LPTCTL_WCTL  0x05
#define LPTCTL_RSTAT 0x06
  unsigned int clen;
  unsigned char *wdata;
  unsigned int wlen;
  unsigned char *rdata;
  unsigned int rlen;
  } ;

#define LPTIOC_IO _IOWR('|',1,struct lpt_io)

#endif
