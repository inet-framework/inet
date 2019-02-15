/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */

#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"


#define GRAT_RREP_TBL_MAX_LEN 64
#define GRAT_REPLY_HOLDOFF 1


namespace inet {

namespace inetmanet {
/*
static inline int crit_query(void *pos, void *query)
{
    struct grat_rrep_entry *p = (struct grat_rrep_entry *)pos;
    struct grat_rrep_query *q = (struct grat_rrep_query *)query;

    if (p->src.s_addr == q->src->s_addr &&
            p->prev_hop.s_addr == q->prev_hop->s_addr)
        return 1;
    return 0;
}
static inline int crit_time(void *pos, void *time)
{
    struct grat_rrep_entry *p = (struct grat_rrep_entry *)pos;
    struct timeval *t = (struct timeval *)time;

    if (timeval_diff(&p->expires, t) < 0)
        return 1;

    return 0;
}
*/
void NSCLASS grat_rrep_tbl_timeout(void *data)
{

    grat_rrep_entry *e = gratRrep.front();
    gratRrep.erase(gratRrep.begin());

    delete e;

    if (gratRrep.empty())
        return;
    e = gratRrep.front();
    grat_rrep_tbl_timer.function = &NSCLASS grat_rrep_tbl_timeout;
    set_timer(&grat_rrep_tbl_timer, &e->expires);
}

int NSCLASS grat_rrep_tbl_add(struct in_addr src, struct in_addr prev_hop)
{
    struct grat_rrep_entry *e;

    for (unsigned int i = 0; i < gratRrep.size(); i++)
    {
        if (gratRrep[i]->src.s_addr ==  src.s_addr && gratRrep[i]->prev_hop.s_addr ==  prev_hop.s_addr)
            return 0;
    }

    e = new grat_rrep_entry;

    if (!e)
        return -1;

    e->src = src;
    e->prev_hop = prev_hop;
    gettime(&e->expires);

    timeval_add_usecs(&e->expires, ConfValToUsecs(GratReplyHoldOff));

    /* Remove timer before we modify the table */
    if (timer_pending(&grat_rrep_tbl_timer))
        del_timer_sync(&grat_rrep_tbl_timer);

    if (gratRrep.empty() || timeval_diff(&gratRrep.back()->expires,&e->expires) < 0 )
        gratRrep.push_back(e);
    else if (timeval_diff(&gratRrep.front()->expires,&e->expires) > 0 )
    {
        gratRrep.push_front(e);
    }
    else
    {
        for (auto it = gratRrep.begin() ; it != gratRrep.end() ; ++it)
        {
            if (timeval_diff(&((*it)->expires), &(e->expires)) > 0)
            {
                gratRrep.insert(it,e);
                break;
            }
        }
    }

    e = gratRrep.front();
    grat_rrep_tbl_timer.function = &NSCLASS grat_rrep_tbl_timeout;
    set_timer(&grat_rrep_tbl_timer, &e->expires);

    return 1;
}

int NSCLASS grat_rrep_tbl_find(struct in_addr src, struct in_addr prev_hop)
{
    for (unsigned int i = 0; i < gratRrep.size(); i++)
    {
        if (gratRrep[i]->src.s_addr ==  src.s_addr && gratRrep[i]->prev_hop.s_addr ==  prev_hop.s_addr)
            return 1;
    }
    return 0;
}


static inline int
dsr_rrep_add_srt(struct dsr_rrep_opt *rrep_opt, struct dsr_srt *srt)
{
    if (!rrep_opt | !srt)
        return -1;

    //n = srt->laddrs / sizeof(struct in_addr);
    rrep_opt->addrs.clear();
    for (unsigned int i = 0; i< srt->addrs.size(); i++)
        rrep_opt->addrs.push_back(srt->addrs[i].s_addr);
    // memcpy(&rrep_opt->addrs[0], &srt->addrs[], srt->laddrs);
    rrep_opt->addrs.push_back(srt->dst.s_addr);
    rrep_opt->cost = srt->cost;

    return 0;
}

static struct dsr_rrep_opt *dsr_rrep_opt_add(struct dsr_opt_hdr *opt_hdr, int len,
        struct dsr_srt *srt)
{


    if (!opt_hdr || !srt || (unsigned int)len < DSR_RREP_OPT_LEN(srt))
        return nullptr;

    struct dsr_rrep_opt *rrep_opt = new dsr_rrep_opt;

    rrep_opt->type = DSR_OPT_RREP;
    rrep_opt->length = srt->laddrs + DSR_ADDRESS_SIZE + 1;
    rrep_opt->l = 0;
    rrep_opt->res = 0;

    /* Add source route to RREP */
    dsr_rrep_add_srt(rrep_opt, srt);
    opt_hdr->option.push_back(rrep_opt);


