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
#include <linux/timer.h>
#include <net/ip.h>
#include <linux/random.h>

#include "dsr-dev.h"
#endif

#ifndef OMNETPP
#ifdef NS2
#include "ns-agent.h"
#endif
#else
#include "dsr-uu-omnetpp.h"
#endif


#include "debug_dsr.h"
#include "dsr.h"
#include "tbl.h"
#include "dsr-rrep.h"
#include "dsr-rreq.h"
#include "dsr-opt.h"
#include "link-cache.h"
#include "send-buf.h"
#include "neigh.h"

#ifndef OMNETPP
#ifndef NS2

#define RREQ_TBL_PROC_NAME "dsr_rreq_tbl"

static TBL(rreq_tbl, RREQ_TBL_MAX_LEN);
static unsigned int rreq_seqno;
#endif
#endif

#ifndef MAXTTL
#define MAXTTL 255
#endif

#define STATE_IDLE          0
#define STATE_IN_ROUTE_DISC 1

struct rreq_tbl_entry
{
    list_t l;
    int state;
    struct in_addr node_addr;
    int ttl;
    DSRUUTimer *timer;
    struct timeval tx_time;
    struct timeval last_used;
    usecs_t timeout;
    unsigned int num_rexmts;
    struct tbl rreq_id_tbl;
};

struct id_entry_route
{
    list_t l;
    double cost;
    unsigned int length;
    char * add;
};

struct id_entry
{
    list_t l;
    struct in_addr trg_addr;
    unsigned short id;
    struct tbl rreq_id_tbl_routes;
};

struct rreq_tbl_query
{
    struct in_addr *initiator;
    struct in_addr *target;
    unsigned int *id;
    double *cost;
    unsigned int *length;
    char **addrs;
};

static inline int crit_addr(void *pos, void *data)
{
    struct rreq_tbl_entry *e = (struct rreq_tbl_entry *)pos;
    struct in_addr *a = (struct in_addr *)data;

    if (e->node_addr.s_addr == a->s_addr)
        return 1;
    return 0;
}



static inline int crit_delete_tbl_enty(void *pos, void *data)
{

    list_t *p,*tmp;
    struct id_entry *e = (struct id_entry *)pos;
    list_for_each_safe(p,tmp, &e->rreq_id_tbl_routes.head)
    {
        list_del(p);
        struct id_entry_route *id_e = (struct id_entry_route *)p;
        if (id_e->add!=NULL)
            FREE (id_e->add);
        e->rreq_id_tbl_routes.len--;
        FREE(p);
    }
    return 1;
}


static inline int crit_duplicate_path(struct id_entry *e, struct rreq_tbl_query *q)
{
    list_t *p;
    id_entry_route *worst=NULL;
    list_for_each(p, &e->rreq_id_tbl_routes.head)
    {
        struct id_entry_route *id_e = (struct id_entry_route *)p;
        if (id_e->length<*(q->length))
            return 1;
        if (id_e->length==*(q->length) && memcmp(id_e->add,*(q->addrs),id_e->length)==0)
            return 1;
        if (worst==NULL)
            worst = id_e;
        else
        {
            if (worst != id_e && id_e->cost>= worst->cost)
                worst = id_e;
        }
    }
    /*
        if (worst->cost>=*(q->cost))
            return 0;
        else
            return 1;
    */
    return 0;
}

static inline int crit_duplicate_2(void *pos, void *data)
{
    struct rreq_tbl_entry *e = (struct rreq_tbl_entry *)pos;
    struct rreq_tbl_query *q = (struct rreq_tbl_query *)data;

    if (e->node_addr.s_addr == q->initiator->s_addr)
    {
        list_t *p;
        list_for_each(p, &e->rreq_id_tbl.head)
        {
            struct id_entry *id_e = (struct id_entry *)p;
            if (id_e->trg_addr.s_addr == q->target->s_addr &&
                    id_e->id == *(q->id))
            {
                list_t *pos;
                list_for_each(pos, &id_e->rreq_id_tbl_routes.head)
                {
                    struct id_entry_route *id_e_route = (struct id_entry_route *)pos;
                    if (id_e_route->length<*(q->length))
                        return 1;
                    if (id_e_route->length==*(q->length) &&
                            memcmp(id_e_route->add,*(q->addrs),id_e_route->length)==0)
                        return 1;
                }
            }
        }
    }
    return 0;
}


