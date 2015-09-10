/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#define OMNETPP
#ifdef __KERNEL__
#include <linux/proc_fs.h>
#include <linux/module.h>
#endif


#ifndef OMNETPP
#ifdef NS2
#include "inet/routing/extras/dsr/dsr-uu/ns-agent.h"
#else
#include "inet/routing/extras/dsr/dsr-uu/dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/tbl.h"
#include "inet/routing/extras/dsr/dsr-uu/neigh.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-ack.h"
#include "inet/routing/extras/dsr/dsr-uu/link-cache.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-rerr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-dev.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-srt.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-opt.h"
#include "inet/routing/extras/dsr/dsr-uu/timer.h"
#include "inet/routing/extras/dsr/dsr-uu/maint-buf.h"


#define MAINT_BUF_PROC_FS_NAME "maint_buf"

TBL(maint_buf, MAINT_BUF_MAX_LEN);

static DSRUUTimer ack_timer;

#endif              /* NS2 */
#else
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#endif /* omnetpp */

namespace inet {

namespace inetmanet {

struct maint_entry
{
    dsr_list_t l;
    struct in_addr nxt_hop;
    unsigned int rexmt;
    unsigned short id;
    struct timeval tx_time, expires;
    usecs_t rto;
    int ack_req_sent;
    struct dsr_pkt *dp;
};

struct maint_buf_query
{
    struct in_addr *nxt_hop;
    unsigned short *id;
    usecs_t rtt;
};

#ifdef __KERNEL__
static int maint_buf_print(struct tbl *t, char *buffer);
#endif

/* Criteria function for deleting packets from buffer based on next hop and
 * id */
static inline int crit_addr_id_del(void *pos, void *data)
{
    struct maint_entry *m = (struct maint_entry *)pos;
    struct maint_buf_query *q = (struct maint_buf_query *)data;

    if (m->nxt_hop.s_addr == q->nxt_hop->s_addr && m->id <= *(q->id))
    {
        struct timeval now;

        gettime(&now);

        /* Only update RTO if this was not a retransmission */
        if (m->id == *(q->id) && m->rexmt == 0)
            q->rtt = timeval_diff(&now, &m->tx_time);

        if (m->dp)
        {
#ifdef NS2
            if (m->dp->p)
                Packet::free(m->dp->p);
#endif
            dsr_pkt_free(m->dp);
            return 1;
        }
    }
    return 0;
}

/* Criteria function for deleting packets from buffer based on next hop */
static inline int crit_addr_del(void *pos, void *data)
{
    struct maint_entry *m = (struct maint_entry *)pos;
    struct maint_buf_query *q = (struct maint_buf_query *)data;

    if (m->nxt_hop.s_addr == q->nxt_hop->s_addr)
    {
        struct timeval now;

        gettime(&now);

        if (m->rexmt == 0)
            q->rtt = timeval_diff(&now, &m->tx_time);

        if (m->dp)
        {
#ifdef NS2
            if (m->dp->p)
                Packet::free(m->dp->p);
#endif
            dsr_pkt_free(m->dp);
            return 1;
        }
    }
    return 0;
}


/* Criteria function for buffered packets based on next hop */
static inline int crit_addr(void *pos, void *data)
{
    struct maint_entry *m = (struct maint_entry *)pos;
    struct in_addr *nxt_hop = (struct in_addr *)data;

    if (m->nxt_hop.s_addr == nxt_hop->s_addr)
        return 1;

    return 0;
}

/* Criteria function for buffered packets based on expire time */
static inline int crit_expires(void *pos, void *data)
{
    struct maint_entry *m = (struct maint_entry *)pos;
    struct maint_entry *m_new = (struct maint_entry *)data;

    if (timeval_diff(&m->expires, &m_new->expires) > 0)
        return 1;
    return 0;

}

/* Criteria function for buffered packets based on sent ACK REQ */
static inline int crit_ack_req_sent(void *pos, void *data)
{
    struct maint_entry *m = (struct maint_entry *)pos;

    if (m->ack_req_sent)
        return 1;
    return 0;
}

void NSCLASS maint_buf_set_max_len(unsigned int max_len)
{
    maint_buf.max_len = max_len;
}

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
        dgram->setMoreFragments(dp->nh.iph->frag_off & 0x2000);
        dgram->setDontFragment (dp->nh.iph->frag_off & 0x4000);
        dgram->setTimeToLive (dp->nh.iph->ttl); // TTL

