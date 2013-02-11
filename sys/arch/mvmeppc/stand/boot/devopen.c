/*	$NetBSD: devopen.c,v 1.3 2005/12/11 12:18:20 christos Exp $	*/

#include <sys/param.h>
#include <lib/libsa/stand.h>

/*
 * Open the device named by the combined device/file name
 * given as the "fname" arg, something like: "sd()bsd"
 *
 * However, Sun PROMs don't really let you choose which
 * device you will talk to.  You can only open the device
 * that was used to load the boot program.  Therefore, we
 * do not accept a "device" part in the "fname" string.
 * Pass the PROM device name to open in case it needs it.
 */
int
devopen(struct open_file *f, const char *fname, char **file)
{
	struct devsw *dp;
	int error;

	*file = (char*)fname;
	dp = &devsw[0];
	f->f_dev = dp;
	error = (*dp->dv_open)(f, "net");	/* XXXSCW: Fixme */

	return error;
}
