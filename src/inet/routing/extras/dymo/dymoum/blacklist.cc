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
#include "inet/routing/extras/dymo/dymoum/blacklist.h"
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"

#include <stdlib.h>
#include <errno.h>

static DLIST_HEAD(BLACKLIST);
#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

#ifdef MAPROUTINGTABLE
blacklist_t *NS_CLASS blacklist_add(struct in_addr addr)
{
    blacklist_t *entry =blacklist_find(addr);
    if (entry==nullptr)
        entry = new blacklist_t;
    else
        return entry;
    if (entry  == nullptr)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "failed malloc()");
        exit(EXIT_FAILURE);
    }
    entry->addr.s_addr  = addr.s_addr;
    dymoBlackList->insert(std::make_pair(addr.s_addr, entry));
    return entry;
}

int NS_CLASS blacklist_remove(blacklist_t *entry)
{
    if (!entry)
        return 0;

    for (auto it = dymoBlackList->begin(); it != dymoBlackList->end(); )
    {
        auto cur = it;
        it++;
        if ((*cur).second==entry)
        {
            timer_remove(&entry->timer);
            dymoBlackList->erase(cur);
        }
    }
    delete entry;
    return 1;
}

blacklist_t *NS_CLASS blacklist_find(struct in_addr addr)
{
    auto it = dymoBlackList->find(addr.s_addr);
    if (it != dymoBlackList->end())
    {
        if ((*it).second)
        {
            return (*it).second;
        }
        else
            dymoBlackList->erase(it);
    }
    return nullptr;
}

void NS_CLASS blacklist_erase()
{
    while (!dymoBlackList->empty())
    {
        if (dymoBlackList->begin()->second)
        {
            delete dymoBlackList->begin()->second;
        }
        dymoBlackList->erase(dymoBlackList->begin());
    }
}

#else
blacklist_t *NS_CLASS blacklist_add(struct in_addr addr)
{
    blacklist_t *entry;

    if ((entry = (blacklist_t *)malloc(sizeof(blacklist_t))) == nullptr)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "failed malloc()");
        exit(EXIT_FAILURE);
    }

    entry->addr.s_addr  = addr.s_addr;
    dlist_add(&entry->list_head, &BLACKLIST);

    return entry;
}

int NS_CLASS blacklist_remove(blacklist_t *entry)
{
    if (!entry)
        return 0;

    dlist_del(&entry->list_head);
    timer_remove(&entry->timer);

    free(entry);

    return 1;
}

blacklist_t *NS_CLASS blacklist_find(struct in_addr addr)
{
    dlist_head_t *pos;

    dlist_for_each(pos, &BLACKLIST)
    {
        blacklist_t *entry = (blacklist_t *) pos;
        if (entry->addr.s_addr == addr.s_addr)
            return entry;
    }

    return nullptr;
}
#endif

} // namespace inetmanet

} // namespace inet

