/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#define OMNETPP

#ifdef __KERNEL__
#include <net/ip.h>
#endif

#ifndef OMNETPP
#ifdef NS2
#include "inet/routing/extras/dsr/dsr-uu/ns-agent.h"
#endif
#else
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#endif

#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-opt.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-rreq.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-rrep.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-rerr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-srt.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-ack.h"

namespace inet {

namespace inetmanet {

struct dsr_opt_hdr *dsr_opt_hdr_add(char *buf, unsigned int len,
                                    unsigned int protocol)
{
    struct dsr_opt_hdr *opt_hdr;

    if (len < DSR_OPT_HDR_LEN)
        return nullptr;

    opt_hdr = (struct dsr_opt_hdr *)buf;

    opt_hdr->nh = protocol;
    opt_hdr->f = 0;
    opt_hdr->res = 0;
    opt_hdr->p_len = htons(len - DSR_OPT_HDR_LEN);

    return opt_hdr;
}

#ifdef __KERNEL__
struct iphdr *dsr_build_ip(struct dsr_pkt *dp, struct in_addr src,
                           struct in_addr dst, int ip_len, int tot_len,
                           int protocol, int ttl)
{
    struct iphdr *iph;

    dp->nh.iph = iph = (struct iphdr *)dp->ip_data;

    if (dp->skb && dp->skb->nh.raw)
    {
        memcpy(dp->ip_data, dp->skb->nh.raw, ip_len);
    }
    else
    {
        iph->version = IPVERSION;
        iph->ihl = 5;
        iph->tos = 0;
        iph->id = 0;
        iph->frag_off = 0;
        iph->ttl = (ttl ? ttl : IPDEFTTL);
        iph->saddr = src.s_addr;
        iph->daddr = dst.s_addr;
    }

    iph->tot_len = htons(tot_len);
    iph->protocol = protocol;

    ip_send_check(iph);

    return iph;
}
#endif

struct dsr_opt *dsr_opt_find_opt(struct dsr_pkt *dp, int type)
{
    int dsr_len, l;
    struct dsr_opt *dopt;

    dsr_len = dsr_pkt_opts_len(dp);

    l = DSR_OPT_HDR_LEN;
    dopt = DSR_GET_OPT(dp->dh.opth);

