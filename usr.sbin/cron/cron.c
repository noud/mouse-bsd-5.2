/*	$NetBSD: cron.c,v 1.13 2006/12/18 20:11:10 christos Exp $	*/

/* Copyright 1988,1990,1993,1994 by Paul Vixie
 * All rights reserved
 *
 * Distribute freely, except: don't remove my name from the source or
 * documentation (don't take credit for my work), mark your changes (don't
 * get me blamed for your possible bugs), don't alter or remove this
 * notice.  May be sold if buildable source is provided to buyer.  No
 * warrantee of any kind, express or implied, is included with this
 * software; use at your own risk, responsibility for damages (if any) to
 * anyone resulting from the use of this software rests entirely with the
 * user.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Paul Vixie          <paul@vix.com>          uunet!decwrl!vixie!paul
 */

#include <sys/cdefs.h>
#if !defined(lint) && !defined(LINT)
#if 0
static char rcsid[] = "Id: cron.c,v 2.11 1994/01/15 20:43:43 vixie Exp";
#else
__RCSID("$NetBSD: cron.c,v 1.13 2006/12/18 20:11:10 christos Exp $");
#endif
#endif


#define	MAIN_PROGRAM


#include "cron.h"
#include <sys/signal.h>
#if SYS_TIME_H
# include <sys/time.h>
#else
# include <time.h>
#endif


static	void	usage(void),
		run_reboot_jobs(cron_db *),
		cron_tick(cron_db *),
		cron_sync(void),
		cron_sleep(void),
#ifdef USE_SIGCHLD
		sigchld_handler(int),
#endif
		sighup_handler(int),
		parse_args(int c, char *v[]);


static void
usage(void) {
	fprintf(stderr, "usage:  %s [-x debugflag[,...]]\n", getprogname());
	exit(ERROR_EXIT);
}


int
main(int argc, char **argv)
{
	cron_db	database;

	setprogname(argv[0]);

#if defined(BSD)
	setlinebuf(stdout);
	setlinebuf(stderr);
#endif

	parse_args(argc, argv);

#ifdef USE_SIGCHLD
	(void) signal(SIGCHLD, sigchld_handler);
#else
	(void) signal(SIGCLD, SIG_IGN);
#endif
	(void) signal(SIGHUP, sighup_handler);

	acquire_daemonlock(0);
	set_cron_uid();
	set_cron_cwd();

#if defined(POSIX)
	setenv("PATH", _PATH_DEFPATH, 1);
#endif

	/* if there are no debug flags turned on, fork as a daemon should.
	 */
# if DEBUGGING
	if (DebugFlags) {
# else
	if (0) {
# endif
		(void) fprintf(stderr, "[%d] cron started\n", getpid());
	} else {
		if (daemon(1, 0)) {
			log_it("CRON",getpid(),"DEATH","can't fork");
			exit(1);
		}
	}

	acquire_daemonlock(0);
	database.head = NULL;
	database.tail = NULL;
	database.mtime = (time_t) 0;
	load_database(&database);
	run_reboot_jobs(&database);
	cron_sync();
	while (TRUE) {
# if DEBUGGING
		if (!(DebugFlags & DTEST))
# endif /*DEBUGGING*/
			cron_sleep();

		load_database(&database);

		/* do this iteration
		 */
		cron_tick(&database);

		/* sleep 1 minute
		 */
		TargetTime += 60;
	}
}


static void
run_reboot_jobs(cron_db *db)
{
	user		*u;
	entry		*e;

	for (u = db->head;  u != NULL;  u = u->next) {
		for (e = u->crontab;  e != NULL;  e = e->next) {
			if (e->flags & WHEN_REBOOT) {
				job_add(e, u);
			}
		}
	}
	(void) job_runqueue();
}


static void
cron_tick(cron_db *db)
{
	char		*orig_tz, *job_tz;
 	struct tm	*tm;
	int		minute, hour, dom, month, dow;
	user		*u;
	entry		*e;

	/* make 0-based values out of these so we can use them as indicies
	 */
#define maketime(tz1, tz2) do { \
	char *t = tz1; \
	if (t != NULL && *t != '\0') \
		setenv("TZ", t, 1); \
	else if ((tz2) != NULL) \
		setenv("TZ", (tz2), 1); \
	else \
		unsetenv("TZ"); \
	tm = localtime(&TargetTime); \
	minute = tm->tm_min -FIRST_MINUTE; \
	hour = tm->tm_hour -FIRST_HOUR; \
	dom = tm->tm_mday -FIRST_DOM; \
	month = tm->tm_mon +1 /* 0..11 -> 1..12 */ -FIRST_MONTH; \
	dow = tm->tm_wday -FIRST_DOW; \
	} while (0)

	orig_tz = getenv("TZ");
	maketime(NULL, orig_tz);

	Debug(DSCH, ("[%d] tick(%d,%d,%d,%d,%d)\n",
		getpid(), minute, hour, dom, month, dow))

	/* the dom/dow situation is odd.  '* * 1,15 * Sun' will run on the
	 * first and fifteenth AND every Sunday;  '* * * * Sun' will run *only*
	 * on Sundays;  '* * 1,15 * *' will run *only* the 1st and 15th.  this
	 * is why we keep 'e->dow_star' and 'e->dom_star'.  yes, it's bizarre.
	 * like many bizarre things, it's the standard.
	 */
	for (u = db->head;  u != NULL;  u = u->next) {
		for (e = u->crontab;  e != NULL;  e = e->next) {
			Debug(DSCH|DEXT, ("user [%s:%d:%d:...] cmd=\"%s\"\n",
					  env_get("LOGNAME", e->envp),
					  e->uid, e->gid, e->cmd))
			job_tz = env_get("CRON_TZ", e->envp);
			maketime(job_tz, orig_tz);
			if (bit_test(e->minute, minute)
			 && bit_test(e->hour, hour)
			 && bit_test(e->month, month)
			 && ( ((e->flags & DOM_STAR) || (e->flags & DOW_STAR))
			      ? (bit_test(e->dow,dow) && bit_test(e->dom,dom))
			      : (bit_test(e->dow,dow) || bit_test(e->dom,dom))
			    )
			   ) {
				job_add(e, u);
			}
		}
	}
	if (orig_tz != NULL)
		setenv("TZ", orig_tz, 1);
	else
		unsetenv("TZ");
}


