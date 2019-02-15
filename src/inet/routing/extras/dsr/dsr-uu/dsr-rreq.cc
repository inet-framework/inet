/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"

#ifndef MAXTTL
#define MAXTTL 255
#endif

#define STATE_IDLE          0
#define STATE_IN_ROUTE_DISC 1

namespace inet {

namespace inetmanet {

struct rreq_tbl_query
{
    struct in_addr *initiator;
    struct in_addr *target;
    unsigned int *id;
    double *cost;
    unsigned int *length;
    VectorAddress *addrs;
};


static bool compareAddress(const VectorAddress &a, const VectorAddress &b)
{
    if (a.size() != b.size())
        return false;
    for (unsigned int i = 0; i < a.size(); i++)
        if (a[i] != b[i])
            return false;
    return true;
}




void NSCLASS rreq_tbl_timeout(void *data)
{
    struct rreq_tbl_entry *e = (struct rreq_tbl_entry *)data;
    struct timeval expires;

    if (!e)
        return;

    auto it = dsrRreqTbl.find(e->node_addr.s_addr);
    if (it != dsrRreqTbl.end())
    {
        if (it->second != e)
            throw cRuntimeError("dsrRreqTbl");
    }


    DEBUG("RREQ Timeout dst=%s timeout=%lu rexmts=%d \n",
          print_ip(e->node_addr), e->timeout, e->num_rexmts);

    if (e->num_rexmts >= ConfVal(MaxRequestRexmt))
    {
        DEBUG("MAX RREQs reached for %s\n", print_ip(e->node_addr));

        dsrRreqTbl.erase(it);

        e->state = STATE_IDLE;

        /*      DSR_WRITE_UNLOCK(&rreq_tbl); */
        //if (e->timer)
        delete e;
        //tbl_add_tail(&rreq_tbl, &e->l);
        return;
    }

    e->num_rexmts++;

    /* if (e->ttl == 1) */
    /*      e->timeout = ConfValToUsecs(RequestPeriod);  */
    /*  else */
    e->timeout *= 2;    /* Double timeout */

    e->ttl *= 2;        /* Double TTL */

    if (e->ttl > MAXTTL)
        e->ttl = MAXTTL;

    if (e->timeout > ConfValToUsecs(MaxRequestPeriod))
        e->timeout = ConfValToUsecs(MaxRequestPeriod);

    gettime(&e->last_used);

    dsr_rreq_send(e->node_addr, e->ttl);

    expires = e->last_used;
    timeval_add_usecs(&expires, e->timeout);

    set_timer(e->timer, &expires);
}

NSCLASS rreq_tbl_entry *NSCLASS __rreq_tbl_entry_create(struct in_addr node_addr)
{
    struct rreq_tbl_entry *e;

    e = new rreq_tbl_entry;

    if (!e)
        return nullptr;

    e->state = STATE_IDLE;
    e->node_addr = node_addr;
    e->ttl = 0;
    memset(&e->tx_time, 0, sizeof(struct timeval));
    e->num_rexmts = 0;
#ifndef OMNETPP
#ifdef NS2
    e->timer = new DSRUUTimer(this, "RREQTblTimer");
#else
    e->timer = MALLOC(sizeof(DSRUUTimer), GFP_ATOMIC);
#endif
#else
    e->timer = new DSRUUTimer(this, "RREQTblTimer");
#endif
    if (!e->timer)
    {
        delete e;
        return nullptr;
    }
    init_timer(e->timer);

    e->timer->function = &NSCLASS rreq_tbl_timeout;
    e->timer->data = e;
    return e;
}

NSCLASS rreq_tbl_entry * NSCLASS __rreq_tbl_add(struct in_addr node_addr)
{
    struct rreq_tbl_entry *e;

    e = __rreq_tbl_entry_create(node_addr);

    if (!e)
        return nullptr;
    auto it = dsrRreqTbl.find(node_addr.s_addr);
    if (it != dsrRreqTbl.end())
    {
        throw cRuntimeError("dsrRreqTbl entry already in the table");
    }
    dsrRreqTbl.insert(std::make_pair(node_addr.s_addr, e));
    return e;
}

int NSCLASS
rreq_tbl_add_id(struct in_addr initiator, struct in_addr target,
                unsigned short id,double cost,const VectorAddress &addr,int length)
{
    struct rreq_tbl_entry *e = nullptr;
    struct Id_Entry *id_e = nullptr;
    struct Id_Entry *id_entry=nullptr;
    int exist=1;
    struct Id_Entry_Route *id_r = nullptr;
    int res = 0;

    auto it = dsrRreqTbl.find(initiator.s_addr);
    if (it != dsrRreqTbl.end())
    {
        e = it->second;
    }

    if (!e)
        e = __rreq_tbl_add(initiator);

    if (!e)
    {
        res = -ENOMEM;
        return 1;
    }

    gettime(&e->last_used);

    for (unsigned int i = 0; i < e->rreq_id_tbl.size();i++)
    {
        id_e = e->rreq_id_tbl[i];
        if ((id ==id_e->id) && (id_e->trg_addr.s_addr == target.s_addr))
        {
            exist=0;
            id_entry= id_e;
        }
    }

    if (exist)
    {
        if (e->rreq_id_tbl.size() >= ConfVal(RequestTableIds))
        {
            delete e->rreq_id_tbl.front();
            e->rreq_id_tbl.pop_front();
        }

        id_e = new Id_Entry();
        if (!id_e)
        {
            res = -ENOMEM;
            return 1;
        }
        id_r = new Id_Entry_Route;
        id_r->length = length;
        id_r->cost = cost;
        id_r->add = addr;
        id_e->rreq_id_tbl_routes.push_back(id_r);
        id_e->trg_addr = target;
        id_e->id = id;
        e->rreq_id_tbl.push_back(id_e);
    }
    else
    {
        if (ConfVal(RREQMulVisit))
        {
            if (id_entry->rreq_id_tbl_routes.size() >= ConfVal(RREQMaxVisit))
                return 1;
            for (unsigned int i = 0; i < id_entry->rreq_id_tbl_routes.size();i++)
            {
                id_r = id_entry->rreq_id_tbl_routes[i];
                if ((int)id_r->length<length)
                    return 1;
                if (compareAddress(id_r->add,id_r->add))
                    return 1;
            }
            id_r = new Id_Entry_Route;
            id_r->length = length;
            id_r->cost = cost;
            id_r->add = addr;
            id_entry->rreq_id_tbl_routes.push_back(id_r);
        }
    }
    return 1;
}

int NSCLASS rreq_tbl_route_discovery_cancel(struct in_addr dst)
{
    if (dsrRreqTbl.empty())
        return 1; // nothing to-to
    struct rreq_tbl_entry *e = nullptr;

    auto it = dsrRreqTbl.find(dst.s_addr);
    if (it != dsrRreqTbl.end())
    {
        e = it->second;
        dsrRreqTbl.erase(it);
    }


    if (!e)
    {
        DEBUG("%s not in RREQ table\n", print_ip(dst));
        return -1;
    }

    if (e->state == STATE_IN_ROUTE_DISC)
        del_timer_sync(e->timer);

    e->state = STATE_IDLE;
    gettime(&e->last_used);
    //if (e->timer)
    delete e;
    //tbl_add_tail(&rreq_tbl, &e->l);
    return 1;
}

int NSCLASS dsr_rreq_route_discovery(struct in_addr target)
{
    struct rreq_tbl_entry *e = nullptr;
    int ttl, res = 0;
    struct timeval expires;

#define TTL_START 10

    auto it = dsrRreqTbl.find(target.s_addr);
    if (it != dsrRreqTbl.end())
        e = it->second;


    if (!e)
        e = __rreq_tbl_add(target);

    if (!e)
    {
        res = -ENOMEM;
        return res;
    }

    if (e->state == STATE_IN_ROUTE_DISC)
    {
        DEBUG("Route discovery for %s already in progress\n",
              print_ip(target));
        return res;
    }
    DEBUG("Route discovery for %s\n", print_ip(target));

    gettime(&e->last_used);
    e->ttl = ttl = TTL_START;
    /* The draft does not actually specify how these Request Timeout values
     * should be used... ??? I am just guessing here. */

    if (e->ttl == 1)
        e->timeout = ConfValToUsecs(NonpropRequestTimeout);
    else
        e->timeout = ConfValToUsecs(RequestPeriod);

    e->state = STATE_IN_ROUTE_DISC;
    e->num_rexmts = 0;

    expires = e->last_used;
    timeval_add_usecs(&expires, e->timeout);

    set_timer(e->timer, &expires);

    dsr_rreq_send(target, ttl);

    return 1;
}

int NSCLASS dsr_rreq_duplicate(struct in_addr initiator, struct in_addr target,
                               unsigned int id,double cost,unsigned int length, VectorAddress &addrs)
{
    struct
    {
        struct in_addr *initiator;
        struct in_addr *target;
        unsigned int *id;
        double *cost;
        unsigned int *length;
        VectorAddress *addrs;
    } d;

    d.initiator = &initiator;
    d.target = &target;
    d.id = &id;
    d.cost=&cost;
    d.length=&length;
    d.addrs = &addrs;

    auto ite = rreqInfoMap.find(initiator.s_addr);
    if (ite != rreqInfoMap.end())
    {
        for (auto itId = ite->second.begin() ; itId != ite->second.end(); )
        {
            if (simTime() - itId->time > 100)
            {
                itId = ite->second.erase(itId);
                continue;
            }
            if (itId->seq != id)
            {
                ++itId;
                continue;
            }
            if (itId->seq == id)
            {

                if (ConfVal(RREQMulVisit) && itId->paths.size() < ConfVal(RREQMulVisit))
                {
                    for (unsigned int i = 0; i < itId->paths.size(); i++)
                    {
                        VectorAddress ad = itId->paths[i];
                        if (ad == addrs)
                            return 1;
                    }
                }
                else
                    return 1;
            }
        }
    }

    if (ite == rreqInfoMap.end())
    {
       RreqSeqInfo infoData;
       infoData.seq = id;
       infoData.time = simTime();
       infoData.paths.push_back(addrs);
       RreqSeqInfoVector vec;
       vec.push_back(infoData);
       rreqInfoMap[initiator.s_addr] = vec;
    }
    else
    {
        RreqSeqInfo infoData;
        infoData.seq = id;
        infoData.time = simTime();
        infoData.paths.push_back(addrs);
        ite->second.push_back(infoData);
    }


    auto it = dsrRreqTbl.find(initiator.s_addr);
    if (it == dsrRreqTbl.end())
        return 0;
    struct rreq_tbl_entry *e = it->second;

    if (ConfVal(RREQMulVisit))
    {

        for (unsigned int i = 0; i < e->rreq_id_tbl.size();i++)
         {
             struct Id_Entry *id_e = e->rreq_id_tbl[i];
             if (id_e->trg_addr.s_addr == d.target->s_addr &&
                     id_e->id == *(d.id))
             {
                 for (unsigned int j = 0; j < id_e->rreq_id_tbl_routes.size();j++)
                 {
                     struct Id_Entry_Route *id_e_route = id_e->rreq_id_tbl_routes[j];
                     if (id_e_route->length<*(d.length))
                         return 1;
                     if (compareAddress(id_e_route->add,*(d.addrs)))
                         return 1;
                 }
             }
         }
    }
    else
    {
        for (unsigned int i = 0; i < e->rreq_id_tbl.size();i++)
         {
             struct Id_Entry *id_e = e->rreq_id_tbl[i];
             if (id_e->trg_addr.s_addr == d.target->s_addr &&  id_e->id == *(d.id))
             {
                 return 1;
             }
         }
    }
    return 0;
}

static struct dsr_rreq_opt *dsr_rreq_opt_add(dsr_opt_hdr *buf, unsigned int len,
        struct in_addr target,
        unsigned int seqno)
{


    if (!buf || len < DSR_RREQ_HDR_LEN)
        return nullptr;

    struct dsr_rreq_opt *rreq_opt =  new dsr_rreq_opt;

    rreq_opt->type = DSR_OPT_RREQ;
    rreq_opt->length = 6;
    rreq_opt->id = htons(seqno);
    rreq_opt->target = target.s_addr;
    buf->option.push_back(rreq_opt);

    return rreq_opt;
}

int NSCLASS dsr_rreq_send(struct in_addr target, int ttl)
{
    struct dsr_pkt *dp;
    struct dsr_opt_hdr *buf;
    int len = DSR_OPT_HDR_LEN + DSR_RREQ_HDR_LEN;

    dp = dsr_pkt_alloc(nullptr);

    if (!dp)
    {
        DEBUG("Could not allocate DSR packet\n");
        return -1;
    }
    dp->dst.s_addr = L3Address(Ipv4Address(DSR_BROADCAST));
    dp->nxt_hop.s_addr = L3Address(Ipv4Address(DSR_BROADCAST));
    dp->src = my_addr();

    buf = dsr_pkt_alloc_opts(dp);


    if (!buf)
        goto out_err;

    dp->nh.iph =
        dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN, IP_HDR_LEN + len,
                     IPPROTO_DSR, ttl);

