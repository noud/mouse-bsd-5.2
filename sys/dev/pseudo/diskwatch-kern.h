#ifndef _DISKWATCH_KERN_H_12ca6b9b_
#define _DISKWATCH_KERN_H_12ca6b9b_

/* Interface between diskwatch.c and the rest of the kernel. */

#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/ioccom.h>

#define DWIOCSET _IOW('w',1,int)
#define DWIOCCLR _IOW('w',2,int)

extern const struct cdevsw diskwatch_cdevsw;

extern void diskwatchattach(int);
extern void diskwatch_watch(int, struct buf *);
extern void diskwatch_detach(int);

#endif