        if (dp->nh.iph->protocol == IP_PROT_DSR)
        {
            dgram->setTransportProtocol(IP_PROT_DSR);
            struct dsr_opt_hdr *opth;
            struct dsr_opt_hdr * options;
            opth = dp->dh.opth;

            int dsr_opts_len = opth->p_len + DSR_OPT_HDR_LEN;
            options = (dsr_opt_hdr *)MALLOC (dsr_opts_len, GFP_ATOMIC);
            memcpy((char*)options,(char*)opth,dsr_pkt_opts_len(dp));
            dsrPkt->setDsrOptions(options);
            dsrPkt->setBitLength (dsrPkt->getBitLength()+((DSR_OPT_HDR_LEN+options->p_len)*8));
            dsrPkt->setEncapProtocol((IPProtocolId)dp->encapsulate_protocol);
        }
        else
            dgram->setTransportProtocol(dp->encapsulate_protocol); // Transport protocol
        if (!dp->moreFragments && dp->fragmentOffset==0)
            dgram->encapsulate(dp->payload->dup());
        m->dp = dsr_pkt_alloc(dgram);
        if (dp->srt)
        {
            int size = sizeof(struct dsr_srt) + dp->srt->laddrs+dp->srt->cost_size;
            m->dp->srt = (struct dsr_srt *)MALLOC(size,GFP_ATOMIC);
            memcpy(m->dp->srt,dp->srt,size);
        }

        if (dgram && (m->dp->ip_pkt!=dgram))
            delete dgram;
        if (m->dp->ip_pkt)
        {
            delete m->dp->ip_pkt;
        }
        m->dp->ip_pkt = nullptr;
    }
#endif
    if (!m->dp)
    {
        FREE(m);
        return nullptr;
    }
    m->dp->nxt_hop = dp->nxt_hop;
    return m;
}


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
        FREE(dp->srt);
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
        FREE(alt_srt);
        return -1;
    }
#ifdef OMNETPP
    old_srt = dsr_srt_new(dp->src, dp->dst, dp->srt_opt->length - 2,
                          (char *)dp->srt_opt->addrs,dp->costVector,dp->costVectorSize);
#else
    old_srt = dsr_srt_new(dp->src, dp->dst, dp->srt_opt->length - 2,
                          (char *)dp->srt_opt->addrs);
