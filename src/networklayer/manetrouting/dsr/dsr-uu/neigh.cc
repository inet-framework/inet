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
#endif
#ifndef OMNETPP
#ifdef NS2
#include "ns-agent.h"
#endif              /* NS2 */
#else
#include "dsr-uu-omnetpp.h"
#endif
#include "tbl.h"
#include "neigh.h"
#include "debug_dsr.h"
#include "timer.h"

#define NEIGH_TBL_MAX_LEN 50

/* We calculate RTO in milliseconds */
#define DSR_SRTTBASE 0
#define DSR_SRTTDFLT 60
#define DSR_MIN 20
#define DSR_REXMTMAX 1280
#define DSR_RTTDFLT 30
#define PR_SLOWHZ 2

#define RTT_SHIFT 3
#define RTTVAR_SHIFT 2

#define NEIGH_TBL_GARBAGE_COLLECT_TIMEOUT 3000
#define NEIGH_TBL_TIMEOUT 2000

#define DSR_RANGESET(tv, value, tvmin, tvmax) { \
        (tv) = (value); \
        if ((tv) < (tvmin)) \
            (tv) = (tvmin); \
        else if ((tv) > (tvmax)) \
            (tv) = (tvmax); \
}
#define MAX(a,b) ( a > b ? a : b)
#undef K
//#define K 4

#define DSR_REXMTVAL(val) \
        (((val) >> RTT_SHIFT) + (val))

#ifdef __KERNEL__
static TBL(neigh_tbl, NEIGH_TBL_MAX_LEN);

#define NEIGH_TBL_PROC_NAME "dsr_neigh_tbl"

static DSRUUTimer neigh_tbl_timer;
#endif


struct neighbor
{
    list_t l;
    struct in_addr addr;
    struct sockaddr hw_addr;
    unsigned short id;
    struct timeval last_ack_req;
    usecs_t t_srtt, rto, t_rxtcur, t_rttmin, t_rttvar, jitter;  /* RTT in usec */
};

struct neighbor_query
{
    struct in_addr *addr;
    struct neighbor_info *info;
};

static inline int crit_addr(void *pos, void *query)
{
    struct neighbor_query *q = (struct neighbor_query *)query;
    struct neighbor *n = (struct neighbor *)pos;

    if (n->addr.s_addr == q->addr->s_addr)
    {
        if (q->info)
        {

            q->info->id = n->id;
            q->info->last_ack_req = n->last_ack_req;
            memcpy(&q->info->hw_addr, &n->hw_addr,
                   sizeof(struct sockaddr));

            /* Return current RTO */
            q->info->rto = n->t_rxtcur * 1000 / PR_SLOWHZ;

            /*  if (q->info->rto < 1000000)  */
            /*              q->info->rto = 1000000; */
        }
        return 1;
    }
    return 0;
}

static inline int crit_addr_id_inc(void *pos, void *addr)
{
    struct in_addr *a = (struct in_addr *)addr;
    struct neighbor *n = (struct neighbor *)pos;

    if (n->addr.s_addr == a->s_addr)
    {
        n->id++;
        //gettime(&n->last_ack_req);
        return 1;
    }
    return 0;
}

static inline int set_ack_req_time(void *pos, void *addr)
{
    struct in_addr *a = (struct in_addr *)addr;
    struct neighbor *n = (struct neighbor *)pos;

    if (n->addr.s_addr == a->s_addr)
    {
        gettime(&n->last_ack_req);
        return 1;
    }
    return 0;
}


static inline int rto_calc(void *pos, void *query)
{
    struct neighbor_query *q = (struct neighbor_query *)query;
    struct neighbor *n = (struct neighbor *)pos;

    if (n->addr.s_addr == q->addr->s_addr)
    {
        struct timeval now;
        usecs_t rtt = q->info->rtt;
        int delta;

        gettime(&now);

        if (n->t_srtt != 0)
        {
            delta = rtt - 1 - (n->t_srtt >> RTT_SHIFT);

            if ((n->t_srtt += delta) <= 0)
                n->t_srtt = 1;

            if (delta < 0)
                delta = -delta;

            delta -= (n->t_rttvar >> RTTVAR_SHIFT);

            if ((n->t_rttvar += delta) <= 0)
                n->t_rttvar = 1;
        }
        else
        {
            n->t_srtt = rtt << RTT_SHIFT;
            n->t_rttvar = rtt << (RTTVAR_SHIFT - 1);
        }

        DSR_RANGESET(n->t_rxtcur, DSR_REXMTVAL(n->t_srtt),
                     n->t_rttmin, DSR_REXMTMAX);

        return 1;
    }
    return 0;
}
/* TODO: Implement neighbor table garbage collection */
void NSCLASS neigh_tbl_garbage_timeout(unsigned long data)
{
    /* tbl_for_each_del(&neigh_tbl, NULL, crit_garbage); */

    /*  if (!tbl_empty(&neigh_tbl)) { */
    /*      garbage_timer.expires = TimeNow +  */
    /*          MSECS_TO_TIMENOW(NEIGH_TBL_GARBAGE_COLLECT_TIMEOUT); */
    /*      add_timer(&garbage_timer);       */
    /* } */
}

