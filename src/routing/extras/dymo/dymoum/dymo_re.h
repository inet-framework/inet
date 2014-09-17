/***************************************************************************
 *   Copyright (C) 2005 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __DYMO_RE_H__
#define __DYMO_RE_H__

#ifndef NS_NO_GLOBALS

#include "inet/routing/extras/dymo/dymoum/defs_dymo.h"
#include "inet/routing/extras/dymo/dymoum/rtable.h"
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"
#ifndef OMNETPP
#include <sys/types.h>
#include <netinet/in.h>
#else
#include "inet/routing/extras/base/compatibility.h"
#endif
#include <stdlib.h>
#include <assert.h>

#ifndef OMNETPP
#define RREQ_WAIT_TIME  1000
#define RREQ_TRIES  3
#endif

#define RB_STALE    0
#define RB_LOOP_PRONE   1
#define RB_INFERIOR 2
#define RB_FRESH    3
#define RB_SELF_GEN 4
#define RB_PROACTIVE 5

#ifndef OMNETPP
/* Routing blocks advertised in a RE message */
struct re_block
{
# if __BYTE_ORDER == __BIG_ENDIAN
    u_int16_t   g : 1;
    u_int16_t   prefix : 7;
    u_int16_t   res : 2;
    u_int16_t   re_hopcnt : 6;
# elif __BYTE_ORDER == __LITTLE_ENDIAN
    u_int16_t   res : 2;
    u_int16_t   re_hopcnt : 6;
    u_int16_t   g : 1;
    u_int16_t   prefix : 7;
# else
#   error "Adjust your <bits/endian.h> defines"
# endif
    u_int32_t   re_node_addr;
    u_int32_t   re_node_seqnum;
};
#define MAX_RE_BLOCKS 50

/* RE message */
typedef struct      // FIXME: adjust byte ordering
{
    u_int32_t   m : 1;
    u_int32_t   h : 2;
    u_int32_t   type : 5;
    u_int32_t   len : 12;
    u_int32_t   ttl : 6;
    u_int32_t   i : 1;
    u_int32_t   a : 1;
    u_int32_t   s : 1;
    u_int32_t   res1 : 3;

    u_int32_t   target_addr;
    u_int32_t   target_seqnum;

    u_int8_t    thopcnt : 6;
    u_int8_t    res2 : 2;

    struct re_block re_blocks[MAX_RE_BLOCKS];
} RE;

#define RE_BLOCK_SIZE   sizeof(struct re_block)
#define RE_SIZE     sizeof(RE)
#define RE_BASIC_SIZE   (RE_SIZE - (MAX_RE_BLOCKS * RE_BLOCK_SIZE))
#endif

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
/* Create a RREQ */
RE *re_create_rreq(struct in_addr target_addr,
                   u_int32_t target_seqnum,
                   struct in_addr re_node_addr,
                   u_int32_t re_node_seqnum,
                   u_int8_t prefix, u_int8_t g,
                   u_int8_t ttl, u_int8_t thopcnt);

/* Create a RREP */
RE *re_create_rrep(struct in_addr target_addr,
                   u_int32_t target_seqnum,
                   struct in_addr re_node_addr,
                   u_int32_t re_node_seqnum,
                   u_int8_t prefix, u_int8_t g,
                   u_int8_t ttl, u_int8_t thopcnt);

/* Process a RE message */
void re_process(RE *re,struct in_addr ip_src, u_int32_t ifindex);

/* Process a RE block updating suitable fields and modifying the routing
   table accordingly. Return a negative value if the block was not processed */
int re_process_block(struct re_block *block,
                     u_int8_t is_rreq,
                     rtable_entry_t *entry,
                     struct in_addr ip_src,
                     u_int32_t ifindex);

/* Auxilliary function, do not use */
void __re_send(RE *re);

/* Send a given RREP */
void re_send_rrep(RE *rrep);

/* Send a RREQ given the destination address and the destination sequence
   number */
void re_send_rreq(struct in_addr dest_addr, u_int32_t seqnum, u_int8_t thopcnt);

/* Forward a given RE (RREQ/RREP) when there is no path accumulation */
void re_forward(RE *re);

/* Forward a given RREQ when there is path accumulation. Given index is used
   to access the RE block which has been added */
void re_forward_rreq_path_acc(RE *rreq, int blindex);

/* Forward a RREP when there is path accumulation */
void re_forward_rrep_path_acc(RE *rrep);

/* Implement route discovery */
void route_discovery(struct in_addr dest_addr);

void re_intermediate_rrep (struct in_addr src_addr,struct in_addr dest_addr, rtable_entry_t *entry,int ifindex);
int re_mustAnswer(RE *re,u_int32_t ifindex);

/* Return the number of blocks contained inside a RE */
#ifdef OMNETPP
//static NS_INLINE int re_numblocks(RE *re)
//{
//  assert(re);
//
//  if ((re->getSizeExtension()) % RE_BLOCK_LENGTH != 0)
//  {
//      throw cRuntimeError("re size error");
//      return -1;
//  }
//
//  return (re->getSizeExtension()/ RE_BLOCK_LENGTH);
//}

void re_answer(RE *re, u_int32_t ifindex);
#else
void re_answer(RE *re, u_int32_t ifindex) {}
#endif

NS_INLINE int re_numblocks(RE *re)
{
    assert(re);

    if ((re->len - RE_BASIC_SIZE) % RE_BLOCK_SIZE != 0)
        return -1;
    return (re->len - RE_BASIC_SIZE) / RE_BLOCK_SIZE;
}


/* Return the routing information type: self-generated, stale, loop-prone,
   inferior or stale */
NS_STATIC int re_info_type(struct re_block *b, rtable_entry_t *e, u_int8_t is_rreq);
#if 0
NS_STATIC NS_INLINE int re_info_type(struct re_block *b, rtable_entry_t *e, u_int8_t is_rreq)
{
    u_int32_t node_seqnum;
    int32_t sub;
    int i;

    assert(b);

    // If the block was issued from one interface of the processing node,
    // then the block is considered stale
    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
    {
        if (this_host.devs[i].enabled &&
                this_host.devs[i].ipaddr.s_addr == b->re_node_addr)
        {
            return RB_SELF_GEN;
        }
    }

    if (e)
    {
        node_seqnum = ntohl(b->re_node_seqnum);
        sub     = ((int32_t) node_seqnum) - ((int32_t) e->rt_seqnum);

        if (b->from_proactive)
        {
            if (e->rt_state != RT_VALID)
                return RB_PROACTIVE;

            if (sub == 0 && e->rt_hopcnt != 0 && b->re_hopcnt != 0 && b->re_hopcnt < e->rt_hopcnt)
                return RB_PROACTIVE;
        }

        if (sub < 0)
            return RB_STALE;
        if (sub == 0)
        {
            if (e->rt_hopcnt == 0 || b->re_hopcnt == 0 || b->re_hopcnt > e->rt_hopcnt + 1)
                return RB_LOOP_PRONE;
            if (e->rt_state == RT_VALID && (b->re_hopcnt > e->rt_hopcnt || (b->re_hopcnt == e->rt_hopcnt && is_rreq)))
                return RB_INFERIOR;
        }
    }
    return RB_FRESH;
}
#endif

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __DYMO_RE_H__ */

