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
#ifndef _AODV_HELLO_H
#define _AODV_HELLO_H

#ifndef NS_NO_GLOBALS
#include "defs_aodv.h"
#include "aodv_rrep.h"
#include "routing_table.h"
#endif              /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS

#define ROUTE_TIMEOUT_SLACK 100
#define JITTER_INTERVAL 100

void hello_start();
void hello_stop();
void hello_send(void *arg);
void hello_process(RREP * hello, int rreplen, unsigned int ifindex);
void hello_process_non_hello(AODV_msg * aodv_msg, const struct in_addr &source,
                             unsigned int ifindex);
NS_INLINE void hello_update_timeout(rt_table_t * rt, struct timeval *now,
                                    long time);

#ifdef NS_PORT
long hello_jitter();
#endif
#endif              /* NS_NO_DECLARATIONS */

#endif              /* AODV_HELLO_H */
