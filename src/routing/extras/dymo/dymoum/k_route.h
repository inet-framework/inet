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

#ifndef __K_ROUTE_H__
#define __K_ROUTE_H__

#include <netinet/in.h>

namespace inet {

namespace inetmanet {

/* Add a new route in the kernel routing table */
int k_add_rte(struct in_addr dest_addr,
              struct in_addr nxthop_addr,
              struct in_addr netmask,
              u_int8_t hopcnt,
              u_int32_t ifindex);

/* Change a existing route in the kernel routing table */
int k_chg_rte(struct in_addr dest_addr,
              struct in_addr nxthop_addr,
              struct in_addr netmask,
              u_int8_t hopcnt,
              u_int32_t ifindex);

/* Delete a route in the kernel routing table */
int k_del_rte(struct in_addr dest_addr);

} // namespace inetmanet

} // namespace inet

#endif  /* __K_ROUTE_H__ */

