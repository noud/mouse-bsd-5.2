#	$NetBSD: Makefile.inc,v 1.6 2007/07/24 10:49:27 pavel Exp $
#
#	etc.playstation2/Makefile.inc --
#		playstation2-specific etc Makefile targets
#

# If you change the list of distributed kernels, don't forget
# to update the release documentation in distrib/notes/common/contents

KERNEL_SETS=		GENERIC

BUILD_KERNELS=	 	RAMDISK

INSTALLATION_DIRS+=	installation/miniroot
