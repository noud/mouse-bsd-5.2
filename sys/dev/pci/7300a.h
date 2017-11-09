#ifndef _7300A_H_a1317501_
#define _7300A_H_a1317501_

#include <sys/ioctl.h>

/*
 * ioctls for the Adlink 7300A driver.
 *
 * ADLINK7300A_DO_AUX and ADLINK7300A_DI_AUX write and read
 *  (respectively) the four auxiliary I/O pins (DO_AUX ignores all but
 *  the low four bits of its argument; DI_AUX returns its results in
 *  the low four bits, with the other bits all zero).
 *
 * ADLINK7300A_FLUSH flushes everything currently buffered.  (Thus, for
 *  example, if no samples are coming in, a read() after this will
 *  return zero.)
 */

#define ADLINK7300A_DO_AUX _IOW('7',0,char)
#define ADLINK7300A_DI_AUX _IOR('7',1,char)
#define ADLINK7300A_FLUSH  _IO('7',2)

#endif
