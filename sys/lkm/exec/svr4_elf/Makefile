#	$NetBSD: Makefile,v 1.8 2007/05/12 10:45:01 dsl Exp $

.include "../Makefile.inc"

.if ${MACHINE} == sparc64 && ${MACHINE_ARCH} == sparc64
ELFSIZE=64
.else
ELFSIZE=32
.endif

.PATH:	$S/compat/svr4

CPPFLAGS+=	-nostdinc -D_KERNEL -DEXEC_ELF${ELFSIZE} \
		-DELFSIZE=${ELFSIZE}

KMOD=	exec_svr4_elf

SRCS=	lkminit_exec.c
SRCS+=	svr4_exec_elf${ELFSIZE}.c

.include <bsd.kmod.mk>