#endif
    if (!old_srt)
    {
        FREE(alt_srt);
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

    if (old_srt->addrs[0].s_addr == dp->nxt_hop.s_addr)
    {
        srt = alt_srt;
        sleft = (srt->laddrs) / 4;
    }
    else
    {
        struct dsr_srt *srt_to_me;

        srt_to_me = dsr_srt_new_split(old_srt, my_addr());

        if (!srt_to_me)
        {
            FREE(alt_srt);
            FREE(old_srt);
            return -1;
        }
        srt = dsr_srt_concatenate(srt_to_me, alt_srt);

        sleft = (srt->laddrs) / 4 - (srt_to_me->laddrs / 4) - 1;

        DEBUG("old_srt: %s\n", print_srt(old_srt));
        DEBUG("alt_srt: %s\n", print_srt(alt_srt));

        FREE(alt_srt);
        FREE(srt_to_me);
    }

    FREE(old_srt);

    if (!srt)
        return -1;

    DEBUG("Salvage packet sleft=%d srt: %s\n", sleft, print_srt(srt));

    if (dsr_srt_check_duplicate(srt))
    {
        DEBUG("Duplicate address in new source route, aborting salvage\n");
        FREE(srt);
        return -1;
    }

    /* TODO: Check unidirectional MAC tx support and potentially discard
     * RREP option... */

    /* TODO: Check/set First and Last hop external bits */

    old_srt_opt_len = dp->srt_opt->length + 2;
    new_srt_opt_len = DSR_SRT_OPT_LEN(srt);
    salv = dp->srt_opt->salv;

    DEBUG("Salvage - source route length new=%d old=%d\n",
          new_srt_opt_len, old_srt_opt_len);

    if (old_srt_opt_len == new_srt_opt_len)
    {
        DEBUG("new and old srt of same length\n");

        dp->srt_opt = dsr_srt_opt_add((char *)dp->srt_opt,
                                      new_srt_opt_len, 0,
                                      salv + 1, srt);
    }
    else
    {
        int old_opt_len, new_opt_len;
        char *old_opt = dp->dh.raw;
        char *old_srt_opt = (char *)dp->srt_opt;
        char *buf;

        DEBUG("Creating new options header\n");

        old_opt_len = dsr_pkt_opts_len(dp);
        new_opt_len = old_opt_len - old_srt_opt_len + new_srt_opt_len;

        DEBUG("opt_len old=%d new=%d srt: %s\n",
              old_opt_len, new_opt_len, print_srt(srt));

        /* Allocate new options space */
        buf = dsr_pkt_alloc_opts(dp, new_opt_len);

        if (!buf)
        {
            FREE(srt);
            return -1;
        }

        /* Copy everything up to old source route option */
        memcpy(buf, old_opt, old_srt_opt - old_opt);

        buf += (old_srt_opt - old_opt);

        /* Add new source route option */
        dp->srt_opt = dsr_srt_opt_add(buf, new_srt_opt_len, 0,
                                      salv + 1, srt);

        buf += new_srt_opt_len;

        /* Copy everything from after old source route option and to the
         * end */
        memcpy(buf, old_srt_opt + old_srt_opt_len,
               old_opt + old_opt_len -
               (old_srt_opt + old_srt_opt_len));

        FREE(old_opt);

        /* Set new length in DSR header */
        dp->dh.opth->p_len = htons(new_opt_len - DSR_OPT_HDR_LEN);
    }

    /* We got this packet directly from the previous hop */
    dp->srt_opt->sleft = sleft;

    dp->nxt_hop = dsr_srt_next_hop(srt, dp->srt_opt->sleft);

    DEBUG("Next hop=%s p_len=%d\n", print_ip(dp->nxt_hop), ntohs(dp->dh.opth->p_len));

    dp->srt = srt;

    XMIT(dp);

    return 0;
}


void NSCLASS maint_buf_timeout(unsigned long data)
{
    struct maint_entry *m, *m2;

    if (timer_pending(&ack_timer))
        return;

    /* Get the first packet */
    m = (struct maint_entry *)tbl_detach_first(&maint_buf);

    if (!m)
    {
        DEBUG("Nothing in maint buf\n");
        return;
    }

    m->rexmt++;

    DEBUG("nxt_hop=%s id=%u rexmt=%d\n",
          print_ip(m->nxt_hop), m->id, m->rexmt);

    /* Increase the number of retransmits */
    if (m->rexmt >= ConfVal(MaxMaintRexmt))
    {
        DEBUG("MaxMaintRexmt reached!\n");
        if (m->ack_req_sent)
        {
            int n = 0;
#ifdef OMNETPP
            if (ConfVal(PathCache))
                ph_srt_delete_link(my_addr(), m->nxt_hop);
            else
                lc_link_del(my_addr(), m->nxt_hop);
#endif

#ifdef NS2
            /* Remove packets from interface queue */
            Packet *qp;

            while ((qp = ifq_->prq_get_nexthop((nsaddr_t)m->nxt_hop.s_addr)))
            {
                Packet::free(qp);
            }
#endif
            dsr_rerr_send(m->dp, m->nxt_hop);
            /* Salvage timed out packet */
            if (maint_buf_salvage(m->dp) < 0)
            {
#ifndef OMNETPP
#ifdef NS2
                if (m->dp->p)
                    drop(m->dp->p, DROP_RTR_SALVAGE);
#endif
#else
                if (m->dp->payload)
                    drop(m->dp->payload, -1);
                m->dp->payload = nullptr;

#endif
                dsr_pkt_free(m->dp);
            }
            else
                n++;
            /* Salvage other packets in maintenance buffer with the
             * same next hop */
            while ((m2 = (struct maint_entry *)tbl_find_detach(&maint_buf, &m->nxt_hop, crit_addr)))
            {
                if (maint_buf_salvage(m2->dp) < 0)
                {
#ifndef OMNETPP
#ifdef NS2
                    if (m2->dp->p)
                        drop(m2->dp->p, DROP_RTR_SALVAGE);
#endif
#else
                    if (m2->dp->payload)
                        drop(m2->dp->payload, -1);
                    m2->dp->payload = nullptr;
#endif
                    dsr_pkt_free(m2->dp);
                }
                FREE(m2);
                n++;
            }
            DEBUG("Salvaged %d packets from maint_buf\n", n);
        }
        else
        {
            DEBUG("No ACK REQ sent for this packet\n");

            if (m->dp)
            {
#ifndef OMNETPP
#ifdef NS2
                if (m->dp->p)
                    drop(m->dp->p, DROP_RTR_SALVAGE);
#endif
#else
                if (m->dp->payload)
                    drop(m->dp->payload, -1);
                m->dp->payload = nullptr;
#endif

                dsr_pkt_free(m->dp);
            }
        }
        FREE(m);
        goto out;
    }

    /* Set new Transmit time */
    gettime(&m->tx_time);
    m->expires = m->tx_time;
    timeval_add_usecs(&m->expires, m->rto);

    /* Send new ACK REQ */
    if (m->ack_req_sent)
    {
        if (ConfVal(RetryPacket))
            dsr_ack_req_send(m->dp);
        else
            dsr_ack_req_send(m->nxt_hop, m->id);
    }

    /* Add to maintenence buffer again */
    tbl_add(&maint_buf, &m->l, crit_expires);
out:
    maint_buf_set_timeout();
    return;
}

