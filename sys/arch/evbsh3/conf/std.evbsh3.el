# $NetBSD: std.evbsh3.el,v 1.7 2008/02/02 18:28:33 uwe Exp $
#
# standard, required NetBSD/evbsh3 'options'

machine evbsh3 sh3
include		"conf/std"	# MI standard options
include		"arch/sh3/conf/std.sh3el"	# arch standard options

makeoptions	DEFTEXTADDR="0x8c010000"
