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
#include "inet/routing/extras/dymo/dymoum/rtable.h"
#include "inet/routing/extras/dymo/dymoum/defs_dymo.h"
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"
#include "inet/routing/extras/dymo/dymoum/k_route.h"
#include "inet/routing/extras/dymo/dymoum/pending_rreq.h"
#include "inet/routing/extras/dymo/dymoum/dymo_timeout.h"
#include "inet/routing/extras/dymo/dymoum/dymo_netlink.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

#ifndef MAPROUTINGTABLE

void NS_CLASS rtable_init()
{
    INIT_DLIST_HEAD(&rtable.l);
}

void NS_CLASS rtable_destroy()
{
    dlist_head_t *pos, *tmp;

    dlist_for_each_safe(pos, tmp, &rtable.l)
    {
        rtable_entry_t *e = (rtable_entry_t *) pos;
        rtable_delete(e);
    }
}

rtable_entry_t *NS_CLASS rtable_find(struct in_addr dest_addr)
{
    dlist_head_t *pos;

    dlist_for_each(pos, &rtable.l)
    {
        rtable_entry_t *e = (rtable_entry_t *) pos;
        if (e->rt_dest_addr.s_addr == dest_addr.s_addr)
            return e;
    }
    return NULL;
}

rtable_entry_t *NS_CLASS rtable_insert(struct in_addr dest_addr,
                                       struct in_addr nxthop_addr,
                                       u_int32_t ifindex,
                                       u_int32_t seqnum,
                                       u_int8_t prefix,
                                       u_int8_t hopcnt,
                                       u_int8_t is_gw,
                                       uint32_t cost,
                                       uint8_t hopfix)
{
    rtable_entry_t *entry;
    struct in_addr netmask;

    // Create the new entry
    if ((entry = (rtable_entry_t *) malloc(sizeof(rtable_entry_t)))
            == NULL)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "malloc() failed");
        exit(EXIT_FAILURE);
    }
    memset(entry, 0, sizeof(rtable_entry_t));

    entry->rt_ifindex   = ifindex;
    entry->rt_seqnum    = seqnum;
    entry->rt_prefix    = prefix;
    entry->rt_hopcnt    = hopcnt;
    entry->rt_is_gw     = is_gw;
    entry->rt_is_used   = 0;
    entry->rt_state     = RT_VALID;
    netmask.s_addr      = 0;
    entry->rt_dest_addr.s_addr  = dest_addr.s_addr;
    entry->rt_nxthop_addr.s_addr = nxthop_addr.s_addr;
    entry->cost         = cost;
    entry->rt_hopfix    = hopfix;

    timer_init(&entry->rt_validtimer, &NS_CLASS route_valid_timeout, entry);
    timer_set_timeout(&entry->rt_validtimer, ROUTE_TIMEOUT);
    timer_add(&entry->rt_validtimer);

    timer_init(&entry->rt_deltimer, &NS_CLASS route_del_timeout, entry);
    /*timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
    timer_add(&entry->rt_deltimer);*/

    // Add the entry to the routing table
    dlist_add(&entry->l, &rtable.l);

#ifndef NS_PORT
    // Add route to kernel routing table
    if (k_add_rte(dest_addr, nxthop_addr, netmask, hopcnt, ifindex) < 0)
        dlog(LOG_WARNING, errno, __FUNCTION__,
             "could not add kernel route");
    netlink_add_route(dest_addr);
#else
#ifdef OMNETPP
    /* Add route to omnet inet routing table ... */
    //netmask.s_addr = IPv4Address((uint32_t)nxthop_addr.s_addr).getNetworkMask().getInt();
    netmask.s_addr = IPv4Address::ALLONES_ADDRESS.getInt();
    if (useIndex)
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,ifindex);
    else
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,DEV_NR(ifindex).ipaddr);
#endif
#endif  /* NS_PORT */

    // If there are buffered packets for this destination
    // now we send them
    std::vector<L3Address> list;
    getListRelatedAp(dest_addr.s_addr, list);
    for (unsigned int i = 0; i < list.size(); i ++)
    {
        struct in_addr auxAaddr;
        auxAaddr.s_addr = list[i];
        if (pending_rreq_remove(pending_rreq_find(auxAaddr)))
        {
            packet_queue_set_verdict(auxAaddr, PQ_SEND);
        }
    }

    return entry;
}

