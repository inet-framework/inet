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
#include "dymo_nb.h"
#include "debug_dymo.h"
#include "dymo_timeout.h"
#include <stdlib.h>
#include <errno.h>

static DLIST_HEAD(NBLIST);
extern int hello_ival;
#endif  /* NS_PORT */

#ifdef MAPROUTINGTABLE
nb_t *NS_CLASS nb_insert(struct in_addr nb_addr, u_int32_t ifindex)
{
    nb_t *nb  = nb_find(nb_addr,ifindex);
    if (nb)
        return nb;
    nb=new nb_t;
    if (nb == NULL)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "failed malloc()");
        exit(EXIT_FAILURE);
    }

    nb->nb_addr.s_addr  = nb_addr.s_addr;
    nb->ifindex     = ifindex;
    timer_init(&nb->timer, &NS_CLASS nb_timeout, nb);
    nb_update(nb);
    dymoNbList->push_back(nb);
    return nb;
}

void NS_CLASS nb_update(nb_t *nb)
{
    timer_set_timeout(&nb->timer, NB_TIMEOUT);
    timer_add(&nb->timer);
}

int NS_CLASS nb_remove(nb_t *nb)
{
    if (!nb)
        return 0;
    DymoNbList::iterator it;
    for (it =dymoNbList->begin();it !=dymoNbList->end();it++)
    {
        if (*it==nb)
        {
            dymoNbList->erase(it);
            break;
        }
    }
    timer_remove(&nb->timer);
    delete nb;
    return 1;
}

nb_t *NS_CLASS nb_find(struct in_addr nb_addr, u_int32_t ifindex)
{
    DymoNbList::iterator it;
    for (it =dymoNbList->begin();it !=dymoNbList->end();it++)
    {
    	nb_t *nb = *it;
        if (*it==nb)
        {
            if (nb->nb_addr.s_addr == nb_addr.s_addr &&
                    nb->ifindex == ifindex)
            {
                return nb;
            }
        }
    }
    return NULL;
}
#else

nb_t *NS_CLASS nb_insert(struct in_addr nb_addr, u_int32_t ifindex)
{
    nb_t *nb;

    if ((nb = (nb_t *) malloc(sizeof(nb_t))) == NULL)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "failed malloc()");
        exit(EXIT_FAILURE);
    }

    nb->nb_addr.s_addr  = nb_addr.s_addr;
    nb->ifindex     = ifindex;
    timer_init(&nb->timer, &NS_CLASS nb_timeout, nb);
    nb_update(nb);
    dlist_add(&nb->list_head, &NBLIST);

    return nb;
}

void NS_CLASS nb_update(nb_t *nb)
{
    timer_set_timeout(&nb->timer, NB_TIMEOUT);
    timer_add(&nb->timer);
}

int NS_CLASS nb_remove(nb_t *nb)
{
    if (!nb)
        return 0;

    dlist_del(&nb->list_head);
    timer_remove(&nb->timer);

    free(nb);

    return 1;
}

nb_t *NS_CLASS nb_find(struct in_addr nb_addr, u_int32_t ifindex)
{
    dlist_head_t *pos;

    dlist_for_each(pos, &NBLIST)
    {
        nb_t *nb = (nb_t *) pos;
        if (nb->nb_addr.s_addr == nb_addr.s_addr &&
                nb->ifindex == ifindex)
        {
            return nb;
        }
    }

    return NULL;
}
#endif