    if (!dp->nh.iph)
        goto out_err;

    dsr_opt_hdr_add(buf, len, DSR_NO_NEXT_HDR_TYPE);

    if (dp->dh.opth.empty())
    {
        DEBUG("Could not create DSR opt header\n");
        goto out_err;
    }

    len -= DSR_OPT_HDR_LEN;
    dp->dh.opth.begin()->p_len = len;

    dp->rreq_opt.push_back(dsr_rreq_opt_add(buf, len, target, ++rreq_seqno));

    if (!dp->rreq_opt.back())
    {
        DEBUG("Could not create RREQ opt\n");
        goto out_err;
    }
#ifdef NS2
    DEBUG("Sending RREQ src=%s dst=%s target=%s ttl=%d iph->saddr()=%d\n",
          print_ip(dp->src), print_ip(dp->dst), print_ip(target), ttl,
          dp->nh.iph->saddr());
#endif

    dp->flags |= PKT_XMIT_JITTER;

    XMIT(dp);

    return 0;

out_err:
    dsr_pkt_free(dp);

    return -1;
}

int NSCLASS dsr_rreq_opt_recv(struct dsr_pkt *dp, struct dsr_rreq_opt *rreq_opt)
{
    struct in_addr myaddr;
    struct in_addr trg;
    struct dsr_srt *srt_rev, *srt_rc;
    int action = DSR_PKT_NONE;
    int i, n;
    double cost;

    if (!dp || !rreq_opt || (dp->flags & PKT_PROMISC_RECV))
        return DSR_PKT_DROP;

    if (dp->rreq_opt.size() > 1)
    {
        DEBUG("More than one RREQ opt!!! - Ignoring\n");
        return DSR_PKT_ERROR;
    }

    dp->rreq_opt.push_back(rreq_opt);

    myaddr = my_addr();

    if (ConfVal(PathCache))
    {
// To avoid the path cache problems
        if (dp->src.s_addr == myaddr.s_addr)
            return DSR_PKT_DROP;
        n = DSR_RREQ_ADDRS_LEN(rreq_opt) / sizeof(struct in_addr);
        for (i = 0; i < n; i++)
            if (rreq_opt->addrs[i] == myaddr.s_addr)
            {
                return DSR_PKT_DROP;
            }
    }



    trg.s_addr = rreq_opt->target;

#ifdef OMNETPP
    cost = PathCost(dp);
#else
    cost = (double)(DSR_RREQ_ADDRS_LEN(rreq_opt)/sizeof(struct in_addr))+1;
#endif

    if (dsr_rreq_duplicate(dp->src, trg, ntohs(rreq_opt->id),cost,DSR_RREQ_ADDRS_LEN(rreq_opt),rreq_opt->addrs))
    {
        DEBUG("Duplicate RREQ from %s\n", print_ip(dp->src));
        return DSR_PKT_DROP;
    }

    rreq_tbl_add_id(dp->src, trg, ntohs(rreq_opt->id),cost,rreq_opt->addrs,DSR_RREQ_ADDRS_LEN(rreq_opt));
#ifdef OMNETPP
    ExpandCost(dp);
    dp->srt = dsr_srt_new(dp->src, myaddr, DSR_RREQ_ADDRS_LEN(rreq_opt),
                          rreq_opt->addrs,dp->costVector);
#else
    dp->srt = dsr_srt_new(dp->src, myaddr, DSR_RREQ_ADDRS_LEN(rreq_opt),
                          (char *)rreq_opt->addrs);
#endif

    if (!dp->srt)
    {
        DEBUG("Could not extract source route\n");
        return DSR_PKT_ERROR;
    }
    DEBUG("RREQ target=%s src=%s dst=%s laddrs=%d\n",
          print_ip(trg), print_ip(dp->src),
          print_ip(dp->dst), DSR_RREQ_ADDRS_LEN(rreq_opt));

    /* Add reversed source route */
    srt_rev = dsr_srt_new_rev(dp->srt);

    if (!srt_rev)
    {
        DEBUG("Could not reverse source route\n");
        return DSR_PKT_ERROR;
    }
    DEBUG("srt: %s\n", print_srt(dp->srt));
    DEBUG("srt_rev: %s\n", print_srt(srt_rev));

    dsr_rtc_add(srt_rev, ConfValToUsecs(RouteCacheTimeout), 0);

    /* Set previous hop */
    if (srt_rev->laddrs > 0)
        dp->prv_hop = srt_rev->addrs[0];
    else
        dp->prv_hop = srt_rev->dst;

    neigh_tbl_add(dp->prv_hop, dp->mac.ethh);

    /* Send buffered packets and cancel discovery*/
    for (unsigned int i = 0; i < srt_rev->addrs.size(); i++ )
    {
        send_buf_set_verdict(SEND_BUF_SEND, srt_rev->addrs[i]);
        rreq_tbl_route_discovery_cancel(srt_rev->addrs[i]);
    }

    send_buf_set_verdict(SEND_BUF_SEND, srt_rev->dst);
    rreq_tbl_route_discovery_cancel(srt_rev->dst);

    if (rreq_opt->target == myaddr.s_addr)
    {

        DEBUG("RREQ OPT for me - Send RREP\n");

        /* According to the draft, the dest addr in the IP header must
         * be updated with the target address */
#ifdef NS2
        dp->nh.iph->daddr() = (nsaddr_t) rreq_opt->target;
#else
        dp->nh.iph->daddr = rreq_opt->target;
#endif
        dsr_rrep_send(srt_rev, dp->srt);

        action = DSR_PKT_NONE;
        goto out;
    }

    n = DSR_RREQ_ADDRS_LEN(rreq_opt) / sizeof(struct in_addr);

    if (dp->srt->src.s_addr == myaddr.s_addr)
        return DSR_PKT_DROP;

    for (i = 0; i < n; i++)
        if (dp->srt->addrs[i].s_addr == myaddr.s_addr)
        {
            action = DSR_PKT_DROP;
            goto out;
        }

    /* TODO: Check Blacklist */

    srt_rc = nullptr;
    if (ConfVal(RREPDestinationOnly)==0)
    {
        srt_rc = dsr_rtc_find(myaddr, trg);
    }

    if (srt_rc)
    {
        struct dsr_srt *srt_cat;
        /* Send cached route reply */

        DEBUG("Send cached RREP\n");

        srt_cat = dsr_srt_concatenate(dp->srt, srt_rc);

        //FREE(srt_rc);

        if (!srt_cat)
        {
            DEBUG("Could not concatenate\n");
            delete srt_rc;
            goto rreq_forward;
        }

        DEBUG("srt_cat: %s\n", print_srt(srt_cat));

        if (dsr_srt_check_duplicate(srt_cat) > 0)
        {
            DEBUG("Duplicate address in source route!!!\n");
            delete srt_rc;
            delete srt_cat;
            goto rreq_forward;
        }
#ifdef NS2
        dp->nh.iph->daddr() = (nsaddr_t) rreq_opt->target;
#else
        dp->nh.iph->daddr = rreq_opt->target;
#endif
        DEBUG("Sending cached RREP to %s\n", print_ip(dp->src));
        dsr_rrep_send(srt_rev, srt_cat);

        action = DSR_PKT_NONE;

        delete srt_rc;
        delete srt_cat;
    }
    else
    {

rreq_forward:

        rreq_opt->addrs.push_back(myaddr.s_addr);
        rreq_opt->length += DSR_ADDRESS_SIZE;
        dp->dh.opth.begin()->p_len += DSR_ADDRESS_SIZE;
#ifdef __KERNEL__
        dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                     ntohs(dp->nh.iph->tot_len) +
                     sizeof(struct in_addr), IPPROTO_DSR,
                     dp->nh.iph->ttl);
#endif

        /* Forward RREQ */
        action = DSR_PKT_FORWARD_RREQ;
    }
out:
    delete srt_rev;
    return action;
}

#ifdef __KERNEL__

static int
rreq_tbl_proc_info(char *buffer, char **start, off_t offset, int length)
{
    int len;

    len = rreq_tbl_print(&rreq_tbl, buffer);

    *start = buffer + offset;
    len -= offset;
    if (len > length)
        len = length;
    else if (len < 0)
        len = 0;
    return len;
}

#endif              /* __KERNEL__ */

int __init NSCLASS rreq_tbl_init(void)
{
    rreq_seqno = 0;
    return 0;
}

void __exit NSCLASS rreq_tbl_cleanup(void)
{
    while (!dsrRreqTbl.empty())
    {
        auto it =  dsrRreqTbl.begin();
        rreq_tbl_entry *e = it->second;
        del_timer_sync(e->timer);
        delete e;
        dsrRreqTbl.erase(it);
    }
}

#ifdef OMNETPP
void NSCLASS rreq_timer_test(cMessage *msg)
{
    for (auto it =  dsrRreqTbl.begin();it !=dsrRreqTbl.end();++it)
    {
        if (it->second->timer->testAndExcute(msg))
            return;
    }
}
} // namespace inetmanet

} // namespace inet

#endif
