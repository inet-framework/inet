/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University and Ericsson AB.
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
#define NS_PORT
#define OMNETPP

#include <stdlib.h>

#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/aodv-uu.h"
#else
#include "../aodv_uu_omnet.h"
#endif
#include "inet/routing/extras/aodv-uu/aodv-uu/list.h"
#else
#include "inet/routing/extras/aodv-uu/aodv-uu/seek_list.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/timer_queue_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/aodv_timeout.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/defs_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/params.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/debug_aodv.h"
#include "list.h"
#endif

#ifndef NS_PORT
/* The seek list is a linked list of destinations we are seeking
   (with RREQ's). */

static LIST(seekhead);

#ifdef SEEK_LIST_DEBUG
void seek_list_print();
#endif
#endif              /* NS_PORT */

namespace inet {

namespace inetmanet {

#ifndef AODV_USE_STL
seek_list_t *NS_CLASS seek_list_insert(struct in_addr dest_addr,
                                       u_int32_t dest_seqno,
                                       int ttl, u_int8_t flags,
                                       struct ip_data *ipd)
{
    seek_list_t *entry;

    if ((entry = (seek_list_t *) malloc(sizeof(seek_list_t))) == nullptr)
    {
        fprintf(stderr, "Failed malloc\n");
        exit(-1);
    }

    entry->dest_addr = dest_addr;
    entry->dest_seqno = dest_seqno;
    entry->flags = flags;
    entry->reqs = 0;
    entry->ttl = ttl;
    entry->ipd = ipd;

    timer_init(&entry->seek_timer, &NS_CLASS route_discovery_timeout, entry);

    list_add(&seekhead, &entry->l);
#ifdef SEEK_LIST_DEBUG
    seek_list_print();
#endif
    return entry;
}

int NS_CLASS seek_list_remove(seek_list_t * entry)
{
    if (!entry)
        return 0;

    list_detach(&entry->l);

    /* Make sure any timers are removed */
    timer_remove(&entry->seek_timer);

    if (entry->ipd)
        free(entry->ipd);

    free(entry);
    return 1;
}

seek_list_t *NS_CLASS seek_list_find(struct in_addr dest_addr)
{
    list_t *pos;

    list_foreach(pos, &seekhead)
    {
        seek_list_t *entry = (seek_list_t *) pos;

        if (entry->dest_addr.s_addr == dest_addr.s_addr)
            return entry;
    }
    return nullptr;
}

#ifdef SEEK_LIST_DEBUG
void NS_CLASS seek_list_print()
{
    list_t *pos;

    list_foreach(pos, &seekhead)
    {
        seek_list_t *entry = (seek_list_t *) pos;
        printf("%s %u %d %d\n", ip_to_str(entry->dest_addr),
               entry->dest_seqno, entry->reqs, entry->ttl);
    }
}
#endif
#else
seek_list_t * NS_CLASS seek_list_insert(struct in_addr dest_addr,
                                       u_int32_t dest_seqno,
                                       int ttl, u_int8_t flags,
                                       struct ip_data *ipd)
{
    seek_list_t *entry;

    entry = new seek_list_t;
    if (entry == nullptr)
    {
        fprintf(stderr, "Failed malloc\n");
        exit(-1);
    }

    entry->dest_addr = dest_addr;
    entry->dest_seqno = dest_seqno;
    entry->flags = flags;
    entry->reqs = 0;
    entry->ttl = ttl;
    entry->ipd = ipd;

    timer_init(&entry->seek_timer, &NS_CLASS route_discovery_timeout, entry);
    seekhead.insert(std::make_pair(dest_addr.s_addr,entry));

#ifdef SEEK_LIST_DEBUG
    seek_list_print();
#endif
    return entry;
}

int NS_CLASS seek_list_remove(seek_list_t * entry)
{
    if (!entry)
        return 0;

    for (auto it =seekhead.begin();it != seekhead.end(); it++)
    {
        if (it->second == entry)
        {
            seekhead.erase(it);
            break;
        }
    }

    /* Make sure any timers are removed */
    timer_remove(&entry->seek_timer);

    if (entry->ipd)
        free(entry->ipd);

    delete entry;
    return 1;
}

seek_list_t *NS_CLASS seek_list_find(struct in_addr dest_addr)
{
    auto it =seekhead.find(dest_addr.s_addr);
    if (it != seekhead.end())
        return it->second;
    return nullptr;
}

#ifdef SEEK_LIST_DEBUG
void NS_CLASS seek_list_print()
{
    for (auto it =seekhead.begin();it != seekhead.end(); it++)
    {
        seek_list_t *entry = it->second;
        printf("%s %u %d %d\n", ip_to_str(entry->dest_addr),
                      entry->dest_seqno, entry->reqs, entry->ttl);    }
}
#endif

#endif

} // namespace inetmanet

} // namespace inet

