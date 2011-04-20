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
#include "defs_dymo.h"
#include "dymo_hello.h"
#include "timer_queue.h"
#include "dymo_socket.h"
#include "dymo_nb.h"
#include <sys/time.h>
#include <stdlib.h>

static struct timer hello_timer;
extern int hello_ival;
#endif  /* NS_PORT */

void NS_CLASS hello_init(void)
{
    if (hello_timer.used || hello_ival <= 0)
        return;

    timer_init(&hello_timer, &NS_CLASS hello_send, NULL);
    hello_send(NULL);
}

void NS_CLASS hello_fini(void)
{
    timer_remove(&hello_timer);
}

HELLO *NS_CLASS hello_create(void)
{
    HELLO *hello;
#ifndef OMNETPP
    hello       = (HELLO *) dymo_socket_new_element();
#else
    hello = new HELLO();
#endif
    hello->m    = 0;
    hello->h    = 0;
    hello->type = DYMO_HELLO_TYPE;
    hello->len  = HELLO_BASIC_SIZE;
    hello->ttl  = 1;
    hello->i    = 0;
    hello->res  = 0;

    return hello;
}

void NS_CLASS hello_send(void *arg)
{
    int i;
    struct in_addr dest_addr;

    dlog(LOG_DEBUG, 0, __FUNCTION__, "sending HELLO");

    HELLO *hello = hello_create();
    dest_addr.s_addr = DYMO_BROADCAST;
#ifdef OMNETPP
    double delay = -1;
    int cont = numInterfacesActive;
    if (par("EqualDelay"))
        delay = par("broadCastDelay");

// Send HELLO over all enabled interfaces
    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
        if (DEV_NR(i).enabled)
        {
            if (cont>1)
                dymo_socket_queue((DYMO_element *) hello->dup());
            else
                dymo_socket_queue((DYMO_element *) hello);
            dymo_socket_send(dest_addr, &DEV_NR(i),delay);
            cont--;
        }

#else
    // Queue the new HELLO
    hello = (HELLO *) dymo_socket_queue((DYMO_element *) hello);
    // Send HELLO over all enabled interfaces
    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
        if (DEV_NR(i).enabled)
            dymo_socket_send(dest_addr, &DEV_NR(i));

#endif


    // Schedule next HELLO
    timer_set_timeout(&hello_timer, (hello_ival*1000) + hello_jitter());
    timer_add(&hello_timer);
}

void NS_CLASS hello_process(HELLO *hello,struct in_addr ip_src, u_int32_t ifindex)
{
    nb_t *nb;

    // Insert or update a neighbor entry
    nb = nb_find(ip_src, ifindex);
    if (!nb)
        nb_insert(ip_src, ifindex);
    else
        nb_update(nb);
#ifdef OMNETPP
    delete hello;
    hello=NULL;
#endif
}

long NS_CLASS hello_jitter(void)
{
    long jitter;
#ifdef NS_PORT
#ifndef OMNETPP
    jitter = (long) (Random::uniform() * 0.1 * hello_ival * 1000);
    if (Random::uniform() > 0.5)
        return jitter;
    return -jitter;
#else
    jitter = (long) (uniform(0,1) * 0.1 * hello_ival * 1000);
    if (uniform(0,1) > 0.5)
        return jitter;
    return -jitter;
#endif
#else
    jitter = (long) (((float) random() / (float) RAND_MAX) * 0.1 * hello_ival * 1000);
    if ((float) random() / (float) RAND_MAX > 0.5)
        return jitter;
    return -jitter;
#endif
}

