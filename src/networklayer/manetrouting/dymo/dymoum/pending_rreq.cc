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
#include <errno.h>
#else
#include "debug_dymo.h"
#include "defs_dymo.h"
#include "pending_rreq.h"

#include <stdlib.h>
#include <errno.h>

static DLIST_HEAD(PENDING_RREQ);
#endif  /* NS_PORT */

#ifdef MAPROUTINGTABLE
pending_rreq_t *NS_CLASS pending_rreq_add(struct in_addr dest_addr, u_int32_t seqnum)
{

    pending_rreq_t *entry = pending_rreq_find(dest_addr);
    if (entry)
          return entry;
    entry = new pending_rreq_t;

    if (entry== NULL)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "failed malloc()");
        exit(EXIT_FAILURE);
    }

    Uint128 dest = dest_addr.s_addr;
    Uint128 apAddr;
    if (getAp(dest,apAddr))
        dest = apAddr;

    entry->dest_addr.s_addr = dest;
    entry->seqnum       = seqnum;
    entry->tries        = 0;
    dymoPendingRreq->insert(std::make_pair(dest, entry));
    return entry;
}

int NS_CLASS pending_rreq_remove(pending_rreq_t *entry)
{
    if (!entry)
        return 0;

    DymoPendingRreq::iterator it = dymoPendingRreq->find(entry->dest_addr.s_addr);
    if (it != dymoPendingRreq->end())
    {
        if ((*it).second == entry)
        {
            dymoPendingRreq->erase(it);
        }
        else
            opp_error("Error in dymoPendingRreq table");

    }
    timer_remove(&entry->timer);
    delete entry;
    return 1;
}

pending_rreq_t *NS_CLASS pending_rreq_find(struct in_addr dest_addr)
{
    Uint128 dest = dest_addr.s_addr;
    Uint128 apAddr;
    if (getAp(dest,apAddr))
        dest = apAddr;

    DymoPendingRreq::iterator it = dymoPendingRreq->find(dest);
    if (it != dymoPendingRreq->end())
    {
        pending_rreq_t *entry = it->second;
        if (entry->dest_addr.s_addr == dest)
            return entry;
        else
            opp_error("Error in dymoPendingRreq table");
    }

    return NULL;
}

#else
pending_rreq_t *NS_CLASS pending_rreq_add(struct in_addr dest_addr, u_int32_t seqnum)
{
    pending_rreq_t *entry;

    if ((entry = (pending_rreq_t *)malloc(sizeof(pending_rreq_t))) == NULL)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "failed malloc()");
        exit(EXIT_FAILURE);
    }

    entry->dest_addr.s_addr = dest_addr.s_addr;
    entry->seqnum       = seqnum;
    entry->tries        = 0;

    dlist_add(&entry->list_head, &PENDING_RREQ);

    return entry;
}

int NS_CLASS pending_rreq_remove(pending_rreq_t *entry)
{
    if (!entry)
        return 0;

    dlist_del(&entry->list_head);
    timer_remove(&entry->timer);

    free(entry);

    return 1;
}

pending_rreq_t *NS_CLASS pending_rreq_find(struct in_addr dest_addr)
{
    dlist_head_t *pos;

    dlist_for_each(pos, &PENDING_RREQ)
    {
        pending_rreq_t *entry = (pending_rreq_t *) pos;
        if (entry->dest_addr.s_addr == dest_addr.s_addr)
            return entry;
    }

    return NULL;
}
#endif
