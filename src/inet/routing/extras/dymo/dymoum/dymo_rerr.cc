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
#define NS_PORT
#define OMNETPP

#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/dymo_um.h"
#else
#include "../dymo_um_omnet.h"
#endif
#else
#include "inet/routing/extras/dymo/dymoum/dymo_rerr.h"
#include "inet/routing/extras/dymo/dymoum/dymo_socket.h"
#include <string.h>

#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

RERR *NS_CLASS rerr_create(struct rerr_block *blocks, int nblocks, int ttl)
{
    int i;
    RERR *rerr;
#ifndef OMNETPP
    rerr        = (RERR *) dymo_socket_new_element();
#else
    rerr        =  new RERR();
    rerr->newBocks(nblocks);
#endif
    rerr->m     = 0;
    rerr->h     = 0;
    rerr->type  = DYMO_RERR_TYPE;
    rerr->len   = RERR_BASIC_SIZE + (nblocks * RERR_BLOCK_SIZE);
    rerr->ttl   = ttl;
    rerr->i     = 0;

    for (i = 0; i < nblocks; i++)
    {
        rerr->rerr_blocks[i].unode_addr = blocks[i].unode_addr;
        rerr->rerr_blocks[i].unode_seqnum = blocks[i].unode_seqnum;
    }

    return rerr;
}

void NS_CLASS rerr_send(struct in_addr addr, int ttl, rtable_entry_t *entry)
{
    int i = 1;
    struct rerr_block blocks[MAX_RERR_BLOCKS];

    dlog(LOG_DEBUG, 0, __FUNCTION__, "sending RERR");

    memset(blocks, '\0', MAX_RERR_BLOCKS * sizeof(struct rerr_block));
    blocks[0].unode_addr = addr.s_addr;

    if (entry)
    {
#ifndef MAPROUTINGTABLE
        dlist_head_t *pos;
#endif
        blocks[0].unode_seqnum = entry->rt_seqnum;
#ifndef MAPROUTINGTABLE
        dlist_for_each(pos, &rtable.l)
        {
            if (i >= MAX_RERR_BLOCKS)
                continue;
            rtable_entry_t *e = (rtable_entry_t *) pos;
            if (e != entry && (e->rt_nxthop_addr.s_addr
                               == entry->rt_nxthop_addr.s_addr) &&
                    (e->rt_ifindex == entry->rt_ifindex))
            {
                i++;
                blocks[i-1].unode_addr      = e->rt_dest_addr.s_addr;
                blocks[i-1].unode_seqnum    = e->rt_seqnum;
            }
        }
#else
        for (auto & elem : *dymoRoutingTable)
        {
            if (i >= MAX_RERR_BLOCKS)
                continue;
            rtable_entry_t *e = elem.second;
            if (e != entry && (e->rt_nxthop_addr.s_addr
                               == entry->rt_nxthop_addr.s_addr) &&
                    (e->rt_ifindex == entry->rt_ifindex))
            {
                i++;
                blocks[i-1].unode_addr      = e->rt_dest_addr.s_addr;
                blocks[i-1].unode_seqnum    = e->rt_seqnum;
            }
        }

#endif

    }
    else
        blocks[0].unode_seqnum = 0;

    RERR *rerr = rerr_create(blocks, i, ttl);
    rerr_forward(rerr);
}

void NS_CLASS rerr_process(RERR *rerr,struct in_addr src, u_int32_t ifindex)
{
    struct in_addr node_addr;
    rtable_entry_t *entry;
    int i;
#ifdef OMNETPP
    int num_blk_del;
#endif
    // Be sure that there is a block at least
    if (rerr_numblocks(rerr) <= 0)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__, "malformed RERR received");
#ifdef OMNETPP
        delete rerr;
        rerr=nullptr;
#endif
        return;
    }

#ifdef OMNETPP
    num_blk_del=0;
    totalRerrRec++;
#endif

    for (i = 0; i < rerr_numblocks(rerr); i++)
    {
        int changed = 0;

        node_addr.s_addr    = rerr->rerr_blocks[i].unode_addr;
        entry           = rtable_find(node_addr);

        if (entry && entry->rt_state == RT_VALID)
        {
            int32_t sub;
            u_int32_t unode_seqnum =
                ntohl(rerr->rerr_blocks[i].unode_seqnum);
            u_int32_t rt_seqnum = ntohl(entry->rt_seqnum);

            sub = (int32_t) unode_seqnum - (int32_t) rt_seqnum;

            if (entry->rt_nxthop_addr.s_addr == src.s_addr &&
                    entry->rt_ifindex == ifindex &&
                    (unode_seqnum == 0 || sub <= 0))
            {
                rtable_expire_timeout(entry);
                changed = 1;
            }
        }

        if (!changed)
        {
            // drop block
            int n = rerr_numblocks(rerr) - i - 1;

            memmove(&rerr->rerr_blocks[i], &rerr->rerr_blocks[i+1],
                    n * sizeof(struct rerr_block));
            memset(&rerr->rerr_blocks[i + n], 0, sizeof(struct rerr_block));

            rerr->len -= RERR_BLOCK_SIZE;
            i--;
#ifdef OMNETPP
            num_blk_del++;
#endif
        }
    }

