#ifndef _TIMERSOCK_H_53053ab7_
#define _TIMERSOCK_H_53053ab7_

#include <sys/time.h>
#include <sys/ioccom.h>

struct timersock_event {
  int tse_type;
#define TS_EV_TICK  1
#define TS_EV_ERROR 2
  union {
    struct timeval tse_u_tv;
    int tse_u_err;
    } tse_u;
#define tse_tv tse_u.tse_u_tv
#define tse_err tse_u.tse_u_err
  } ;

#define TIMERIOC_GDEBUG _IOR('~',0,int)
#define TIMERIOC_SDEBUG _IOW('~',1,int)

#endif
