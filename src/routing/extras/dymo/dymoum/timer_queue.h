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

#ifndef __TIMER_QUEUE_H__
#define __TIMER_QUEUE_H__

#ifndef NS_NO_GLOBALS
#include "defs_dymo.h"
#include "dlist.h"

#ifndef _WIN32
#include <sys/time.h>
#endif



typedef void (NS_CLASS*timeout_func_t) (void *);

#if defined(OMNETPP) && defined(TIMERMAPLIST)

struct timer
{
    int     used;
    struct timeval  timeout;
    timeout_func_t  handler;
    void        *data;
};

#else

struct timer
{
    dlist_head_t    list_head;
    int     used;
    struct timeval  timeout;
    timeout_func_t  handler;
    void        *data;
};

#endif

NS_STATIC NS_INLINE long timeval_diff(struct timeval *t1, struct timeval *t2)
{
    long long res;

    // Sanity check
    if (t1 && t2)
    {
        res = t1->tv_sec;
        res = ((res - t2->tv_sec) * 1000000 + t1->tv_usec - t2->tv_usec) / 1000;
        return (long) res;
    }
    return -1;
}
#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
/* This should be called for every newly allocated timer */
int timer_init(struct timer *t, timeout_func_t f, void *data);

/* Check whether a timer is queued or not */
int timer_is_queued(struct timer *t);

/* Add a new timer timer to the queue (lower to higher timeout order) */
int timer_add(struct timer *t);

/* Remove a timer from the queue */
int timer_remove(struct timer *t);

/* Set the timer to timeout msec miliseconds in the future. The timer is not
   enqueued (you must call timer_add() if you need it) */
int timer_set_timeout(struct timer *t, long msec);

/* Execute all expired timers and remove them from the queue */
void timer_timeout(struct timeval *now);

/* This function must be called by the main loop. It calls timer_timeout()
   and returns the remaining time until the next scheduled timeout in the
   queue */
struct timeval *timer_age_queue();

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __TIMER_QUEUE_H__ */