rtable_entry_t *NS_CLASS rtable_update(rtable_entry_t *entry,
                                       struct in_addr dest_addr,
                                       struct in_addr nxthop_addr,
                                       u_int32_t ifindex,
                                       u_int32_t seqnum,
                                       u_int8_t prefix,
                                       u_int8_t hopcnt,
                                       u_int8_t is_gw,
                                       uint32_t cost,
                                       uint8_t hopfix)
{
#ifndef NS_PORT
    struct in_addr netmask;

    netmask.s_addr = 0;

    // If this route was expired but it isn't now,
    // a new kernel route is added
    //if (!timer_is_queued(&entry->rt_validtimer))
    if (entry->rt_state == RT_INVALID)
    {
        if (k_add_rte(dest_addr, nxthop_addr, netmask, hopcnt, ifindex) < 0)
            dlog(LOG_WARNING, errno, __FUNCTION__,
                 "could not add kernel route");
        netlink_add_route(dest_addr);
    }
    // Else the kernel route is updated
    else
    {
        if (k_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt, ifindex) < 0)
            dlog(LOG_WARNING, errno, __FUNCTION__,
                 "could not add kernel route");
        netlink_add_route(dest_addr);
    }
#else
#ifdef OMNETPP
    struct in_addr netmask;
    /* Add route to omnet inet routing table ... */
    //netmask.s_addr = IPv4Address((uint32_t)nxthop_addr.s_addr).getNetworkMask().getInt();
    netmask.s_addr = IPv4Address::ALLONES_ADDRESS.getInt();
    if (useIndex)
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,ifindex);
    else
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,DEV_NR(ifindex).ipaddr);


#endif
#endif  /* NS_PORT */

    timer_remove(&entry->rt_validtimer);
    timer_remove(&entry->rt_deltimer);

    entry->rt_ifindex   = ifindex;
    entry->rt_seqnum    = seqnum;
    entry->rt_prefix    = prefix;
    entry->rt_hopcnt    = hopcnt;
    entry->rt_is_gw     = is_gw;
    entry->rt_state     = RT_VALID;
    entry->rt_dest_addr.s_addr  = dest_addr.s_addr;
    entry->rt_nxthop_addr.s_addr    = nxthop_addr.s_addr;
    entry->cost         = cost;
    entry->rt_hopfix    = hopfix;

    timer_set_timeout(&entry->rt_validtimer, ROUTE_TIMEOUT);
    timer_add(&entry->rt_validtimer);

    /*timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
    timer_add(&entry->rt_deltimer);*/
//  timer_remove(&entry->rt_deltimer);

    // If there are buffered packets for this destination
    // now we send them
    std::vector<L3Address> list;
    getListRelatedAp(dest_addr.s_addr, list);
    for (unsigned int i = 0; i < list.size(); i ++)
    {
        struct in_addr auxAaddr;
        auxAaddr.s_addr = list[i];
        if (pending_rreq_remove(pending_rreq_find(auxAaddr)))
        {
            packet_queue_set_verdict(auxAaddr, PQ_SEND);
        }
    }

    return entry;
}

void NS_CLASS rtable_delete(rtable_entry_t *entry)
{
    if (!entry)
        return;

#ifdef OMNETPP
    struct in_addr netmask;
    /* delete route in the omnet inet routing table ... */
    netmask.s_addr = IPv4Address::ALLONES_ADDRESS.getInt();
    omnet_chg_rte(entry->rt_dest_addr,entry->rt_nxthop_addr, netmask,0,true);
#endif
    timer_remove(&entry->rt_deltimer);
    timer_remove(&entry->rt_validtimer);
    dlist_del(&entry->l);

    free(entry);
}

void NS_CLASS rtable_invalidate(rtable_entry_t *entry)
{
    if (!entry)
        return;

#ifndef NS_PORT
    //if (timer_is_queued(&entry->rt_validtimer))
    if (entry->rt_state == RT_VALID)
    {
        if (k_del_rte(entry->rt_dest_addr) < 0)
            dlog(LOG_WARNING, errno, __FUNCTION__,
                 "could not delete kernel route");
        netlink_del_route(entry->rt_dest_addr);
    }
#endif  /* NS_PORT */

#ifdef OMNETPP
    struct in_addr netmask;
    /* delete route in the omnet inet routing table ... */
    netmask.s_addr = IPv4Address::ALLONES_ADDRESS.getInt();
    omnet_chg_rte(entry->rt_dest_addr,entry->rt_nxthop_addr, netmask, 0,true);
#endif
    entry->rt_state = RT_INVALID;

    timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
    timer_add(&entry->rt_deltimer);
}