    while (l < dsr_len && (dsr_len - l) > 2)
    {
        if (type == dopt->type)
            return dopt;

        l += dopt->length + 2;
        dopt = DSR_GET_NEXT_OPT(dopt);
    }
    return nullptr;
}

int NSCLASS dsr_opt_remove(struct dsr_pkt *dp)
{
    int len, ip_len, prot, ttl;

    if (!dp || !dp->dh.raw)
        return -1;

    prot = dp->dh.opth->nh;
#ifdef NS2
    ip_len = 20;
    ttl = dp->nh.iph->ttl();
#else
    ip_len = (dp->nh.iph->ihl << 2);
    ttl = dp->nh.iph->ttl;
#endif
    dsr_build_ip(dp, dp->src, dp->dst, ip_len,
                 ip_len + dp->payload_len, prot, ttl);

    len = dsr_pkt_free_opts(dp);

    /* Return bytes removed */
    return len;
}

int dsr_opt_parse(struct dsr_pkt *dp)
{
    int dsr_len, l, n = 0;
    struct dsr_opt *dopt;

    if (!dp)
        return -1;

    dsr_len = dsr_pkt_opts_len(dp);

    l = DSR_OPT_HDR_LEN;
    dopt = DSR_GET_OPT(dp->dh.opth);

    dp->num_rrep_opts = dp->num_rerr_opts = dp->num_rreq_opts = dp->num_ack_opts = 0;

    dp->srt_opt = nullptr;
    dp->ack_req_opt = nullptr;

    while (l < dsr_len && (dsr_len - l) > 2)
    {
        switch (dopt->type)
        {
        case DSR_OPT_PADN:
            break;
        case DSR_OPT_RREQ:
            if (dp->num_rreq_opts == 0)
                dp->rreq_opt = (struct dsr_rreq_opt *)dopt;
#ifndef NS2
            else
                DEBUG("ERROR: More than one RREQ option!!\n");
#endif
            break;
        case DSR_OPT_RREP:
            if (dp->num_rrep_opts < MAX_RREP_OPTS)
                dp->rrep_opt[dp->num_rrep_opts++] = (struct dsr_rrep_opt *)dopt;
#ifndef NS2
            else
                DEBUG("Maximum RREP opts in one packet reached\n");
#endif
            break;
        case DSR_OPT_RERR:
            if (dp->num_rerr_opts < MAX_RERR_OPTS)
                dp->rerr_opt[dp->num_rerr_opts++] = (struct dsr_rerr_opt *)dopt;
#ifndef NS2
            else
                DEBUG("Maximum RERR opts in one packet reached\n");
#endif
            break;
        case DSR_OPT_PREV_HOP:
            break;
        case DSR_OPT_ACK:
            if (dp->num_ack_opts < MAX_ACK_OPTS)
                dp->ack_opt[dp->num_ack_opts++] = (struct dsr_ack_opt *)dopt;
#ifndef NS2
            else
                DEBUG("Maximum ACK opts in one packet reached\n");
#endif
            break;
        case DSR_OPT_SRT:
            if (!dp->srt_opt)
                dp->srt_opt = (struct dsr_srt_opt *)dopt;
#ifndef NS2
            else
                DEBUG("More than one source route in packet\n");
#endif
            break;
        case DSR_OPT_TIMEOUT:
            break;
        case DSR_OPT_FLOWID:
            break;
        case DSR_OPT_ACK_REQ:
            if (!dp->ack_req_opt)
                dp->ack_req_opt = (struct dsr_ack_req_opt *)dopt;
#ifndef NS2
            else
                DEBUG("More than one ACK REQ in packet\n");
#endif
            break;
        case DSR_OPT_PAD1:
            l++;
            dopt++;
            continue;
#ifndef NS2
        default:
            DEBUG("Unknown DSR option type=%d\n", dopt->type);
#endif
        }
        l += dopt->length + 2;
        dopt = DSR_GET_NEXT_OPT(dopt);
        n++;
    }

    return n;
}

int NSCLASS dsr_opt_recv(struct dsr_pkt *dp)
{
    int dsr_len, l;
    int action = 0;
    struct dsr_opt *dopt;
    struct in_addr myaddr;

    if (!dp)
        return DSR_PKT_ERROR;

    myaddr = my_addr();

    /* Packet for us ? */
#ifdef NS2
    //DEBUG("Next header=%s\n", packet_info.name((packet_t)dp->dh.opth->nh));

    if (dp->dst.s_addr == myaddr.s_addr &&
            (DATA_PACKET(dp->dh.opth->nh) || dp->dh.opth->nh == PT_PING))
        action |= DSR_PKT_DELIVER;
#else
    if (dp->dst.s_addr == myaddr.s_addr && dp->payload_len != 0)
        action |= DSR_PKT_DELIVER;
#endif
    dsr_len = dsr_pkt_opts_len(dp);

    l = DSR_OPT_HDR_LEN;
    dopt = DSR_GET_OPT(dp->dh.opth);

    //DEBUG("Parsing DSR packet l=%d dsr_len=%d\n", l, dsr_len);

    while (l < dsr_len && (dsr_len - l) > 2)
    {
        //DEBUG("dsr_len=%d l=%d\n", dsr_len, l);
        switch (dopt->type)
        {
        case DSR_OPT_PADN:
            break;
        case DSR_OPT_RREQ:
            if (dp->flags & PKT_PROMISC_RECV)
                break;

            action |= dsr_rreq_opt_recv(dp, (struct dsr_rreq_opt *)dopt);
            break;
        case DSR_OPT_RREP:
            if (dp->flags & PKT_PROMISC_RECV)
                break;

            action |= dsr_rrep_opt_recv(dp, (struct dsr_rrep_opt *)dopt);
            break;
        case DSR_OPT_RERR:
            if (dp->flags & PKT_PROMISC_RECV)
                break;
            if (dp->num_rerr_opts < MAX_RERR_OPTS)
            {
                action |=
                    dsr_rerr_opt_recv(dp, (struct dsr_rerr_opt *)dopt);
            }

            break;
        case DSR_OPT_PREV_HOP:
            break;
        case DSR_OPT_ACK:
            if (dp->flags & PKT_PROMISC_RECV)
                break;

            if (dp->num_ack_opts < MAX_ACK_OPTS)
            {
                dp->ack_opt[dp->num_ack_opts++] =
                    (struct dsr_ack_opt *)dopt;
                action |=
                    dsr_ack_opt_recv((struct dsr_ack_opt *)
                                     dopt);
            }
            break;
        case DSR_OPT_SRT:
            action |= dsr_srt_opt_recv(dp, (struct dsr_srt_opt *)dopt);
            break;
        case DSR_OPT_TIMEOUT:
            break;
        case DSR_OPT_FLOWID:
            break;
        case DSR_OPT_ACK_REQ:
            action |=
                dsr_ack_req_opt_recv(dp, (struct dsr_ack_req_opt *)
                                     dopt);
            break;
        case DSR_OPT_PAD1:
            l++;
            dopt++;
            continue;
        default:
            DEBUG("Unknown DSR option type=%d\n", dopt->type);
        }
        l += dopt->length + 2;
        dopt = DSR_GET_NEXT_OPT(dopt);
    }
    return action;
}

} // namespace inetmanet

} // namespace inet