    return rrep_opt;
}

int NSCLASS dsr_rrep_send(struct dsr_srt *srt, struct dsr_srt *srt_to_me)
{
    struct dsr_pkt *dp = nullptr;
    struct dsr_opt_hdr *buf;
    int len, ttl, n;

    if (!srt || !srt_to_me)
        return -1;

    dp = dsr_pkt_alloc(nullptr);

    if (!dp)
    {
        DEBUG("Could not allocate DSR packet\n");
        return -1;
    }

    dp->src = my_addr();
    dp->dst = srt->dst;

    if (srt->laddrs == 0)
        dp->nxt_hop = dp->dst;
    else
        dp->nxt_hop = srt->addrs[0];

    len = DSR_OPT_HDR_LEN + DSR_SRT_OPT_LEN(srt) +
          DSR_RREP_OPT_LEN(srt_to_me)/*  + DSR_OPT_PAD1_LEN */;

    n = srt->laddrs / DSR_ADDRESS_SIZE;

    DEBUG("srt: %s\n", print_srt(srt));
    DEBUG("srt_to_me: %s\n", print_srt(srt_to_me));
    DEBUG("next_hop=%s\n", print_ip(dp->nxt_hop));
    DEBUG
    ("IP_HDR_LEN=%d DSR_OPT_HDR_LEN=%d DSR_SRT_OPT_LEN=%d DSR_RREP_OPT_LEN=%d DSR_OPT_PAD1_LEN=%d RREP len=%d\n",
     IP_HDR_LEN, DSR_OPT_HDR_LEN, DSR_SRT_OPT_LEN(srt),
     DSR_RREP_OPT_LEN(srt_to_me), DSR_OPT_PAD1_LEN, len);

    ttl = n + 1;

    DEBUG("TTL=%d, n=%d\n", ttl, n);

    buf = dsr_pkt_alloc_opts(dp);

    if (!buf)
        goto out_err;

    dp->nh.iph = dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                              IP_HDR_LEN + len, IPPROTO_DSR, ttl);

    if (!dp->nh.iph)
    {
        DEBUG("Could not create IP header\n");
        goto out_err;
    }

    dsr_opt_hdr_add(buf, len, DSR_NO_NEXT_HDR_TYPE);

    if (dp->dh.opth.empty())
    {
        DEBUG("Could not create DSR options header\n");
        goto out_err;
    }

    len -= DSR_OPT_HDR_LEN;
    dp->dh.opth.begin()->p_len = len;

    /* Add the source route option to the packet */
    dp->srt_opt = dsr_srt_opt_add(buf, len, 0, dp->salvage, srt);

    if (!dp->srt_opt)
    {
        DEBUG("Could not create Source Route option header\n");
        goto out_err;
    }

    len -= DSR_SRT_OPT_LEN(srt);

    dp->rrep_opt.push_back(dsr_rrep_opt_add(buf, len, srt_to_me));

    if (!dp->rrep_opt.back())
    {
        DEBUG("Could not create RREP option header\n");
        goto out_err;
    }


    /* TODO: Should we PAD? The rrep struct is padded and aligned
     * automatically by the compiler... How to fix this? */

    /*  buf += DSR_RREP_OPT_LEN(srt_to_me); */
    /*  len -= DSR_RREP_OPT_LEN(srt_to_me); */

    /*  pad1_opt = (struct dsr_pad1_opt *)buf; */
    /*  pad1_opt->type = DSR_OPT_PAD1; */

    /* if (ConfVal(UseNetworkLayerAck)) */
    /*      dp->flags |= PKT_REQUEST_ACK; */

    dp->flags |= PKT_XMIT_JITTER;

#ifdef OMNETPP
    AddCostRrep(dp, srt_to_me);
    //AddCost(dp,srt_to_me);
#endif

    XMIT(dp);

    return 0;
out_err:
    if (dp)
        dsr_pkt_free(dp);

    return -1;
}

int NSCLASS dsr_rrep_opt_recv(struct dsr_pkt *dp, struct dsr_rrep_opt *rrep_opt)
{
    struct in_addr myaddr, srt_dst;
    struct dsr_srt *rrep_opt_srt;

    if (!dp || !rrep_opt || dp->flags & PKT_PROMISC_RECV)
        return DSR_PKT_ERROR;

    if (dp->rrep_opt.size() < MAX_RREP_OPTS)
        dp->rrep_opt.push_back(rrep_opt);
    else
        return DSR_PKT_ERROR;

    myaddr = my_addr();

    srt_dst.s_addr = rrep_opt->addrs.back();
    if (srt_dst.s_addr != myaddr.s_addr)
        ActualizeMyCostRrep(rrep_opt);

#ifndef OMNETPP
    rrep_opt_srt = dsr_srt_new(dp->dst, srt_dst,
                               DSR_RREP_ADDRS_LEN(rrep_opt),
                               (char *)rrep_opt->addrs);
#else
    rrep_opt_srt = dsr_srt_new(dp->dst, srt_dst,
                               DSR_RREP_ADDRS_LEN(rrep_opt),
                               VectorAddress(rrep_opt->addrs.begin(),rrep_opt->addrs.end()-1));
    if (etxActive && rrep_opt->cost.size() == rrep_opt->addrs.size())
    {
        rrep_opt_srt->cost = rrep_opt->cost;
    }
#endif
    if (!rrep_opt_srt)
        return DSR_PKT_ERROR;

    dsr_rtc_add(rrep_opt_srt, ConfValToUsecs(RouteCacheTimeout), 0);

    /* Remove pending RREQs */
    rreq_tbl_route_discovery_cancel(rrep_opt_srt->dst);

    delete rrep_opt_srt;

    if (dp->dst.s_addr == myaddr.s_addr)
    {
        /*RREP for this node */

        DEBUG("RREP for me!\n");

        return DSR_PKT_SEND_BUFFERED;
    }

    DEBUG("I am not RREP destination\n");

    /* Forward */
    return DSR_PKT_FORWARD;
}

int __init NSCLASS grat_rrep_tbl_init(void)
{
    grat_rrep_tbl_cleanup();
    init_timer(&grat_rrep_tbl_timer);
    return 0;
}

void __exit NSCLASS grat_rrep_tbl_cleanup(void)
{
    while (!gratRrep.empty())
    {
        delete gratRrep.back();
        gratRrep.pop_back();
    }
    del_timer_sync(&grat_rrep_tbl_timer);
}
} // namespace inetmanet

} // namespace inet