int NS_CLASS rtable_update_timeout(rtable_entry_t *entry)
{
    if (entry && entry->rt_state == RT_VALID) // this comparison seems ok
    {
        timer_set_timeout(&entry->rt_validtimer, ROUTE_TIMEOUT);
        timer_add(&entry->rt_validtimer);

        /*timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
        timer_add(&entry->rt_deltimer);*/

        // Mark the entry as used
        entry->rt_is_used = 1;

        return 1;
    }
    return 0;
}

int NS_CLASS rtable_expire_timeout(rtable_entry_t *entry)
{
    if (!entry)
        return 0;

    timer_set_timeout(&entry->rt_validtimer, 0);
    timer_add(&entry->rt_validtimer);

    return 1;
}

int NS_CLASS rtable_expire_timeout_all(struct in_addr nxthop_addr, u_int32_t ifindex)
{
    dlist_head_t *pos;
    int count = 0;

    dlist_for_each(pos, &rtable.l)
    {
        rtable_entry_t *entry = (rtable_entry_t *) pos;
        if (entry->rt_nxthop_addr.s_addr == nxthop_addr.s_addr &&
                entry->rt_ifindex == ifindex)
            count += rtable_expire_timeout(entry);
    }

    return count;
}


#else

// Map routing table

void NS_CLASS rtable_init()
{
    while (!dymoRoutingTable->empty())
    {
        if (dymoRoutingTable->begin()->second)
        {
            timer_remove(&dymoRoutingTable->begin()->second->rt_deltimer);
            timer_remove(&dymoRoutingTable->begin()->second->rt_validtimer);
            delete dymoRoutingTable->begin()->second;
        }
        dymoRoutingTable->erase(dymoRoutingTable->begin());
    }
    dymoRoutingTable->clear();
}

void NS_CLASS rtable_destroy()
{

    while (!dymoRoutingTable->empty())
    {
        if (dymoRoutingTable->begin()->second)
        {
            timer_remove(&dymoRoutingTable->begin()->second->rt_deltimer);
            timer_remove(&dymoRoutingTable->begin()->second->rt_validtimer);
            delete dymoRoutingTable->begin()->second;
        }
        dymoRoutingTable->erase(dymoRoutingTable->begin());
    }
}

rtable_entry_t *NS_CLASS rtable_find(struct in_addr dest_addr)
{
    if (dymoRoutingTable->empty())
        return NULL;
    DymoRoutingTable::iterator it = dymoRoutingTable->find(dest_addr.s_addr);
    if (it != dymoRoutingTable->end())
    {
        if (it->second)
        {
            // sanity check
            if (it->second->rt_dest_addr.s_addr != dest_addr.s_addr)
            {
                throw cRuntimeError("Dymo routing data base error");
            }
            return it->second;
        }
        else
            throw cRuntimeError("Dymo routing data base error, NULL entry");
    }
    else
    {
        L3Address apAdd;
        if (getAp(dest_addr.s_addr, apAdd))
        {
            it = dymoRoutingTable->find(apAdd);
            if (it != dymoRoutingTable->end())
            {
                if (it->second)
                {
                    // sanity check
                    if (it->second->rt_dest_addr.s_addr != apAdd)
                    {
                        throw cRuntimeError("Dymo routing data base error AP");
                    }
                    return it->second;
                }
                else
                    throw cRuntimeError("Dymo routing data base error, NULL entry AP");
            }
        }
        return NULL;
    }
    return NULL;
}

