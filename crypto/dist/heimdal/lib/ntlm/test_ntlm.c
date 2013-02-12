/*
 * Copyright (c) 2006 - 2007 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of KTH nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <stdio.h>
#include <err.h>
#include <roken.h>
#include <getarg.h>

__RCSID("$Heimdal: test_ntlm.c 22377 2007-12-28 18:38:53Z lha $"
        "$NetBSD: test_ntlm.c,v 1.1 2008/03/22 09:42:42 mlelstv Exp $");

#include <krb5.h>
#include <heimntlm.h>

static int
test_parse(void)
{
    const char *user = "foo",
	*domain = "mydomain",
	*password = "digestpassword",
	*target = "DOMAIN";
    struct ntlm_type1 type1;
    struct ntlm_type2 type2;
    struct ntlm_type3 type3;
    struct ntlm_buf data;
    krb5_error_code ret;
    int flags;

    memset(&type1, 0, sizeof(type1));

    type1.flags = NTLM_NEG_UNICODE|NTLM_NEG_TARGET|NTLM_NEG_NTLM;
    type1.domain = rk_UNCONST(domain);
    type1.hostname = NULL;
    type1.os[0] = 0;
    type1.os[1] = 0;

    ret = heim_ntlm_encode_type1(&type1, &data);
    if (ret)
	errx(1, "heim_ntlm_encode_type1");

    memset(&type1, 0, sizeof(type1));

    ret = heim_ntlm_decode_type1(&data, &type1);
    free(data.data);
    if (ret)
	errx(1, "heim_ntlm_encode_type1");

    heim_ntlm_free_type1(&type1);

    /*
     *
     */

    memset(&type2, 0, sizeof(type2));

    flags = NTLM_NEG_UNICODE | NTLM_NEG_NTLM | NTLM_TARGET_DOMAIN;
    type2.flags = flags;

    memset(type2.challange, 0x7f, sizeof(type2.challange));
    type2.targetname = rk_UNCONST(target);
    type2.targetinfo.data = NULL;
    type2.targetinfo.length = 0;

    ret = heim_ntlm_encode_type2(&type2, &data);
    if (ret)
	errx(1, "heim_ntlm_encode_type2");

    memset(&type2, 0, sizeof(type2));

    ret = heim_ntlm_decode_type2(&data, &type2);
    free(data.data);
    if (ret)
	errx(1, "heim_ntlm_decode_type2");

    heim_ntlm_free_type2(&type2);

    /*
     *
     */

    memset(&type3, 0, sizeof(type3));

    type3.flags = flags;
    type3.username = rk_UNCONST(user);
    type3.targetname = rk_UNCONST(target);
    type3.ws = rk_UNCONST("workstation");

    {
	struct ntlm_buf key;
	heim_ntlm_nt_key(password, &key);

	heim_ntlm_calculate_ntlm1(key.data, key.length,
				  type2.challange,
				  &type3.ntlm);
	free(key.data);
    }

    ret = heim_ntlm_encode_type3(&type3, &data);
    if (ret)
	errx(1, "heim_ntlm_encode_type3");

    free(type3.ntlm.data);

    memset(&type3, 0, sizeof(type3));

    ret = heim_ntlm_decode_type3(&data, 1, &type3);
    free(data.data);
    if (ret)
	errx(1, "heim_ntlm_decode_type3");

    if (strcmp("workstation", type3.ws) != 0)
	errx(1, "type3 ws wrong");

    if (strcmp(target, type3.targetname) != 0)
	errx(1, "type3 targetname wrong");

    if (strcmp(user, type3.username) != 0)
	errx(1, "type3 username wrong");


    heim_ntlm_free_type3(&type3);

    /*
     * NTLMv2
     */

    memset(&type2, 0, sizeof(type2));

    flags = NTLM_NEG_UNICODE | NTLM_NEG_NTLM | NTLM_TARGET_DOMAIN;
    type2.flags = flags;

    memset(type2.challange, 0x7f, sizeof(type2.challange));
    type2.targetname = rk_UNCONST(target);
    type2.targetinfo.data = "\x00\x00";
    type2.targetinfo.length = 2;

    ret = heim_ntlm_encode_type2(&type2, &data);
    if (ret)
	errx(1, "heim_ntlm_encode_type2");

    memset(&type2, 0, sizeof(type2));

    ret = heim_ntlm_decode_type2(&data, &type2);
    free(data.data);
    if (ret)
	errx(1, "heim_ntlm_decode_type2");

    heim_ntlm_free_type2(&type2);

    return 0;
}

