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

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <linux/netlink.h>

#include "dymo_netlink.h"
#include "dymo_generic.h"
#include "dymo_re.h"
#include "dymo_rerr.h"
#include "rtable.h"
#include "timer_queue.h"
#include "defs.h"
#include "debug.h"
#include "lnx/kdymo_netlink.h"

#define BUFLEN	256

static int nlfd;
static struct sockaddr_nl local;
static struct sockaddr_nl peer;

static void netlink_callback(int fd);
static int netlink_send_msg(int type, struct in_addr addr);

void netlink_init(void)
{
	if ((nlfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_DYMO)) < 0)
	{
		dlog(LOG_ERR, errno, __FUNCTION__, "socket() failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&local, 0, sizeof(struct sockaddr_nl));
	local.nl_family	= AF_NETLINK;
	local.nl_pid	= getpid();
	local.nl_groups	= DYMOGRP_NOTIFY;
	
	memset(&peer, 0, sizeof(struct sockaddr_nl));
	peer.nl_family	= AF_NETLINK;
	peer.nl_pid	= 0;
	peer.nl_groups	= 0;
	
	if (bind(nlfd, (struct sockaddr *) &local, sizeof(local)) < 0)
	{
		dlog(LOG_ERR, errno, __FUNCTION__, "bind() failed");
		exit(EXIT_FAILURE);
	}
	
	if (attach_callback_func(nlfd, netlink_callback) < 0)
	{
		dlog(LOG_ERR, 0, __FUNCTION__, "Could not attach netlink"
			" callback");
		exit(EXIT_FAILURE);
	}
}

void netlink_fini(void)
{
	close(nlfd);
}

inline void netlink_add_route(struct in_addr addr)
{
	dlog(LOG_DEBUG, 0, __FUNCTION__, "added route to %s",
		ip2str(addr.s_addr));
	netlink_send_msg(KDYMO_ADDROUTE, addr);
}

inline void netlink_del_route(struct in_addr addr)
{
	dlog(LOG_DEBUG, 0, __FUNCTION__, "deleted route to %s",
		ip2str(addr.s_addr));
	netlink_send_msg(KDYMO_DELROUTE, addr);
}

inline void netlink_no_route_found(struct in_addr addr)
{
	dlog(LOG_DEBUG, 0, __FUNCTION__, "no route found to %s",
		ip2str(addr.s_addr));
	netlink_send_msg(KDYMO_NOROUTE_FOUND, addr);
}

static int netlink_send_msg(int type, struct in_addr addr)
{
	struct
	{
		struct nlmsghdr nlh;
		struct kdymo_rtmsg rtm;
	} req;
	
	memset(&req, 0, sizeof(req));
	
	req.nlh.nlmsg_len	= NLMSG_LENGTH(sizeof(req));
	req.nlh.nlmsg_flags	= NLM_F_REQUEST;
	req.nlh.nlmsg_type	= type;
	req.nlh.nlmsg_pid	= local.nl_pid;
	
	req.rtm.addr	= addr.s_addr;
	req.rtm.ifindex	= 0;
	
	return sendto(nlfd, &req, req.nlh.nlmsg_len, 0,
		(struct sockaddr *) &peer, sizeof(peer));
}

/* This function processes those messages which are sent from kernel space */
static void netlink_callback(int fd)
{
	socklen_t addrlen;
	char buf[BUFLEN];
	
	addrlen = sizeof(peer);
	if (recvfrom(nlfd, buf, BUFLEN, 0, (struct sockaddr *) &peer,
		&addrlen) > 0)
	{
		int type;
		struct kdymo_rtmsg *msg;
		rtable_entry_t *entry;
		struct in_addr addr;
		
		type = ((struct nlmsghdr *) buf)->nlmsg_type;
		
		switch (type)
		{
		case KDYMO_NOROUTE:
			msg = NLMSG_DATA((struct nlmsghdr *) buf);
			
			addr.s_addr = msg->addr;
			route_discovery(addr);
			break;
			
		case KDYMO_ROUTE_UPDATE:
			msg = NLMSG_DATA((struct nlmsghdr *) buf);
			
			addr.s_addr = msg->addr;
			if (addr.s_addr == DYMO_BROADCAST || addr.s_addr ==
				DEV_IFINDEX(msg->ifindex).bcast.s_addr)
				return;
			
			entry = rtable_find(addr);
			if (entry)
				rtable_update_timeout(entry);
			break;
		
		case KDYMO_SEND_RERR:
			msg = NLMSG_DATA((struct nlmsghdr *) buf);
			
			addr.s_addr = msg->addr;
			if (addr.s_addr == DYMO_BROADCAST || addr.s_addr ==
				DEV_IFINDEX(msg->ifindex).bcast.s_addr)
				return;
			
			entry = rtable_find(addr);
			rerr_send(addr, NET_DIAMETER, entry);
			break;
		
		default:
			dlog(LOG_DEBUG, 0, __FUNCTION__,
				"Got an unknown netlink message: %d\n",
				type);
			break;
		}
	}
}