static inline int crit_duplicate(void *pos, void *data)
{
    struct rreq_tbl_entry *e = (struct rreq_tbl_entry *)pos;
    struct rreq_tbl_query *q = (struct rreq_tbl_query *)data;

    if (e->node_addr.s_addr == q->initiator->s_addr)
    {
        list_t *p;
        list_for_each(p, &e->rreq_id_tbl.head)
        {
            struct id_entry *id_e = (struct id_entry *)p;

            if (id_e->trg_addr.s_addr == q->target->s_addr &&
                    id_e->id == *(q->id))
            {

                return 1;
            }
        }
    }
    return 0;
}



void NSCLASS rreq_tbl_set_max_len(unsigned int max_len)
{
    rreq_tbl.max_len = max_len;
}
#ifdef __KERNEL__
static int rreq_tbl_print(struct tbl *t, char *buf)
{
    list_t *pos1, *pos2;
    int len = 0;
    int first = 1;
    struct timeval now;

    gettime(&now);

    DSR_READ_LOCK(&t->lock);

    len +=
        sprintf(buf, "# %-15s %-6s %-8s %15s:%s\n", "IPAddr", "TTL", "Used",
                "TargetIPAddr", "ID");

    list_for_each(pos1, &t->head)
    {
        struct rreq_tbl_entry *e = (struct rreq_tbl_entry *)pos1;
        struct id_entry *id_e;

        if (TBL_EMPTY(&e->rreq_id_tbl))
            len +=
                sprintf(buf + len, "  %-15s %-6u %-8lu %15s:%s\n",
                        print_ip(e->node_addr), e->ttl,
                        timeval_diff(&now, &e->last_used) / 1000000,
                        "-", "-");
        else
        {
            id_e = (struct id_entry *)TBL_FIRST(&e->rreq_id_tbl);
            len +=
                sprintf(buf + len, "  %-15s %-6u %-8lu %15s:%u\n",
                        print_ip(e->node_addr), e->ttl,
                        timeval_diff(&now, &e->last_used) / 1000000,
                        print_ip(id_e->trg_addr), id_e->id);
        }
        list_for_each(pos2, &e->rreq_id_tbl.head)
        {
            id_e = (struct id_entry *)pos2;
            if (!first)
                len +=
                    sprintf(buf + len, "%49s:%u\n",
                            print_ip(id_e->trg_addr), id_e->id);
            first = 0;
        }
    }

    DSR_READ_UNLOCK(&t->lock);
    return len;

}
#endif /* __KERNEL__ */

void NSCLASS rreq_tbl_timeout(unsigned long data)
{
    struct rreq_tbl_entry *e = (struct rreq_tbl_entry *)data;
    struct timeval expires;

    if (!e)
        return;

    tbl_detach(&rreq_tbl, &e->l);

    DEBUG("RREQ Timeout dst=%s timeout=%lu rexmts=%d \n",
          print_ip(e->node_addr), e->timeout, e->num_rexmts);

    if (e->num_rexmts >= ConfVal(MaxRequestRexmt))
    {
        DEBUG("MAX RREQs reached for %s\n", print_ip(e->node_addr));

        e->state = STATE_IDLE;

        /*      DSR_WRITE_UNLOCK(&rreq_tbl); */
        //if (e->timer)
        delete e->timer;
        FREE(e);
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

    /* Put at end of list */
    tbl_add_tail(&rreq_tbl, &e->l);

    set_timer(e->timer, &expires);
}

struct rreq_tbl_entry *NSCLASS __rreq_tbl_entry_create(struct in_addr node_addr)
{
    struct rreq_tbl_entry *e;

    e = (struct rreq_tbl_entry *)MALLOC(sizeof(struct rreq_tbl_entry),
                                        GFP_ATOMIC);

    if (!e)
        return NULL;

    memset(e, 0, sizeof(struct rreq_tbl_entry));
    e->state = STATE_IDLE;
    e->node_addr = node_addr;
    e->ttl = 0;
    memset(&e->tx_time, 0, sizeof(struct timeval));
    e->num_rexmts = 0;
    e->timer=NULL;
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
        FREE(e);
        return NULL;
    }
    init_timer(e->timer);


    e->timer->function = &NSCLASS rreq_tbl_timeout;
    e->timer->data = (unsigned long)e;

    INIT_TBL(&e->rreq_id_tbl, ConfVal(RequestTableIds));

    return e;
}

