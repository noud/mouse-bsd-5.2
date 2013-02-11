# $NetBSD: t_readdir.sh,v 1.2 2008/04/30 13:11:00 martin Exp $
#
# Copyright (c) 2005, 2006, 2007 The NetBSD Foundation, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

#
# Verifies that the readdir operation works.
#

atf_test_case dots
dots_head() {
	atf_set "descr" "Verifies that readdir returns the '.' and '..'" \
	                "entries"
	atf_set "require.user" "root"
}
dots_body() {
	test_mount

	atf_check '/bin/ls -a' 0 stdout null
	atf_check "grep '^\.$' ../stdout" 0 ignore null
	atf_check "grep '^\..$' ../stdout" 0 ignore null

	test_unmount
}

atf_test_case types
types_head() {
	atf_set "descr" "Verifies that readdir works for all different" \
	                "file types"
	atf_set "require.user" "root"
}
types_body() {
	test_mount

	atf_check 'mkdir dir' 0 null null
	atf_check 'touch reg' 0 null null
	atf_check 'ln -s reg lnk' 0 null null
	atf_check 'mknod blk b 0 0' 0 null null
	atf_check 'mknod chr c 0 0' 0 null null
	atf_check 'mknod fifo p' 0 null null
	atf_check "$(atf_get_srcdir)/h_tools sockets sock" 0 null null

	atf_check 'ls' 0 ignore null
	atf_check 'rm -rf *' 0 null null

	test_unmount
}

atf_test_case caching
caching_head() {
	atf_set "descr" "Catch a bug caused by incorrect invalidation of" \
	                "readdir caching variables"
	atf_set "require.user" "root"
}
caching_body() {
	test_mount

	atf_check 'touch $(jot 10)' 0 null null
	atf_check 'rm *' 0 null null
	atf_check 'touch $(jot 20)' 0 null null
	atf_check 'ls >/dev/null' 0 null null

	test_unmount
}

atf_test_case many
many_head() {
	atf_set "descr" "Verifies that readdir works with many files"
	atf_set "require.user" "root"
}
many_body() {
	test_mount

	atf_check 'mkdir a' 0 null null
	echo "Creating 500 files"
	for f in $(jot 500); do
		touch a/$f
	done
	atf_check 'rm a/*' 0 null null
	atf_check 'rmdir a' 0 null null

	test_unmount
}

atf_init_test_cases() {
	. $(atf_get_srcdir)/../h_funcs.subr
	. $(atf_get_srcdir)/h_funcs.subr

	atf_add_test_case dots
	atf_add_test_case types
	atf_add_test_case caching
	atf_add_test_case many
}
