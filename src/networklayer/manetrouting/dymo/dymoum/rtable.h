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

#ifndef __RTABLE_H__
#define __RTABLE_H__

#ifndef NS_NO_GLOBALS

#include "defs_dymo.h"
#include "dlist.h"
#include "timer_queue.h"

#ifndef OMNETPP
#include <sys/types.h>
#include <netinet/in.h>
#else
#include "compatibility.h"
#endif

#ifndef OMNETPP
#define ROUTE_TIMEOUT       3000
#define ROUTE_DELETE_TIMEOUT    (5 * ROUTE_TIMEOUT)
#endif

#define RT_INVALID  0
#define RT_VALID    1

/* Routing table entry type */
typedef struct rt_entry
{
    dlist_head_t    l;
    struct in_addr  rt_dest_addr;
    struct in_addr  rt_nxthop_addr;
    struct timer    rt_deltimer;
    struct timer    rt_validtimer;
    u_int32_t   rt_ifindex;
    u_int32_t   rt_seqnum;
    u_int8_t    rt_prefix;
    u_int8_t    rt_hopcnt;
    u_int8_t    rt_is_gw;
    u_int8_t    rt_is_used : 4;
    u_int8_t    rt_state : 4;
    uint32_t    cost; // without the last node
    uint8_t     rt_hopfix;
} rtable_entry_t;

#endif  /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

/* Routing table */
rtable_entry_t rtable;

/* Initialize routing table */
void rtable_init();

/* Destroy routing table */
void rtable_destroy();


/* Find an routing entry given the destination address */
rtable_entry_t *rtable_find(struct in_addr dest_addr);

/* Insert a new entry */
rtable_entry_t *rtable_insert(struct in_addr dest_addr,
                              struct in_addr nxthop_addr,
                              u_int32_t ifindex,
                              u_int32_t seqnum,
                              u_int8_t prefix,
                              u_int8_t hopcnt,
                              u_int8_t is_gw,
                              uint32_t cost,uint8_t hopfix);

/* Update an existing entry */
rtable_entry_t *rtable_update(rtable_entry_t *entry,
                              struct in_addr dest_addr,
                              struct in_addr nxthop_addr,
                              u_int32_t  ifindex,
                              u_int32_t seqnum,
                              u_int8_t prefix,
                              u_int8_t hopcnt,
                              u_int8_t is_gw,
                              uint32_t cost, uint8_t hopfix);

/* Delete an entry */
void rtable_delete(rtable_entry_t *entry);

/* Mark an entry as invalid and schedule its deletion */
void rtable_invalidate(rtable_entry_t *entry);

/* Update the timeout of a valid entry */
int rtable_update_timeout(rtable_entry_t *entry);

/* Expire the timeout of a valid entry */
int rtable_expire_timeout(rtable_entry_t *entry);

/* Expire all entries which use the given next hop and interface */
int rtable_expire_timeout_all(struct in_addr nxthop_addr, u_int32_t ifindex);

#endif  /* NS_NO_DECLARATIONS */

#endif  /* __RTABLE_H__ */

