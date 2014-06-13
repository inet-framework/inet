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
#include <linux/string.h>
#include <linux/if_ether.h>
#include <net/ip.h>
#include "dsr-dev.h"

#endif              /* __KERNEL__ */

#ifndef OMNETPP
#ifdef NS2
#include "ns-agent.h"
#endif
#else
#include "dsr-uu-omnetpp.h"
#endif


#include "dsr.h"
#include "debug_dsr.h"
#include "tbl.h"
#include "dsr-rrep.h"
#include "dsr-rreq.h"
#include "dsr-opt.h"
#include "dsr-srt.h"
#include "link-cache.h"
#include "send-buf.h"
#include "timer.h"

#define GRAT_RREP_TBL_MAX_LEN 64
#define GRAT_REPLY_HOLDOFF 1

#ifdef __KERNEL__
#define GRAT_RREP_TBL_PROC_NAME "dsr_grat_rrep_tbl"
static TBL(grat_rrep_tbl, GRAT_RREP_TBL_MAX_LEN);
DSRUUTimer grat_rrep_tbl_timer;
#endif

struct grat_rrep_entry
{
    dsr_list_t l;
    struct in_addr src, prev_hop;
    struct timeval expires;
};

struct grat_rrep_query
{
    struct in_addr *src, *prev_hop;
};

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

void NSCLASS grat_rrep_tbl_timeout(unsigned long data)
{
    struct grat_rrep_entry *e =
        (struct grat_rrep_entry *)tbl_detach_first(&grat_rrep_tbl);

    FREE(e);

    if (tbl_empty(&grat_rrep_tbl))
        return;

    DSR_READ_LOCK(&grat_rrep_tbl.lock);

    e = (struct grat_rrep_entry *)TBL_FIRST(&grat_rrep_tbl);

    grat_rrep_tbl_timer.function = &NSCLASS grat_rrep_tbl_timeout;

    set_timer(&grat_rrep_tbl_timer, &e->expires);

    DSR_READ_UNLOCK(&grat_rrep_tbl.lock);
}

int NSCLASS grat_rrep_tbl_add(struct in_addr src, struct in_addr prev_hop)
{
    struct grat_rrep_query q = { &src, &prev_hop };
    struct grat_rrep_entry *e;

    if (in_tbl(&grat_rrep_tbl, &q, crit_query))
        return 0;

    e = (struct grat_rrep_entry *)MALLOC(sizeof(struct grat_rrep_entry),
                                         GFP_ATOMIC);

    if (!e)
        return -1;

    e->src = src;
    e->prev_hop = prev_hop;
    gettime(&e->expires);

    timeval_add_usecs(&e->expires, ConfValToUsecs(GratReplyHoldOff));

    /* Remove timer before we modify the table */
    if (timer_pending(&grat_rrep_tbl_timer))
        del_timer_sync(&grat_rrep_tbl_timer);

    if (tbl_add(&grat_rrep_tbl, &e->l, crit_time))
    {

        DSR_READ_LOCK(&grat_rrep_tbl.lock);
        e = (struct grat_rrep_entry *)TBL_FIRST(&grat_rrep_tbl);

        grat_rrep_tbl_timer.function = &NSCLASS grat_rrep_tbl_timeout;
        set_timer(&grat_rrep_tbl_timer, &e->expires);
        DSR_READ_UNLOCK(&grat_rrep_tbl.lock);
    }
    return 1;
}

int NSCLASS grat_rrep_tbl_find(struct in_addr src, struct in_addr prev_hop)
{
    struct grat_rrep_query q = { &src, &prev_hop };

    if (in_tbl(&grat_rrep_tbl, &q, crit_query))
        return 1;
    return 0;
}

#ifdef __KERNEL__

static int grat_rrep_tbl_print(struct tbl *t, char *buf)
{
    dsr_list_t *pos;
    int len = 0;
    struct timeval now;

    gettime(&now);

    DSR_READ_LOCK(&t->lock);

    len += sprintf(buf, "# %-15s %-15s Time\n", "Source", "Prev hop");

    list_for_each(pos, &t->head)
    {
        struct grat_rrep_entry *e = (struct grat_rrep_entry *)pos;

        len += sprintf(buf + len, "  %-15s %-15s %lu\n",
                       print_ip(e->src),
                       print_ip(e->prev_hop),
                       timeval_diff(&e->expires, &now) / 1000000);
    }

    DSR_READ_UNLOCK(&t->lock);

    return len;
}

static int
grat_rrep_tbl_proc_info(char *buffer, char **start, off_t offset, int length)
{
    int len;

    len = grat_rrep_tbl_print(&grat_rrep_tbl, buffer);

    *start = buffer + offset;
    len -= offset;
    if (len > length)
        len = length;
    else if (len < 0)
        len = 0;
    return len;
}

#endif              /* __KERNEL__ */

static inline int
dsr_rrep_add_srt(struct dsr_rrep_opt *rrep_opt, struct dsr_srt *srt)
{
    int n;

    if (!rrep_opt | !srt)
        return -1;

    n = srt->laddrs / sizeof(struct in_addr);

    memcpy(rrep_opt->addrs, srt->addrs, srt->laddrs);
    rrep_opt->addrs[n] = srt->dst.s_addr;

    return 0;
}

