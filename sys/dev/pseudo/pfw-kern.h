#ifndef _PFW_KERN_H_1731e1ce_
#define _PFW_KERN_H_1731e1ce_

/* Interface between pfw.c and the rest of kernel. */

#include "pfw-vers.h"

#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/ioccom.h>

extern void pfwattach(int);
DEVICE_ROUTINE_DECLS

#endif