/* the task here is to figure out how long it's going to be until :00 of the
 * following minute and initialize TargetTime to this value.  TargetTime
 * will subsequently slide 60 seconds at a time, with correction applied
 * implicitly in cron_sleep().  it would be nice to let cron execute in
 * the "current minute" before going to sleep, but by restarting cron you
 * could then get it to execute a given minute's jobs more than once.
 * instead we have the chance of missing a minute's jobs completely, but
 * that's something sysadmin's know to expect what with crashing computers..
 */
static void
cron_sync(void) {
 	struct tm	*tm;

	TargetTime = time((time_t*)0);
	tm = localtime(&TargetTime);
	TargetTime += (60 - tm->tm_sec);
}


static void
cron_sleep(void) {
	int	seconds_to_wait;

	do {
		seconds_to_wait = (int) (TargetTime - time((time_t*)0));
		Debug(DSCH, ("[%d] TargetTime=%ld, sec-to-wait=%d\n",
			getpid(), (long)TargetTime, seconds_to_wait))

		/* if we intend to sleep, this means that it's finally
		 * time to empty the job queue (execute it).
		 *
		 * if we run any jobs, we'll probably screw up our timing,
		 * so go recompute.
		 *
		 * note that we depend here on the left-to-right nature
		 * of &&, and the short-circuiting.
		 */
	} while (seconds_to_wait > 0 && job_runqueue());

	while ((seconds_to_wait = (int) (TargetTime - time((time_t *)0))) > 0) {
		Debug(DSCH, ("[%d] sleeping for %d seconds\n",
			getpid(), seconds_to_wait))
		sleep((unsigned int) seconds_to_wait);
	}
}


#ifdef USE_SIGCHLD
static void
sigchld_handler(int x __unused)
{
	WAIT_T		waiter;
	PID_T		pid;

	for (;;) {
#ifdef POSIX
		pid = waitpid(-1, &waiter, WNOHANG);
#else
		pid = wait3(&waiter, WNOHANG, (struct rusage *)0);
#endif
		switch (pid) {
		case -1:
			Debug(DPROC,
				("[%d] sigchld...no children\n", getpid()))
			return;
		case 0:
			Debug(DPROC,
				("[%d] sigchld...no dead kids\n", getpid()))
			return;
		default:
			Debug(DPROC,
				("[%d] sigchld...pid #%d died, stat=%d\n",
				getpid(), pid, WEXITSTATUS(waiter)))
		}
	}
}
#endif /*USE_SIGCHLD*/


static void
sighup_handler(int x __unused)
{
	log_close();
}


static void
parse_args(int argc, char **argv)
{
	int	argch;

	while (-1 != (argch = getopt(argc, argv, "x:"))) {
		switch (argch) {
		default:
			usage();
		case 'x':
			if (!set_debug_flags(optarg))
				usage();
			break;
		}
	}
}
