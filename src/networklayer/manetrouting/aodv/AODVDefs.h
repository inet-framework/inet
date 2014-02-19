//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef AODVDEFS_H_
#define AODVDEFS_H_

#define ACTIVE_ROUTE_TIMEOUT     3
#define ALLOWED_HELLO_LOSS       2
#define BLACKLIST_TIMEOUT        RREQ_RETRIES * NET_TRAVERSAL_TIME
#define DELETE_PERIOD            1 // FIXME: it is just a temporary value
#define HELLO_INTERVAL           1
#define LOCAL_ADD_TTL            2
#define MAX_REPAIR_TTL           0.3 * NET_DIAMETER
// #define MIN_REPAIR_TTL
#define MY_ROUTE_TIMEOUT         2 * ACTIVE_ROUTE_TIMEOUT
#define NET_DIAMETER             35
#define NET_TRAVERSAL_TIME       2 * NODE_TRAVERSAL_TIME * NET_DIAMETER
#define NEXT_HOP_WAIT            NODE_TRAVERSAL_TIME + 10
#define NODE_TRAVERSAL_TIME      0.04
#define PATH_DISCOVERY_TIME      2 * NET_TRAVERSAL_TIME
#define RERR_RATELIMIT           10
#define RING_TRAVERSAL_TIME      2 * NODE_TRAVERSAL_TIME * (TTL_VALUE + TIMEOUT_BUFFER)
#define RREQ_RETRIES             2
#define RREQ_RATELIMIT           10
#define TIMEOUT_BUFFER           2
#define TTL_START                1
#define TTL_INCREMENT            2
#define TTL_THRESHOLD            7
#define TTL_VALUE                1 // FIXME



#endif /* AODVDEFS_H_ */