static struct neighbor *neigh_tbl_create(struct in_addr addr,
        struct sockaddr *hw_addr,
        unsigned short id)
{
    struct neighbor *neigh;

    neigh = (struct neighbor *)MALLOC(sizeof(struct neighbor), GFP_ATOMIC);

    if (!neigh)
        return NULL;

    memset(neigh, 0, sizeof(struct neighbor));

    neigh->id = id;
    neigh->addr = addr;
    neigh->t_srtt = DSR_SRTTBASE;
    neigh->t_rttvar = DSR_RTTDFLT * PR_SLOWHZ << 2;
    neigh->t_rttmin = DSR_MIN;
    DSR_RANGESET(neigh->t_rxtcur,
                 ((DSR_SRTTBASE >> 2) + (DSR_SRTTDFLT << 2)) >> 1,
                 DSR_MIN, DSR_REXMTMAX);

    memset(&neigh->last_ack_req, 0, sizeof(struct timeval));
    memcpy(&neigh->hw_addr, hw_addr, sizeof(struct sockaddr));

    /*  garbage_timer.expires = TimeNow + NEIGH_TBL_GARBAGE_COLLECT_TIMEOUT / 1000*HZ; */
    /*  add_timer(&garbage_timer); */

    return neigh;
}

#ifdef NS2
int NSCLASS neigh_tbl_add(struct in_addr neigh_addr, struct hdr_mac *mach)
#else
int NSCLASS neigh_tbl_add(struct in_addr neigh_addr, struct ethhdr *ethh)
#endif
{
    struct sockaddr hw_addr;
    struct neighbor *neigh;
    struct neighbor_query q;

    q.addr = &neigh_addr;
    q.info = NULL;

    if (in_tbl(&neigh_tbl, &q, crit_addr))
        return 0;
#ifdef NS2
    /* This should probably be changed to lookup the MAC type
     * dynamically in case the simulation is run over a non 802.11
     * mac layer... Or is there a uniform way to get hold of the mac
     * source for all mac headers? */
    struct hdr_mac802_11 *mh_802_11 = (struct hdr_mac802_11 *)mach;

    int mac_src = ETHER_ADDR(mh_802_11->dh_ta);

    inttoeth(&mac_src, (char *)&hw_addr);

    DEBUG("ADD %s, %d\n", print_ip(neigh_addr), mac_src);
#else
    memcpy(hw_addr.sa_data, ethh->h_source, ETH_ALEN);
#endif

    neigh = neigh_tbl_create(neigh_addr, &hw_addr, 1);

    if (!neigh)
    {
        DEBUG("Could not create new neighbor entry\n");
        return -1;
    }
    tbl_add(&neigh_tbl, &neigh->l, crit_none);

    return 1;
}

int NSCLASS neigh_tbl_del(struct in_addr neigh_addr)
{
    struct tbl *tblptr;
    tblptr = &neigh_tbl;
    return tbl_for_each_del(tblptr, &neigh_addr, crit_addr);
}

int NSCLASS neigh_tbl_set_ack_req_time(struct in_addr neigh_addr)
{
    struct tbl *tblptr;
    criteria_t function;
    tblptr = &neigh_tbl;
    function = set_ack_req_time;
    return tbl_find_do(tblptr, &neigh_addr, function);
}

int NSCLASS
neigh_tbl_set_rto(struct in_addr neigh_addr, struct neighbor_info *neigh_info)
{
    struct neighbor_query q;
    struct tbl *tblptr;

    q.addr = &neigh_addr;
    q.info = neigh_info;
    tblptr = &neigh_tbl;

    return tbl_find_do(tblptr, &q, rto_calc);
}

int NSCLASS
neigh_tbl_query(struct in_addr neigh_addr, struct neighbor_info *neigh_info)
{
    struct neighbor_query q;

    q.addr = &neigh_addr;
    q.info = neigh_info;

    return in_tbl(&neigh_tbl, &q, crit_addr);
}

int NSCLASS neigh_tbl_id_inc(struct in_addr neigh_addr)
{
    return tbl_find_do(&neigh_tbl, &neigh_addr, crit_addr_id_inc);
}

#ifdef __KERNEL__
static int neigh_tbl_print(char *buf)
{
    list_t *pos;
    int len = 0;

    DSR_READ_LOCK(&neigh_tbl.lock);

    len +=
        sprintf(buf, "# %-15s %-17s %-10s %-6s\n", "Addr", "HwAddr",
                "RTO (usec)", "Id" /*, "AckRxTime","AckTxTime" */ );

    list_for_each(pos, &neigh_tbl.head)
    {
        struct neighbor *neigh = (struct neighbor *)pos;

        len += sprintf(buf + len, "  %-15s %-17s %-10lu %-6u\n",
                       print_ip(neigh->addr),
                       print_eth(neigh->hw_addr.sa_data),
                       neigh->t_rxtcur, neigh->id);
    }

    DSR_READ_UNLOCK(&neigh_tbl.lock);
    return len;

}

static int
neigh_tbl_proc_info(char *buffer, char **start, off_t offset, int length)
{
    int len;

    len = neigh_tbl_print(buffer);

    *start = buffer + offset;
    len -= offset;
    if (len > length)
        len = length;
    else if (len < 0)
        len = 0;
    return len;
}

#endif              /* __KERNEL__ */

int __init NSCLASS neigh_tbl_init(void)
{
    INIT_TBL(&neigh_tbl, NEIGH_TBL_MAX_LEN);

    init_timer(&neigh_tbl_timer);

    neigh_tbl_timer.function = &NSCLASS neigh_tbl_garbage_timeout;

#ifdef __KERNEL__
    proc_net_create(NEIGH_TBL_PROC_NAME, 0, neigh_tbl_proc_info);
#endif
    return 0;
}

void __exit NSCLASS neigh_tbl_cleanup(void)
{
    tbl_flush(&neigh_tbl, crit_none);

#ifdef __KERNEL__
    proc_net_remove(NEIGH_TBL_PROC_NAME);
#endif
}