#ifdef OMNETPP
    rerr->delBocks(num_blk_del);
#endif

    if (rerr_numblocks(rerr) > 0 &&
            generic_postprocess((DYMO_element *) rerr))
    {
        rerr_forward(rerr);
    }
#ifdef OMNETPP
    else
    {
        delete rerr;
        rerr=nullptr;
    }
#endif
}

void NS_CLASS rerr_forward(RERR *rerr)
{
    struct in_addr dest_addr;
    int i;

    dlog(LOG_DEBUG, 0, __FUNCTION__, "forwarding RERR");
#ifdef OMNETPP

    double delay = -1;
    if (par("EqualDelay"))
        delay = par("broadcastDelay");

    rerr->setByteLength(0);

    int cont = numInterfacesActive;
    if (numInterfacesActive==0)
    {
        delete rerr;
        rerr=nullptr;
        return;
    }
    dest_addr.s_addr = L3Address(IPv4Address(DYMO_BROADCAST));
    // Send RE over all enabled interfaces
    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
    {
        if (DEV_NR(i).enabled)
        {
            if (cont>1)
                dymo_socket_queue((DYMO_element *) rerr->dup());
            else
                dymo_socket_queue((DYMO_element *) rerr);
            dymo_socket_send(dest_addr, &DEV_NR(i),delay);
            cont--;
        }
    }

#else
    // Queue the new RERR
    rerr = (RERR *) dymo_socket_queue((DYMO_element *) rerr);

    // Send RERR over all enabled interfaces
    dest_addr.s_addr = DYMO_BROADCAST;
    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
        if (DEV_NR(i).enabled)
            dymo_socket_send(dest_addr, &DEV_NR(i));

#endif
}

#ifdef OMNETPP

void NS_CLASS rerr_send(struct in_addr addr, int ttl, rtable_entry_t *entry,struct in_addr dest_addr)
{
    int i = 1;
    struct rerr_block blocks[MAX_RERR_BLOCKS];

    dlog(LOG_DEBUG, 0, __FUNCTION__, "sending RERR");

    memset(blocks, '\0', MAX_RERR_BLOCKS * sizeof(struct rerr_block));
    blocks[0].unode_addr = addr.s_addr;

    if (entry)
    {
        blocks[0].unode_seqnum = entry->rt_seqnum;
#ifndef MAPROUTINGTABLE
        dlist_head_t *pos;
        dlist_for_each(pos, &rtable.l)
        {
            rtable_entry_t *e = (rtable_entry_t *) pos;
            if (e != entry && (e->rt_nxthop_addr.s_addr
                               == entry->rt_nxthop_addr.s_addr) &&
                    (e->rt_ifindex == entry->rt_ifindex))
            {
                i++;
                blocks[i-1].unode_addr      = e->rt_dest_addr.s_addr;
                blocks[i-1].unode_seqnum    = e->rt_seqnum;
            }
        }
#else
        for (auto & elem : *dymoRoutingTable)
        {
            rtable_entry_t *e = elem.second;
            if (e != entry && (e->rt_nxthop_addr.s_addr
                               == entry->rt_nxthop_addr.s_addr) &&
                    (e->rt_ifindex == entry->rt_ifindex))
            {
                i++;
                blocks[i-1].unode_addr      = e->rt_dest_addr.s_addr;
                blocks[i-1].unode_seqnum    = e->rt_seqnum;
            }
        }
#endif

    }
    else
        blocks[0].unode_seqnum = 0;

    RERR *rerr = rerr_create(blocks, i, ttl);
    rerr_forward(rerr,dest_addr);
}


void NS_CLASS rerr_forward(RERR *rerr,struct in_addr dest_addr)
{

    //int i;

    dlog(LOG_DEBUG, 0, __FUNCTION__, "forwarding RERR");
    //int cont = numInterfacesActive;
    if (numInterfacesActive==0)
    {
        delete rerr;
        rerr=nullptr;
        return;
    }
    // Send RE over all enabled interfaces
    rtable_entry_t *entry   = rtable_find(dest_addr);
    dymo_socket_queue((DYMO_element *) rerr);
    if (entry)
    {
        dymo_socket_send(dest_addr, &DEV_IFINDEX(entry->rt_ifindex));
    }
    else
        dymo_socket_send(dest_addr, &DEV_IFINDEX(NS_IFINDEX));

}
#endif

} // namespace inetmanet

} // namespace inet

