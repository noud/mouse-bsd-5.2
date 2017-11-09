#ifndef _7300A_H_11c69066_
#define _7300A_H_11c69066_

#include <sys/ioctl.h>

#define ADLINK7300A_DO_AUX _IOW('7',0,char)
#define ADLINK7300A_DI_AUX _IOR('7',1,char)
#define ADLINK7300A_FLUSH  _IO('7',2)

#endif
