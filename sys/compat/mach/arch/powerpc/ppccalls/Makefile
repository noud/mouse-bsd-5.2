#	$NetBSD: Makefile,v 1.4 2005/12/11 12:20:22 christos Exp $

.include <bsd.sys.mk>		# for HOST_SH

DEP=	syscalls.conf syscalls.master ../../../../../kern/makesyscalls.sh
OBJS=	mach_ppccalls_sysent.c mach_ppccalls_syscalls.c \
	mach_ppccalls_syscall.h mach_ppccalls_syscallargs.h

${OBJS}: ${DEP}
	${HOST_SH} ../../../../../kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}

.include <bsd.kinc.mk>
