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
#include "inet/routing/extras/dymo/dymoum/timer_queue.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>


static DLIST_HEAD(TQ);
#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

#if defined(OMNETPP) && defined(TIMERMAPLIST)

int NS_CLASS timer_init(struct timer *t, timeout_func_t f, void *data)
{
    // Sanity check
    if (t)
    {
        t->used     = 0;
        t->handler  = f;
        t->data     = data;
        t->timeout.tv_sec   = 0;
        t->timeout.tv_usec  = 0;
        return 0;
    }
    return -1;
}

int NS_CLASS timer_is_queued(struct timer *t)
{
    if (t)
        for (auto & elem : *dymoTimerList)
        {
            if ((elem).second == t)return 1;
        }
    return 0;
}

int NS_CLASS timer_add(struct timer *t)
{
    // Sanity checks
    if (!t || !t->handler)
        return -1;

    // If the timer is already in the queue we firstly remove it
    if (t->used)
        timer_remove(t);
    t->used = 1;

    simtime_t timeout = t->timeout.tv_sec;
    timeout += ((double)(t->timeout.tv_usec)/1000000.0);

    dymoTimerList->insert(std::make_pair(timeout,t));
    return DLIST_SUCCESS;
}

int NS_CLASS timer_remove(struct timer *t)
{
    // Sanity check
    if (!t)
        return -1;

    t->used = 0;
    for (auto i = dymoTimerList->begin(); i != dymoTimerList->end(); i++)
    {
        if ((*i).second == t)
        {
            dymoTimerList->erase(i);
            return DLIST_SUCCESS;
        }
    }
    return DLIST_FAILURE;
}

int NS_CLASS timer_set_timeout(struct timer *t, long msec)
{
    // Sanity checks
    if (!t || msec < 0)
        return -1;

    gettimeofday(&t->timeout, nullptr);

    t->timeout.tv_usec += msec * 1000;
    t->timeout.tv_sec += t->timeout.tv_usec / 1000000;
    t->timeout.tv_usec = t->timeout.tv_usec % 1000000;

    return 0;
}

void NS_CLASS timer_timeout(struct timeval *now)
{

    while (!dymoTimerList->empty() && (timeval_diff(&(dymoTimerList->begin()->second->timeout), now) <= 0))
    {
        struct timer * t = dymoTimerList->begin()->second;
        dymoTimerList->erase(dymoTimerList->begin());
        if (t==nullptr)
            throw cRuntimeError("timer ower is bad");
        else
        {
            if (t->handler)
                (this->*t->handler)(t->data);
        }
    }
}

struct timeval *NS_CLASS timer_age_queue()
{
    struct timer *t;
    static struct timeval remaining;
    struct timeval now;
    gettimeofday(&now, nullptr);

    while (!dymoTimerList->empty())
    {
        t = dymoTimerList->begin()->second;
        if (t==nullptr)
            throw cRuntimeError("timer ower is bad");
        if (timeval_diff(&(t->timeout), &now)>0)
            break;
        dymoTimerList->erase(dymoTimerList->begin());
        if (t->handler)
            (this->*t->handler)(t->data);
    }

    if (dymoTimerList->empty())
        return nullptr;

    t = dymoTimerList->begin()->second;
    if (timeval_diff(&(dymoTimerList->begin()->second->timeout), &now)<=0)
        throw cRuntimeError("Dymo Time queue error");
    remaining.tv_usec   = (t->timeout.tv_usec - now.tv_usec);
    remaining.tv_sec    = (t->timeout.tv_sec - now.tv_sec);
    if (remaining.tv_usec < 0)
    {
        remaining.tv_usec += 1000000;
        remaining.tv_sec -= 1;
    }

    return (&remaining);
}
#else
int NS_CLASS timer_init(struct timer *t, timeout_func_t f, void *data)
{
    // Sanity check
    if (t)
    {
        INIT_DLIST_ELEM(&t->list_head);
        t->used     = 0;
        t->handler  = f;
        t->data     = data;
        t->timeout.tv_sec   = 0;
        t->timeout.tv_usec  = 0;

        return 0;
    }
    return -1;
}

