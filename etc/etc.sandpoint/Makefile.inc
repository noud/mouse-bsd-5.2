#	$NetBSD: Makefile.inc,v 1.10 2007/10/22 06:46:14 nisimura Exp $
#
#	etc.sandpoint/Makefile.inc -- sandpoint-specific etc Makefile targets
#

# If you change the list of distributed kernels, don't forget
# to update the release documentation in distrib/notes/common/contents

KERNEL_SETS=		GENERIC
KERNEL_SUFFIXES=	srec

BUILD_KERNELS=  	INSTALL