static struct dsr_rrep_opt *dsr_rrep_opt_add(char *buf, int len,
        struct dsr_srt *srt)
{
    struct dsr_rrep_opt *rrep_opt;

    if (!buf || !srt || (unsigned int)len < DSR_RREP_OPT_LEN(srt))
        return NULL;

    rrep_opt = (struct dsr_rrep_opt *)buf;

    rrep_opt->type = DSR_OPT_RREP;
    rrep_opt->length = srt->laddrs + sizeof(struct in_addr) + 1;
    rrep_opt->l = 0;
    rrep_opt->res = 0;

    /* Add source route to RREP */
    dsr_rrep_add_srt(rrep_opt, srt);

    return rrep_opt;
}

int NSCLASS dsr_rrep_send(struct dsr_srt *srt, struct dsr_srt *srt_to_me)
{
    struct dsr_pkt *dp = NULL;
    char *buf;
    int len, ttl, n;

    if (!srt || !srt_to_me)
        return -1;

    dp = dsr_pkt_alloc(NULL);

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

    n = srt->laddrs / sizeof(struct in_addr);

    DEBUG("srt: %s\n", print_srt(srt));
    DEBUG("srt_to_me: %s\n", print_srt(srt_to_me));
    DEBUG("next_hop=%s\n", print_ip(dp->nxt_hop));
    DEBUG
    ("IP_HDR_LEN=%d DSR_OPT_HDR_LEN=%d DSR_SRT_OPT_LEN=%d DSR_RREP_OPT_LEN=%d DSR_OPT_PAD1_LEN=%d RREP len=%d\n",
     IP_HDR_LEN, DSR_OPT_HDR_LEN, DSR_SRT_OPT_LEN(srt),
     DSR_RREP_OPT_LEN(srt_to_me), DSR_OPT_PAD1_LEN, len);

    ttl = n + 1;

    DEBUG("TTL=%d, n=%d\n", ttl, n);

    buf = dsr_pkt_alloc_opts(dp, len);

    if (!buf)
        goto out_err;

    dp->nh.iph = dsr_build_ip(dp, dp->src, dp->dst, IP_HDR_LEN,
                              IP_HDR_LEN + len, IPPROTO_DSR, ttl);

    if (!dp->nh.iph)
    {
        DEBUG("Could not create IP header\n");
        goto out_err;
    }

    dp->dh.opth = dsr_opt_hdr_add(buf, len, DSR_NO_NEXT_HDR_TYPE);

    if (!dp->dh.opth)
    {
        DEBUG("Could not create DSR options header\n");
        goto out_err;
    }

    buf += DSR_OPT_HDR_LEN;
    len -= DSR_OPT_HDR_LEN;

    /* Add the source route option to the packet */
    dp->srt_opt = dsr_srt_opt_add(buf, len, 0, dp->salvage, srt);

    if (!dp->srt_opt)
    {
        DEBUG("Could not create Source Route option header\n");
        goto out_err;
    }

    buf += DSR_SRT_OPT_LEN(srt);
    len -= DSR_SRT_OPT_LEN(srt);

    dp->rrep_opt[dp->num_rrep_opts++] =
        dsr_rrep_opt_add(buf, len, srt_to_me);

    if (!dp->rrep_opt[dp->num_rrep_opts - 1])
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
    AddCost(dp,srt_to_me);
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

    if (dp->num_rrep_opts < MAX_RREP_OPTS)
        dp->rrep_opt[dp->num_rrep_opts++] = rrep_opt;
    else
        return DSR_PKT_ERROR;

    myaddr = my_addr();

    srt_dst.s_addr = rrep_opt->addrs[DSR_RREP_ADDRS_LEN(rrep_opt) / sizeof(struct in_addr)];
#ifndef OMNETPP
    rrep_opt_srt = dsr_srt_new(dp->dst, srt_dst,
                               DSR_RREP_ADDRS_LEN(rrep_opt),
                               (char *)rrep_opt->addrs);
#else
    rrep_opt_srt = dsr_srt_new(dp->dst, srt_dst,
                               DSR_RREP_ADDRS_LEN(rrep_opt),
                               (char *)rrep_opt->addrs,dp->costVector,dp->costVectorSize);
#endif
    if (!rrep_opt_srt)
        return DSR_PKT_ERROR;

    dsr_rtc_add(rrep_opt_srt, ConfValToUsecs(RouteCacheTimeout), 0);

    /* Remove pending RREQs */
    rreq_tbl_route_discovery_cancel(rrep_opt_srt->dst);

    FREE(rrep_opt_srt);

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
    INIT_TBL(&grat_rrep_tbl, GRAT_RREP_TBL_MAX_LEN);

    init_timer(&grat_rrep_tbl_timer);

#ifdef __KERNEL__
    proc_net_create(GRAT_RREP_TBL_PROC_NAME, 0, grat_rrep_tbl_proc_info);
#endif
    return 0;
}

void __exit NSCLASS grat_rrep_tbl_cleanup(void)
{
    tbl_flush(&grat_rrep_tbl, NULL);

    del_timer_sync(&grat_rrep_tbl_timer);

#ifdef __KERNEL__
    proc_net_remove(GRAT_RREP_TBL_PROC_NAME);
#endif
}