struct rreq_tbl_entry *NSCLASS __rreq_tbl_add(struct in_addr node_addr)
{
    struct rreq_tbl_entry *e;

    e = __rreq_tbl_entry_create(node_addr);

    if (!e)
        return NULL;

    if (TBL_FULL(&rreq_tbl))
    {
        struct rreq_tbl_entry *f;

        f = (struct rreq_tbl_entry *)TBL_FIRST(&rreq_tbl);

        __tbl_detach(&rreq_tbl, &f->l);

        del_timer_sync(f->timer);
#ifndef OMNETPP
#ifdef NS2
        delete f->timer;
#else
        FREE(f->timer);
#endif
#else
        delete f->timer;
#endif
        tbl_flush(&f->rreq_id_tbl, NULL);

        FREE(f);
    }
    __tbl_add_tail(&rreq_tbl, &e->l);

    return e;
}

int NSCLASS
rreq_tbl_add_id(struct in_addr initiator, struct in_addr target,
                unsigned short id,double cost,char *addr,int length)
{
    struct rreq_tbl_entry *e;
    struct id_entry *id_e;
    struct id_entry *id_entry=NULL;
    int exist=1;
    list_t *pos;
    struct id_entry_route *id_r;

    int res = 0;

    DSR_WRITE_LOCK(&rreq_tbl.lock);

    e = (struct rreq_tbl_entry *)__tbl_find(&rreq_tbl, &initiator,
                                            crit_addr);

    if (!e)
        e = __rreq_tbl_add(initiator);
    else
    {
        /* Put it last in the table */
        __tbl_detach(&rreq_tbl, &e->l);
        __tbl_add_tail(&rreq_tbl, &e->l);
    }

    if (!e)
    {
        res = -ENOMEM;
        goto out;
    }

    gettime(&e->last_used);

    list_for_each(pos, &e->rreq_id_tbl.head)
    {
        id_e = (struct id_entry *)pos;
        if ((id ==id_e->id) && (id_e->trg_addr.s_addr == target.s_addr))
        {
            exist=0;
            id_entry= id_e;
        }

    }


    if (exist)
    {
        if (TBL_FULL(&e->rreq_id_tbl))
            tbl_del_first(&e->rreq_id_tbl);
        id_e = (struct id_entry *)MALLOC(sizeof(struct id_entry), GFP_ATOMIC);
        if (!id_e)
        {
            res = -ENOMEM;
            goto out;
        }
        memset(id_e, 0, sizeof(struct id_entry));
        INIT_TBL(&id_e->rreq_id_tbl_routes,(ConfVal(RREQMaxVisit)));
        if (!TBL_FULL(&id_e->rreq_id_tbl_routes))
        {
            id_r = (struct id_entry_route *)MALLOC(sizeof(struct id_entry_route), GFP_ATOMIC);
            if (length>0)
                id_r->add = (char*)MALLOC(length, GFP_ATOMIC);
            else
                id_r->add =NULL;
            id_r->length=length;
            id_r->cost=cost;
            memcpy(id_r->add,addr,length);
            tbl_add_tail(&id_e->rreq_id_tbl_routes, &id_r->l);
        }

        id_e->trg_addr = target;
        id_e->id = id;
        tbl_add_tail(&e->rreq_id_tbl, &id_e->l);
    }
    else
    {
        if (ConfVal(RREQMulVisit))
        {
            if (TBL_FULL(&id_entry->rreq_id_tbl_routes))
                goto out;
            list_for_each(pos, &id_entry->rreq_id_tbl_routes.head)
            {
                id_r = (struct id_entry_route *)pos;
                if ((int)id_r->length<length)
                    goto out;
                if ((int)id_r->length==length && memcmp(id_r->add,addr,length)==0)
                    goto out;
            }
            id_r = (struct id_entry_route *)MALLOC(sizeof(struct id_entry_route), GFP_ATOMIC);
            if (length>0)
                id_r->add = (char*)MALLOC(length, GFP_ATOMIC);
            else
                id_r->add =NULL;
            id_r->length=length;
            id_r->cost=cost;
            memcpy(id_r->add,addr,length);
            tbl_add_tail(&id_entry->rreq_id_tbl_routes, &id_r->l);
        }
    }
out:
    DSR_WRITE_UNLOCK(&rreq_tbl.lock);

    return 1;
}

