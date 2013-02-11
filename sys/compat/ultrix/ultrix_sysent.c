/* $NetBSD: ultrix_sysent.c,v 1.58 2008/06/19 12:29:34 hans Exp $ */

/*
 * System call switch table.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.46 2008/06/19 12:28:12 hans Exp
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: ultrix_sysent.c,v 1.58 2008/06/19 12:29:34 hans Exp $");

#if defined(_KERNEL_OPT)
#include "opt_nfsserver.h"
#include "fs_nfs.h"
#endif
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/mount.h>
#include <sys/sched.h>
#include <sys/syscallargs.h>
#include <compat/ultrix/ultrix_syscallargs.h>

#define	s(type)	sizeof(type)
#define	n(type)	(sizeof(type)/sizeof (register_t))
#define	ns(type)	n(type), s(type)

struct sysent ultrix_sysent[] = {
	{ 0, 0, 0,
	    (sy_call_t *)sys_nosys },		/* 0 = syscall */
	{ ns(struct sys_exit_args), 0,
	    (sy_call_t *)sys_exit },		/* 1 = exit */
	{ 0, 0, 0,
	    (sy_call_t *)sys_fork },		/* 2 = fork */
	{ ns(struct sys_read_args), 0,
	    (sy_call_t *)sys_read },		/* 3 = read */
	{ ns(struct sys_write_args), 0,
	    (sy_call_t *)sys_write },		/* 4 = write */
	{ ns(struct ultrix_sys_open_args), 0,
	    (sy_call_t *)ultrix_sys_open },	/* 5 = open */
	{ ns(struct sys_close_args), 0,
	    (sy_call_t *)sys_close },		/* 6 = close */
	{ 0, 0, 0,
	    (sy_call_t *)compat_43_sys_wait },	/* 7 = owait */
	{ ns(struct ultrix_sys_creat_args), 0,
	    (sy_call_t *)ultrix_sys_creat },	/* 8 = creat */
	{ ns(struct sys_link_args), 0,
	    (sy_call_t *)sys_link },		/* 9 = link */
	{ ns(struct sys_unlink_args), 0,
	    (sy_call_t *)sys_unlink },		/* 10 = unlink */
	{ ns(struct ultrix_sys_execv_args), 0,
	    (sy_call_t *)ultrix_sys_execv },	/* 11 = execv */
	{ ns(struct sys_chdir_args), 0,
	    (sy_call_t *)sys_chdir },		/* 12 = chdir */
	{ 0, 0, 0,
	    sys_nosys },			/* 13 = obsolete time */
	{ ns(struct ultrix_sys_mknod_args), 0,
	    (sy_call_t *)ultrix_sys_mknod },	/* 14 = mknod */
	{ ns(struct sys_chmod_args), 0,
	    (sy_call_t *)sys_chmod },		/* 15 = chmod */
	{ ns(struct sys___posix_chown_args), 0,
	    (sy_call_t *)sys___posix_chown },	/* 16 = __posix_chown */
	{ ns(struct sys_obreak_args), 0,
	    (sy_call_t *)sys_obreak },		/* 17 = break */
	{ 0, 0, 0,
	    sys_nosys },			/* 18 = obsolete stat */
	{ ns(struct compat_43_sys_lseek_args), 0,
	    (sy_call_t *)compat_43_sys_lseek },	/* 19 = lseek */
	{ 0, 0, 0,
	    (sy_call_t *)sys_getpid },		/* 20 = getpid */
	{ ns(struct ultrix_sys_mount_args), 0,
	    (sy_call_t *)ultrix_sys_mount },	/* 21 = mount */
	{ 0, 0, 0,
	    sys_nosys },			/* 22 = obsolete sysV_unmount */
	{ ns(struct sys_setuid_args), 0,
	    (sy_call_t *)sys_setuid },		/* 23 = setuid */
	{ 0, 0, 0,
	    (sy_call_t *)sys_getuid },		/* 24 = getuid */
	{ 0, 0, 0,
	    sys_nosys },			/* 25 = obsolete v7 stime */
	{ 0, 0, 0,
	    sys_nosys },			/* 26 = obsolete v7 ptrace */
	{ 0, 0, 0,
	    sys_nosys },			/* 27 = obsolete v7 alarm */
	{ 0, 0, 0,
	    sys_nosys },			/* 28 = obsolete v7 fstat */
	{ 0, 0, 0,
	    sys_nosys },			/* 29 = obsolete v7 pause */
	{ 0, 0, 0,
	    sys_nosys },			/* 30 = obsolete v7 utime */
	{ 0, 0, 0,
	    sys_nosys },			/* 31 = obsolete v7 stty */
	{ 0, 0, 0,
	    sys_nosys },			/* 32 = obsolete v7 gtty */
	{ ns(struct ultrix_sys_access_args), 0,
	    (sy_call_t *)ultrix_sys_access },	/* 33 = access */
	{ 0, 0, 0,
	    sys_nosys },			/* 34 = obsolete v7 nice */
	{ 0, 0, 0,
	    sys_nosys },			/* 35 = obsolete v7 ftime */
	{ 0, 0, 0,
	    (sy_call_t *)sys_sync },		/* 36 = sync */
	{ ns(struct sys_kill_args), 0,
	    (sy_call_t *)sys_kill },		/* 37 = kill */
	{ ns(struct ultrix_sys_stat_args), 0,
	    (sy_call_t *)ultrix_sys_stat },	/* 38 = stat43 */
	{ 0, 0, 0,
	    sys_nosys },			/* 39 = obsolete v7 setpgrp */
	{ ns(struct ultrix_sys_lstat_args), 0,
	    (sy_call_t *)ultrix_sys_lstat },	/* 40 = lstat43 */
	{ ns(struct sys_dup_args), 0,
	    (sy_call_t *)sys_dup },		/* 41 = dup */
	{ 0, 0, 0,
	    (sy_call_t *)sys_pipe },		/* 42 = pipe */
	{ 0, 0, 0,
	    sys_nosys },			/* 43 = obsolete v7 times */
	{ ns(struct sys_profil_args), 0,
	    (sy_call_t *)sys_profil },		/* 44 = profil */
	{ 0, 0, 0,
	    sys_nosys },			/* 45 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 46 = obsolete v7 setgid */
	{ 0, 0, 0,
	    (sy_call_t *)sys_getgid },		/* 47 = getgid */
	{ 0, 0, 0,
	    sys_nosys },			/* 48 = unimplemented ssig */
	{ 0, 0, 0,
	    sys_nosys },			/* 49 = unimplemented reserved for USG */
	{ 0, 0, 0,
	    sys_nosys },			/* 50 = unimplemented reserved for USG */
	{ ns(struct sys_acct_args), 0,
	    (sy_call_t *)sys_acct },		/* 51 = acct */
	{ 0, 0, 0,
	    sys_nosys },			/* 52 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 53 = unimplemented syslock */
	{ ns(struct ultrix_sys_ioctl_args), 0,
	    (sy_call_t *)ultrix_sys_ioctl },	/* 54 = ioctl */
	{ ns(struct sys_reboot_args), 0,
	    (sy_call_t *)sys_reboot },		/* 55 = reboot */
	{ 0, 0, 0,
	    sys_nosys },			/* 56 = unimplemented v7 mpxchan */
	{ ns(struct sys_symlink_args), 0,
	    (sy_call_t *)sys_symlink },		/* 57 = symlink */
	{ ns(struct sys_readlink_args), 0,
	    (sy_call_t *)sys_readlink },	/* 58 = readlink */
	{ ns(struct ultrix_sys_execve_args), 0,
	    (sy_call_t *)ultrix_sys_execve },	/* 59 = execve */
	{ ns(struct sys_umask_args), 0,
	    (sy_call_t *)sys_umask },		/* 60 = umask */
	{ ns(struct sys_chroot_args), 0,
	    (sy_call_t *)sys_chroot },		/* 61 = chroot */
	{ ns(struct compat_43_sys_fstat_args), 0,
	    (sy_call_t *)compat_43_sys_fstat },	/* 62 = fstat */
	{ 0, 0, 0,
	    sys_nosys },			/* 63 = unimplemented */
	{ 0, 0, 0,
	    (sy_call_t *)compat_43_sys_getpagesize },/* 64 = getpagesize */
	{ 0, 0, 0,
	    sys_nosys },			/* 65 = unimplemented mremap */
	{ 0, 0, 0,
	    (sy_call_t *)sys_vfork },		/* 66 = vfork */
	{ 0, 0, 0,
	    sys_nosys },			/* 67 = obsolete vread */
	{ 0, 0, 0,
	    sys_nosys },			/* 68 = obsolete vwrite */
	{ ns(struct sys_sbrk_args), 0,
	    (sy_call_t *)sys_sbrk },		/* 69 = sbrk */
	{ ns(struct sys_sstk_args), 0,
	    (sy_call_t *)sys_sstk },		/* 70 = sstk */
	{ ns(struct ultrix_sys_mmap_args), 0,
	    (sy_call_t *)ultrix_sys_mmap },	/* 71 = mmap */
	{ ns(struct sys_ovadvise_args), 0,
	    (sy_call_t *)sys_ovadvise },	/* 72 = vadvise */
	{ ns(struct sys_munmap_args), 0,
	    (sy_call_t *)sys_munmap },		/* 73 = munmap */
	{ ns(struct sys_mprotect_args), 0,
	    (sy_call_t *)sys_mprotect },	/* 74 = mprotect */
	{ ns(struct sys_madvise_args), 0,
	    (sy_call_t *)sys_madvise },		/* 75 = madvise */
	{ 0, 0, 0,
	    (sy_call_t *)ultrix_sys_vhangup },	/* 76 = vhangup */
	{ 0, 0, 0,
	    sys_nosys },			/* 77 = unimplemented old vlimit */
	{ ns(struct sys_mincore_args), 0,
	    (sy_call_t *)sys_mincore },		/* 78 = mincore */
	{ ns(struct sys_getgroups_args), 0,
	    (sy_call_t *)sys_getgroups },	/* 79 = getgroups */
	{ ns(struct sys_setgroups_args), 0,
	    (sy_call_t *)sys_setgroups },	/* 80 = setgroups */
	{ 0, 0, 0,
	    (sy_call_t *)sys_getpgrp },		/* 81 = getpgrp */
	{ ns(struct ultrix_sys_setpgrp_args), 0,
	    (sy_call_t *)ultrix_sys_setpgrp },	/* 82 = setpgrp */
	{ ns(struct sys_setitimer_args), 0,
	    (sy_call_t *)sys_setitimer },	/* 83 = setitimer */
	{ ns(struct ultrix_sys_wait3_args), 0,
	    (sy_call_t *)ultrix_sys_wait3 },	/* 84 = wait3 */
	{ ns(struct compat_12_sys_swapon_args), 0,
	    (sy_call_t *)compat_12_sys_swapon },/* 85 = swapon */
	{ ns(struct sys_getitimer_args), 0,
	    (sy_call_t *)sys_getitimer },	/* 86 = getitimer */
	{ ns(struct compat_43_sys_gethostname_args), 0,
	    (sy_call_t *)compat_43_sys_gethostname },/* 87 = gethostname */
	{ ns(struct compat_43_sys_sethostname_args), 0,
	    (sy_call_t *)compat_43_sys_sethostname },/* 88 = sethostname */
	{ 0, 0, 0,
	    (sy_call_t *)compat_43_sys_getdtablesize },/* 89 = getdtablesize */
	{ ns(struct sys_dup2_args), 0,
	    (sy_call_t *)sys_dup2 },		/* 90 = dup2 */
	{ 0, 0, 0,
	    sys_nosys },			/* 91 = unimplemented getdopt */
	{ ns(struct ultrix_sys_fcntl_args), 0,
	    (sy_call_t *)ultrix_sys_fcntl },	/* 92 = fcntl */
	{ ns(struct ultrix_sys_select_args), 0,
	    (sy_call_t *)ultrix_sys_select },	/* 93 = select */
	{ 0, 0, 0,
	    sys_nosys },			/* 94 = unimplemented setdopt */
	{ ns(struct sys_fsync_args), 0,
	    (sy_call_t *)sys_fsync },		/* 95 = fsync */
	{ ns(struct sys_setpriority_args), 0,
	    (sy_call_t *)sys_setpriority },	/* 96 = setpriority */
	{ ns(struct compat_30_sys_socket_args), 0,
	    (sy_call_t *)compat_30_sys_socket },/* 97 = socket */
	{ ns(struct sys_connect_args), 0,
	    (sy_call_t *)sys_connect },		/* 98 = connect */
	{ ns(struct compat_43_sys_accept_args), 0,
	    (sy_call_t *)compat_43_sys_accept },/* 99 = accept */
	{ ns(struct sys_getpriority_args), 0,
	    (sy_call_t *)sys_getpriority },	/* 100 = getpriority */
	{ ns(struct compat_43_sys_send_args), 0,
	    (sy_call_t *)compat_43_sys_send },	/* 101 = send */
	{ ns(struct compat_43_sys_recv_args), 0,
	    (sy_call_t *)compat_43_sys_recv },	/* 102 = recv */
	{ ns(struct ultrix_sys_sigreturn_args), 0,
	    (sy_call_t *)ultrix_sys_sigreturn },/* 103 = sigreturn */
	{ ns(struct sys_bind_args), 0,
	    (sy_call_t *)sys_bind },		/* 104 = bind */
	{ ns(struct ultrix_sys_setsockopt_args), 0,
	    (sy_call_t *)ultrix_sys_setsockopt },/* 105 = setsockopt */
	{ ns(struct sys_listen_args), 0,
	    (sy_call_t *)sys_listen },		/* 106 = listen */
	{ 0, 0, 0,
	    sys_nosys },			/* 107 = unimplemented vtimes */
	{ ns(struct ultrix_sys_sigvec_args), 0,
	    (sy_call_t *)ultrix_sys_sigvec },	/* 108 = sigvec */
	{ ns(struct compat_43_sys_sigblock_args), 0,
	    (sy_call_t *)compat_43_sys_sigblock },/* 109 = sigblock */
	{ ns(struct compat_43_sys_sigsetmask_args), 0,
	    (sy_call_t *)compat_43_sys_sigsetmask },/* 110 = sigsetmask */
	{ ns(struct ultrix_sys_sigsuspend_args), 0,
	    (sy_call_t *)ultrix_sys_sigsuspend },/* 111 = sigsuspend */
	{ ns(struct compat_43_sys_sigstack_args), 0,
	    (sy_call_t *)compat_43_sys_sigstack },/* 112 = sigstack */
	{ ns(struct compat_43_sys_recvmsg_args), 0,
	    (sy_call_t *)compat_43_sys_recvmsg },/* 113 = recvmsg */
	{ ns(struct compat_43_sys_sendmsg_args), 0,
	    (sy_call_t *)compat_43_sys_sendmsg },/* 114 = sendmsg */
	{ 0, 0, 0,
	    sys_nosys },			/* 115 = obsolete vtrace */
	{ ns(struct sys_gettimeofday_args), 0,
	    (sy_call_t *)sys_gettimeofday },	/* 116 = gettimeofday */
	{ ns(struct sys_getrusage_args), 0,
	    (sy_call_t *)sys_getrusage },	/* 117 = getrusage */
	{ ns(struct sys_getsockopt_args), 0,
	    (sy_call_t *)sys_getsockopt },	/* 118 = getsockopt */
	{ 0, 0, 0,
	    sys_nosys },			/* 119 = unimplemented resuba */
	{ ns(struct sys_readv_args), 0,
	    (sy_call_t *)sys_readv },		/* 120 = readv */
	{ ns(struct sys_writev_args), 0,
	    (sy_call_t *)sys_writev },		/* 121 = writev */
	{ ns(struct sys_settimeofday_args), 0,
	    (sy_call_t *)sys_settimeofday },	/* 122 = settimeofday */
	{ ns(struct sys___posix_fchown_args), 0,
	    (sy_call_t *)sys___posix_fchown },	/* 123 = __posix_fchown */
	{ ns(struct sys_fchmod_args), 0,
	    (sy_call_t *)sys_fchmod },		/* 124 = fchmod */
	{ ns(struct compat_43_sys_recvfrom_args), 0,
	    (sy_call_t *)compat_43_sys_recvfrom },/* 125 = recvfrom */
	{ ns(struct sys_setreuid_args), 0,
	    (sy_call_t *)sys_setreuid },	/* 126 = setreuid */
	{ ns(struct sys_setregid_args), 0,
	    (sy_call_t *)sys_setregid },	/* 127 = setregid */
	{ ns(struct sys_rename_args), 0,
	    (sy_call_t *)sys_rename },		/* 128 = rename */
	{ ns(struct compat_43_sys_truncate_args), 0,
	    (sy_call_t *)compat_43_sys_truncate },/* 129 = truncate */
	{ ns(struct compat_43_sys_ftruncate_args), 0,
	    (sy_call_t *)compat_43_sys_ftruncate },/* 130 = ftruncate */
	{ ns(struct sys_flock_args), 0,
	    (sy_call_t *)sys_flock },		/* 131 = flock */
	{ 0, 0, 0,
	    sys_nosys },			/* 132 = unimplemented */
	{ ns(struct sys_sendto_args), 0,
	    (sy_call_t *)sys_sendto },		/* 133 = sendto */
	{ ns(struct sys_shutdown_args), 0,
	    (sy_call_t *)sys_shutdown },	/* 134 = shutdown */
	{ ns(struct sys_socketpair_args), 0,
	    (sy_call_t *)sys_socketpair },	/* 135 = socketpair */
	{ ns(struct sys_mkdir_args), 0,
	    (sy_call_t *)sys_mkdir },		/* 136 = mkdir */
	{ ns(struct sys_rmdir_args), 0,
	    (sy_call_t *)sys_rmdir },		/* 137 = rmdir */
	{ ns(struct sys_utimes_args), 0,
	    (sy_call_t *)sys_utimes },		/* 138 = utimes */
	{ ns(struct ultrix_sys_sigcleanup_args), 0,
	    (sy_call_t *)ultrix_sys_sigcleanup },/* 139 = sigcleanup */
	{ ns(struct sys_adjtime_args), 0,
	    (sy_call_t *)sys_adjtime },		/* 140 = adjtime */
	{ ns(struct compat_43_sys_getpeername_args), 0,
	    (sy_call_t *)compat_43_sys_getpeername },/* 141 = getpeername */
	{ 0, 0, 0,
	    (sy_call_t *)compat_43_sys_gethostid },/* 142 = gethostid */
	{ 0, 0, 0,
	    sys_nosys },			/* 143 = unimplemented old sethostid */
	{ ns(struct compat_43_sys_getrlimit_args), 0,
	    (sy_call_t *)compat_43_sys_getrlimit },/* 144 = getrlimit */
	{ ns(struct compat_43_sys_setrlimit_args), 0,
	    (sy_call_t *)compat_43_sys_setrlimit },/* 145 = setrlimit */
	{ ns(struct compat_43_sys_killpg_args), 0,
	    (sy_call_t *)compat_43_sys_killpg },/* 146 = killpg */
	{ 0, 0, 0,
	    sys_nosys },			/* 147 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 148 = unimplemented setquota */
	{ 0, 0, 0,
	    sys_nosys },			/* 149 = unimplemented quota / * needs to be nullop to boot on Ultrix root partition * / */
	{ ns(struct compat_43_sys_getsockname_args), 0,
	    (sy_call_t *)compat_43_sys_getsockname },/* 150 = getsockname */
	{ 0, 0, 0,
	    sys_nosys },			/* 151 = unimplemented sysmips / * 4 args * / */
