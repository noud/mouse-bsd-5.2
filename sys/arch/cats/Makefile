#	$NetBSD: Makefile,v 1.3 2008/10/25 22:27:37 apb Exp $

# Makefile for cats tags file and boot blocks

TCATS=	../cats/tags
SCATS=	../cats/cats/*.[ch] ../cats/include/*.h \
		../cats/isa/*.[ch]
ACATS=	../cats/cats/*.S

# Directories in which to place tags links
DCATS=	isa include

.include "../../kern/Make.tags.inc"

tags:
	-ctags -wdtf ${TCATS} ${SCATS} ${COMM}
	egrep "^ENTRY(.*)|^ALTENTRY(.*)" ${ACATS} | \
	    ${TOOL_SED} -e \
		"s;\([^:]*\):\([^(]*\)(\([^, )]*\)\(.*\);\3 \1 /^\2(\3\4$$/;" \
	    >> ${TCATS}
	sort -o ${TCATS} ${TCATS}

links:
	-for i in ${DCATS}; do \
	    cd $$i && rm -f tags; ln -s ../tags tags; done


SUBDIR=	compile include

.include <bsd.subdir.mk>
