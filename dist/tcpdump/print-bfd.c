/*	$NetBSD: print-bfd.c,v 1.2 2007/07/24 11:53:42 drochner Exp $	*/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code
 * distributions retain the above copyright notice and this paragraph
 * in its entirety, and (2) distributions including binary code include
 * the above copyright notice and this paragraph in its entirety in
 * the documentation or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * Original code by Hannes Gredler (hannes@juniper.net)
 */

#include <sys/cdefs.h>
#ifndef lint
#if 0
static const char rcsid[] _U_ =
    "@(#) Header: /tcpdump/master/tcpdump/print-bfd.c,v 1.5.2.5 2006/02/02 06:36:37 hannes Exp";
#else
__RCSID("$NetBSD: print-bfd.c,v 1.2 2007/07/24 11:53:42 drochner Exp $");
#endif
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <stdlib.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"

#include "udp.h"

/*
 * Control packet, BFDv0, draft-katz-ward-bfd-01.txt
 *
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |Vers |  Diag   |H|D|P|F| Rsvd  |  Detect Mult  |    Length     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                       My Discriminator                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                      Your Discriminator                       |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                    Desired Min TX Interval                    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                   Required Min RX Interval                    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                 Required Min Echo RX Interval                 |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/*
 *  Control packet, BFDv1, draft-ietf-bfd-base-02.txt
 *
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |Vers |  Diag   |Sta|P|F|C|A|D|R|  Detect Mult  |    Length     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                       My Discriminator                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                      Your Discriminator                       |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                    Desired Min TX Interval                    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                   Required Min RX Interval                    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                 Required Min Echo RX Interval                 |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

struct bfd_header_t {
    u_int8_t version_diag;
    u_int8_t flags;
    u_int8_t detect_time_multiplier;
    u_int8_t length;
    u_int8_t my_discriminator[4];
    u_int8_t your_discriminator[4];
    u_int8_t desired_min_tx_interval[4];
    u_int8_t required_min_rx_interval[4];
    u_int8_t required_min_echo_interval[4];
};

/*
 *    An optional Authentication Header may be present
 *
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |   Auth Type   |   Auth Len    |    Authentication Data...     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

struct bfd_auth_header_t {
    u_int8_t auth_type;
    u_int8_t auth_len;
    u_int8_t auth_data;
};

static const struct tok bfd_v1_authentication_values[] = {
    { 0,        "Reserved" },
    { 1,        "Simple Password" },
    { 2,        "Keyed MD5" },
    { 3,        "Meticulous Keyed MD5" },
    { 4,        "Keyed SHA1" },
    { 5,        "Meticulous Keyed SHA1" },
    { 0, NULL }
};

#define	BFD_EXTRACT_VERSION(x) (((x)&0xe0)>>5)
#define	BFD_EXTRACT_DIAG(x)     ((x)&0x1f)

static const struct tok bfd_port_values[] = {
    { BFD_CONTROL_PORT, "Control" },
    { BFD_ECHO_PORT,    "Echo" },
    { 0, NULL }
};


static const struct tok bfd_diag_values[] = {
    { 0, "No Diagnostic" },
    { 1, "Control Detection Time Expired" },
    { 2, "Echo Function Failed" },
    { 3, "Neighbor Signaled Session Down" },
    { 4, "Forwarding Plane Reset" },
    { 5, "Path Down" },
    { 6, "Concatenated Path Down" },
    { 7, "Administratively Down" },
    { 8, "Reverse Concatenated Path Down" },
    { 0, NULL }
};

static const struct tok bfd_v0_flag_values[] = {
    { 0x80,	"I Hear You" },
    { 0x40,	"Demand" },
    { 0x20,	"Poll" },
    { 0x10,	"Final" },
    { 0x08,	"Reserved" },
    { 0x04,	"Reserved" },
    { 0x02,	"Reserved" },
    { 0x01,	"Reserved" },
    { 0, NULL }
};

#define BFD_FLAG_AUTH 0x04

static const struct tok bfd_v1_flag_values[] = {
    { 0x20, "Poll" },
    { 0x10, "Final" },
    { 0x08, "Control Plane Independent" },
    { BFD_FLAG_AUTH, "Authentication Present" },
    { 0x02, "Demand" },
    { 0x01, "Reserved" },
    { 0, NULL }
};

static const struct tok bfd_v1_state_values[] = {
    { 0, "AdminDown" },
    { 1, "Down" },
    { 2, "Init" },
    { 3, "Up" },
    { 0, NULL }
};

void
bfd_print(register const u_char *pptr, register u_int len, register u_int port)
{
        const struct bfd_header_t *bfd_header;
        const struct bfd_auth_header_t *bfd_auth_header;
        u_int8_t version;

        bfd_header = (const struct bfd_header_t *)pptr;
        TCHECK(*bfd_header);
        version = BFD_EXTRACT_VERSION(bfd_header->version_diag);

        switch (port << 8 | version) {

            /* BFDv0 */
        case (BFD_CONTROL_PORT << 8):
            if (vflag < 1 )
            {
                printf("BFDv%u, %s, Flags: [%s], length: %u",
                       version,
                       tok2str(bfd_port_values, "unknown (%u)", port),
                       bittok2str(bfd_v0_flag_values, "none", bfd_header->flags),
                       len);
                return;
            }

            printf("BFDv%u, length: %u\n\t%s, Flags: [%s], Diagnostic: %s (0x%02x)",
                   version,
                   len,
                   tok2str(bfd_port_values, "unknown (%u)", port),
                   bittok2str(bfd_v0_flag_values, "none", bfd_header->flags),
                   tok2str(bfd_diag_values,"unknown",BFD_EXTRACT_DIAG(bfd_header->version_diag)),
                   BFD_EXTRACT_DIAG(bfd_header->version_diag));

            printf("\n\tDetection Timer Multiplier: %u (%u ms Detection time), BFD Length: %u",
                   bfd_header->detect_time_multiplier,
                   bfd_header->detect_time_multiplier * EXTRACT_32BITS(bfd_header->desired_min_tx_interval)/1000,
                   bfd_header->length);


            printf("\n\tMy Discriminator: 0x%08x", EXTRACT_32BITS(bfd_header->my_discriminator));
            printf(", Your Discriminator: 0x%08x", EXTRACT_32BITS(bfd_header->your_discriminator));
            printf("\n\t  Desired min Tx Interval:    %4u ms", EXTRACT_32BITS(bfd_header->desired_min_tx_interval)/1000);
            printf("\n\t  Required min Rx Interval:   %4u ms", EXTRACT_32BITS(bfd_header->required_min_rx_interval)/1000);
            printf("\n\t  Required min Echo Interval: %4u ms", EXTRACT_32BITS(bfd_header->required_min_echo_interval)/1000);
            break;

            /* BFDv1 */
        case (BFD_CONTROL_PORT << 8 | 1):
            if (vflag < 1 )
            {
                printf("BFDv%u, %s, State %s, Flags: [%s], length: %u",
                       version,
                       tok2str(bfd_port_values, "unknown (%u)", port),
                       tok2str(bfd_v1_state_values, "unknown (%u)", (bfd_header->flags & 0xc0) >> 6),
                       bittok2str(bfd_v1_flag_values, "none", bfd_header->flags & 0x3f),
                       len);
                return;
            }

            printf("BFDv%u, length: %u\n\t%s, State %s, Flags: [%s], Diagnostic: %s (0x%02x)",
                   version,
                   len,
                   tok2str(bfd_port_values, "unknown (%u)", port),
                   tok2str(bfd_v1_state_values, "unknown (%u)", (bfd_header->flags & 0xc0) >> 6),
                   bittok2str(bfd_v1_flag_values, "none", bfd_header->flags & 0x3f),
                   tok2str(bfd_diag_values,"unknown",BFD_EXTRACT_DIAG(bfd_header->version_diag)),
                   BFD_EXTRACT_DIAG(bfd_header->version_diag));

            printf("\n\tDetection Timer Multiplier: %u (%u ms Detection time), BFD Length: %u",
                   bfd_header->detect_time_multiplier,
                   bfd_header->detect_time_multiplier * EXTRACT_32BITS(bfd_header->desired_min_tx_interval)/1000,
                   bfd_header->length);


            printf("\n\tMy Discriminator: 0x%08x", EXTRACT_32BITS(bfd_header->my_discriminator));
            printf(", Your Discriminator: 0x%08x", EXTRACT_32BITS(bfd_header->your_discriminator));
            printf("\n\t  Desired min Tx Interval:    %4u ms", EXTRACT_32BITS(bfd_header->desired_min_tx_interval)/1000);
            printf("\n\t  Required min Rx Interval:   %4u ms", EXTRACT_32BITS(bfd_header->required_min_rx_interval)/1000);
            printf("\n\t  Required min Echo Interval: %4u ms", EXTRACT_32BITS(bfd_header->required_min_echo_interval)/1000);

            if (bfd_header->flags & BFD_FLAG_AUTH) {
                pptr += sizeof (const struct bfd_header_t);
                bfd_auth_header = (const struct bfd_auth_header_t *)pptr;
                TCHECK2(*bfd_auth_header, sizeof(const struct bfd_auth_header_t));
                printf("\n\t%s (%u) Authentication, length %u present",
                       tok2str(bfd_v1_authentication_values,"Unknown",bfd_auth_header->auth_type),
                       bfd_auth_header->auth_type,
                       bfd_auth_header->auth_len);
            }
            break;

            /* BFDv0 */
        case (BFD_ECHO_PORT << 8): /* not yet supported - fall through */
            /* BFDv1 */
        case (BFD_ECHO_PORT << 8 | 1):

        default:
            printf("BFD, %s, length: %u",
                   tok2str(bfd_port_values, "unknown (%u)", port),
                   len);
            if (vflag >= 1) {
                if(!print_unknown_data(pptr,"\n\t",len))
                    return;
            }
            break;
        }
        return;

trunc:
        printf("[|BFD]");
}