static int
test_keys(void)
{
    const char
	*username = "test",
	*password = "test1234",
	*target = "TESTNT";
    const unsigned char
	serverchallange[8] = "\x67\x7f\x1c\x55\x7a\x5e\xe9\x6c";
    struct ntlm_buf infotarget, infotarget2, answer, key;
    unsigned char ntlmv2[16], ntlmv2_1[16];
    int ret;

    infotarget.length = 70;
    infotarget.data =
	"\x02\x00\x0c\x00\x54\x00\x45\x00\x53\x00\x54\x00\x4e\x00\x54\x00"
	"\x01\x00\x0c\x00\x4d\x00\x45\x00\x4d\x00\x42\x00\x45\x00\x52\x00"
	"\x03\x00\x1e\x00\x6d\x00\x65\x00\x6d\x00\x62\x00\x65\x00\x72\x00"
	    "\x2e\x00\x74\x00\x65\x00\x73\x00\x74\x00\x2e\x00\x63\x00\x6f"
	    "\x00\x6d\x00"
	"\x00\x00\x00\x00";

    answer.length = 0;
    answer.data = NULL;

    heim_ntlm_nt_key(password, &key);

    ret = heim_ntlm_calculate_ntlm2(key.data,
				    key.length,
				    username,
				    target,
				    serverchallange,
				    &infotarget,
				    ntlmv2,
				    &answer);
    if (ret)
	errx(1, "heim_ntlm_calculate_ntlm2");

    ret = heim_ntlm_verify_ntlm2(key.data,
				 key.length,
				 username,
				 target,
				 0,
				 serverchallange,
				 &answer,
				 &infotarget2,
				 ntlmv2_1);
    if (ret)
	errx(1, "heim_ntlm_verify_ntlm2");

    if (memcmp(ntlmv2, ntlmv2_1, sizeof(ntlmv2)) != 0)
	errx(1, "ntlm master key not same");

    if (infotarget.length > infotarget2.length)
	errx(1, "infotarget length");

    if (memcmp(infotarget.data, infotarget2.data, infotarget.length) != 0)
	errx(1, "infotarget not the same");

    free(key.data);
    free(answer.data);
    free(infotarget2.data);

    return 0;
}

static int
test_ntlm2_session_resp(void)
{
    int ret;
    struct ntlm_buf lm, ntlm;

    const unsigned char lm_resp[24] =
	"\xff\xff\xff\x00\x11\x22\x33\x44"
	"\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00";
    const unsigned char ntlm2_sess_resp[24] =
	"\x10\xd5\x50\x83\x2d\x12\xb2\xcc"
	"\xb7\x9d\x5a\xd1\xf4\xee\xd3\xdf"
	"\x82\xac\xa4\xc3\x68\x1d\xd4\x55";

    const unsigned char client_nonce[8] =
	"\xff\xff\xff\x00\x11\x22\x33\x44";
    const unsigned char server_challange[8] =
	"\x01\x23\x45\x67\x89\xab\xcd\xef";

    const unsigned char ntlm_hash[16] =
	"\xcd\x06\xca\x7c\x7e\x10\xc9\x9b"
	"\x1d\x33\xb7\x48\x5a\x2e\xd8\x08";

    ret = heim_ntlm_calculate_ntlm2_sess(client_nonce,
					 server_challange,
					 ntlm_hash,
					 &lm,
					 &ntlm);
    if (ret)
	errx(1, "heim_ntlm_calculate_ntlm2_sess_resp");

    if (lm.length != 24 || memcmp(lm.data, lm_resp, 24) != 0)
	errx(1, "lm_resp wrong");
    if (ntlm.length != 24 || memcmp(ntlm.data, ntlm2_sess_resp, 24) != 0)
	errx(1, "ntlm2_sess_resp wrong");

    free(lm.data);
    free(ntlm.data);


    return 0;
}

static int version_flag = 0;
static int help_flag	= 0;

static struct getargs args[] = {
    {"version",	0,	arg_flag,	&version_flag, "print version", NULL },
    {"help",	0,	arg_flag,	&help_flag,  NULL, NULL }
};

static void
usage (int ret)
{
    arg_printusage (args, sizeof(args)/sizeof(*args),
		    NULL, "");
    exit (ret);
}

int
main(int argc, char **argv)
{
    int ret = 0, optind = 0;

    setprogname(argv[0]);

    if(getarg(args, sizeof(args) / sizeof(args[0]), argc, argv, &optind))
	usage(1);

    if (help_flag)
	usage (0);

    if(version_flag){
	print_version(NULL);
	exit(0);
    }

    argc -= optind;
    argv += optind;

    printf("test_parse\n");
    ret += test_parse();
    printf("test_keys\n");
    ret += test_keys();
    printf("test_ntlm2_session_resp\n");
    ret += test_ntlm2_session_resp();

    return 0;
}
