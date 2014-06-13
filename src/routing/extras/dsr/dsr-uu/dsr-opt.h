/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_OPT_H
#define _DSR_OPT_H

#ifdef NS2
#ifndef OMNETPP
#include <packet.h>
#include "endian.h"
#endif
#endif

//#ifdef OMNETPP
//#ifndef _WIN32
//#include <endian.h>
//#endif
//#endif

#ifndef __LITTLE_ENDIAN_BITFIELD
#define __LITTLE_ENDIAN_BITFIELD 1234
#endif

#include "dsr.h"

#ifndef NO_GLOBALS

/* Generic header for all options */
struct dsr_opt
{
    u_int8_t type;
    u_int8_t length;
};

/* The DSR options header (always comes first) */
struct dsr_opt_hdr
{
    u_int8_t nh;
#if defined(__LITTLE_ENDIAN_BITFIELD)

    u_int8_t res:7;
    u_int8_t f:1;
#elif defined (__BIG_ENDIAN_BITFIELD)
    u_int8_t f:1;
    u_int8_t res:7;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
    u_int16_t p_len;    /* payload length */
#ifndef OMNETPP
#ifdef NS2
    static int offset_;

    inline static int &offset()
    {
        return offset_;
    }
    inline static dsr_opt_hdr *access(const Packet * p)
    {
        return (dsr_opt_hdr *) p->access(offset_);
    }

    int size()
    {
        return ntohs(p_len) + sizeof(struct dsr_opt_hdr);
    }
#endif              /* NS2 */
#endif
    struct dsr_opt option[0];
};

struct dsr_pad1_opt
{
    u_int8_t type;
};

#ifdef NS2
#define DSR_NO_NEXT_HDR_TYPE PT_NTYPE
#else
#define DSR_NO_NEXT_HDR_TYPE 0
#endif

/* Header lengths */
#define DSR_FIXED_HDR_LEN 4 /* Should be the same as DSR_OPT_HDR_LEN, but that
* is not the case in ns-2 */
#define DSR_OPT_HDR_LEN sizeof(struct dsr_opt_hdr)
#define DSR_OPT_PAD1_LEN 1
#define DSR_PKT_MIN_LEN 24  /* IPv4 header + DSR header =  20 + 4 */

    /* Header types */
#define DSR_OPT_PADN       0
#define DSR_OPT_RREP       1
#define DSR_OPT_RREQ       2
#define DSR_OPT_RERR       3
#define DSR_OPT_PREV_HOP   5
#define DSR_OPT_ACK       32
#define DSR_OPT_SRT       96
#define DSR_OPT_TIMEOUT  128
#define DSR_OPT_FLOWID   129
#define DSR_OPT_ACK_REQ  160
#define DSR_OPT_PAD1     224

    /* #define DSR_FIXED_HDR(iph) (struct dsr_opt_hdr *)((char *)iph + (iph->ihl << 2)) */
#define DSR_GET_OPT(opt_hdr) ((struct dsr_opt *)(((char *)opt_hdr) + DSR_OPT_HDR_LEN))
#define DSR_GET_NEXT_OPT(dopt) ((struct dsr_opt *)((char *)dopt + dopt->length + 2))
#define DSR_LAST_OPT(dp, opt) ((dp->dh.raw + ntohs(dp->dh.opth->p_len) + 4) == ((char *)opt + opt->length + 2))

    struct dsr_opt_hdr *dsr_opt_hdr_add(char *buf, unsigned int len, unsigned int protocol);
struct dsr_opt *dsr_opt_find_opt(struct dsr_pkt *dp, int type);
int dsr_opt_parse(struct dsr_pkt *dp);

#ifdef __KERNEL__
struct iphdr *dsr_build_ip(struct dsr_pkt *dp, struct in_addr src,
                           struct in_addr dst, int ip_len, int totlen,
                           int protocol, int ttl);
#endif

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

int dsr_opt_remove(struct dsr_pkt *dp);
int dsr_opt_recv(struct dsr_pkt *dp);

#endif              /* NO_DECLS */

#endif