int NSCLASS rreq_tbl_route_discovery_cancel(struct in_addr dst)
{
    struct rreq_tbl_entry *e;

    e = (struct rreq_tbl_entry *)tbl_find_detach(&rreq_tbl, &dst,
            crit_addr);

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
    delete e->timer;
    FREE(e);
    //tbl_add_tail(&rreq_tbl, &e->l);

    return 1;
}

int NSCLASS dsr_rreq_route_discovery(struct in_addr target)
{
    struct rreq_tbl_entry *e;
    int ttl, res = 0;
    struct timeval expires;

#define TTL_START 10

    DSR_WRITE_LOCK(&rreq_tbl.lock);

    e = (struct rreq_tbl_entry *)__tbl_find(&rreq_tbl, &target, crit_addr);

    if (!e)
        e = __rreq_tbl_add(target);
    else
    {
        /* Put it last in the table */
        __tbl_detach(&rreq_tbl, &e->l);
        __tbl_add_tail(&rreq_tbl, &e->l);
    }

    if (!e)
    {
        res = -ENOMEM;
        goto out;
    }

    if (e->state == STATE_IN_ROUTE_DISC)
    {
        DEBUG("Route discovery for %s already in progress\n",
              print_ip(target));
        goto out;
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

    DSR_WRITE_UNLOCK(&rreq_tbl.lock);

    dsr_rreq_send(target, ttl);

    return 1;
out:
    DSR_WRITE_UNLOCK(&rreq_tbl.lock);

    return res;
}

int NSCLASS dsr_rreq_duplicate(struct in_addr initiator, struct in_addr target,
                               unsigned int id,double cost,unsigned int length, char *addrs)
{
    struct
    {
        struct in_addr *initiator;
        struct in_addr *target;
        unsigned int *id;
        double *cost;
        unsigned int *length;
        char **addrs;
    } d;

    d.initiator = &initiator;
    d.target = &target;
    d.id = &id;
    d.cost=&cost;
    d.length=&length;
    d.addrs=&addrs;
    if (ConfVal(RREQMulVisit))
        return in_tbl(&rreq_tbl, &d, crit_duplicate_2);
    else
        return in_tbl(&rreq_tbl, &d, crit_duplicate);

}

static struct dsr_rreq_opt *dsr_rreq_opt_add(char *buf, unsigned int len,
        struct in_addr target,
        unsigned int seqno)
{
    struct dsr_rreq_opt *rreq_opt;

    if (!buf || len < DSR_RREQ_HDR_LEN)
        return NULL;

    rreq_opt = (struct dsr_rreq_opt *)buf;

    rreq_opt->type = DSR_OPT_RREQ;
    rreq_opt->length = 6;
    rreq_opt->id = htons(seqno);
    rreq_opt->target = target.s_addr;

    return rreq_opt;
}

int NSCLASS dsr_rreq_send(struct in_addr target, int ttl)
{
    struct dsr_pkt *dp;
    char *buf;
    int len = DSR_OPT_HDR_LEN + DSR_RREQ_HDR_LEN;

    dp = dsr_pkt_alloc(NULL);

    if (!dp)
    {
        DEBUG("Could not allocate DSR packet\n");
        return -1;
    }
    dp->dst.s_addr = DSR_BROADCAST;
    dp->nxt_hop.s_addr = DSR_BROADCAST;
    dp->src = my_addr();

    buf = dsr_pkt_alloc_opts(dp, len);


    if (!buf)
        goto out_err;

    dp->nh.iph =
        dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN, IP_HDR_LEN + len,
                     IPPROTO_DSR, ttl);

    if (!dp->nh.iph)
        goto out_err;

    dp->dh.opth = dsr_opt_hdr_add(buf, len, DSR_NO_NEXT_HDR_TYPE);

    if (!dp->dh.opth)
    {
        DEBUG("Could not create DSR opt header\n");
        goto out_err;
    }

    buf += DSR_OPT_HDR_LEN;
    len -= DSR_OPT_HDR_LEN;

    dp->rreq_opt = dsr_rreq_opt_add(buf, len, target, ++rreq_seqno);

    if (!dp->rreq_opt)
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

    if (!dp || !rreq_opt || dp->flags & PKT_PROMISC_RECV)
        return DSR_PKT_DROP;

    dp->num_rreq_opts++;

    if (dp->num_rreq_opts > 1)
    {
        DEBUG("More than one RREQ opt!!! - Ignoring\n");
        return DSR_PKT_ERROR;
    }

    dp->rreq_opt = rreq_opt;

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

    if (dsr_rreq_duplicate(dp->src, trg, ntohs(rreq_opt->id),cost,DSR_RREQ_ADDRS_LEN(rreq_opt),(char *)rreq_opt->addrs))
    {
        DEBUG("Duplicate RREQ from %s\n", print_ip(dp->src));
        return DSR_PKT_DROP;
    }

    rreq_tbl_add_id(dp->src, trg, ntohs(rreq_opt->id),cost,(char *)rreq_opt->addrs,DSR_RREQ_ADDRS_LEN(rreq_opt));
#ifdef OMNETPP
    ExpandCost(dp);
    dp->srt = dsr_srt_new(dp->src, myaddr, DSR_RREQ_ADDRS_LEN(rreq_opt),
                          (char *)rreq_opt->addrs,dp->costVector,dp->costVectorSize);
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

    /* Send buffered packets */
    send_buf_set_verdict(SEND_BUF_SEND, srt_rev->dst);

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

    srt_rc = NULL;
    if (ConfVal(RREPDestinationOnly)==0)
    {
#ifdef OMNETPP
        if (ConfVal(PathCache))
            srt_rc = ph_srt_find(myaddr, trg,0,0);
        else
            srt_rc = lc_srt_find(myaddr, trg);
#else
        srt_rc = lc_srt_find(myaddr, trg);
#endif
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
            FREE(srt_rc);
            goto rreq_forward;
        }

        DEBUG("srt_cat: %s\n", print_srt(srt_cat));

        if (dsr_srt_check_duplicate(srt_cat) > 0)
        {
            DEBUG("Duplicate address in source route!!!\n");
            FREE(srt_rc);
            FREE(srt_cat);
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

        FREE(srt_rc);
        FREE(srt_cat);
    }
    else
    {

rreq_forward:
        dsr_pkt_alloc_opts_expand(dp, sizeof(struct in_addr));

        if (!DSR_LAST_OPT(dp, rreq_opt))
        {
            char *to, *from;
            to = (char *)rreq_opt + rreq_opt->length + 2 +
                 sizeof(struct in_addr);
            from = (char *)rreq_opt + rreq_opt->length + 2;

            memmove(to, from, sizeof(struct in_addr));
        }
        rreq_opt->addrs[n] = myaddr.s_addr;
        rreq_opt->length += sizeof(struct in_addr);

        dp->dh.opth->p_len = htons(ntohs(dp->dh.opth->p_len) +
                                   sizeof(struct in_addr));
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
    FREE(srt_rev);
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
    INIT_TBL(&rreq_tbl, RREQ_TBL_MAX_LEN);

#ifdef __KERNEL__
    proc_net_create(RREQ_TBL_PROC_NAME, 0, rreq_tbl_proc_info);
    get_random_bytes(&rreq_seqno, sizeof(unsigned int));
#else
    rreq_seqno = 0;
#endif
    return 0;
}

void __exit NSCLASS rreq_tbl_cleanup(void)
{
    struct rreq_tbl_entry *e;

    while ((e = (struct rreq_tbl_entry *)tbl_detach_first(&rreq_tbl)))
    {
        del_timer_sync(e->timer);

#ifdef NS2
        delete e->timer;
#elif defined(OMNETPP)
        delete e->timer;
#else
        FREE(e->timer);
#endif
        // tbl_flush(&e->rreq_id_tbl, crit_none);
        tbl_flush(&e->rreq_id_tbl, crit_delete_tbl_enty);
    }
#ifdef __KERNEL__
    proc_net_remove(RREQ_TBL_PROC_NAME);
#endif
}


#ifdef OMNETPP
void NSCLASS rreq_timer_test(cMessage *msg)
{
    list_t *pos1;
    list_t *head;
    head = &rreq_tbl.head;
    list_for_each(pos1, head)
    {
        struct rreq_tbl_entry *e = (struct rreq_tbl_entry *)pos1;
//      struct id_entry *id_e;
        if (e->timer->testAndExcute(msg))
            return;
    }
}
#endif
