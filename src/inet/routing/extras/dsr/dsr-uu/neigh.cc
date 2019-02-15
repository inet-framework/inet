/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#define OMNETPP
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"

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


namespace inet {

namespace inetmanet {


/* TODO: Implement neighbor table garbage collection */
void NSCLASS neigh_tbl_garbage_timeout(unsigned long data)
{
    /* tbl_for_each_del(&neigh_tbl, nullptr, crit_garbage); */

    /*  if (!tbl_empty(&neigh_tbl)) { */
    /*      garbage_timer.expires = TimeNow +  */
    /*          MSECS_TO_TIMENOW(NEIGH_TBL_GARBAGE_COLLECT_TIMEOUT); */
    /*      add_timer(&garbage_timer);       */
    /* } */
}

NSCLASS neighbor * NSCLASS neigh_tbl_create(struct in_addr addr,
        struct sockaddr *hw_addr,
        unsigned short id)
{
    struct neighbor *neigh;

    neigh = new neighbor;

    if (!neigh)
        return nullptr;
    memset(neigh, 0, sizeof(neighbor));
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


int NSCLASS neigh_tbl_add(struct in_addr neigh_addr, struct ethhdr *ethh)
{
    struct sockaddr hw_addr;
    struct neighbor *neigh;

    L3Address nAddress(neigh_addr.s_addr);

    auto it = neighborMap.find(nAddress);
    if (it != neighborMap.end())
        return 0;

    memcpy(hw_addr.sa_data, ethh->h_source, ETH_ALEN);

    neigh = neigh_tbl_create(neigh_addr, &hw_addr, 1);

    if (!neigh)
    {
        DEBUG("Could not create new neighbor entry\n");
        return -1;
    }
    neighborMap.insert(std::make_pair(nAddress,neigh));
    return 1;
}

int NSCLASS neigh_tbl_del(struct in_addr neigh_addr)
{
    L3Address nAddress(neigh_addr.s_addr);
    auto it = neighborMap.find(nAddress);
    if (it != neighborMap.end())
    {
        delete it->second;
        neighborMap.erase(it);
        return 1;
    }
    return 0;
}

int NSCLASS neigh_tbl_set_ack_req_time(struct in_addr neigh_addr)
{
    L3Address nAddress(neigh_addr.s_addr);
    auto it = neighborMap.find(nAddress);
    if (it != neighborMap.end())
    {
        gettime(&it->second->last_ack_req);
        return 1;
    }
    return 0;
}

int NSCLASS
neigh_tbl_set_rto(struct in_addr neigh_addr, struct neighbor_info *neigh_info)
{
    L3Address nAddress(neigh_addr.s_addr);
    auto it = neighborMap.find(nAddress);
    if (it != neighborMap.end())
    {
        struct timeval now;
        usecs_t rtt = neigh_info->rtt;
        int delta;
        neighbor *n = it->second;
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

int NSCLASS
neigh_tbl_query(struct in_addr neigh_addr, struct neighbor_info *neigh_info)
{
    L3Address nAddress(neigh_addr.s_addr);
    auto it = neighborMap.find(nAddress);
    if (it != neighborMap.end())
    {
        if (!neigh_info)
            return 0;
        neighbor *n = it->second;

        neigh_info->id = n->id;
        neigh_info->last_ack_req = n->last_ack_req;
        memcpy(&neigh_info->hw_addr, &n->hw_addr,sizeof(struct sockaddr));
        /* Return current RTO */
        neigh_info->rto = n->t_rxtcur * 1000 / PR_SLOWHZ;
        /*  if (q->info->rto < 1000000)  */
        /*              q->info->rto = 1000000; */
        return 1;
    }
    return 0;
}

int NSCLASS neigh_tbl_id_inc(struct in_addr neigh_addr)
{
    L3Address nAddress(neigh_addr.s_addr);
    auto it = neighborMap.find(nAddress);
    if (it != neighborMap.end())
    {
        it->second->id++;
        return 1;
    }
    return 0;
}


int __init NSCLASS neigh_tbl_init(void)
{
    neigh_tbl_cleanup();
    return 0;
}

void __exit NSCLASS neigh_tbl_cleanup(void)
{
    while (!neighborMap.empty())
    {
        delete neighborMap.begin()->second;
        neighborMap.erase(neighborMap.begin());

    }
}

} // namespace inetmanet

} // namespace inet

