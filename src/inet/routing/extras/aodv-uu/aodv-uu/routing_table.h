/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University & Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *
 *
 *****************************************************************************/
#ifndef _ROUTING_TABLE_H
#define _ROUTING_TABLE_H

#ifndef NS_NO_GLOBALS
#include "inet/routing/extras/aodv-uu/aodv-uu/defs_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/list.h"

namespace inet {

namespace inetmanet {

typedef struct rt_table rt_table_t;

/* Neighbor struct for active routes in Route Table */
#define FIRST_PREC(h) ((precursor_t *)((h).next))

#define seqno_incr(s) ((s == 0) ? 0 : ((s == 0xFFFFFFFF) ? s = 1 : s++))

typedef u_int32_t hash_value;   /* A hash value */

#ifdef AODV_USE_STL_RT
typedef struct precursor
{
    struct in_addr neighbor;
} precursor_t;

struct rt_table
{
    struct in_addr dest_addr;   /* IP address of the destination */
    u_int32_t dest_seqno;
    unsigned int ifindex;   /* Network interface index... */
    struct in_addr next_hop;    /* IP address of the next hop to the dest */
    u_int8_t hcnt;      /* Distance (in hops) to the destination */
    u_int16_t flags;        /* Routing flags */
    u_int8_t state;     /* The state of this entry */
    uint32_t    cost; // without the last node
    uint8_t     hopfix;

    struct timer rt_timer;  /* The timer associated with this entry */
    struct timer ack_timer; /* RREP_ack timer for this destination */
    struct timer hello_timer;
    struct timeval last_hello_time;
    hash_value hash;
    u_int8_t hello_cnt;
    int nprec;          /* Number of precursors */
    std::vector<precursor_t> precursors;      /* List of neighbors using the route */
};
#else
typedef struct precursor
{
    list_t l;
    struct in_addr neighbor;
} precursor_t;

/* Route table entries */
struct rt_table
{
    list_t l;
    struct in_addr dest_addr;   /* IP address of the destination */
    u_int32_t dest_seqno;
    unsigned int ifindex;   /* Network interface index... */
    struct in_addr next_hop;    /* IP address of the next hop to the dest */
    u_int8_t hcnt;      /* Distance (in hops) to the destination */
    u_int16_t flags;        /* Routing flags */
    u_int8_t state;     /* The state of this entry */
    uint32_t    cost; // without the last node
    uint8_t     hopfix;

    struct timer rt_timer;  /* The timer associated with this entry */
    struct timer ack_timer; /* RREP_ack timer for this destination */
    struct timer hello_timer;
    struct timeval last_hello_time;
    u_int8_t hello_cnt;
    hash_value hash;
    int nprec;          /* Number of precursors */
    list_t precursors;      /* List of neighbors using the route */
};
#endif

/* Route entry flags */
#define RT_UNIDIR        0x1
#define RT_REPAIR        0x2
#define RT_INV_SEQNO     0x4
#define RT_INET_DEST     0x8    /* Mark for Internet destinations (to be relayed
* through a Internet gateway. */
#define RT_GATEWAY       0x10

/* Route entry states */
#define INVALID   0
#define VALID     1
#define IMMORTAL  2


#define RT_TABLESIZE 64     /* Must be a power of 2 */
#define RT_TABLEMASK (RT_TABLESIZE - 1)

struct routing_table
{
    unsigned int num_entries;
    unsigned int num_active;
    list_t tbl[RT_TABLESIZE];
};
void precursor_list_destroy(rt_table_t * rt);

} // namespace inetmanet

} // namespace inet

#endif              /* NS_NO_GLOBALS */


#ifndef NS_NO_DECLARATIONS
struct routing_table rt_tbl;

void rt_table_init();
void rt_table_destroy();
rt_table_t *rt_table_insert(struct in_addr dest, struct in_addr next,
                            u_int8_t hops, u_int32_t seqno, u_int32_t life,
                            u_int8_t state, u_int16_t flags,
                            unsigned int ifindex,
                            uint32_t cost,uint8_t hopfix);
rt_table_t *rt_table_update(rt_table_t * rt, struct in_addr next, u_int8_t hops,
                            u_int32_t seqno, u_int32_t lifetime, u_int8_t state,
                            u_int16_t flags,int iface,
                            uint32_t cost,uint8_t hopfix);

NS_INLINE rt_table_t *rt_table_update_timeout(rt_table_t * rt,
        u_int32_t lifetime);
void rt_table_update_route_timeouts(rt_table_t * fwd_rt, rt_table_t * rev_rt);
rt_table_t *rt_table_find(struct in_addr dest);
rt_table_t *rt_table_find_gateway();
int rt_table_update_inet_rt(rt_table_t * gw, u_int32_t life);
int rt_table_invalidate(rt_table_t * rt);
void rt_table_delete(rt_table_t * rt);
void precursor_add(rt_table_t * rt, struct in_addr addr);
void precursor_remove(rt_table_t * rt, struct in_addr addr);

#ifdef OMNETPP
rt_table_t * modifyAODVTables(struct in_addr,
                              struct in_addr next,
                              u_int8_t hops, u_int32_t seqno,
                              u_int32_t life, u_int8_t state,
                              u_int16_t flags, unsigned int ifindex);
#endif

#endif              /* NS_NO_DECLARATIONS */


#endif              /* ROUTING_TABLE_H */
