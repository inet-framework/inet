/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University and Ericsson AB.
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
 * Author: Erik Nordström, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/
#ifndef _NL_H
#define _NL_H

/* #include "routing_table.h" */

void nl_init(void);
void nl_cleanup(void);
int nl_send_add_route_msg(struct in_addr dest, struct in_addr next_hop,
                          int metric, u_int32_t lifetime, int rt_flags,
                          int ifindex);
int nl_send_del_route_msg(struct in_addr dest, struct in_addr next_hop, int metric);

int nl_send_no_route_found_msg(struct in_addr dest);
int nl_send_conf_msg(void);

#endif
