/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"

/* Criteria function for deleting packets from buffer based on next hop and
 * id */


namespace inet {

namespace inetmanet {


void NSCLASS maint_buf_set_max_len(unsigned int max_len)
{
    MaxMaintBuff = max_len;
}

#if 0


static struct maint_entry *maint_entry_create(struct dsr_pkt *dp,
        unsigned short id,
        unsigned long rto)
{
    struct maint_entry *m;

    m = (struct maint_entry *)MALLOC(sizeof(struct maint_entry),
                                     GFP_ATOMIC);

    if (!m)
        return nullptr;

    m->nxt_hop = dp->nxt_hop;
    gettime(&m->tx_time);
    m->expires = m->tx_time;
    timeval_add_usecs(&m->expires, rto);
    m->rexmt = 0;
    m->id = id;
    m->rto = rto;
    m->ack_req_sent = 0;
#ifndef OMNETPP
#ifdef NS2
    if (dp->p)
        m->dp = dsr_pkt_alloc(dp->p->copy());
#else
    m->dp = dsr_pkt_alloc(skb_copy(dp->skb, GFP_ATOMIC));
#endif
#else
    m->dp = nullptr;

#if 0
    if (dp->payload || (!dp->moreFragments || dp->fragmentOffset!=0))
    {
        IPv4Datagram *dgram;
        DSRPkt *dsrPkt;
        if (dp->nh.iph->protocol == IP_PROT_DSR)
        {
            if (dp->moreFragments || dp->fragmentOffset!=0)
                dsrPkt = dp->ip_pkt->dup();
            else
                dsrPkt = new DSRPkt();
            dgram = dsrPkt;
        }
        else
            dgram = new IPv4Datagram();

        IPv4Address destAddress_var((uint32_t)dp->dst.s_addr);
        dgram->setDestAddress(destAddress_var);
        IPv4Address srcAddress_var((uint32_t)dp->src.s_addr);
        dgram->setSrcAddress(srcAddress_var);
        dgram->setHeaderLength(dp->nh.iph->ihl); // Header length
        dgram->setVersion(dp->nh.iph->version); // Ip version
        dgram->setTypeOfService(dp->nh.iph->tos); // ToS
        dgram->setIdentification(dp->nh.iph->id); // Identification
        dgram->setMoreFragments(dp->nh.iph->tos & 0x2000);
        dgram->setDontFragment (dp->nh.iph->frag_off & 0x4000);
        dgram->setTimeToLive (dp->nh.iph->ttl); // TTL

        if (dp->nh.iph->protocol == IP_PROT_DSR)
        {
            dgram->setTransportProtocol(IP_PROT_DSR);
            dsrPkt->setDsrOptions(dp->dh.opth);
            dsrPkt->setBitLength (dsrPkt->getBitLength()+((DSR_OPT_HDR_LEN+dp->dh.opth.begin()->p_len)*8));
            dsrPkt->setEncapProtocol((IPProtocolId)dp->encapsulate_protocol);
        }
        else
            dgram->setTransportProtocol(dp->encapsulate_protocol); // Transport protocol
        if (!dp->moreFragments && dp->fragmentOffset==0)
            dgram->encapsulate(dp->payload->dup());
        m->dp = dsr_pkt_alloc(dgram);
        if (dp->srt)
        {
            m->dp->srt = new dsr_srt;
            *m->dp->srt = *dp->srt;
        }

        if (dgram && (m->dp->ip_pkt != dgram))
            delete dgram;
        if (m->dp->ip_pkt)
        {
            delete m->dp->ip_pkt;
        }
        m->dp->ip_pkt = nullptr;
    }
#else
    if (dp->payload || (!dp->moreFragments || dp->fragmentOffset!=0))
    {
        m->dp = dp->dup();
        if (m->dp->ip_pkt)
            delete m->dp->ip_pkt;
        m->dp->ip_pkt = nullptr;
    }
#endif
#endif
    if (!m->dp)
    {
        delete m;
        return nullptr;
    }
    m->dp->nxt_hop = dp->nxt_hop;
    return m;
}




#else

void NSCLASS maint_insert(struct maint_entry *m)
{
    if (maint_buf.size () >= MaxMaintBuff) // Drop the most aged packet
    {
        if (maint_buf.begin()->second->dp->payload)
            drop(maint_buf.begin()->second->dp->payload, -1);
        maint_buf.begin()->second->dp->payload = nullptr;
        dsr_pkt_free(maint_buf.begin()->second->dp);
        maint_buf.erase(maint_buf.begin());
    }
    maint_buf.insert(std::make_pair(m->expires,m));
}

NSCLASS maint_entry * NSCLASS maint_entry_create(struct dsr_pkt *dp, unsigned short id, unsigned long rto)
{
    struct maint_entry *m;

    m = new struct maint_entry;

    if (!m)
        return nullptr;

    m->nxt_hop = dp->nxt_hop;
    m->tx_time = simTime();
    m->expires = m->tx_time + ((double)rto/1000000.0);
    m->rexmt = 0;
    m->id = id;
    m->rto = rto;
    m->ack_req_sent = 0;
    m->dp = nullptr;

    if (dp->payload || (!dp->moreFragments || dp->fragmentOffset!=0))
    {
        m->dp = dp->dup();
    }
    if (!m->dp)
    {
        delete m;
        return nullptr;
    }
    m->dp->nxt_hop = dp->nxt_hop;
    return m;
}

#endif

int NSCLASS maint_buf_salvage(struct dsr_pkt *dp)
{
    struct dsr_srt *alt_srt, *old_srt, *srt;
    int old_srt_opt_len, new_srt_opt_len, sleft, salv;

    if (!dp)
        return -1;

    alt_srt = dsr_rtc_find(my_addr(), dp->dst);
    if (dp->srt)
    {
        DEBUG("old internal source route exists\n");
        delete dp->srt;
        dp->srt=nullptr;
    }

    if (!alt_srt)
    {
        DEBUG("No alt. source route - cannot salvage packet\n");
        return -1;
    }

    if (!dp->srt_opt)
    {
        DEBUG("No old source route\n");
        delete alt_srt;
        return -1;
    }

    old_srt = dsr_srt_new(dp->src, dp->dst, dp->srt_opt->length - 2, dp->srt_opt->addrs,dp->costVector);

    if (!old_srt)
    {
        delete alt_srt;
        return -1;
    }

    DEBUG("opt_len old srt: %s\n", print_srt(old_srt));

    /* Salvaging as described in the draft does not really make that much
     * sense to me... For example, why should the new source route be
     * <orig_src> -> <this_node> -> < ... > -> <dst> ?. Then it looks like
     * this node has one hop connectivity with the src? Further, the draft
     * does not mention anything about checking for loops or "going back"
     * the same way the packet arrived, i.e, <orig_src> -> <this_node> ->
     * <orig_src> -> <...> -> <dst>. */

    /* Rip out the source route to me */

    if ((old_srt->addrs.empty() && old_srt->dst.s_addr == dp->nxt_hop.s_addr)  || (old_srt->addrs[0].s_addr == dp->nxt_hop.s_addr))
    {
        srt = alt_srt;
        //sleft = (srt->laddrs) / 4;
        sleft = srt->addrs.size();
    }
    else
    {
        struct dsr_srt *srt_to_me;

        srt_to_me = dsr_srt_new_split(old_srt, my_addr());

        if (!srt_to_me)
        {
            delete alt_srt;
            delete old_srt;
            return -1;
        }
        srt = dsr_srt_concatenate(srt_to_me, alt_srt);

        //sleft = (srt->laddrs) / 4 - (srt_to_me->laddrs / 4) - 1;
        sleft = srt->addrs.size() - srt_to_me->addrs.size() -1;

        DEBUG("old_srt: %s\n", print_srt(old_srt));
        DEBUG("alt_srt: %s\n", print_srt(alt_srt));

        delete alt_srt;
        delete srt_to_me;
    }

    delete old_srt;

    if (!srt)
        return -1;

    DEBUG("Salvage packet sleft=%d srt: %s\n", sleft, print_srt(srt));

    if (dsr_srt_check_duplicate(srt))
    {
        DEBUG("Duplicate address in new source route, aborting salvage\n");
        delete srt;
        return -1;
    }

    /* TODO: Check unidirectional MAC tx support and potentially discard
     * RREP option... */

    /* TODO: Check/set First and Last hop external bits */

    old_srt_opt_len = dp->srt_opt->length + 2;
    new_srt_opt_len = DSR_SRT_OPT_LEN(srt);
    salv = dp->srt_opt->salv;

    int old_opt_len = dp->dh.opth.begin()->p_len + DSR_OPT_HDR_LEN;
    int new_opt_len = old_opt_len - old_srt_opt_len + new_srt_opt_len;

    DEBUG("Salvage - source route length new=%d old=%d\n",
          new_srt_opt_len, old_srt_opt_len);

    // actualize the information

    dp->srt_opt = dsr_srt_opt_add_char((char*)dp->srt_opt,
                                      new_srt_opt_len, 0,
                                      salv + 1, srt);

    if (old_srt_opt_len != new_srt_opt_len)
    {
        /* Set new length in DSR header */
        dp->dh.opth.begin()->p_len = htons(new_opt_len - DSR_OPT_HDR_LEN);
    }

    /* We got this packet directly from the previous hop */
    dp->srt_opt->sleft = sleft;

    dp->nxt_hop = dsr_srt_next_hop(srt, dp->srt_opt->sleft);

    DEBUG("Next hop=%s p_len=%d\n", print_ip(dp->nxt_hop), ntohs(dp->dh.opth.begin()->p_len));

    dp->srt = srt;
    dp->costVector.clear(); // the route has changed. is not valid

    XMIT(dp);
    return 0;
}


void NSCLASS maint_buf_timeout(void *data)
{
    struct maint_entry *m, *m2;

    if (timer_pending(&ack_timer))
        return;

    if (maint_buf.empty())
    {
        DEBUG("Nothing in maint buf\n");
        return;
    }

    // check time
    if (maint_buf.begin()->first > simTime()+0.0001)
    {
        maint_buf_set_timeout();
        return;
    }
    /* Get the first packet */
    m = maint_buf.begin()->second;
    maint_buf.erase(maint_buf.begin());
    if (!m)
    {
        DEBUG("Nothing in maint buf\n");
        return;
    }

    m->rexmt++;

    DEBUG("nxt_hop=%s id=%u rexmt=%d\n", print_ip(m->nxt_hop), m->id, m->rexmt);

    /* Increase the number of retransmits */
    if (m->rexmt >= ConfVal(MaxMaintRexmt))
    {
        // execute this also with link layer feedback, if after the TX_ACK and LINK_BREAK the packet continue in the maintenance , send a route error
        DEBUG("MaxMaintRexmt reached!\n");
        int n = 0;
        ph_srt_delete_link_map(my_addr(), m->nxt_hop);
        dsr_rerr_send(m->dp, m->nxt_hop);
        /* Salvage timed out packet */
        if (maint_buf_salvage(m->dp) < 0)
        {
            if (m->dp->payload)
                drop(m->dp->payload, -1);
            m->dp->payload = nullptr;
            dsr_pkt_free(m->dp);
        }
        else
            n++;
        /* Salvage other packets in maintenance buffer with the
         *              * same next hop */
        for (auto it = maint_buf.begin(); it != maint_buf.end();)
        {
            m2 = it->second;
            if (m2->nxt_hop.s_addr == m->nxt_hop.s_addr)
            {
                maint_buf.erase(it++);
                if (maint_buf_salvage(m2->dp) < 0)
                {
                    if (m2->dp->payload)
                        drop(m2->dp->payload, -1);
                    m2->dp->payload = nullptr;
                    dsr_pkt_free(m2->dp);
                }
                delete m2;
                n++;
            }
            else
                ++it;
        }
        DEBUG("Salvaged %d packets from maint_buf\n", n);
        delete m;
        maint_buf_set_timeout();
        return;
    }

    /* Set new Transmit time */
    m->tx_time = simTime();
    m->expires = m->tx_time + ((double)m->rto/1000000.0);
    // timeval_add_usecs(&m->expires, m->rto);

    /* Send new ACK REQ */
    if (m->ack_req_sent)
    {
        if (ConfVal(RetryPacket))
            dsr_ack_req_send(m->dp);
        else
            dsr_ack_req_send(m->nxt_hop, m->id);
    }
    /* Add to maintenence buffer again */
    maint_insert(m);
    maint_buf_set_timeout();
    return;
}

void NSCLASS maint_buf_set_timeout(void)
{

    if (maint_buf.empty())
        return;

// I am not sure if the time out must be, in theory only m->ack_req_sent active must be in the queue, in other case the packets aren't included
 /*
    MaintBuf::iterator it;
    for (it = maint_buf.begin(); it != maint_buf.end();++it)
    {
        m = it->second;
        if (m->ack_req_sent)
            break;
    }

    if (it == maint_buf.end())
    {
        DEBUG("No packet to set timeout for\n");
        return;
    }
*/
    auto it =  maint_buf.begin();

    if (it->first <= simTime()+0.0001)
        maint_buf_timeout(0);
    else
    {
        struct timeval expires,now;
        gettime(&now);

        DEBUG("ACK Timer: exp=%ld.%06ld now=%ld.%06ld\n",
              expires.tv_sec, expires.tv_usec, now.tv_sec, now.tv_usec);
        /*      ack_timer.data = (unsigned long)m; */
        timevalFromSimTime(&expires,it->first);
        set_timer(&ack_timer, &expires);
    }
}

int NSCLASS maint_buf_add(struct dsr_pkt *dp)
{
    struct neighbor_info neigh_info;
    struct timeval now;
    int res;
    struct maint_entry *m;

    if (!dp)
    {
        DEBUG("dp is nullptr!?\n");
        return -1;
    }

    if (!(dp->flags & PKT_REQUEST_ACK)) // do nothing
        return 1;

    gettime(&now);

    res = neigh_tbl_query(dp->nxt_hop, &neigh_info);

    if (!res)
    {
        DEBUG("No neighbor info about %s\n", print_ip(dp->nxt_hop));
        return -1;
    }

    m = maint_entry_create(dp, neigh_info.id, neigh_info.rto);

    if (!m)
        return -1;

    maint_insert(m);

    /* Check if we should add an ACK REQ */
    if ((usecs_t) timeval_diff(&now, &neigh_info.last_ack_req) >  ConfValToUsecs(MaintHoldoffTime))
    {
        m->ack_req_sent = 1;
        /* Set last_ack_req time */
        neigh_tbl_set_ack_req_time(m->nxt_hop);
        neigh_tbl_id_inc(m->nxt_hop);
        dsr_ack_req_opt_add(dp, m->id);
    }
    else
    {
        DEBUG("Delaying ACK REQ for %s since_last=%ld limit=%ld\n",
              print_ip(dp->nxt_hop),
              timeval_diff(&now, &neigh_info.last_ack_req),
              ConfValToUsecs(MaintHoldoffTime));
    }
    maint_buf_set_timeout();
    return 1;
}

/* Remove all packets for a next hop */
int NSCLASS maint_buf_del_all(struct in_addr nxt_hop)
{
    int n = 0;
    if (timer_pending(&ack_timer))
        del_timer_sync(&ack_timer);

    for (auto it = maint_buf.begin(); it != maint_buf.end();)
    {
        if (it->second->nxt_hop.s_addr == nxt_hop.s_addr)
        {
            dsr_pkt_free(it->second->dp);
            delete it->second;
            maint_buf.erase(it++);
            n++;
        }
        else
            ++it;
    }
    maint_buf_set_timeout();
    return n;
}

/* Remove packets for a next hop with a specific ID */
int NSCLASS maint_buf_del_all_id(struct in_addr nxt_hop, unsigned short id)
{
    int n = 0;
    usecs_t rtt = 0;

    if (timer_pending(&ack_timer))
        del_timer_sync(&ack_timer);
    for (auto it = maint_buf.begin(); it != maint_buf.end();)
    {
        if (it->second->nxt_hop.s_addr == nxt_hop.s_addr && it->second->id <= id)
        {
            /* Only update RTO if this was not a retransmission */
            if ((it->second->id == id) && (it->second->rexmt == 0))
                rtt = SIMTIME_DBL(simTime() - it->second->tx_time)*1000000.0;
            dsr_pkt_free(it->second->dp);
            delete it->second;
            maint_buf.erase(it++);
            n++;
        }
        else
            ++it;
    }

    if (rtt > 0)
    {
        struct neighbor_info neigh_info;
        neigh_info.id = id;
        neigh_info.rtt = rtt;
        neigh_tbl_set_rto(nxt_hop, &neigh_info);
    }
    maint_buf_set_timeout();
    return n;
}

int NSCLASS maint_buf_del_addr(struct in_addr nxt_hop)
{
    int n = 0;
    usecs_t rtt = 0;
    if (timer_pending(&ack_timer))
        del_timer_sync(&ack_timer);

    for (auto it = maint_buf.begin(); it != maint_buf.end();)
    {
        if (it->second->nxt_hop.s_addr == nxt_hop.s_addr)
        {
            /* Only update RTO if this was not a retransmission */
            if (it->second->rexmt == 0)
                rtt = SIMTIME_DBL(simTime() - it->second->tx_time)*1000000.0;
            dsr_pkt_free(it->second->dp);
            delete it->second;
            maint_buf.erase(it++);
            n++;
        }
        else
            ++it;
    }
    if (rtt > 0)
    {
        struct neighbor_info neigh_info;
        neigh_info.id = 0;
        neigh_info.rtt = rtt;
        neigh_tbl_set_rto(nxt_hop, &neigh_info);
    }

    maint_buf_set_timeout();
    return n;
}


int NSCLASS maint_buf_init(void)
{

    MaxMaintBuff = MAINT_BUF_MAX_LEN;
    init_timer(&ack_timer);
    ack_timer.function = &NSCLASS maint_buf_timeout;
    ack_timer.expires = 0;
    return 1;
}

void NSCLASS maint_buf_cleanup(void)
{
    del_timer_sync(&ack_timer);
    while (!maint_buf.empty())
    {
        dsr_pkt_free(maint_buf.begin()->second->dp);
        delete maint_buf.begin()->second;
        maint_buf.erase(maint_buf.begin());
    }
}


} // namespace inetmanet

} // namespace inet
