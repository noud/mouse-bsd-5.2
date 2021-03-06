#	$NetBSD: Makefile.inc,v 1.22 2003/10/26 07:25:33 lukem Exp $

# TOP is assumed to be defined by Makefile including this one.

CBIN=		instbin
COMMONDIR=	${TOP}/inst-common

MOUNT_POINT?=	/mnt
VND?=		vnd0
VND_DEV=	/dev/${VND}c
VND_RDEV=	/dev/r${VND}c
IMAGE?=		xxx-${DISTRIBREV}.fs
MDEC=		${DESTDIR}/usr/mdec

LISTS=		${COMMONDIR}/list ${.CURDIR}/list
CRUNCHCONF=	${COMMONDIR}/${CBIN}.conf
MTREECONF=	${COMMONDIR}/mtree.conf

DISKTYPE=	miniroot

all: ${CBIN}
	dd if=/dev/zero of=${IMAGE} bs=128k count=32
	vnconfig -t ${DISKTYPE} -v -c ${VND_DEV} ${IMAGE}
	installboot ${MDEC}/xxboot ${VND_RDEV}
	newfs -B be -c 64 -m 0 -o space ${VND_RDEV} ${DISKTYPE}
	mount -t ffs ${VND_DEV} ${MOUNT_POINT}
	${TOOL_MTREE} -def ${MTREECONF} -p ${MOUNT_POINT}/ -u
	TOPDIR=${TOP} CURDIR=${.CURDIR} OBJDIR=${.OBJDIR} \
	    KERNOBJDIR=${KERNOBJDIR} \
	    TARGDIR=${MOUNT_POINT} ${HOST_SH} ${TOP}/runlist.sh ${LISTS}
	@echo ""
	@df -i ${MOUNT_POINT}
	@echo ""
	umount ${MOUNT_POINT}
	vnconfig -u ${VND_DEV}

unconfig:
	-umount -f ${MOUNT_POINT}
	-vnconfig -u ${VND_DEV}
	-/bin/rm -f ${IMAGE}

${CBIN}.mk ${CBIN}.cache ${CBIN}.c: ${CRUNCHCONF}
	${TOOL_CRUNCHGEN} -D ${TOP}/../../.. -L ${DESTDIR}/usr/lib ${CRUNCHCONF}

${CBIN}: ${CBIN}.mk ${CBIN}.cache ${CBIN}.c
	${MAKE} -f ${CBIN}.mk all

clean cleandir distclean:
	/bin/rm -f core ${IMAGE} ${CBIN} ${CBIN}.mk ${CBIN}.cache *.o *.cro *.c

.include <bsd.obj.mk>
.include <bsd.subdir.mk>