int NS_CLASS timer_is_queued(struct timer *t)
{
    if (t)
        return !dlist_unattached(&t->list_head);
    return 0;
}

int NS_CLASS timer_add(struct timer *t)
{
    dlist_head_t *pos;
    int status;

    // Sanity checks
    if (!t || !t->handler)
        return -1;

    // If the timer is already in the queue we firstly remove it
    if (t->used)
        timer_remove(t);
    t->used = 1;
#ifdef OMNETPP
    timer_elem++;
    int cont=0;
#endif
    // Add the timer
    if (dlist_empty(&TQ))
        status = dlist_add(&t->list_head, &TQ);
    else
    {
        dlist_for_each(pos, &TQ)
        {
            cont++;
            struct timer *curr = (struct timer *) pos;
            if (timeval_diff(&t->timeout, &curr->timeout) < 0)
                break;
        }
        status = dlist_add(&t->list_head, pos->prev);
    }
#ifdef OMNETPP
    // comprobacion integridad
    /*
        cont =0;
        struct timer *curr;
        struct timer *prev;
        dlist_for_each(pos, &TQ)
        {
            cont++;
            curr = (struct timer *) pos;
            if (cont>timer_elem)
                break;
            prev = (struct timer *) pos->prev;
        }
        if (cont!=timer_elem)
                printf ("error\n");
    */
#endif
    return status;
}

int NS_CLASS timer_remove(struct timer *t)
{
    // Sanity check
    if (!t)
        return -1;

    t->used = 0;
    if (dlist_unattached(&t->list_head))
        return 0;
    else
    {
#ifdef OMNETPP
        int status =dlist_del(&t->list_head);
        if (status==DLIST_SUCCESS)
            timer_elem--;
        return status;
#else
return dlist_del(&t->list_head);
#endif
    }
}

int NS_CLASS timer_set_timeout(struct timer *t, long msec)
{
    // Sanity checks
    if (!t || msec < 0)
        return -1;

    gettimeofday(&t->timeout, nullptr);

    t->timeout.tv_usec += msec * 1000;
    t->timeout.tv_sec += t->timeout.tv_usec / 1000000;
    t->timeout.tv_usec = t->timeout.tv_usec % 1000000;

    return 0;
}

void NS_CLASS timer_timeout(struct timeval *now)
{
    dlist_head_t *pos, *tmp;

    dlist_for_each_safe(pos, tmp, &TQ)
    {
        struct timer *t = (struct timer *) pos;

        if (timeval_diff(&t->timeout, now) > 0)
            break;
        else
        {
            t->used = 0;
#ifdef OMNETPP
            timer_elem--;
            //if (t->list_head.next==nullptr)
//              (this->*t->handler)(t->data);
#endif

            dlist_del(&t->list_head);
            if (t->handler)
#ifdef NS_PORT
                (this->*t->handler)(t->data);
#else
t->handler(t->data);
#endif  /* NS_PORT */

        }
    }
}

struct timeval *NS_CLASS timer_age_queue()
{
    static struct timeval remaining;
    struct timeval now;
    struct timer *t;

    gettimeofday(&now, nullptr);

    if (dlist_empty(&TQ))
        return nullptr;

    timer_timeout(&now);

    if (dlist_empty(&TQ))
        return nullptr;

    t = (struct timer *) TQ.next;
    remaining.tv_usec   = (t->timeout.tv_usec - now.tv_usec);
    remaining.tv_sec    = (t->timeout.tv_sec - now.tv_sec);

    if (remaining.tv_usec < 0)
    {
        remaining.tv_usec += 1000000;
        remaining.tv_sec -= 1;
    }

    return (&remaining);
}
#endif

} // namespace inetmanet

} // namespace inet