#ifdef __mips
	{ ns(struct ultrix_sys_cacheflush_args), 0,
	    (sy_call_t *)ultrix_sys_cacheflush },/* 152 = cacheflush */
	{ ns(struct ultrix_sys_cachectl_args), 0,
	    (sy_call_t *)ultrix_sys_cachectl },	/* 153 = cachectl */
#else	/* !mips */
	{ 0, 0, 0,
	    sys_nosys },			/* 152 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 153 = unimplemented */
#endif	/* !mips */
	{ 0, 0, 0,
	    sys_nosys },			/* 154 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 155 = unimplemented atomic_op */
	{ 0, 0, 0,
	    sys_nosys },			/* 156 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 157 = unimplemented */
#ifdef NFSSERVER
	{ ns(struct ultrix_sys_nfssvc_args), 0,
	    (sy_call_t *)ultrix_sys_nfssvc },	/* 158 = nfssvc */
#else
	{ 0, 0, 0,
	    sys_nosys },			/* 158 = unimplemented */
#endif
	{ ns(struct compat_43_sys_getdirentries_args), 0,
	    (sy_call_t *)compat_43_sys_getdirentries },/* 159 = getdirentries */
	{ ns(struct ultrix_sys_statfs_args), 0,
	    (sy_call_t *)ultrix_sys_statfs },	/* 160 = statfs */
	{ ns(struct ultrix_sys_fstatfs_args), 0,
	    (sy_call_t *)ultrix_sys_fstatfs },	/* 161 = fstatfs */
	{ 0, 0, 0,
	    sys_nosys },			/* 162 = unimplemented umount */
