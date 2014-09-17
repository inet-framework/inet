/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */

#define OMNETPP
#ifdef __KERNEL__
#include <linux/config.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <net/sock.h>
#include <linux/icmp.h>
#include <net/icmp.h>
#endif

#ifndef OMNETPP
#ifdef NS2
#include "inet/routing/extras/dsr/dsr-uu/ns-agent.h"
#endif
#else
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#endif

#include "inet/routing/extras/dsr/dsr-uu/tbl.h"
#include "inet/routing/extras/dsr/dsr-uu/send-buf.h"
#include "inet/routing/extras/dsr/dsr-uu/debug_dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/link-cache.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-srt.h"
#include "inet/routing/extras/dsr/dsr-uu/timer.h"

#ifdef __KERNEL__
#define SEND_BUF_PROC_FS_NAME "send_buf"

TBL(send_buf, SEND_BUF_MAX_LEN);
static DSRUUTimer send_buf_timer;
static int send_buf_print(struct tbl *t, char *buffer);
#endif

namespace inet {

namespace inetmanet {

struct send_buf_entry
{
    dsr_list_t l;
    struct dsr_pkt *dp;
    struct timeval qtime;
    xmit_fct_t okfn;
};


static inline int crit_addr(void *pos, void *addr)
{
    struct in_addr *a = (struct in_addr *)addr;
    struct send_buf_entry *e = (struct send_buf_entry *)pos;

    if (e->dp->dst.s_addr == a->s_addr)
        return 1;
    return 0;
}

static inline int crit_garbage(void *pos, void *n)
{
    struct timeval *now = (struct timeval *)n;
    struct send_buf_entry *e = (struct send_buf_entry *)pos;

    if (timeval_diff(now, &e->qtime) >=
            (int)ConfValToUsecs(SendBufferTimeout))
    {
        if (e->dp)
            dsr_pkt_free(e->dp);
        return 1;
    }
    return 0;
}



void NSCLASS send_buf_set_max_len(unsigned int max_len)
{
    send_buf.max_len = max_len;
}

void NSCLASS send_buf_timeout(unsigned long data)
{
    struct send_buf_entry *e;
    int pkts;
    struct timeval expires, now;
    /*  char buf[2048]; */

    gettime(&now);

    /*  send_buf_print(&send_buf, buf); */
    /*  DEBUG("\n%s\n", buf); */

    pkts = tbl_for_each_del(&send_buf, &now, crit_garbage);

    DEBUG("%d packets garbage collected\n", pkts);

    DSR_READ_LOCK(&send_buf.lock);
    /* Get first packet in maintenance buffer */
    e = (struct send_buf_entry *)__tbl_find(&send_buf, NULL, crit_none);

    if (!e)
    {
        DEBUG("No packet to set timeout for\n");
        DSR_READ_UNLOCK(&send_buf.lock);
        return;
    }
    expires = e->qtime;

    timeval_add_usecs(&expires, ConfValToUsecs(SendBufferTimeout));

    DEBUG("now=%s qtime=%s exp=%s\n", print_timeval(&now), print_timeval(&e->qtime), print_timeval(&expires));
    DSR_READ_UNLOCK(&send_buf.lock);

    set_timer(&send_buf_timer, &expires);
}
#ifdef OMNETPP
struct send_buf_entry * NSCLASS send_buf_entry_create(struct dsr_pkt *dp,xmit_fct_t okfn)
#else
static struct send_buf_entry *  send_buf_entry_create(struct dsr_pkt *dp,xmit_fct_t okfn)
#endif
{
    struct send_buf_entry *e;

    e = (struct send_buf_entry *)MALLOC(sizeof(*e), GFP_ATOMIC);

    if (!e)
        return NULL;

    e->dp = dp;
    e->okfn = okfn;
    gettime(&e->qtime);

    return e;
}

int NSCLASS send_buf_enqueue_packet(struct dsr_pkt *dp, xmit_fct_t okfn)
{
    struct send_buf_entry *e;
    struct timeval expires;
    int res, empty = 0;

    if (tbl_empty(&send_buf))
        empty = 1;

    e = send_buf_entry_create(dp, okfn);

    if (!e)
        return -ENOMEM;

    DEBUG("enqueing packet to %s\n", print_ip(dp->dst));

    res = tbl_add_tail(&send_buf, &e->l);

    if (res < 0)
    {
        struct send_buf_entry *f;

        DEBUG("buffer full, removing first\n");
        f = (struct send_buf_entry *)tbl_detach_first(&send_buf);

        if (f)
        {
            dsr_pkt_free(f->dp);
            FREE(f);
        }

        res = tbl_add_tail(&send_buf, &e->l);

        if (res < 0)
        {
            DEBUG("Could not buffer packet\n");
            FREE(e);
            return -1;
        }
    }

    if (empty)
    {
        gettime(&expires);
        timeval_add_usecs(&expires, ConfValToUsecs(SendBufferTimeout));
        set_timer(&send_buf_timer, &expires);
    }

    return res;
}

int NSCLASS send_buf_set_verdict(int verdict, struct in_addr dst)
{
    struct send_buf_entry *e;
    int pkts = 0;

    switch (verdict)
    {
    case SEND_BUF_DROP:

        while ((e =
                    (struct send_buf_entry *)tbl_find_detach(&send_buf,
                            &dst,
                            crit_addr)))
        {
            /* Only send one ICMP message */
#ifdef __KERNEL__
            if (pkts == 0)
                icmp_send(e->dp->skb, ICMP_DEST_UNREACH,
                          ICMP_HOST_UNREACH, 0);
#endif
            dsr_pkt_free(e->dp);
            FREE(e);
            pkts++;
        }
        DEBUG("Dropped %d queued pkts for %s\n", pkts, print_ip(dst));
        break;
    case SEND_BUF_SEND:

        while ((e =
                    (struct send_buf_entry *)tbl_find_detach(&send_buf,
                            &dst,
                            crit_addr)))
        {
            DEBUG("Send packet\n");
            /* Get source route */
            e->dp->srt = dsr_rtc_find(e->dp->src, e->dp->dst);

            if (e->dp->srt)
            {

                if (dsr_srt_add(e->dp) < 0)
                {
                    DEBUG("Could not add source route\n");
                    dsr_pkt_free(e->dp);
                }
                else
                    /* Send packet */
#ifdef OMNETPP

                    AddCost(e->dp,e->dp->srt);
                (this->*e->okfn) (e->dp);
#else
#ifdef NS2
                    (this->*e->okfn) (e->dp);
#else
                e->okfn(e->dp);
#endif
#endif
            }
            else
            {
                DEBUG("No source route found for %s!\n",
                      print_ip(dst));

                dsr_pkt_free(e->dp);
            }
            pkts++;
            FREE(e);
        }
        DEBUG("Sent %d queued packets to %s\n", pkts, print_ip(dst));

        /*      if (pkts == 0) */
        /*          DEBUG("No packets for dest %s\n", print_ip(dst)); */
        break;
    }
    return pkts;
}

static inline int send_buf_flush(struct tbl *t)
{
    struct send_buf_entry *e;
    int pkts = 0;
    /* Flush send buffer */
    while ((e =
                (struct send_buf_entry *)tbl_find_detach(t, NULL, crit_none)))
    {
        dsr_pkt_free(e->dp);
        FREE(e);
        pkts++;
    }
    return pkts;
}

#ifdef __KERNEL__
static int send_buf_print(struct tbl *t, char *buffer)
{
    dsr_list_t *p;
    int len;
    struct timeval now;

    gettime(&now);

    len = sprintf(buffer, "# %-15s %-8s\n", "IPAddr", "Age (s)");

    DSR_READ_LOCK(&t->lock);

    list_for_each(p, &t->head)
    {
        struct send_buf_entry *e = (struct send_buf_entry *)p;

        if (e && e->dp)
            len += sprintf(buffer + len, "  %-15s %-8lu\n",
                           print_ip(e->dp->dst),
                           timeval_diff(&now, &e->qtime) / 1000000);
    }

    len += sprintf(buffer + len,
                   "\nQueue length      : %u\n"
                   "Queue max. length : %u\n", t->len, t->max_len);

    DSR_READ_UNLOCK(&t->lock);

    return len;
}

static int
send_buf_get_info(char *buffer, char **start, off_t offset, int length)
{
    int len;

    len = send_buf_print(&send_buf, buffer);

    *start = buffer + offset;
    len -= offset;

    if (len > length)
        len = length;
    else if (len < 0)
        len = 0;
    return len;
}

#endif              /* __KERNEL__ */

int __init NSCLASS send_buf_init(void)
{
#ifdef __KERNEL__
    struct proc_dir_entry *proc;

    proc = proc_net_create(SEND_BUF_PROC_FS_NAME, 0, send_buf_get_info);
    if (proc)
        proc->owner = THIS_MODULE;
    else
    {
        printk(KERN_ERR "send_buf: failed to create proc entry\n");
        return -1;
    }
#endif

    INIT_TBL(&send_buf, SEND_BUF_MAX_LEN);

    init_timer(&send_buf_timer);

    send_buf_timer.function = &NSCLASS send_buf_timeout;

    return 1;
}

void __exit NSCLASS send_buf_cleanup(void)
{
    int pkts;
#ifdef KERNEL26
    synchronize_net();
#endif
    if (timer_pending(&send_buf_timer))
        del_timer_sync(&send_buf_timer);

    pkts = send_buf_flush(&send_buf);

    DEBUG("Flushed %d packets\n", pkts);

#ifdef __KERNEL__
    proc_net_remove(SEND_BUF_PROC_FS_NAME);
#endif
}

} // namespace inetmanet

} // namespace inet

