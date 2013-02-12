/*
 * Copyright (c) 2005 Kungliga Tekniska H�gskolan
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
__RCSID("$Heimdal: destest.c 15269 2005-05-29 16:14:38Z lha $"
        "$NetBSD: destest.c,v 1.1 2008/03/22 09:39:27 mlelstv Exp $");
#endif

#ifdef KRB5
#include <krb5-types.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <err.h>

#include "des.h"

static void
ecb_test(char key[8], char in[8], char out[8])
{
    unsigned char k[8], indata[8], outdata[8], outdata2[8], ansdata[8];
    DES_key_schedule s;

    memcpy(k, key, 8);
    DES_set_odd_parity(&k);
    memcpy(indata, in, 8);
    memcpy(ansdata, out, 8);
    DES_set_key(&k, &s);
    DES_ecb_encrypt(&indata, &outdata, &s, 1);
    if (memcmp(outdata, ansdata, sizeof(ansdata)) != 0)
	errx(1, "des: encrypt");
    DES_ecb_encrypt(&outdata, &outdata2, &s, 0);
    if (memcmp(indata, outdata2, sizeof(outdata2)) != 0)
	errx(1, "des: decrypt");
}

static void
ebc3_test(char key1[8], char key2[8], char key3[8], char in[8], char out[8])
{
    unsigned char k1[8], k2[8], k3[8],
	indata[8], outdata[8], outdata2[8], ansdata[8];
    DES_key_schedule s1, s2, s3;

    memcpy(k1, key1, 8);
    memcpy(k2, key2, 8);
    memcpy(k3, key3, 8);
    memcpy(indata, in, 8);
    memcpy(ansdata, out, 8);
    DES_set_key(&k1, &s1);
    DES_set_key(&k2, &s2);
    DES_set_key(&k3, &s3);
    DES_ecb3_encrypt(&indata, &outdata, &s1, &s2, &s3, 1);
    if (memcmp(outdata, ansdata, sizeof(ansdata)) != 0)
	errx(1, "des3: encrypt");
    DES_ecb3_encrypt(&outdata, &outdata2, &s1, &s2, &s3, 0);
    if (memcmp(indata, outdata2, sizeof(outdata2)) != 0)
	errx(1, "des3: decrypt");
}

static void
cbc_test(char key1[8], char iv[8], char in[24], char out[24])
{
    unsigned char k1[8],
	indata[24], outdata[24], outdata2[24], ansdata[24];
    DES_key_schedule s1;
    DES_cblock ivdata;

    memcpy(k1, key1, 8);
    memcpy(ivdata, iv, 8);
    memcpy(indata, in, 24);
    memcpy(ansdata, out, 24);
    DES_set_key(&k1, &s1);
    DES_cbc_encrypt(indata, outdata, 24, &s1, &ivdata, 1);
    if (memcmp(outdata, ansdata, sizeof(ansdata)) != 0)
	errx(1, "cbc: encrypt");
    DES_cbc_encrypt(outdata, outdata2, 24, &s1, &ivdata, 0);
    if (memcmp(indata, outdata2, sizeof(outdata2)) != 0)
	errx(1, "cbc: decrypt");
}

static void
cfb64_test(char key1[8], char iv[8], char in[23], char out[23])
{
    unsigned char k1[8],
	indata[23], outdata[23], outdata2[23], ansdata[23];
    DES_key_schedule s1;
    DES_cblock ivdata;
    int num;

    memcpy(k1, key1, 8);
    memcpy(indata, in, 23);
    memcpy(ansdata, out, 23);
    DES_set_key(&k1, &s1);
    num = 0;
    memcpy(ivdata, iv, 8);
    DES_cfb64_encrypt(indata, outdata, 23, &s1, &ivdata, &num, 1);
    if (memcmp(outdata, ansdata, sizeof(ansdata)) != 0)
	errx(1, "cfb64: encrypt");
    num = 0;
    memcpy(ivdata, iv, 8);
    DES_cfb64_encrypt(outdata, outdata2, 23, &s1, &ivdata, &num, 0);
    if (memcmp(indata, outdata2, sizeof(outdata2)) != 0)
	errx(1, "cfb64: decrypt");
}

static void
cbc3_test(char key1[8], char key2[8], char key3[8],
	  char iv[8], char in[24], char out[24])
{
    unsigned char k1[8], k2[8], k3[8],
	indata[24], outdata[24], outdata2[24], ansdata[24];
    DES_key_schedule s1, s2, s3;
    DES_cblock ivdata, ivec_copy;

    memcpy(k1, key1, 8);
    memcpy(k2, key2, 8);
    memcpy(k3, key3, 8);
    memcpy(ivdata, iv, 8);
    memcpy(indata, in, 24);
    memcpy(ansdata, out, 24);
    DES_set_key(&k1, &s1);
    DES_set_key(&k2, &s2);
    DES_set_key(&k3, &s3);
    memcpy(&ivec_copy, &ivdata, sizeof(ivec_copy));
    DES_ede3_cbc_encrypt(indata, outdata, 24,
			 &s1, &s2, &s3, &ivec_copy, 1);
    if (memcmp(outdata, ansdata, sizeof(ansdata)) != 0)
	errx(1, "cbc3: encrypt");
    memcpy(&ivec_copy, &ivdata, sizeof(ivec_copy));
    DES_ede3_cbc_encrypt(outdata, outdata2, 24,
			 &s1, &s2, &s3, &ivec_copy, 0);
    if (memcmp(indata, outdata2, sizeof(outdata2)) != 0)
	errx(1, "cbc3: decrypt");
}


static void
pcbc_test(char key1[8], char iv[8], char in[24], char out[24])
{
    unsigned char k1[8],
	indata[24], outdata[24], outdata2[24], ansdata[24];
    DES_key_schedule s1;
    DES_cblock ivdata;

    memcpy(k1, key1, 8);
    memcpy(ivdata, iv, 8);
    memcpy(indata, in, 24);
    memcpy(ansdata, out, 24);
    DES_set_key(&k1, &s1);
    DES_pcbc_encrypt(indata, outdata, 24, &s1, &ivdata, 1);
    if (memcmp(outdata, ansdata, sizeof(ansdata)) != 0)
	errx(1, "pcbc: encrypt");
    DES_pcbc_encrypt(outdata, outdata2, 24, &s1, &ivdata, 0);
    if (memcmp(indata, outdata2, sizeof(outdata2)) != 0)
	errx(1, "pcbc: decrypt");
}

static void
cbc_cksum(char key1[8], char iv[8], char *in, size_t len,
	  uint32_t ret, char out[8])
{
    unsigned char k1[8], indata[24], ansdata[8];
    DES_key_schedule s1;
    DES_cblock ivdata, outdata;
    uint32_t r;

    memcpy(k1, key1, 8);
    memcpy(ivdata, iv, 8);
    memcpy(indata, in, len);
    memcpy(ansdata, out, 8);
    DES_set_key(&k1, &s1);
    r = DES_cbc_cksum(indata, &outdata, len, &s1, &ivdata);
    if (ret != r)
	errx(1, "cbc_cksum: cksum error");
    if (memcmp(outdata, ansdata, sizeof(ansdata)) != 0)
	errx(1, "cbc_cksum: checksum");
}

static void
s2k(char *password, const char *salt, char akey[8])
{
    DES_cblock k;
    size_t l = strlen(password) + strlen(salt);
    char *pw = malloc(l + 1);
    strcpy(pw, password);
    strcat(pw, salt);

    DES_string_to_key(pw, &k);
    if (memcmp(akey, &k, 8) != 0)
	errx(1, "key wrong for '%s'", pw);
    free(pw);
}

/*
 *
 */

