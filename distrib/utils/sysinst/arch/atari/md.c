/*	$NetBSD: md.c,v 1.22.2.1 2009/01/16 22:57:33 bouyer Exp $ */

/*
 * Copyright 1997 Piermont Information Systems Inc.
 * All rights reserved.
 *
 * Written by Philip A. Nelson for Piermont Information Systems Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed for the NetBSD Project by
 *      Piermont Information Systems Inc.
 * 4. The name of Piermont Information Systems Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY PIERMONT INFORMATION SYSTEMS INC. ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PIERMONT INFORMATION SYSTEMS INC. BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* md.c -- Machine specific code for atari */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>

#include "defs.h"
#include "md.h"
#include "msg_defs.h"
#include "menu_defs.h"


/*
 * any additional partition validation
 */
int
md_check_partitions(void)
{

	return 1;
}

int
md_get_info(void)
{
	return 1;
}

int
md_pre_disklabel(void)
{
	return 0;
}

int
md_post_disklabel(void)
{
	return 0;
}

int
md_post_newfs(void)
{
	static const int mib[2] = {CTL_HW, HW_MODEL};
	size_t len;
	char *cpu_model;
	int milan;
	char bootpath[MAXPATHLEN];
	int rv;

	/* check machine type via sysctl to select appropriate bootloaders */
	milan = 0;	/* std is default */
	sysctl(mib, 2, NULL, &len, NULL, 0);
	cpu_model = malloc(len);
	sysctl(mib, 2, cpu_model, &len, NULL, 0);
	/* XXX model strings should be a common macro to sync with kernel */
	if (strstr(cpu_model, "Milan") != NULL)
		milan = 1;
	free(cpu_model);

	/* copy tertiary boot and install boot blocks */
	msg_display(MSG_dobootblks, diskdev);
	snprintf(bootpath, sizeof(bootpath), "/usr/mdec/%s/boot.atari",
	    milan ? "milan" : "std");
	rv = cp_to_target(bootpath, "/");
	if (rv != 0)
		return rv;

	rv = run_program(RUN_DISPLAY, "/usr/mdec/installboot -v%s /dev/r%sc",
	    milan ? "m" : "", diskdev);

	return rv;
}

int
md_copy_filesystem(void)
{
	return 0;
}

/*
 * md back-end code for menu-driven BSD disklabel editor.
 */
int
md_make_bsd_partitions(void)
{

	msg_display(MSG_infoahdilabel, diskdev);
	process_menu(MENU_noyes, NULL);
	if (yesno) {
		run_program(RUN_DISPLAY, "ahdilabel /dev/r%sc", diskdev);
	}
	if (!make_bsd_partitions())
		return 0;

	/*
	 * Setup the disktype so /etc/disktab gets proper info
	 */
	if (strncmp (diskdev, "sd", 2) == 0)
		disktype = "SCSI";
	else
		disktype = "ST506";

	return 1;
}

/* Upgrade support */
int
md_update(void)
{
	move_aout_libs();
	endwin();
	md_copy_filesystem();
	md_post_newfs();
	wrefresh(curscr);
	wmove(stdscr, 0, 0);
	wclear(stdscr);
	wrefresh(stdscr);
	return 1;
}


void
md_cleanup_install(void)
{

	enable_rc_conf();
}

int
md_pre_update(void)
{
	return 1;
}

void
md_init(void)
{
}

void
md_init_set_status(int minimal)
{
	(void)minimal;
}

int
md_post_extract(void)
{
	return 0;
}
