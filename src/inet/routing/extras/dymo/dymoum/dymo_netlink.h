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

#ifndef __DYMO_NETLINK_H__
#define __DYMO_NETLINK_H__

#include <netinet/in.h>

namespace inet {

namespace inetmanet {

/* Here we implement communications from user space to kernel space via
   netlink sockets */

/* Set up netlink socket */
void netlink_init(void);

/* Close netlink socket */
void netlink_fini(void);

/* Send a message to kernel space to inform that a new routing entry must be
   added */
void netlink_add_route(struct in_addr addr);

/* Send a message to kernel space to inform that a routing entry must be
   deleted */
void netlink_del_route(struct in_addr addr);

/* Send a message to kernel space to inform that a route discovery failed */
void netlink_no_route_found(struct in_addr addr);

} // namespace inetmanet

} // namespace inet

#endif  /* __DYMO_NETLINK_H__ */

