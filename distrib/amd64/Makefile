#	$NetBSD: Makefile,v 1.4 2007/03/06 21:52:44 bouyer Exp $

SUBDIR=
SUBDIR+=	ramdisks
SUBDIR+=	.WAIT
SUBDIR+=	instkernel
SUBDIR+=	.WAIT
SUBDIR+=	cdroms
SUBDIR+=	floppies
TARGETS+=	release

iso_image:
	${MAKEDIRTARGET} cdroms iso_image

.include <bsd.subdir.mk>
