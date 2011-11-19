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

#include "k_route.h"
#include "defs.h"

#include <netinet/in.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"


int k_add_rte(struct in_addr dest_addr,
		struct in_addr nxthop_addr,
		struct in_addr netmask,
		u_int8_t hopcnt,
		u_int32_t ifindex)
{
	int sock, ret;
	unsigned short int flags;
	struct rtentry entry;
	struct sockaddr_in dest_saddr, nxthop_saddr, nm_saddr;
	
	if (dest_addr.s_addr == nxthop_addr.s_addr)
	{
		nxthop_addr.s_addr = 0;
		flags = RTF_HOST | RTF_UP | RTF_DYNAMIC;
	}
	else
		flags = RTF_HOST | RTF_GATEWAY | RTF_UP | RTF_DYNAMIC;
	
	dest_saddr.sin_family	= AF_INET;
	nxthop_saddr.sin_family	= AF_INET;
	nm_saddr.sin_family	= AF_INET;
	
	dest_saddr.sin_addr.s_addr	= dest_addr.s_addr;
	nxthop_saddr.sin_addr.s_addr	= nxthop_addr.s_addr;
	nm_saddr.sin_addr.s_addr	= netmask.s_addr;
	
	memset(&entry, 0, sizeof(struct rtentry));
	memcpy(&entry.rt_dst, &dest_saddr, sizeof(struct sockaddr_in));
	memcpy(&entry.rt_gateway, &nxthop_saddr, sizeof(struct sockaddr_in));
	memcpy(&entry.rt_genmask, &nm_saddr, sizeof(struct sockaddr_in));
	entry.rt_flags	= flags;
	entry.rt_metric	= hopcnt + 1;
	entry.rt_dev	= DEV_IFINDEX(ifindex).ifname;
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		dlog(LOG_WARNING, errno, __FUNCTION__, "socket() failed, "
			"couldn't create route to %s",
			ip2str(dest_addr.s_addr));
		return sock;
	}
	
	if ((ret = ioctl(sock, SIOCADDRT, &entry)) < 0)
	{
		close(sock);
		dlog(LOG_WARNING, errno, __FUNCTION__, "ioctl() failed, "
			"couldn't create route to %s",
			ip2str(dest_addr.s_addr));
		return ret;
	}
	
	close(sock);
	
	return 0;
}

int k_chg_rte(struct in_addr dest_addr,
		struct in_addr nxthop_addr,
		struct in_addr netmask,
		u_int8_t hopcnt,
		u_int32_t ifindex)
{
	int ret;
	
	if ((ret = k_del_rte(dest_addr)) < 0)
		return ret;
	
	if ((ret = k_add_rte(dest_addr, nxthop_addr, netmask, hopcnt, ifindex)) < 0)
		return ret;
	
	return 0;
}

int k_del_rte(struct in_addr dest_addr)
{
	int sock, ret;
	struct rtentry entry;
	struct sockaddr_in dest_saddr, nxthop_saddr, nm_saddr;
	
	dest_saddr.sin_family	= AF_INET;
	nxthop_saddr.sin_family	= AF_INET;
	nm_saddr.sin_family	= AF_INET;
	
	dest_saddr.sin_addr.s_addr	= dest_addr.s_addr;
	nxthop_saddr.sin_addr.s_addr	= 0;
	nm_saddr.sin_addr.s_addr	= 0;
	
	memset(&entry, 0, sizeof(struct rtentry));
	memcpy(&entry.rt_dst, &dest_saddr, sizeof(struct sockaddr_in));
	memcpy(&entry.rt_gateway, &nxthop_saddr, sizeof(struct sockaddr_in));
	memcpy(&entry.rt_genmask, &nm_saddr, sizeof(struct sockaddr_in));
	entry.rt_flags	= RTF_HOST;
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		dlog(LOG_WARNING, errno, __FUNCTION__, "socket() failed, "
			"couldn't delete route to %s",
			ip2str(dest_addr.s_addr));
		return sock;
	}
	
	if ((ret = ioctl(sock, SIOCDELRT, &entry)) < 0)
	{
		close(sock);
		dlog(LOG_WARNING, errno, __FUNCTION__, "ioctl() failed, "
			"couldn't delete route to %s",
			ip2str(dest_addr.s_addr));
		return ret;
	}
	
	close(sock);
	
	return 0;
}
