/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */

#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"

#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"

namespace inet {

namespace inetmanet {

int NSCLASS dsr_recv(struct dsr_pkt *dp)
{
    int i = 0, action;
    int mask = DSR_PKT_NONE;

    /* Process DSR Options */
    action = dsr_opt_recv(dp);

    /* Add mac address of previous hop to the neighbor table */

    if (dp->flags & PKT_PROMISC_RECV)
    {
        dsr_pkt_free(dp);
        return 0;
    }
    for (i = 0; i < DSR_PKT_ACTION_LAST; i++)
    {

        switch (action & mask)
        {
        case DSR_PKT_NONE:
            break;
        case DSR_PKT_DROP:
        case DSR_PKT_ERROR:
            DEBUG("DSR_PKT_DROP or DSR_PKT_ERROR\n");
            dsr_pkt_free(dp);
            return 0;
        case DSR_PKT_SEND_ACK:
            /* Moved to dsr-ack.c */
            break;
        case DSR_PKT_SRT_REMOVE:
            //DEBUG("Remove source route\n");
            // Hmm, we remove the DSR options when we deliver a
            //packet
            //dsr_opt_remove(dp);
            break;
        case DSR_PKT_FORWARD:
#ifdef OMNETPP
            if (dp->nh.iph->ttl < 1)
#else
#ifdef NS2
            if (dp->nh.iph->ttl() < 1)
#else
                if (dp->nh.iph->ttl < 1)
#endif
#endif
            {
                DEBUG("ttl=0, dropping!\n");
                dsr_pkt_free(dp);
                return 0;
            }
            else
            {
                DEBUG("Forwarding %s %s nh %s\n",
                      print_ip(dp->src),
                      print_ip(dp->dst), print_ip(dp->nxt_hop));
                XMIT(dp);
                return 0;
            }
            break;
        case DSR_PKT_FORWARD_RREQ:
            XMIT(dp);
            return 0;
        case DSR_PKT_SEND_RREP:
            /* In dsr-rrep.c */
            break;
        case DSR_PKT_SEND_ICMP:
            DEBUG("Send ICMP\n");
            break;
        case DSR_PKT_SEND_BUFFERED:
            if (!dp->rrep_opt.empty())
            {
                struct in_addr rrep_srt_dst;
                int i;

                for (i = 0; i < (int)dp->rrep_opt.size(); i++)
                {
                    for (unsigned int j = 0; j < dp->rrep_opt[i]->addrs.size(); j++)
                    {
                        rrep_srt_dst.s_addr = dp->rrep_opt[i]->addrs[j];
                        send_buf_set_verdict(SEND_BUF_SEND, rrep_srt_dst);
                    }
                }
            }
            break;
        case DSR_PKT_DELIVER:
            DEBUG("Deliver to DSR device\n");
            DELIVER(dp);
            return 0;
        case 0:
            break;
        default:
            DEBUG("Unknown pkt action\n");
        }
        mask = (mask << 1);
    }

    dsr_pkt_free(dp);

    return 0;
}

void NSCLASS dsr_start_xmit(struct dsr_pkt *dp)
{
    int res;

    if (!dp)
    {
        DEBUG("Could not allocate DSR packet\n");
        return;
    }

    dp->srt = dsr_rtc_find(dp->src, dp->dst);

    if (dp->srt)
    {

        if (dsr_srt_add(dp) < 0)
        {
            DEBUG("Could not add source route\n");
            goto out;
        }
        /* Send packet */
#ifdef OMNETPP
        AddCost(dp, dp->srt);
#endif
        XMIT(dp);

        return;

    }
    else
    {
#ifndef OMNETPP
#ifdef NS2
        res = send_buf_enqueue_packet(dp, &DSRUU::ns_xmit);
#else
        res = send_buf_enqueue_packet(dp, &dsr_dev_xmit);
#endif
#else
        /* OMNET code*/
        res = send_buf_enqueue_packet(dp);
#endif /* OMNET endif*/
        if (res < 0)
        {
            DEBUG("Queueing failed!\n");
            goto out;
        }
        res = dsr_rreq_route_discovery(dp->dst);

        if (res < 0)
            DEBUG("RREQ Transmission failed...");

        return;
    }
out:
    dsr_pkt_free(dp);
}


} // namespace inetmanet

} // namespace inet