rtable_entry_t *NS_CLASS rtable_insert(struct in_addr dest_addr,
                                       struct in_addr nxthop_addr,
                                       u_int32_t ifindex,
                                       u_int32_t seqnum,
                                       u_int8_t prefix,
                                       u_int8_t hopcnt,
                                       u_int8_t is_gw,
                                       uint32_t cost,
                                       uint8_t hopfix)
{
    rtable_entry_t *entry;
    struct in_addr netmask;

    L3Address apAdd;
    if (getAp(dest_addr.s_addr, apAdd))
    {
        struct in_addr dest_addrAux;
        dest_addrAux.s_addr = apAdd;
        rtable_entry_t * e = rtable_find(dest_addrAux);
        if (e)
        {
            if (
                e->rt_ifindex   != ifindex &&
                e->rt_seqnum    != seqnum &&
                e->rt_prefix    != prefix &&
                e->rt_hopcnt    != hopcnt &&
                e->rt_is_gw     != is_gw &&
                e->cost         != cost &&
                e->rt_hopfix    != hopfix)
            return rtable_update(e, dest_addrAux, nxthop_addr, ifindex, seqnum, prefix, hopcnt, is_gw, cost, hopfix);
        }
        else
            return rtable_insert(dest_addrAux, nxthop_addr, ifindex, seqnum, prefix, hopcnt, is_gw, cost, hopfix);
    }


    DymoRoutingTable::iterator it = dymoRoutingTable->find(dest_addr.s_addr);
    if (it != dymoRoutingTable->end())
    {
        delete it->second;
        dymoRoutingTable->erase(it);
    }

    // Create the new entry
    if ((entry = new rtable_entry_t)== NULL)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "malloc() failed");
        exit(EXIT_FAILURE);
    }

    memset(entry, 0, sizeof(rtable_entry_t));

    entry->rt_ifindex   = ifindex;
    entry->rt_seqnum    = seqnum;
    entry->rt_prefix    = prefix;
    entry->rt_hopcnt    = hopcnt;
    entry->rt_is_gw     = is_gw;
    entry->rt_is_used   = 0;
    entry->rt_state     = RT_VALID;
    netmask.s_addr      = L3Address();
    entry->rt_dest_addr.s_addr  = dest_addr.s_addr;
    entry->rt_nxthop_addr.s_addr    = nxthop_addr.s_addr;
    entry->cost         = cost;
    entry->rt_hopfix    = hopfix;

    timer_init(&entry->rt_validtimer, &NS_CLASS route_valid_timeout, entry);
    timer_set_timeout(&entry->rt_validtimer, ROUTE_TIMEOUT);
    timer_add(&entry->rt_validtimer);

    timer_init(&entry->rt_deltimer, &NS_CLASS route_del_timeout, entry);
    /*timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
    timer_add(&entry->rt_deltimer);*/

    // Add the entry to the routing table

    dymoRoutingTable->insert(std::make_pair(dest_addr.s_addr,entry));
    /* Add route to omnet inet routing table ... */
    //netmask.s_addr = IPv4Address((uint32_t)nxthop_addr.s_addr).getNetworkMask().getInt();
    netmask.s_addr.set(IPv4Address::ALLONES_ADDRESS);
    if (useIndex)
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,ifindex);
    else
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,DEV_NR(ifindex).ipaddr);
    // If there are buffered packets for this destination
    // now we send them
    std::vector<L3Address> list;
    getListRelatedAp(dest_addr.s_addr, list);
    for (unsigned int i = 0; i < list.size(); i++)
    {
        struct in_addr auxAaddr;
        auxAaddr.s_addr = list[i];
         if (pending_rreq_remove(pending_rreq_find(auxAaddr)))
         {
             packet_queue_set_verdict(auxAaddr, PQ_SEND);
         }
    }
    return entry;
}

rtable_entry_t *NS_CLASS rtable_update(rtable_entry_t *entry,
                                       struct in_addr dest_addr,
                                       struct in_addr nxthop_addr,
                                       u_int32_t ifindex,
                                       u_int32_t seqnum,
                                       u_int8_t prefix,
                                       u_int8_t hopcnt,
                                       u_int8_t is_gw,
                                       uint32_t cost,
                                       uint8_t hopfix)
{

        // possible AP check
    L3Address apAdd;
    if (getAp(dest_addr.s_addr, apAdd))
    {
        if (entry->rt_dest_addr.s_addr == dest_addr.s_addr)
        {
            rtable_delete(entry);
            struct in_addr inapAdd;
            inapAdd.s_addr = apAdd;
            entry = rtable_find(inapAdd);
        }
        if (entry)
        {
            if (entry->rt_dest_addr.s_addr == apAdd )
                return rtable_update(entry,entry->rt_dest_addr,nxthop_addr, ifindex, seqnum, prefix, hopcnt, is_gw, cost, hopfix);
            else
                throw cRuntimeError("DYMO routing data base error");
        }
        else
        {
            struct in_addr inapAdd;
            inapAdd.s_addr = apAdd;
            return rtable_insert(inapAdd,nxthop_addr, ifindex, seqnum, prefix, hopcnt, is_gw, cost, hopfix);
        }
    }
    struct in_addr netmask;
    /* Add route to omnet inet routing table ... */
    //netmask.s_addr = IPv4Address((uint32_t)nxthop_addr.s_addr).getNetworkMask().getInt();
    netmask.s_addr.set(IPv4Address::ALLONES_ADDRESS);
    if (useIndex)
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,ifindex);
    else
        omnet_chg_rte(dest_addr, nxthop_addr, netmask, hopcnt,false,DEV_NR(ifindex).ipaddr);

    timer_remove(&entry->rt_validtimer);
    timer_remove(&entry->rt_deltimer);

    entry->rt_ifindex   = ifindex;
    entry->rt_seqnum    = seqnum;
    entry->rt_prefix    = prefix;
    entry->rt_hopcnt    = hopcnt;
    entry->rt_is_gw     = is_gw;
    entry->rt_state     = RT_VALID;
    entry->cost         = cost;
    entry->rt_hopfix    = hopfix;
    entry->rt_nxthop_addr.s_addr    = nxthop_addr.s_addr;

    timer_set_timeout(&entry->rt_validtimer, ROUTE_TIMEOUT);
    timer_add(&entry->rt_validtimer);

    /*timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
    timer_add(&entry->rt_deltimer);*/
