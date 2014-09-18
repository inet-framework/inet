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
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *
 *
 *****************************************************************************/
#ifndef _SEEK_LIST_H
#define _SEEK_LIST_H

#ifndef NS_NO_GLOBALS
#include "inet/routing/extras/aodv-uu/aodv-uu/defs_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/timer_queue_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/list.h"

namespace inet {

namespace inetmanet {

#define IP_DATA_MAX_LEN 60 + 8  /* Max IP header + 64 bits of data */

struct ip_data
{
    char data[IP_DATA_MAX_LEN];
    int len;
};

/* This is a list of nodes that route discovery are performed for */
typedef struct seek_list
{
    list_t l;
    struct in_addr dest_addr;
    u_int32_t dest_seqno;
    struct ip_data *ipd;
    u_int8_t flags;     /* The flags we are using for resending the RREQ */
    int reqs;
    int ttl;
    struct timer seek_timer;
} seek_list_t;

} // namespace inetmanet

} // namespace inet

#endif              /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

seek_list_t *seek_list_insert(struct in_addr dest_addr, u_int32_t dest_seqno,
                              int ttl, u_int8_t flags, struct ip_data *ipd);
int seek_list_remove(seek_list_t * entry);
seek_list_t *seek_list_find(struct in_addr dest_addr);

#ifdef NS_PORT
#ifdef SEEK_LIST_DEBUG
void seek_list_print();
#endif
#endif              /* NS_PORT */
#endif              /* NS_NO_DECLARATIONS */


#endif              /* SEEK_LIST_H */
