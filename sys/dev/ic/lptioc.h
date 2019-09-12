#ifndef WH_LPTIOC_H_256e6061_
#define WH_LPTIOC_H_256e6061_

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
#define LPTIOC_ENABLE_D2A _IO('|',2)
#define LPTIOC_SEND_D2A _IOW('|',3,unsigned char)
#define LPTIOC_DISABLE_D2A _IO('|',4)

#endif
