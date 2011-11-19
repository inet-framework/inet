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
 *
 *****************************************************************************/
#ifndef _PARAMS_H
#define _PARAMS_H

#include "defs_aodv.h"

#define K_PARAM   5

/* Dynamic configuration values. Default numbers are for HELLO messages. */
#define ACTIVE_ROUTE_TIMEOUT active_route_timeout
#define TTL_START ttl_start
#define DELETE_PERIOD delete_period

/* Settings for Link Layer Feedback */
#define ACTIVE_ROUTE_TIMEOUT_LLF    10000
#define TTL_START_LLF               1
#define DELETE_PERIOD_LLF           ACTIVE_ROUTE_TIMEOUT_LLF

/* Settings for HELLO messages */
#define ACTIVE_ROUTE_TIMEOUT_HELLO  3000
#define TTL_START_HELLO             2
#define DELETE_PERIOD_HELLO         K_PARAM * max(ACTIVE_ROUTE_TIMEOUT_HELLO, ALLOWED_HELLO_LOSS * HELLO_INTERVAL)

/* Non runtime modifiable settings */
#define ALLOWED_HELLO_LOSS      2
/* If expanding ring search is used, BLACKLIST_TIMEOUT should be?: */
#define BLACKLIST_TIMEOUT       RREQ_RETRIES * NET_TRAVERSAL_TIME + (TTL_THRESHOLD - TTL_START)/TTL_INCREMENT + 1 + RREQ_RETRIES
#define HELLO_INTERVAL          1000
#define LOCAL_ADD_TTL           2
#define MAX_REPAIR_TTL          3 * NET_DIAMETER / 10
#define MY_ROUTE_TIMEOUT        2 * ACTIVE_ROUTE_TIMEOUT
#define NET_DIAMETER            35
#define NET_TRAVERSAL_TIME      2 * NODE_TRAVERSAL_TIME * NET_DIAMETER
#define NEXT_HOP_WAIT           NODE_TRAVERSAL_TIME + 10
#define NODE_TRAVERSAL_TIME     40
#define PATH_DISCOVERY_TIME     2 * NET_TRAVERSAL_TIME
#define RERR_RATELIMIT          10
#define RING_TRAVERSAL_TIME     2 * NODE_TRAVERSAL_TIME * (TTL_VALUE + TIMEOUT_BUFFER)
#define RREQ_RETRIES            2
#define RREQ_RATELIMIT          10
#define TIMEOUT_BUFFER          2
#define TTL_INCREMENT           2
#define TTL_THRESHOLD           7

#ifndef NS_PORT
/* Dynamic configuration values */
extern int active_route_timeout;
extern int ttl_start;
extern int delete_period;
#endif

#endif              /* _PARAMS_H */
