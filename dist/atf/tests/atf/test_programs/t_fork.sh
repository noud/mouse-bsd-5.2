#
# Automated Testing Framework (atf)
#
# Copyright (c) 2007, 2008 The NetBSD Foundation, Inc.
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
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
# CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# TODO: This test program is about checking the test case's "environment"
# (not the variables).  Should be named something else than t_fork.

atf_test_case mangle_fds
mangle_fds_head()
{
    atf_set "descr" "Tests that mangling standard descriptors does not" \
                    "affect the test case's reporting of status"
}
mangle_fds_body()
{
    for h in $(get_helpers); do
        atf_check "${h} -s $(atf_get_srcdir) -r3 -v resfd=3 \
                   fork_mangle_fds 3>resout" 0 ignore ignore
        atf_check "grep 'passed' resout" 0 ignore null
    done
}

atf_test_case stop
stop_head()
{
    atf_set "descr" "Tests that sending a stop signal to a test case does" \
                    "not report it as failed"
}
stop_body()
{
    for h in $(get_helpers); do
        ${h} -s $(atf_get_srcdir) -v pidfile=$(pwd)/pid \
             -v donefile=$(pwd)/done -r3 fork_stop 3>resout &
        ppid=${!}
        echo "Waiting for pid file for test program ${ppid}"
        while test ! -f pid; do sleep 1; done
        pid=$(cat pid)
        echo "Test case's pid is ${pid}"
        kill -STOP ${pid}
        touch done
        echo "Wrote done file"
        kill -CONT ${pid}
        wait ${ppid}
        atf_check "grep 'fork_stop, passed' resout" 0 ignore null
        rm -f pid done
    done
}

atf_test_case umask
umask_head()
{
    atf_set "descr" "Tests that the umask is properly set in the test" \
                    "cases"
}
umask_body()
{
    echo 0022 >expout
    atf_check 'umask' 0 expout null

    for h in $(get_helpers); do
        umask 0000
        atf_check "${h} -s $(atf_get_srcdir) -r3 fork_umask 3>resout" \
                  0 stdout ignore
        atf_check "grep 'umask: 0022' stdout" 0 ignore null
        atf_check "grep 'passed' resout" 0 ignore null
        umask 0022
    done
}

atf_init_test_cases()
{
    atf_add_test_case mangle_fds
    atf_add_test_case stop
    atf_add_test_case umask
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4
