/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University & Ericsson AB.
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
 * Authors: Erik Nordström, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/
#ifndef _TIMER_QUEUE_H
#define _TIMER_QUEUE_H

#ifndef NS_NO_GLOBALS

//#ifndef  _WIN32
#include <sys/time.h>
//#endif

#include "defs_aodv.h"
#include "list.h"

#ifdef NS_PORT
typedef void (AODVUU::*timeout_func_t) (void *);
#else
typedef void (*timeout_func_t) (void *);
#endif


struct timer
{
    list_t l;
    int used;
    struct timeval timeout;
    timeout_func_t handler;
    void *data;
};

static inline long timeval_diff(struct timeval *t1, struct timeval *t2)
{
    long long res;      /* We need this to avoid overflows while calculating... */

    if (!t1 || !t2)
        return -1;
    else
    {

        res = t1->tv_sec;
        res = ((res - t2->tv_sec) * 1000000 + t1->tv_usec - t2->tv_usec) / 1000;
        return (long) res;
    }
}

static inline int timeval_add_msec(struct timeval *t, unsigned long msec)
{
    unsigned long long add; /* Protect against overflows */

    if (!t)
        return -1;

    add = t->tv_usec + (msec * 1000);
    t->tv_sec += add / 1000000;
    t->tv_usec = add % 1000000;

    return 0;
}
#endif              /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
void timer_queue_init();
int timer_remove(struct timer *t);
void timer_set_timeout(struct timer *t, long msec);
int timer_timeout_now(struct timer *t);
struct timeval *timer_age_queue();
/* timer_init should be called for every newly allocated timer */
int timer_init(struct timer *t, timeout_func_t f, void *data);

#ifdef NS_PORT
void timer_add(struct timer *t);
void timer_timeout(struct timeval *now);

#ifdef DEBUG_TIMER_QUEUE
void NS_CLASS printTQ();
#endif              /* DEBUG_TIMER_QUEUE */

#endif              /* NS_PORT */

#endif              /* NS_NO_DECLARATIONS */

#endif              /* TIMER_QUEUE_H */
