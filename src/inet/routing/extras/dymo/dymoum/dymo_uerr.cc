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
#include "inet/routing/extras/dymo/dymoum/dymo_uerr.h"
#include "inet/routing/extras/dymo/dymoum/dymo_socket.h"

#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

UERR *NS_CLASS uerr_create(struct in_addr target_addr,
                           struct in_addr uelem_target_addr,
                           struct in_addr uerr_node_addr,
                           u_int8_t uelem_type, u_int8_t ttl)
{
    UERR *uerr;
#ifndef OMNETPP
    uerr        = (UERR *) dymo_socket_new_element();
#else
    uerr        = new UERR ();
#endif
    uerr->m     = 0;
    uerr->h     = 0;
    uerr->type  = DYMO_UERR_TYPE;
    uerr->len   = UERR_SIZE;
    uerr->ttl   = ttl;
    uerr->i     = 0;
    uerr->res   = 0;

#ifndef OMNETPP
    uerr->target_addr   = (u_int32_t) target_addr.s_addr;
    uerr->uerr_node_addr    = (u_int32_t) uerr_node_addr.s_addr;
    uerr->uelem_target_addr = (u_int32_t) uelem_target_addr.s_addr;
#else
    uerr->target_addr   = target_addr.s_addr;
    uerr->uerr_node_addr    = uerr_node_addr.s_addr;
    uerr->uelem_target_addr = uelem_target_addr.s_addr;
#endif
    uerr->uelem_type    = uelem_type;

    return uerr;
}

void NS_CLASS uerr_send(DYMO_element *e, u_int32_t ifindex)
{
    struct in_addr notify_addr, target_addr;
    rtable_entry_t *entry;

    notify_addr.s_addr  = e->notify_addr;
    target_addr.s_addr  = e->target_addr;

    dlog(LOG_DEBUG, 0, __FUNCTION__, "sending UERR to %s",
         ip2str(notify_addr.s_addr));

    UERR *uerr = uerr_create(notify_addr,
                             target_addr,
                             DEV_IFINDEX(ifindex).ipaddr,
                             e->type,
                             NET_DIAMETER);

    entry = rtable_find(notify_addr);
    if (entry && entry->rt_state == RT_VALID)
    {
        notify_addr.s_addr = entry->rt_nxthop_addr.s_addr;

        // Queue the new UERR
        uerr = (UERR *) dymo_socket_queue((DYMO_element *) uerr);

        // Send UERR over appropriate interface
        if (DEV_IFINDEX(entry->rt_ifindex).enabled)
            dymo_socket_send(notify_addr, &DEV_IFINDEX(entry->rt_ifindex));
    }
    else
    {
        dlog(LOG_DEBUG, 0, __FUNCTION__, "could not send a UERR to %s"
             " because there is no suitable route",
             ip2str(notify_addr.s_addr));
    }
}

void NS_CLASS uerr_process(UERR *e, struct in_addr ip_src, u_int32_t ifindex)
{
#ifndef OMNETPP
    delete e;
    e = nullptr;
#endif
}

} // namespace inetmanet

} // namespace inet

