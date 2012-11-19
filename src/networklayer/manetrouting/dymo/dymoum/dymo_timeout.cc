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
#include "dymo_timeout.h"
#include "dymo_re.h"
#include "dymo_netlink.h"
#include "pending_rreq.h"
#include "rtable.h"
#include "blacklist.h"
#include "dymo_nb.h"
#include "debug_dymo.h"


extern int reissue_rreq;
#endif  /* NS_PORT */

void NS_CLASS route_valid_timeout(void *arg)
{
    rtable_entry_t *entry = (rtable_entry_t *) arg;

    if (!entry)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__,
             "NULL routing table entry, ignoring timeout");
        return;
    }

    rtable_invalidate(entry);
}

void NS_CLASS route_del_timeout(void *arg)
{
    rtable_entry_t *entry = (rtable_entry_t *) arg;

    if (!entry)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__,
             "NULL routing table entry, ignoring timeout");
        return;
    }

    //if (entry->rt_state == RT_INVALID) // I think this isn't needed
    rtable_delete(entry);
}

void NS_CLASS blacklist_timeout(void *arg)
{
    blacklist_t *entry = (blacklist_t *) arg;

    if (!entry)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__,
             "NULL blacklist entry, ignoring timeout");
        return;
    }

    blacklist_remove(entry);
}

void NS_CLASS route_discovery_timeout(void *arg)
{
    pending_rreq_t *entry = (pending_rreq_t *) arg;

    if (!entry)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__,
             "NULL pending route discovery list entry,"
             " ignoring timeout");
        return;
    }

    if (reissue_rreq)
    {
        if (entry->tries < RREQ_TRIES)
        {
            rtable_entry_t *rte;

            entry->tries++;
            int seqnum = entry->seqnum;
            if (entry->tries == RREQ_TRIES)
            {
                seqnum  = 0;
            }

            timer_set_timeout(&entry->timer,
                              RREQ_WAIT_TIME << entry->tries);
            timer_add(&entry->timer);

            rte = rtable_find(entry->dest_addr);
            if (rte)
                re_send_rreq(entry->dest_addr, seqnum,rte->rt_hopcnt);
            else
                re_send_rreq(entry->dest_addr, seqnum,0);

            return;
        }
    }
#ifdef NS_PORT
    std::vector<ManetAddress> list;
    getListRelatedAp(entry->dest_addr.s_addr, list);
    for (unsigned int i = 0; i < list.size(); i ++)
    {
        struct in_addr auxAaddr;
        auxAaddr.s_addr = list[i];
        packet_queue_set_verdict(auxAaddr, PQ_DROP);
    }
#else
    netlink_no_route_found(entry->dest_addr);
#endif  /* NS_PORT */

    pending_rreq_remove(entry);
}

void NS_CLASS nb_timeout(void *arg)
{
    nb_t *nb = (nb_t *) arg;

    if (!nb)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__,
             "NULL nblist entry, ignoring timeout");
        return;
    }

    // A link break has been detected: Expire all routes utilizing the
    // broken link
    rtable_expire_timeout_all(nb->nb_addr, nb->ifindex);
    nb_remove(nb);
}