//  timer_remove(&entry->rt_deltimer);
    // If there are buffered packets for this destination
    // now we send them
    std::vector<L3Address> list;
    getListRelatedAp(dest_addr.s_addr, list);
    for (unsigned int i = 0; i < list.size(); i ++)
    {
        struct in_addr auxAaddr;
        auxAaddr.s_addr = list[i];
        if (pending_rreq_remove(pending_rreq_find(auxAaddr)))
        {
            packet_queue_set_verdict(auxAaddr, PQ_SEND);
        }
    }
    return entry;
}

void NS_CLASS rtable_delete(rtable_entry_t *entry)
{
    if (!entry)
        return;

    struct in_addr netmask;
    /* delete route in the omnet inet routing table ... */
    netmask.s_addr.set(IPv4Address::ALLONES_ADDRESS);
    omnet_chg_rte(entry->rt_dest_addr,entry->rt_nxthop_addr, netmask,0,true);
    timer_remove(&entry->rt_deltimer);
    timer_remove(&entry->rt_validtimer);
    DymoRoutingTable::iterator it = dymoRoutingTable->find(entry->rt_dest_addr.s_addr);
    if (it != dymoRoutingTable->end())
    {
        if ((*it).second == entry)
        {
            dymoRoutingTable->erase(it);
        }
        else
            throw cRuntimeError("Error in dymo routing table");

    }
    delete entry;
}

void NS_CLASS rtable_invalidate(rtable_entry_t *entry)
{
    if (!entry)
        return;


    struct in_addr netmask;
    /* delete route in the omnet inet routing table ... */
    netmask.s_addr.set(IPv4Address::ALLONES_ADDRESS);
    omnet_chg_rte(entry->rt_dest_addr,entry->rt_nxthop_addr, netmask, 0,true);
    entry->rt_state = RT_INVALID;

    timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
    timer_add(&entry->rt_deltimer);
}

int NS_CLASS rtable_update_timeout(rtable_entry_t *entry)
{
    if (entry && entry->rt_state == RT_VALID) // this comparison seems ok
    {
        timer_set_timeout(&entry->rt_validtimer, ROUTE_TIMEOUT);
        timer_add(&entry->rt_validtimer);

        /*timer_set_timeout(&entry->rt_deltimer, ROUTE_DELETE_TIMEOUT);
        timer_add(&entry->rt_deltimer);*/

        // Mark the entry as used
        entry->rt_is_used = 1;

        return 1;
    }
    return 0;
}

int NS_CLASS rtable_expire_timeout(rtable_entry_t *entry)
{
    if (!entry)
        return 0;

    timer_set_timeout(&entry->rt_validtimer, 0);
    timer_add(&entry->rt_validtimer);

    return 1;
}

int NS_CLASS rtable_expire_timeout_all(struct in_addr nxthop_addr, u_int32_t ifindex)
{
    int count = 0;

    for (DymoRoutingTable::iterator it = dymoRoutingTable->begin(); it != dymoRoutingTable->end(); it++)
    {
        rtable_entry_t *entry = (rtable_entry_t *) it->second;
        if (entry->rt_nxthop_addr.s_addr == nxthop_addr.s_addr &&
                entry->rt_ifindex == ifindex)
            count += rtable_expire_timeout(entry);
    }

    return count;
}
#endif

} // namespace inetmanet

} // namespace inet

