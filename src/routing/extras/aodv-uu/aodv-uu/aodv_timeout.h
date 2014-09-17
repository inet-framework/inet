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
#ifndef _AODV_TIMEOUT_H
#define _AODV_TIMEOUT_H

#ifndef NS_NO_GLOBALS
#include "inet/routing/extras/aodv-uu/aodv-uu/defs_aodv.h"

#endif              /* NS_NO_GLOBALS */

#ifndef NS_NO_DECLARATIONS
void route_delete_timeout(void *arg);
void local_repair_timeout(void *arg);
void route_discovery_timeout(void *arg);
void route_expire_timeout(void *arg);
void hello_timeout(void *arg);
void rrep_ack_timeout(void *arg);
void wait_on_reboot_timeout(void *arg);
void packet_queue_timeout(void *arg);
#endif              /* NS_NO_DECLARATIONS */

#endif              /* AODV_TIMEOUT_H */