int
main(int argc, char **argv)
{
    _DES_ipfp_test();

    ecb_test("\x31\x16\xe3\x57\x97\xa8\x68\xe5",
	     "\xbb\xe4\x48\x6e\xdf\x9a\x05\x4f",
	     "\xa8\x82\xa0\x15\x76\xeb\xfd\xc7");
    ecb_test("\xfe\x4a\x19\xa1\x45\xa7\xb9\xd0",
	     "\x2a\x67\x3c\x07\x59\x4d\xde\xb8",
	     "\x9d\x61\xd5\x1c\xd7\xd0\xd3\x8b");
    ecb_test("\xbf\x13\x25\xec\xa4\xbc\x1a\x54",
	     "\x16\xa5\xd9\x30\x0f\x55\x20\x71",
	     "\x04\x44\x6c\xe0\x32\x32\x78\xd2");

    ebc3_test("\x7c\x2f\x79\xd5\xb5\x37\x01\xcb",
	      "\xb9\xbc\x86\xea\x04\x45\xab\x2c",
	      "\x19\x1c\xcd\x83\x8a\x29\x97\x3e",
	      "\x87\x03\x59\xdd\xf4\xc6\xeb\xb7",
	      "\xcc\x72\x66\x85\xed\xa2\xee\x09");
    ebc3_test("\x10\x34\x32\x4c\xc4\x9b\x57\x5b",
	      "\xb0\x6e\xb6\x26\xd6\x52\x2c\x15",
	      "\xa7\x64\xf8\x20\xc1\x89\x73\xc1",
	      "\x37\xa4\xad\x4d\x76\xee\x7c\x02",
	      "\xdf\xb9\x2b\x99\x59\x71\xc4\x89");
    ebc3_test("\xf8\xa7\xfd\xe6\x6d\x73\x34\x26",
	      "\x4c\xbf\x40\x5d\x5d\xf4\x31\xef",
	      "\x04\xdf\xf2\x58\xd0\x5e\x54\x68",
	      "\x44\x2a\xa2\x19\xbd\x0a\x2b\x61",
	      "\x17\x26\x39\xd5\xd5\xd9\x40\x71");
    ebc3_test("\x13\x5e\x23\x07\x2c\x16\x0d\x25",
	      "\x64\x6d\x2f\xe0\x68\xa8\x16\x75",
	      "\x7c\x7c\x19\x64\xbc\xae\xe0\x0e",
	      "\x7b\x8c\x76\x76\xb0\x95\x7f\xed",
	      "\xe2\x6e\x05\x1d\xdc\x74\xc1\xb7");
    ebc3_test("\xbc\x92\x32\xb6\x68\x0d\x73\x19",
	      "\x70\xef\x98\x19\xe9\xec\x04\x1c",
	      "\x02\x4c\x75\x08\xce\xc4\x34\x16",
	      "\x73\xab\x28\x69\x6a\x20\x2f\x99",
	      "\x3b\xb1\x2d\xb6\x21\x0a\x44\xca");
    ebc3_test("\x01\x98\x16\xea\x85\xd5\x3b\x8a",
	      "\x73\x23\xb5\x49\xd9\x10\x5b\xea",
	      "\xb6\xc4\xce\xc4\x89\x92\x0e\x15",
	      "\xd9\x35\xcf\x21\x47\x7b\xdf\xb5",
	      "\xa1\x71\x57\x1f\x1e\x84\x08\xac");
    ebc3_test("\x58\x6d\xbc\x04\x70\x4f\xe6\x3e",
	      "\xcd\x76\x26\x01\xae\xce\x0b\xe5",
	      "\xf2\x4f\x64\x16\x8f\x0d\x4f\x6b",
	      "\xa7\x0d\xa0\x56\xa0\x8b\x2a\x77",
	      "\xe5\x12\x9b\x8a\x92\xc8\xdd\xe1");
    ebc3_test("\x40\xd6\xad\x43\x52\x23\xa7\xcd",
	      "\x04\x19\xae\x94\xce\x46\x31\xd3",
	      "\x45\x6e\x3b\xb5\x4f\x37\x5e\x9d",
	      "\xbd\xb0\x60\x75\x91\x02\x48\xf4",
	      "\xb5\xa1\xe6\x4b\x4e\xa3\x8c\x4b");
    ebc3_test("\x91\xab\x80\x9b\x97\xf4\x58\x5e",
	      "\xc2\x68\x46\x61\x9e\x04\xa1\x29",
	      "\xc7\xe5\x5b\x32\xcb\x43\xc8\xa4",
	      "\x31\x38\x90\x1c\xc8\x78\x12\x50",
	      "\xf8\x65\xae\xa1\xdf\x4e\xbf\xa8");

    cbc_test("\x57\x98\x7a\x8a\x29\x7c\xc1\xad",
	     "\xe1\x28\x69\x58\xd6\x91\x9f\x4e",
	     "\xa0\x11\x1a\xdd\xeb\x62\xb8\x9e\x28\x08\x6e\x0b\x6d\x6d\x57\x31\x1b\x4c\x82\x4c\xc3\x19\xe0\x93",
	     "\x42\xa5\x2f\x26\xbb\x92\x3a\x6b\x64\xe0\x3b\x1a\x33\x5a\x9c\x2b\xc8\xd9\x41\x37\x8d\x3e\x58\xbf");
    cbc_test("\x23\xd6\xec\x86\x86\x4f\x02\xcd",
	     "\xfe\x8e\xa4\x07\x35\x41\x14\x99",
	     "\xe3\xc2\x5d\x6e\x81\xae\xa0\xe8\xc8\xdd\xd2\x0d\xf4\x26\x90\x10\xca\x8c\x07\x58\xb2\x17\xcc\x1a",
	     "\x97\xb9\xbc\xa6\xd1\x98\xc1\x7f\x4b\xac\x61\x8a\x16\xec\x1f\xee\x28\x6f\xe8\x25\xf0\x41\xbc\xde");
    cbc_test("\x07\xe5\xc8\x52\xba\x3d\xef\xcd",
	     "\xa9\x21\x3e\x84\x44\x7c\xce\x1a",
	     "\xfc\x03\x72\x30\xb0\xcb\xe8\x99\x21\x54\x4d\xfa\x86\xdd\x99\xe1\x96\xe7\x7c\xb5\xbd\x5b\x6f\xd0",
	     "\x27\x76\x66\x62\x1f\xcf\x48\xdb\x15\x11\x73\x8b\xe0\xc9\xbd\x2b\x40\xae\x0c\x35\xeb\x93\xa3\x1c");
    cbc_test("\xef\x2f\x07\xd6\x2f\x70\x4f\x68",
	     "\x16\x1e\xaf\x87\x3a\x83\x9f\x33",
	     "\xb8\x4c\xb3\xbf\xfa\x5d\xa9\xc7\x1c\x15\x8d\x39\xf2\x29\xf5\x5a\x3d\x21\x0d\x61\x05\xaa\x48\x92",
	     "\x51\x85\x2f\xad\x67\xb6\x0a\x15\xb8\x73\x15\xf1\x79\x9d\xed\xf5\x6c\x11\x22\xe5\x48\x51\xab\xae");
    cbc_test("\xd0\x2c\x68\xc1\xe6\xb0\x76\x98",
	     "\xc7\x4f\x31\xa9\x5d\xd5\x5b\xcc",
	     "\x9d\x4b\x2a\x54\x60\xf1\xb0\x10\x34\x87\xdc\x25\xa5\x80\x6c\x4d\x0c\x7f\x53\x37\x58\x42\xc7\x26",
	     "\x79\xc5\xf0\x21\x0d\x7a\x38\xc0\x66\x9a\x07\x2f\xa4\x9c\x1f\xbb\x66\x4d\x6c\x86\x5b\x47\x44\x60");
    cbc_test("\xd6\xe3\x75\x92\xb0\x8f\x45\x70",
	     "\xdc\xc6\xab\x3e\xf2\x7e\x13\xd6",
	     "\x38\x57\x27\x0a\xef\x74\x94\x82\x92\xfa\x28\xed\xff\x24\x1e\x0e\x8f\xaa\x9e\x24\x2f\x41\x65\x78",
	     "\x1d\xcc\x07\x55\xe8\xea\xd1\x08\x55\x11\x72\xfe\xdb\xdf\xa0\xc9\xb6\x3a\x2e\xdf\xf0\x67\xd3\xf4");
    cbc_test("\xb3\xbc\xb5\x61\x04\xda\x1a\x34",
	     "\x8e\x4e\xa5\x8a\xeb\x6a\xea\xbb",
	     "\x72\x73\x51\xe0\x58\xc5\x2e\xe1\x64\x10\x05\x59\x64\x70\x3f\xbe\x43\xa2\xed\x7a\x5d\x1b\x9c\xc7",
	     "\xa6\xb2\xf2\xea\x96\x62\xfb\x2f\x2a\x6a\xa1\x2f\x8e\xe1\x12\xd2\xe4\x82\x4c\xc1\x00\x74\x9c\x8f");
    cbc_test("\x8f\xdf\x01\x89\xfe\x13\x9b\x2c",
	     "\x66\x18\xf8\x80\xa1\x3b\x1b\x91",
	     "\x32\xdb\xae\xa7\x3b\x77\xb2\x6e\xcc\xa5\xa1\x2e\x15\x19\x49\x83\x2f\xfb\x94\xcc\xd1\xa1\x4b\x02",
	     "\x47\x31\xca\x04\x4d\x1a\x24\x39\xda\x71\xc5\xb8\x7f\xea\x79\xf5\x43\xa6\x53\x15\x78\x84\x34\x75");
    cbc_test("\xe5\x34\xb6\x75\x68\x07\x70\x85",
	     "\x73\x98\x29\xf7\x7a\xe7\xe7\xb7",
	     "\x9c\x9e\x4c\xa6\x62\x21\xc4\x15\x47\x43\xd5\xf2\x3a\xf3\xfd\xb5\x53\xa7\x16\x9e\xa6\x4f\x0d\xac",
	     "\x81\x2d\xa4\x99\x60\xbf\x9c\xf4\x46\x1d\xee\xc6\xb0\xe1\x4a\x29\xea\xfd\xce\x4b\xa1\x45\x93\x7b");

    cbc3_test("\x61\xcb\x8c\xb0\x32\x2a\xc2\x5d",
	      "\x98\xe3\x49\xc1\x0d\xb5\x67\xce",
	      "\xf2\x43\x10\x61\x85\x6b\xa7\x15",
	      "\x65\xf5\x8f\x1a\x2b\x33\xf2\xb5",
	      "\x8c\x06\xe0\x60\x68\x25\x9c\x95\x81\x46\xda\x41\x9d\xa8\x9c\x49\x2f\xee\x33\x35\x95\x11\xbd\xa0",
	      "\x93\x27\xed\xc7\x35\xb9\xe5\x3c\x7b\x10\x3e\x39\x01\x41\x61\x04\xe7\xf2\xd9\x63\x96\xca\x57\xf1");
    cbc3_test("\x15\x61\x6b\x76\xae\x0e\x98\x01",
	      "\x76\xce\x9d\x94\xa7\xe3\x73\xa4",
	      "\x19\xd9\x15\x98\x9b\xba\x83\x40",
	      "\x60\xef\xc2\xc6\xa2\x40\x01\xc7",
	      "\x8b\x4d\xf4\x37\xad\x1c\xc2\x4e\xcc\xc4\x4b\x17\x67\xf7\xfa\xec\xf8\x94\x6f\x7a\x84\x56\x81\x09",
	      "\x68\xdf\x82\xcb\xd9\xcd\x3d\xca\x12\x0e\x2e\x39\xba\xf7\x5a\x8c\x41\xbd\x6f\x9d\x85\xfe\x1b\x1d");
    cbc3_test("\xd5\x2a\x4f\xa4\x13\x9e\x73\x15",
	      "\x6d\x75\xa8\x15\x07\xd3\x7c\x79",
	      "\xd5\xe0\xa7\x91\xf8\xf2\x9d\xcd",
	      "\x4c\xdb\x56\xb8\x6f\x0e\x2a\x59",
	      "\xbe\x64\x20\x24\x7d\x2b\x6b\xf4\xd9\xc0\xa0\x9b\x8d\x88\x6e\x50\x6f\xf8\xb6\x4a\x7e\x52\x52\x93",
	      "\x01\x83\x75\x7b\xd6\x03\xff\xd8\xe9\x6d\x6c\x92\x24\x25\x35\xfa\x43\x4c\x40\xff\xec\xb0\x8b\x50");
    cbc3_test("\x02\xad\x13\x31\xd5\xd6\xef\x7c",
	      "\x86\x3e\x02\xce\x94\x97\x37\xba",
	      "\x01\x07\x20\x04\xf8\x92\xb6\xb3",
	      "\x26\x79\x1b\xef\x90\x54\xd6\xc1",
	      "\x55\xee\xea\x81\x42\x8b\xbf\xfb\x6c\x14\xec\xbd\xba\x55\x0d\xc4\xd2\xd6\xf0\xea\xd1\x03\xde\x5b",
	      "\x69\x49\xc5\x48\x4f\xda\x03\x90\x84\xef\x86\xd2\x98\xa7\xae\xfa\x17\x35\x7e\x06\xbd\xd3\x51\x0b");
    cbc3_test("\x3d\x9b\xae\x5b\x7f\x91\x85\xe0",
	      "\xdf\x07\xb3\xdf\x97\x0b\x43\x80",
	      "\xe3\x46\x58\xd9\x68\x79\xb3\xae",
	      "\xd4\x27\xee\x5d\x73\xb1\x82\xf5",
	      "\x44\x86\x9a\xa6\x79\x2d\x9e\x94\x11\x6c\x7b\xc6\xe8\xef\x63\x95\x71\xc6\x62\x20\x43\x87\xaf\x65",
	      "\xc2\xf5\xbc\x91\xc5\x7c\x69\xb2\x05\xcc\x28\x92\xc1\x96\x5a\xc2\xcb\x0c\x71\xc7\x51\x7d\x0c\xcc");
    cbc3_test("\x43\x8c\x23\x92\xd5\x92\x67\xfb",
	      "\x5b\x5e\xb0\x31\x1c\x9d\x5d\x10",
	      "\x8a\xa2\x16\x64\xd6\xa4\xc4\x5b",
	      "\x06\xc5\xdd\xa3\x4a\x2b\x37\xb7",
	      "\x99\xd5\x76\xee\x7c\x4d\xcc\x18\x39\x78\x16\x7c\xcc\x1a\x0a\x27\xdb\xf1\x5f\xe1\x87\x86\xb7\x2c",
	      "\x91\xbe\xaf\x79\xd0\x14\x7c\x05\x60\x1c\x7e\xd6\x22\x15\xac\xed\xf3\x78\xa5\xc7\x52\xa0\x60\x49");
    cbc3_test("\x80\xc2\x86\x7a\x51\x45\x29\x1c",
	      "\xc7\xfd\xad\xd0\x7c\x4a\xd0\x3e",
	      "\xe6\x89\x98\xfe\x01\x67\x20\x89",
	      "\x5c\x23\xe4\x26\x82\x27\xad\xeb",
	      "\xa1\x38\x4e\xf1\x07\x1a\xdd\x25\x47\xe6\xda\x9d\xa9\xfe\x98\x55\x05\x95\x75\xc2\x59\x18\xcf\xf1",
	      "\x36\x58\xea\xc5\xf8\x41\xa7\x49\xe8\x22\x75\xfe\xb6\x8b\xdd\x0d\xf0\x66\x42\xe6\x84\x23\x29\xff");
    cbc3_test("\xbc\x68\x54\x85\x2c\xc1\xe0\x07",
	      "\x7c\x6e\x34\x04\x6b\x91\xc4\x54",
	      "\x9d\xa4\xda\xa1\xda\x6d\xdc\xd3",
	      "\x1c\x3d\xa9\x41\xa2\xe5\xff\x8a",
	      "\x0a\x58\xff\x5a\xec\xc1\x7e\x94\x24\xf4\x4f\xdc\x5b\x29\xe2\x78\x62\x8a\xd2\xe2\xd7\x45\x54\x17",
	      "\x80\x68\xa6\xed\x87\x40\xd5\x32\xd2\xb8\x32\x61\x35\xae\xae\xf7\x14\x1f\x98\xdb\xba\x21\x4f\x9f");
    cbc3_test("\xa1\x2a\x7a\x67\xfe\xea\xd3\xe3",
	      "\x70\xe5\xd5\x4c\xf1\xce\x4c\x26",
	      "\x75\x4c\x85\x16\xb5\xc8\x07\xe9",
	      "\x4c\xa4\xb5\xdd\x86\x86\x70\x5a",
	      "\x0d\x07\xfd\x23\xc1\x1d\x65\xd8\xb2\x79\xb8\xa3\xc5\x8e\x47\xbe\x0f\xed\x7b\x15\x43\xe9\x7c\x5e",
	      "\xde\x17\xfe\x05\x43\x80\x85\xd0\x9c\x60\xe0\xbe\x8d\xa2\x65\x0e\x63\x02\x72\xb6\xf3\x7d\xda\x90");


    pcbc_test("\xe3\xf2\xb0\x26\x7c\x4a\x94\x80",
	      "\x40\x08\x4c\x44\xa3\xb5\xf7\x97",
	      "\xe7\xbd\x54\xa1\xbb\x48\x67\xcd\xe0\xee\xff\x8d\x3d\x25\x2b\xf0\x61\x48\xbe\xf2\x63\x5d\xce\x4a",
	      "\xf5\xe9\x48\xdc\xb8\x61\x39\xa9\x90\x27\xec\x09\x23\x50\xe0\xa9\x78\xb2\x1c\x29\x3c\xa7\x6c\x88");
    pcbc_test("\xfd\x54\x2a\x5b\x97\xa4\x5b\x52",
	      "\x37\x36\x6e\x22\x7e\x66\x08\x8c",
	      "\xe4\x2d\x81\x88\x86\xb2\x44\x55\x80\x3d\x3c\xbd\x42\x9f\x5d\xdb\x4b\x63\x23\x1c\x31\x13\xa6\x0f",
	      "\x9c\x9f\x65\x05\x79\x91\x71\x96\x82\x2a\xc0\xe5\xa0\x6f\x71\xab\x68\x32\xd4\xd7\x5e\x38\x38\xf6");
    pcbc_test("\x25\x91\x08\xe5\x57\x85\xb6\x20",
	      "\x47\x6e\xbe\x9f\xb9\x6b\x55\xe9",
	      "\x44\xfd\xdd\x42\x07\x99\xf0\x8f\xdb\xa5\x14\x1e\x76\x07\x90\x5b\x29\x10\x21\xb9\x7e\xac\xc7\x77",
	      "\x88\x4f\xdc\x6e\x37\x5e\x4e\xac\x8d\x3f\x9d\xd1\x82\x51\x65\xf5\xf9\x08\xa7\xac\x01\x61\x19\x85");
    pcbc_test("\x6d\x43\xc7\x9d\x6b\x97\x64\x40",
	      "\x56\xfb\xcb\xb3\x97\xb5\x70\x13",
	      "\x54\x67\xa9\x42\x86\x85\x81\x8f\xb4\x72\xa2\x5f\x2d\x90\xbb\x5c\xb5\xb9\x9b\x71\x8f\x2b\xae\x05",
	      "\x2c\xd1\x63\x6f\x11\x1d\x5e\x40\x8c\x47\x49\x12\x31\x48\xb7\x12\x4c\xc1\x6a\xaf\x0e\x33\x11\xe1");
    pcbc_test("\x3b\xa2\xbc\xd5\x5d\x9d\xdf\x73",
	      "\x43\xb7\x26\x71\xce\x6d\x97\xac",
	      "\x4e\xf6\x7d\xd7\xfc\x6b\x35\x54\xae\xc9\xfe\xf7\xb7\x1e\x47\xa5\x61\x44\x50\xb3\xe4\xe8\x7d\xdc",
	      "\x4d\xda\xbd\xad\xc4\xde\xdc\xf4\xfc\xbd\xfc\xa7\xbd\xe4\x7e\x73\x28\xc5\x5c\xd0\x9a\x35\x39\xa6");
    pcbc_test("\x46\x9e\xda\xdf\x0d\x97\x8a\xd3",
	      "\x6c\x9f\xdf\xc0\x48\x3b\xa5\x17",
	      "\xb9\xd8\x99\x61\x67\xf3\xec\xa9\xc1\x29\xa3\x8b\x63\xe2\xc2\x28\xaf\x56\x2d\x39\x1d\xeb\x7c\xbc",
	      "\x70\x5d\xd4\x54\x90\xb9\x6c\x0c\x93\x96\x6a\x4a\x4e\xb8\x80\xce\xb3\xcd\x64\xa7\x6c\xb2\xe4\xc9");
    pcbc_test("\x31\x89\x51\x38\x2f\x97\xfe\xef",
	      "\x17\xdc\xf8\xde\xcc\x8f\x40\x3e",
	      "\xef\xcf\xe9\x9e\x11\xd8\x35\xdf\x58\x11\xd0\x0a\x68\xce\xe1\x6b\xb5\xca\x68\x47\xb7\xb9\x9a\x34",
	      "\x3a\x93\x47\x3c\x1b\xa9\xeb\x88\x13\xfd\x1b\xd8\x76\xb5\xd3\xe2\xb8\x83\x10\x56\x68\xab\xe1\x28");
    pcbc_test("\xba\x1c\x70\x94\x62\x10\x19\xda",
	      "\x7a\x8b\xc0\x9e\x00\xbb\x7e\xcb",
	      "\x30\x74\x6b\xa6\xd6\x07\xae\x44\xd6\x5c\xe6\x18\x97\x90\xaa\x08\xcb\xa8\xf4\x8b\xea\x8b\x4f\xe6",
	      "\x0a\x77\x24\x7c\xcd\xf8\x06\x01\x20\x02\x14\x33\xd6\xf4\x4e\x89\xc0\x38\x65\x44\x6b\x9c\x92\x16");
    pcbc_test("\xfe\x97\xf2\x6d\x8f\x0d\x86\x94",
	      "\x30\x8a\x7d\x9b\xf4\x28\x6e\x84",
	      "\x82\xb0\x9b\x42\xf6\xdc\x38\x41\x41\x03\x60\x28\x7f\x90\x08\x8b\x6c\x55\xe7\x76\xcd\xa7\xae\xbc",
	      "\x35\x0b\xf1\xc0\x56\x64\x6f\x7b\x3e\x1f\xd1\x90\xbd\xda\x10\xb1\xd1\x49\xc6\x62\x5f\xf9\x6c\xf9");


    cbc_cksum("\x58\x83\x67\xfb\xdf\x51\x7c\xfd",
	      "\x46\x0a\xa5\x94\x6b\xd6\xaa\x91",
	      "\x15\x0b\x16\x3a\x56\x79\x33\xdf\x6e\xa0\xd9\x54\x14\x7b\x37\xa9\xb1\x15\xe1\x28\xfe\x35\xe9\x34",
	      24,
	      0x16466788,
	      "\xa7\xbd\x2a\x1b\x16\x46\x67\x88");
    cbc_cksum("\xf1\xe0\x91\x1c\xfe\x10\xe5\xb5",
	      "\x9c\xc6\x7d\xf3\x3e\x58\x40\x06",
	      "\x9c\x90\x88\xfe\x9c\x38\xc0\xd5\xaa\xc6\xf2\xc2\x7d\x00\xf6\x5f\xbd\x87\x25\xbe\x41\x64\x9f\xb7",
	      24,
	      0xd8a127cc,
	      "\x93\x5d\x75\x62\xd8\xa1\x27\xcc");
    cbc_cksum("\x20\xbf\xdc\xd5\x5b\x9d\xc8\x79",
	      "\x68\xdc\xe2\xfa\x18\xb3\xa9\xe0",
	      "\xef\xba\xc4\x8b\x78\xc2\x02\xc2\x74\x71\x9f\xfa\x4b\xa2\x8a\xe5\xfb\x82\x3d\x48\xcf\x28\x08\x42",
	      24,
	      0x45236285,
	      "\xc0\xb9\x2c\x86\x45\x23\x62\x85");
    cbc_cksum("\x31\x6d\xa8\xc2\x43\x16\x64\xea",
	      "\x7b\x5e\x9f\x7c\xb8\xa3\xbd\x89",
	      "\x8a\xd4\xe4\x77\xbb\x45\x17\x3d\xd2\xef\xe6\xb9\x65\x8b\xb3\xa9\x28\xef\xd7\x0c\xa8\x47\x5d\xb8",
	      24,
	      0x3f021cb2,
	      "\x10\x94\x4c\x2f\x3f\x02\x1c\xb2");
    cbc_cksum("\xd5\x75\x51\x8f\xc8\x97\x1a\xc4",
	      "\xbc\x7a\x70\x58\xae\x29\x60\x3a",
	      "\x8d\x2c\x70\xdb\x53\xda\x0f\x50\xd9\xb5\x81\x18\x26\x66\x84\xda\xf6\x32\xa0\xe5\xf9\x09\xfd\x35",
	      24,
	      0x2f64dd4f,
	      "\x89\xe4\x70\x0d\x2f\x64\xdd\x4f");
    cbc_cksum("\xda\x6e\x32\x80\x20\xbc\x67\x54",
	      "\xf4\x93\x86\x43\x29\x57\x6e\xec",
	      "\xfe\xd8\xfe\xad\x4e\x05\xd8\xb8\x9b\x9f\xaa\xa5\x90\x6d\xcb\xff\x40\xab\xc5\x25\x2b\xda\xa7\x09",
	      24,
	      0x6281ce23,
	      "\xa1\x88\xc2\x3d\x62\x81\xce\x23");
    cbc_cksum("\xb6\xc7\x75\x8a\xfb\xd3\xf8\xad",
	      "\xf1\x4f\xd7\x39\x4b\xec\xa3\x99",
	      "\x31\xd0\x45\x9d\x62\xe3\x49\xbb\x58\xc2\x58\xbe\x13\x51\x1e\x3f\x54\xe5\x31\x7d\xd0\x94\x57\x7a",
	      24,
	      0x09c7ee4e,
	      "\x2f\x40\xb3\xd2\x09\xc7\xee\x4e");
    cbc_cksum("\xa8\x4f\x16\xf4\x89\x3d\xf7\xec",
	      "\x04\x78\xbc\xd3\x4f\x32\xfd\x46",
	      "\xe5\x44\x30\x5e\x55\xa3\x08\xe9\xcd\xd1\xbe\x63\x66\x26\x27\x62\xc3\x4f\x2a\x50\x69\x21\x24\xde",
	      24,
	      0xdf3357c7,
	      "\xa8\x6e\x80\x3b\xdf\x33\x57\xc7");
    cbc_cksum("\xd6\x4f\x40\xef\x8a\x2a\xf1\x20",
	      "\xd5\x40\xe7\x86\x36\x26\x79\xc9",
	      "\xcc\x74\x2b\x78\xca\x47\xb0\xd3\xe6\x72\x42\x76\xee\x80\xb0\xe5\x78\x12\x3b\x4e\x76\x91\xda\x1a",
	      24,
	      0x14a5029a,
	      "\x33\xd2\xb5\x8a\x14\xa5\x02\x9a");

    cbc_cksum("\xfb\x89\xa1\x9d\xa7\xec\xc1\x5e",
	      "\x9c\x7f\x47\xd0\x79\x5d\x4b\x97",
	      "\xb6\x8b\x48\xe0\x01\x78\xec\x50\x7f\xf1\xfd\xd2\x87\x76\xba\x4b\x9c\x5c\xc7\x25",
	      20,
	      0xa1471604,
	      "\x39\x5b\x7d\xb1\xa1\x47\x16\x04");
    cbc_cksum("\x70\xb3\xc4\x0b\x5b\x4f\x98\xe5",
	      "\x86\xc0\x05\x1a\xd5\x8f\x78\x2c",
	      "\xef\x01\x7b\xd8\xff\x68\x5d\x66\xb6\xbe\xd8\xf5\xb9\xed\x4e\xec\xe3\x3c\x12\xc5",
	      20,
	      0xc4b74f9a,
	      "\x2b\x07\xe3\x90\xc4\xb7\x4f\x9a");
    cbc_cksum("\xfe\x04\xcb\xfe\xef\x34\xe9\x58",
	      "\xd9\x28\xae\xc0\x2c\xd3\xf6\xb0",
	      "\x24\x25\x9b\x67\xda\x76\xa6\x64\x6f\x31\x94\x18\x2e\x06\x71\x82\xaf\xbd\x86\x63",
	      20,
	      0xbd7c84e6,
	      "\x70\x3e\x91\xf5\xbd\x7c\x84\xe6");
    cbc_cksum("\x10\xc2\x70\x94\x9b\x16\x20\x1c",
	      "\x62\xed\x5a\x48\x6c\xf3\x51\xa0",
	      "\x90\x3e\x06\xc1\x63\x6a\x1f\x1a\xfe\x9d\x74\xb6\x13\xde\x62\xd2\x6f\x19\x37\x25",
	      20,
	      0x26761f96,
	      "\x8b\x6a\x9c\x85\x26\x76\x1f\x96");
    cbc_cksum("\x61\x32\x7c\x7f\x31\xc7\x98\xe6",
	      "\xd9\xba\x0d\x9d\x9e\xa3\xcc\x66",
	      "\x98\x8f\xc6\x5a\x54\x04\x63\xd9\x53\x86\x5d\x75\x53\x48\xcc\xa3\x00\x7a\x12\xe5",
	      20,
	      0xf0f6ad33,
	      "\x6a\xfb\xed\xd3\xf0\xf6\xad\x33");
    cbc_cksum("\x85\xdf\x01\x2c\xab\x3b\xec\x13",
	      "\xc6\x44\x87\x5b\x78\x2a\x74\x92",
	      "\x8b\xf5\x0d\xff\x5c\xb3\xc1\xcd\x9e\xf7\xb8\x8e\x3b\xf8\x61\x4d\x26\x6a\x7b\xe8",
	      20,
	      0x7acfe214,
	      "\x52\xb7\x05\xe9\x7a\xcf\xe2\x14");
    cbc_cksum("\x49\xdf\xb0\x16\x7f\xec\x10\x52",
	      "\x09\xa3\x36\x8f\xe9\xe0\x06\x19",
	      "\x3a\x0f\x66\xf7\x7a\x47\x34\xe4\xaa\x09\x36\x90\xe9\x90\x19\xff\x99\x94\x92\x04",
	      20,
	      0x9a3a59bb,
	      "\xd3\xe2\xce\xfc\x9a\x3a\x59\xbb");
    cbc_cksum("\x5b\xbf\x4c\xc8\xce\xf4\x51\x1a",
	      "\x7c\xee\xc0\x5a\x20\x2b\x10\x22",
	      "\x05\x1d\xec\xdb\x30\x73\xf2\x21\xbf\x64\xe0\x5f\xdf\x02\x79\xe9\x47\xf2\x9c\x4e",
	      20,
	      0xaf9d3602,
	      "\xaa\xf3\xa2\x5a\xaf\x9d\x36\x02");
    cbc_cksum("\xad\xda\xa2\x19\x6d\x37\xda\x67",
	      "\xb2\x10\x0f\xd5\xda\xdd\x17\xfc",
	      "\x44\x02\x6b\xd6\xd4\x8c\x42\x58\x8b\x59\x35\xce\xd7\x04\x6b\x35\xa6\x5f\x28\x97",
	      20,
	      0xd112a978,
	      "\xb2\x5f\x6a\x07\xd1\x12\xa9\x78");


    s2k("potatoe", "WHITEHOUSE.GOVdanny",
	"\xdf\x3d\x32\xa7\x4f\xd9\x2a\x01");
    s2k("password", "ATHENA.MIT.EDUraeburn",
	"\xCB\xC2\x2F\xAE\x23\x52\x98\xE3");
    s2k("\xf0\x9d\x84\x9e", "EXAMPLE.COMpianist",
	"\x4f\xfb\x26\xba\xb0\xcd\x94\x13");
    s2k("NNNN6666", "FFFFAAAA",
	"\xc4\xbf\x6b\x25\xad\xf7\xa4\xf8");
    s2k("", "",
	"\x01\x01\x01\x01\x01\x01\x01\xf1");

    if (1) {
	cfb64_test("\x45\xc2\x0b\x01\x40\x08\x13\x8a",
		   "\x9a\xef\xf4\x37\x41\x69\x0b\xd6",
		   "\x5d\x12\x5d\xf5\xae\x1d\xc6\x47\x21\xd3\x16\xba\x45\x0e\x9d\x4c\x00\xfd\xf8\x64\xca\x69\x67",
		   "\xff\x99\x06\xd8\xe9\xbc\xae\x7e\xde\x49\x7b\x34\x5d\xa0\x74\x61\x9b\x6f\x70\x38\x40\x40\xba");
	cfb64_test("\xdc\xe9\x51\xc4\x0b\xad\x85\xa8",
		   "\xf5\x56\x6c\xef\x42\xed\x9f\xa8",
		   "\x7d\xe5\xeb\x04\x5c\xaf\x8c\x5b\xf4\x88\xba\x4a\x99\x6a\x3a\x79\xc0\x88\x01\x05\xac\x98\x3c",
		   "\x53\x87\x11\xc4\xa6\xf3\x1e\x67\x56\xfc\x8c\x63\xf0\x2e\xd9\x0e\x4a\x86\x8e\x5b\xa7\xde\xcf");
	cfb64_test("\x25\xf7\xa7\x0e\x85\x4f\x5b\xb6",
		   "\x83\xae\x73\x03\xea\xeb\x82\x05",
		   "\x1b\x80\x23\xdc\x61\x23\xa7\xde\x80\xf6\xec\xb1\xc1\x6d\x3e\x59\x1f\x76\x6d\xdf\xfa\x42\xc7",
		   "\xe2\xf7\x8d\x2f\x86\xce\x1f\xfc\xdb\x82\xb9\xb5\x9c\xa9\xf4\x9c\x2b\x3f\x34\x6c\x83\xf7\x7e");
	cfb64_test("\xab\xd5\xd3\x68\xf1\x2c\x0e\x0d",
		   "\x8a\xea\xe8\xc0\xad\xb9\x51\x83",
		   "\x3d\xcb\x7d\xcf\x57\xa6\xf6\x16\x4f\x34\xb6\x5f\xc2\xa9\xf0\xec\x90\xc5\x43\xa0\x19\xfc\x3f",
		   "\xe9\x2c\x22\x20\xd4\x27\x90\x89\x40\x08\x4a\x23\x4d\x41\x05\x67\xe1\xde\xf5\x0b\x8b\x96\xb1");
	cfb64_test("\x92\x38\xd3\xfd\x61\x83\x92\x0e",
		   "\x25\xb6\x34\x51\x6d\x6a\x35\xa2",
		   "\x98\x55\xab\x2a\xa2\x9e\xcf\xf4\x92\xdf\xb4\xc6\xc1\x34\x55\xf6\x13\x85\x4c\x50\xdc\x82\x1e",
		   "\x87\x96\x47\xa6\xcd\xff\xda\xd2\xad\x88\xaa\x25\xbd\xcd\x72\x61\x37\x14\x42\x14\xc7\x4b\x7f");
	cfb64_test("\xf4\xcb\x97\xad\xef\x7f\x80\xb0",
		   "\xfc\xa0\x7d\xb6\x75\xb8\x48\xea",
		   "\xc2\x1e\x16\x2b\xb7\xcf\xc6\xa0\x4b\x76\x75\x61\x49\x66\x0d\xce\xd2\x12\xf2\x98\x07\x2f\xac",
		   "\xe2\x20\xbf\x29\x5b\x34\x20\x2a\x2e\x99\xa5\x50\x97\x1b\x4b\x18\xb4\xd6\x87\x35\x7b\x5f\x43");
	cfb64_test("\x3b\x1c\x15\xec\xb9\x5e\xe0\xda",
		   "\x7d\x94\x23\x76\x96\x72\x62\xf4",
		   "\x5d\x83\xdb\x76\x52\x46\xa7\x84\x0a\x71\x2c\x09\x40\xbd\x3d\x75\x73\x28\x0b\x22\x07\x6f\x8a",
		   "\xf1\x01\x8f\xe2\x32\x35\xe6\x06\xcf\xbb\xe4\x15\x9e\x4e\xf0\xe8\x2e\xcd\xac\xbf\xa6\xc2\xec");
	cfb64_test("\xc2\xcd\x76\x79\x7f\x51\xce\x86",
		   "\x38\xcf\x55\x7d\x0c\xd5\x35\xfe",
		   "\xc7\xe5\xe8\x1d\x19\x09\x9f\xd5\xdb\x89\x26\xc1\xf1\xc1\x18\x50\xcf\x8b\xf2\xe1\x87\xeb\xe6",
		   "\xd4\x5d\xca\x30\xb9\x41\xfa\x36\x83\xfc\x40\x2d\xd2\xe8\x94\x38\x49\xc8\xa3\x35\xb7\x5d\x9c");
	cfb64_test("\x67\xfd\xc4\x31\x45\x40\xf7\xea",
		   "\xb9\x29\xe6\x78\xdd\x1a\x13\x84",
		   "\x12\x9b\xe5\xb3\xdd\x42\x6f\x45\x86\x97\x25\x87\x05\xee\x7e\x57\x8f\x22\x79\xb3\x22\xa2\x95",
		   "\x38\xef\x49\xbc\xdd\xbb\x6b\x73\xc0\xd7\xa6\x70\xe0\x1b\xde\x8d\xe6\xb4\xc6\x69\xca\x5e\x1e");
    }


    return 0;
}