void NSCLASS maint_buf_set_timeout(void)
{
    struct maint_entry *m;
    usecs_t rto;
    struct timeval tx_time, now, expires;

    if (tbl_empty(&maint_buf))
        return;

    gettime(&now);

    DSR_WRITE_LOCK(&maint_buf.lock);
    /* Get first packet in maintenance buffer */
    m = (struct maint_entry *)__tbl_find(&maint_buf, nullptr,
                                         crit_ack_req_sent);

    if (!m)
    {
        DEBUG("No packet to set timeout for\n");
        DSR_WRITE_UNLOCK(&maint_buf.lock);
        return;
    }

    tx_time = m->tx_time;
    rto = m->rto;
    m->expires = tx_time;
    timeval_add_usecs(&m->expires, m->rto);

    expires = m->expires;

    DSR_WRITE_UNLOCK(&maint_buf.lock);

    /* Check if this packet has already expired */
    if (timeval_diff(&now, &tx_time) > (int)rto)
        maint_buf_timeout(0);
    else
    {
        DEBUG("ACK Timer: exp=%ld.%06ld now=%ld.%06ld\n",
              expires.tv_sec, expires.tv_usec, now.tv_sec, now.tv_usec);
        /*      ack_timer.data = (unsigned long)m; */
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

    /* Check if we should add an ACK REQ */
    if (dp->flags & PKT_REQUEST_ACK)
    {
        if ((usecs_t) timeval_diff(&now, &neigh_info.last_ack_req) >
                ConfValToUsecs(MaintHoldoffTime))
        {
            m->ack_req_sent = 1;

            /* Set last_ack_req time */
            neigh_tbl_set_ack_req_time(m->nxt_hop);

            neigh_tbl_id_inc(m->nxt_hop);

            dsr_ack_req_opt_add(dp, m->id);
        }

        if (tbl_add_tail(&maint_buf, &m->l) < 0)
        {
            DEBUG("Buffer full - not buffering!\n");
            dsr_pkt_free(m->dp);
            FREE(m);
            return -1;
        }

        maint_buf_set_timeout();

    }
    else
    {
        DEBUG("Delaying ACK REQ for %s since_last=%ld limit=%ld\n",
              print_ip(dp->nxt_hop),
              timeval_diff(&now, &neigh_info.last_ack_req),
              ConfValToUsecs(MaintHoldoffTime));
    }

    return 1;
}

/* Remove all packets for a next hop */
int NSCLASS maint_buf_del_all(struct in_addr nxt_hop)
{
    struct maint_buf_query q;
    int n;

    q.id = nullptr;
    q.nxt_hop = &nxt_hop;
    q.rtt = 0;

    if (timer_pending(&ack_timer))
        del_timer_sync(&ack_timer);

    n = tbl_for_each_del(&maint_buf, &q, crit_addr_del);

    maint_buf_set_timeout();

    return n;
}

/* Remove packets for a next hop with a specific ID */
int NSCLASS maint_buf_del_all_id(struct in_addr nxt_hop, unsigned short id)
{
    struct maint_buf_query q;
    int n;

    q.id = &id;
    q.nxt_hop = &nxt_hop;
    q.rtt = 0;

    if (timer_pending(&ack_timer))
        del_timer_sync(&ack_timer);

    /* Find the buffered packet to mark as acked */
    n = tbl_for_each_del(&maint_buf, &q, crit_addr_id_del);

    if (q.rtt > 0)
    {
        struct neighbor_info neigh_info;

        neigh_info.id = id;
        neigh_info.rtt = q.rtt;
        neigh_tbl_set_rto(nxt_hop, &neigh_info);
    }

    maint_buf_set_timeout();

    return n;
}
int NSCLASS maint_buf_del_addr(struct in_addr nxt_hop)
{
    struct maint_buf_query q;
    int n;

    q.id = nullptr;
    q.nxt_hop = &nxt_hop;
    q.rtt = 0;

    if (timer_pending(&ack_timer))
        del_timer_sync(&ack_timer);

    /* Find the buffered packet to mark as acked */
    n = tbl_for_each_del(&maint_buf, &q, crit_addr_del);

    if (q.rtt > 0)
    {
        struct neighbor_info neigh_info;

        neigh_info.id = 0;
        neigh_info.rtt = q.rtt;
        neigh_tbl_set_rto(nxt_hop, &neigh_info);
    }

    maint_buf_set_timeout();

    return n;
}

#ifdef __KERNEL__
static int maint_buf_print(struct tbl *t, char *buffer)
{
    dsr_list_t *p;
    int len;
    struct timeval now;

    gettime(&now);

    len = sprintf(buffer, "# %-15s %-5s %-6s %-2s %-8s %-15s %-15s\n",
                  "NeighAddr", "Rexmt", "Id", "AR", "RTO", "TxTime", "Expires");

    DSR_READ_LOCK(&t->lock);

    list_for_each(p, &t->head)
    {
        struct maint_entry *e = (struct maint_entry *)p;

        if (e && e->dp)
            len +=
                sprintf(buffer + len,
                        "  %-15s %-5d %-6u %-2d %-8u %-15s %-15s\n",
                        print_ip(e->nxt_hop), e->rexmt, e->id,
                        e->ack_req_sent, (unsigned int)e->rto,
                        print_timeval(&e->tx_time),
                        print_timeval(&e->expires));
    }

    len += sprintf(buffer + len,
                   "\nQueue length      : %u\n"
                   "Queue max. length : %u\n", t->len, t->max_len);

    DSR_READ_UNLOCK(&t->lock);

    return len;
}

static int
maint_buf_get_info(char *buffer, char **start, off_t offset, int length)
{
    int len;

    len = maint_buf_print(&maint_buf, buffer);

    *start = buffer + offset;
    len -= offset;

    if (len > length)
        len = length;
    else if (len < 0)
        len = 0;
    return len;
}

#endif              /* __KERNEL__ */

int NSCLASS maint_buf_init(void)
{
#ifdef __KERNEL__
    struct proc_dir_entry *proc;

    proc = proc_net_create(MAINT_BUF_PROC_FS_NAME, 0, maint_buf_get_info);
    if (proc)
        proc->owner = THIS_MODULE;
    else
    {
        printk(KERN_ERR "maint_buf: failed to create proc entry\n");
        return -1;
    }
#endif
    INIT_TBL(&maint_buf, MAINT_BUF_MAX_LEN);

    init_timer(&ack_timer);

    ack_timer.function = &NSCLASS maint_buf_timeout;
    ack_timer.expires = 0;

    return 1;
}

void NSCLASS maint_buf_cleanup(void)
{
    struct maint_entry *m;

    del_timer_sync(&ack_timer);

    while ((m = (struct maint_entry *)tbl_detach_first(&maint_buf)))
    {
#ifndef OMNETPP
#ifdef NS2
        if (m->dp->p)
            Packet::free(m->dp->p);
#endif
#else
        if (m->dp->payload)
            drop(m->dp->payload, -1);
        m->dp->payload = nullptr;
#endif
        dsr_pkt_free(m->dp);

        FREE(m);
    }
#ifdef __KERNEL__
    proc_net_remove(MAINT_BUF_PROC_FS_NAME);
#endif
}

} // namespace inetmanet

} // namespace inet