#ifdef NFS
	{ 0, 0, 0,
	    (sy_call_t *)async_daemon },	/* 163 = async_daemon */
	{ ns(struct compat_30_sys_getfh_args), 0,
	    (sy_call_t *)compat_30_sys_getfh },	/* 164 = getfh */
#else
	{ 0, 0, 0,
	    sys_nosys },			/* 163 = unimplemented async_daemon */
	{ 0, 0, 0,
	    sys_nosys },			/* 164 = unimplemented getfh */
#endif
	{ ns(struct compat_09_sys_getdomainname_args), 0,
	    (sy_call_t *)compat_09_sys_getdomainname },/* 165 = getdomainname */
	{ ns(struct compat_09_sys_setdomainname_args), 0,
	    (sy_call_t *)compat_09_sys_setdomainname },/* 166 = setdomainname */
	{ 0, 0, 0,
	    sys_nosys },			/* 167 = unimplemented */
	{ ns(struct ultrix_sys_quotactl_args), 0,
	    (sy_call_t *)ultrix_sys_quotactl },	/* 168 = quotactl */
	{ ns(struct ultrix_sys_exportfs_args), 0,
	    (sy_call_t *)ultrix_sys_exportfs },	/* 169 = exportfs */
	{ 0, 0, 0,
	    sys_nosys },			/* 170 = unimplemented { int ultrix_sys_mount ( char * special , char * dir , int rdonly , int type , void * data ) ; } */
	{ 0, 0, 0,
	    sys_nosys },			/* 171 = unimplemented 4 hdwconf */
	{ 0, 0, 0,
	    sys_nosys },			/* 172 = unimplemented msgctl */
	{ 0, 0, 0,
	    sys_nosys },			/* 173 = unimplemented msgget */
	{ 0, 0, 0,
	    sys_nosys },			/* 174 = unimplemented msgrcv */
	{ 0, 0, 0,
	    sys_nosys },			/* 175 = unimplemented msgsnd */
	{ 0, 0, 0,
	    sys_nosys },			/* 176 = unimplemented semctl */
	{ 0, 0, 0,
	    sys_nosys },			/* 177 = unimplemented semget */
	{ 0, 0, 0,
	    sys_nosys },			/* 178 = unimplemented semop */
	{ ns(struct ultrix_sys_uname_args), 0,
	    (sy_call_t *)ultrix_sys_uname },	/* 179 = uname */
	{ ns(struct ultrix_sys_shmsys_args), 0,
	    (sy_call_t *)ultrix_sys_shmsys },	/* 180 = shmsys */
	{ 0, 0, 0,
	    sys_nosys },			/* 181 = unimplemented 0 plock */
	{ 0, 0, 0,
	    sys_nosys },			/* 182 = unimplemented 0 lockf */
	{ ns(struct ultrix_sys_ustat_args), 0,
	    (sy_call_t *)ultrix_sys_ustat },	/* 183 = ustat */
	{ ns(struct ultrix_sys_getmnt_args), 0,
	    (sy_call_t *)ultrix_sys_getmnt },	/* 184 = getmnt */
	{ 0, 0, 0,
	    sys_nosys },			/* 185 = unimplemented notdef */
	{ 0, 0, 0,
	    sys_nosys },			/* 186 = unimplemented notdef */
	{ ns(struct ultrix_sys_sigpending_args), 0,
	    (sy_call_t *)ultrix_sys_sigpending },/* 187 = sigpending */
	{ 0, 0, 0,
	    (sy_call_t *)sys_setsid },		/* 188 = setsid */
	{ ns(struct ultrix_sys_waitpid_args), 0,
	    (sy_call_t *)ultrix_sys_waitpid },	/* 189 = waitpid */
	{ 0, 0, 0,
	    sys_nosys },			/* 190 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 191 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 192 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 193 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 194 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 195 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 196 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 197 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 198 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 199 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 200 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 201 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 202 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 203 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 204 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 205 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 206 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 207 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 208 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 209 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 210 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 211 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 212 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 213 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 214 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 215 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 216 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 217 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 218 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 219 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 220 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 221 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 222 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 223 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 224 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 225 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 226 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 227 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 228 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 229 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 230 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 231 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 232 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 233 = unimplemented 1 utc_gettime */
	{ 0, 0, 0,
	    sys_nosys },			/* 234 = unimplemented 2 utc_adjtime */
	{ 0, 0, 0,
	    sys_nosys },			/* 235 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 236 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 237 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 238 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 239 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 240 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 241 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 242 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 243 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 244 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 245 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 246 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 247 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 248 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 249 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 250 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 251 = unimplemented */
	{ 0, 0, 0,
	    sys_nosys },			/* 252 = unimplemented audctl / * Make no-op for installation on Ultrix rootpartition? * / */
	{ 0, 0, 0,
	    sys_nosys },			/* 253 = unimplemented audgen / * Make no-op for installation on Ultrix rootpartition? * / */
	{ 0, 0, 0,
	    sys_nosys },			/* 254 = unimplemented startcpu */
	{ 0, 0, 0,
	    sys_nosys },			/* 255 = unimplemented stopcpu */
	{ ns(struct ultrix_sys_getsysinfo_args), 0,
	    (sy_call_t *)ultrix_sys_getsysinfo },/* 256 = getsysinfo */
	{ ns(struct ultrix_sys_setsysinfo_args), 0,
	    (sy_call_t *)ultrix_sys_setsysinfo },/* 257 = setsysinfo */
	{ 0, 0, 0,
	    sys_nosys },			/* 258 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 259 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 260 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 261 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 262 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 263 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 264 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 265 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 266 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 267 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 268 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 269 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 270 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 271 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 272 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 273 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 274 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 275 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 276 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 277 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 278 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 279 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 280 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 281 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 282 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 283 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 284 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 285 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 286 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 287 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 288 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 289 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 290 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 291 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 292 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 293 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 294 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 295 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 296 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 297 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 298 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 299 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 300 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 301 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 302 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 303 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 304 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 305 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 306 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 307 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 308 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 309 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 310 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 311 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 312 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 313 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 314 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 315 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 316 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 317 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 318 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 319 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 320 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 321 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 322 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 323 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 324 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 325 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 326 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 327 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 328 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 329 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 330 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 331 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 332 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 333 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 334 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 335 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 336 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 337 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 338 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 339 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 340 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 341 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 342 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 343 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 344 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 345 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 346 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 347 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 348 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 349 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 350 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 351 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 352 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 353 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 354 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 355 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 356 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 357 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 358 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 359 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 360 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 361 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 362 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 363 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 364 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 365 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 366 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 367 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 368 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 369 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 370 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 371 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 372 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 373 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 374 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 375 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 376 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 377 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 378 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 379 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 380 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 381 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 382 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 383 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 384 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 385 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 386 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 387 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 388 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 389 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 390 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 391 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 392 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 393 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 394 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 395 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 396 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 397 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 398 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 399 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 400 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 401 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 402 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 403 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 404 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 405 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 406 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 407 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 408 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 409 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 410 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 411 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 412 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 413 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 414 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 415 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 416 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 417 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 418 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 419 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 420 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 421 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 422 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 423 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 424 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 425 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 426 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 427 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 428 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 429 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 430 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 431 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 432 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 433 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 434 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 435 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 436 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 437 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 438 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 439 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 440 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 441 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 442 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 443 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 444 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 445 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 446 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 447 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 448 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 449 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 450 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 451 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 452 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 453 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 454 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 455 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 456 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 457 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 458 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 459 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 460 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 461 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 462 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 463 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 464 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 465 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 466 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 467 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 468 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 469 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 470 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 471 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 472 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 473 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 474 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 475 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 476 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 477 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 478 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 479 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 480 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 481 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 482 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 483 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 484 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 485 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 486 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 487 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 488 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 489 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 490 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 491 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 492 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 493 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 494 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 495 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 496 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 497 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 498 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 499 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 500 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 501 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 502 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 503 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 504 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 505 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 506 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 507 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 508 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 509 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 510 = filler */
	{ 0, 0, 0,
	    sys_nosys },			/* 511 = filler */
};

