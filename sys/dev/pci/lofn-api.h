#ifndef WH_LOFN_API_H_d2351367_
#define WH_LOFN_API_H_d2351367_

#include <sys/ioctl.h>

#define LOFN_IOC_INTRCOUNT _IOR('6',0,unsigned long long int)

struct lofn_compute {
  unsigned int *vec;
  int len;
  } ;
#define LOFN_IOC_COMPUTE _IOW('6',1,struct lofn_compute)
/*
 * vec points to
 *
 *	zero or more of
 *		LOFN_COMPUTE_SETREG regnum 32-words-of-data 1-word-of-length
 *	one of
 *		LOFN_COMPUTE_CODE num-words that-many-words-of-code
 *	zero or more of
 *		LOFN_COMPUTE_GETREG regnum 32-words-of-data-space 1-word-of-length-space
 *
 * They must be in the above order - first a stream of SETREGs, then
 *  exactly one CODE, then a stream of GETREGs.  (The SETREG and GETREG
 *  streams may be zero-length, though I have trouble imagining a use
 *  for either except for performance testing.)
 *
 * The ioctl reads the SETREG(s) and CODE, and their data, pushing them
 *  to the hardware; once computation is complete, it writes the
 *  results into the space following the GETREG(s).  The ioctl does not
 *  complete until the whole sequence is done.  If the ALU is busy, it
 *  first waits for it to be non-busy.
 */
/*
 * These were picked at random, to reduce the chance that data
 *  misinterpreted as a compute vector will be valid.
 */
#define LOFN_COMPUTE_SETREG 0xdc005e73
#define LOFN_COMPUTE_CODE   0x2c345605
#define LOFN_COMPUTE_GETREG 0x3eb4e9e5

#endif
