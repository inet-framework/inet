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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/rtnetlink.h>

#include "defs.h"
#include "lnx/kaodv-netlink.h"
#include "debug.h"
#include "aodv_rreq.h"
#include "aodv_timeout.h"
#include "routing_table.h"
#include "aodv_hello.h"
#include "params.h"
#include "aodv_socket.h"
#include "aodv_rerr.h"

/* Implements a Netlink socket communication channel to the kernel. Route
 * information and refresh messages are passed. */

struct nlsock {
	int sock;
	int seq;
	struct sockaddr_nl local;
};

struct sockaddr_nl peer = { AF_NETLINK, 0, 0, 0 };

struct nlsock aodvnl;
struct nlsock rtnl;

static void nl_kaodv_callback(int sock);
static void nl_rt_callback(int sock);

extern int llfeedback, active_route_timeout, qual_threshold, internet_gw_mode,
    wait_on_reboot;
extern struct timer worb_timer;

#define BUFLEN 256

/* #define DEBUG_NETLINK */

void nl_init(void)
{
	int status;
	unsigned int addrlen;

	memset(&peer, 0, sizeof(struct sockaddr_nl));
	peer.nl_family = AF_NETLINK;
	peer.nl_pid = 0;
	peer.nl_groups = 0;

	memset(&aodvnl, 0, sizeof(struct nlsock));
	aodvnl.seq = 0;
	aodvnl.local.nl_family = AF_NETLINK;
	aodvnl.local.nl_groups = AODVGRP_NOTIFY;
	aodvnl.local.nl_pid = getpid();

	/* This is the AODV specific socket to communicate with the
	   AODV kernel module */
	aodvnl.sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_AODV);

	if (aodvnl.sock < 0) {
		perror("Unable to create AODV netlink socket");
		exit(-1);
	}


	status = bind(aodvnl.sock, (struct sockaddr *) &aodvnl.local,
		      sizeof(aodvnl.local));

	if (status == -1) {
		perror("Bind for AODV netlink socket failed");
		exit(-1);
	}

	addrlen = sizeof(aodvnl.local);

	if (getsockname
	    (aodvnl.sock, (struct sockaddr *) &aodvnl.local, &addrlen) < 0) {
		perror("Getsockname failed ");
		exit(-1);
	}

	if (attach_callback_func(aodvnl.sock, nl_kaodv_callback) < 0) {
		alog(LOG_ERR, 0, __FUNCTION__, "Could not attach callback.");
	}
	/* This socket is the generic routing socket for adding and
	   removing kernel routing table entries */

	memset(&rtnl, 0, sizeof(struct nlsock));
	rtnl.seq = 0;
	rtnl.local.nl_family = AF_NETLINK;
	rtnl.local.nl_groups =
	    RTMGRP_NOTIFY | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;
	rtnl.local.nl_pid = getpid();

	rtnl.sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	if (rtnl.sock < 0) {
		perror("Unable to create RT netlink socket");
		exit(-1);
	}

	addrlen = sizeof(rtnl.local);

	status = bind(rtnl.sock, (struct sockaddr *) &rtnl.local, addrlen);

	if (status == -1) {
		perror("Bind for RT netlink socket failed");
		exit(-1);
	}

	if (getsockname(rtnl.sock, (struct sockaddr *) &rtnl.local, &addrlen) <
	    0) {
		perror("Getsockname failed ");
		exit(-1);
	}

	if (attach_callback_func(rtnl.sock, nl_rt_callback) < 0) {
		alog(LOG_ERR, 0, __FUNCTION__, "Could not attach callback.");
	}
}

void nl_cleanup(void)
{
	close(aodvnl.sock);
	close(rtnl.sock);
}


static void nl_kaodv_callback(int sock)
{
	int len;
	socklen_t addrlen;
	struct nlmsghdr *nlm;
	struct nlmsgerr *nlmerr;
	char buf[BUFLEN];
	struct in_addr dest_addr, src_addr;
	kaodv_rt_msg_t *m;
	rt_table_t *rt, *fwd_rt, *rev_rt = NULL;

	addrlen = sizeof(struct sockaddr_nl);


	len =
	    recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &peer, &addrlen);

	if (len <= 0)
		return;

	nlm = (struct nlmsghdr *) buf;

	switch (nlm->nlmsg_type) {
	case NLMSG_ERROR:
		nlmerr = NLMSG_DATA(nlm);
		if (nlmerr->error == 0) {
		/* 	DEBUG(LOG_DEBUG, 0, "NLMSG_ACK"); */
		} else {
			DEBUG(LOG_DEBUG, 0, "NLMSG_ERROR, error=%d type=%s",
			      nlmerr->error, 
			      kaodv_msg_type_to_str(nlmerr->msg.nlmsg_type));
		}
		break;

	case KAODVM_DEBUG:
		DEBUG(LOG_DEBUG, 0, "kaodv: %s", NLMSG_DATA(nlm));
		break;
       	case KAODVM_TIMEOUT:
		m = NLMSG_DATA(nlm);
		dest_addr.s_addr = m->dst;

		DEBUG(LOG_DEBUG, 0,
		      "Got TIMEOUT msg from kernel for %s",
		      ip_to_str(dest_addr));

		rt = rt_table_find(dest_addr);

		if (rt && rt->state == VALID)
			route_expire_timeout(rt);
		else
			DEBUG(LOG_DEBUG, 0,
			      "Got rt timeoute event but there is no route");
		break;
	case KAODVM_ROUTE_REQ:
		m = NLMSG_DATA(nlm);
		dest_addr.s_addr = m->dst;

		DEBUG(LOG_DEBUG, 0, "Got ROUTE_REQ: %s from kernel",
		      ip_to_str(dest_addr));

		rreq_route_discovery(dest_addr, 0, NULL);
		break;
	case KAODVM_REPAIR:
		m = NLMSG_DATA(nlm);
		dest_addr.s_addr = m->dst;
		src_addr.s_addr = m->src;

		DEBUG(LOG_DEBUG, 0, "Got REPAIR from kernel for %s",
		      ip_to_str(dest_addr));

		fwd_rt = rt_table_find(dest_addr);

		if (fwd_rt)
			rreq_local_repair(fwd_rt, src_addr, NULL);

		break;
	case KAODVM_ROUTE_UPDATE:
		m = NLMSG_DATA(nlm);

		
		dest_addr.s_addr = m->dst;
		src_addr.s_addr = m->src;

		//	DEBUG(LOG_DEBUG, 0, "ROute update s=%s d=%s", ip_to_str(src_addr), ip_to_str(dest_addr));
		if (dest_addr.s_addr == AODV_BROADCAST ||
		    dest_addr.s_addr ==
		    DEV_IFINDEX(m->ifindex).broadcast.s_addr)
			return;

		fwd_rt = rt_table_find(dest_addr);
		rev_rt = rt_table_find(src_addr);

		rt_table_update_route_timeouts(fwd_rt, rev_rt);

		break;
	case KAODVM_SEND_RERR:
		m = NLMSG_DATA(nlm);
		dest_addr.s_addr = m->dst;
		src_addr.s_addr = m->src;

		if (dest_addr.s_addr == AODV_BROADCAST ||
		    dest_addr.s_addr ==
		    DEV_IFINDEX(m->ifindex).broadcast.s_addr)
			return;

		fwd_rt = rt_table_find(dest_addr);
		rev_rt = rt_table_find(src_addr);

		do {
			struct in_addr rerr_dest;
			RERR *rerr;

			DEBUG(LOG_DEBUG, 0,
			      "Sending RERR for unsolicited message from %s to dest %s",
			      ip_to_str(src_addr), ip_to_str(dest_addr));

			if (fwd_rt) {
				rerr = rerr_create(0, fwd_rt->dest_addr,
						   fwd_rt->dest_seqno);

				rt_table_update_timeout(fwd_rt, DELETE_PERIOD);
			} else
				rerr = rerr_create(0, dest_addr, 0);

			/* Unicast the RERR to the source of the data transmission
			 * if possible, otherwise we broadcast it. */

			if (rev_rt && rev_rt->state == VALID)
				rerr_dest = rev_rt->next_hop;
			else
				rerr_dest.s_addr = AODV_BROADCAST;

			aodv_socket_send((AODV_msg *) rerr, rerr_dest,
					 RERR_CALC_SIZE(rerr), 1,
					 &DEV_IFINDEX(m->ifindex));

			if (wait_on_reboot) {
				DEBUG(LOG_DEBUG, 0,
				      "Wait on reboot timer reset.");
				timer_set_timeout(&worb_timer, DELETE_PERIOD);
			}
		} while (0);
		break;
	default:
		DEBUG(LOG_DEBUG, 0, "Got mesg type=%d\n", nlm->nlmsg_type);
	}

}
static void nl_rt_callback(int sock)
{
	int len, attrlen;
	socklen_t addrlen;
	struct nlmsghdr *nlm;
	struct nlmsgerr *nlmerr;
	char buf[BUFLEN];
	struct ifaddrmsg *ifm;
	struct rtattr *rta;

	addrlen = sizeof(struct sockaddr_nl);

	len =
	    recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &peer, &addrlen);

	if (len <= 0)
		return;

	nlm = (struct nlmsghdr *) buf;

	switch (nlm->nlmsg_type) {
	case NLMSG_ERROR:
		nlmerr = NLMSG_DATA(nlm);
		if (nlmerr->error == 0) {
		/* 	DEBUG(LOG_DEBUG, 0, "NLMSG_ACK"); */
		} else {
			DEBUG(LOG_DEBUG, 0, "NLMSG_ERROR, error=%d type=%d",
			      nlmerr->error, nlmerr->msg.nlmsg_type);
		}
		break;
	case RTM_NEWLINK:
		DEBUG(LOG_DEBUG, 0, "RTM_NEWADDR");
		break;
	case RTM_NEWADDR:
		ifm = NLMSG_DATA(nlm);

		rta = (struct rtattr *) ((char *) ifm + sizeof(ifm));

		attrlen = nlm->nlmsg_len -
		    sizeof(struct nlmsghdr) - sizeof(struct ifaddrmsg);

		for (; RTA_OK(rta, attrlen); rta = RTA_NEXT(rta, attrlen)) {

			if (rta->rta_type == IFA_ADDRESS) {
				struct in_addr ifaddr;

				memcpy(&ifaddr, RTA_DATA(rta),
				       RTA_PAYLOAD(rta));

				DEBUG(LOG_DEBUG, 0,
				      "Interface index %d changed address to %s",
				      ifm->ifa_index, ip_to_str(ifaddr));
			}
		}
		break;
	case RTM_NEWROUTE:
		/* DEBUG(LOG_DEBUG, 0, "RTM_NEWROUTE"); */
		break;
	}
	return;
}

int prefix_length(int family, void *nm)
{
	int prefix = 0;

	if (family == AF_INET) {
		unsigned int tmp;
		memcpy(&tmp, nm, sizeof(unsigned int));

		while (tmp) {
			tmp = tmp << 1;
			prefix++;
		}
		return prefix;

	} else {
		DEBUG(LOG_DEBUG, 0, "Unsupported address family");
	}

	return 0;
}

/* Utility function  comes from iproute2. 
   Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru> */
int addattr(struct nlmsghdr *n, int type, void *data, int alen)
{
	struct rtattr *attr;
	int len = RTA_LENGTH(alen);

	attr = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
	attr->rta_type = type;
	attr->rta_len = len;
	memcpy(RTA_DATA(attr), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}


#define ATTR_BUFLEN 512

int nl_send(struct nlsock *nl, struct nlmsghdr *n)
{
	int res;
	struct iovec iov = { (void *) n, n->nlmsg_len };
	struct msghdr msg =
	    { (void *) &peer, sizeof(peer), &iov, 1, NULL, 0, 0 };
	// int flags = 0;

	if (!nl)
		return -1;

	n->nlmsg_seq = ++nl->seq;
	n->nlmsg_pid = nl->local.nl_pid;

	/* Request an acknowledgement by setting NLM_F_ACK */
	n->nlmsg_flags |= NLM_F_ACK;

	/* Send message to netlink interface. */
	res = sendmsg(nl->sock, &msg, 0);

	if (res < 0) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/* Function to add, remove and update entries in the kernel routing
 * table */
int nl_kern_route(int action, int flags, int family,
		  int index, struct in_addr *dst, struct in_addr *gw,
		  struct in_addr *nm, int metric)
{
	struct {
		struct nlmsghdr nlh;
		struct rtmsg rtm;
		char attrbuf[1024];
	} req;

	if (!dst || !gw)
		return -1;

	req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.nlh.nlmsg_type = action;
	req.nlh.nlmsg_flags = NLM_F_REQUEST | flags;
	req.nlh.nlmsg_pid = 0;

	req.rtm.rtm_family = family;

	if (!nm)
		req.rtm.rtm_dst_len = sizeof(struct in_addr) * 8;
	else
		req.rtm.rtm_dst_len = prefix_length(AF_INET, nm);

	req.rtm.rtm_src_len = 0;
	req.rtm.rtm_tos = 0;
	req.rtm.rtm_table = RT_TABLE_MAIN;
	req.rtm.rtm_protocol = 100;
	req.rtm.rtm_scope = RT_SCOPE_LINK;
	req.rtm.rtm_type = RTN_UNICAST;
	req.rtm.rtm_flags = 0;

	addattr(&req.nlh, RTA_DST, dst, sizeof(struct in_addr));

	if (memcmp(dst, gw, sizeof(struct in_addr)) != 0) {
		req.rtm.rtm_scope = RT_SCOPE_UNIVERSE;
		addattr(&req.nlh, RTA_GATEWAY, gw, sizeof(struct in_addr));
	}

	if (index > 0)
		addattr(&req.nlh, RTA_OIF, &index, sizeof(index));

	addattr(&req.nlh, RTA_PRIORITY, &metric, sizeof(metric));

	return nl_send(&rtnl, &req.nlh);
}

int nl_send_add_route_msg(struct in_addr dest, struct in_addr next_hop,
			  int metric, u_int32_t lifetime, int rt_flags,
			  int ifindex)
{
	struct {
		struct nlmsghdr n;
		struct kaodv_rt_msg m;
	} areq;

	DEBUG(LOG_DEBUG, 0, "ADD/UPDATE: %s:%s ifindex=%d",
	      ip_to_str(dest), ip_to_str(next_hop), ifindex);

	memset(&areq, 0, sizeof(areq));

	areq.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct kaodv_rt_msg));
	areq.n.nlmsg_type = KAODVM_ADDROUTE;
	areq.n.nlmsg_flags = NLM_F_REQUEST;

	areq.m.dst = dest.s_addr;
	areq.m.nhop = next_hop.s_addr;
	areq.m.time = lifetime;
	areq.m.ifindex = ifindex;

	if (rt_flags & RT_INET_DEST) {
		areq.m.flags |= KAODV_RT_GW_ENCAP;
	}

	if (rt_flags & RT_REPAIR)
		areq.m.flags |= KAODV_RT_REPAIR;

	if (nl_send(&aodvnl, &areq.n) < 0) {
		DEBUG(LOG_DEBUG, 0, "Failed to send netlink message");
		return -1;
	}
#ifdef DEBUG_NETLINK
	DEBUG(LOG_DEBUG, 0, "Sending add route");
#endif
	return nl_kern_route(RTM_NEWROUTE, NLM_F_CREATE,
			     AF_INET, ifindex, &dest, &next_hop, NULL, metric);
}

int nl_send_no_route_found_msg(struct in_addr dest)
{
	struct {
		struct nlmsghdr n;
		kaodv_rt_msg_t m;
	} areq;

	memset(&areq, 0, sizeof(areq));

	areq.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct kaodv_rt_msg));
	areq.n.nlmsg_type = KAODVM_NOROUTE_FOUND;
	areq.n.nlmsg_flags = NLM_F_REQUEST;

	areq.m.dst = dest.s_addr;

	DEBUG(LOG_DEBUG, 0, "Send NOROUTE_FOUND to kernel: %s",
	      ip_to_str(dest));

	return nl_send(&aodvnl, &areq.n);
}

int nl_send_del_route_msg(struct in_addr dest, struct in_addr next_hop, int metric)
{
	int index = -1;
	struct {
		struct nlmsghdr n;
		struct kaodv_rt_msg m;
	} areq;

	DEBUG(LOG_DEBUG, 0, "Send DEL_ROUTE to kernel: %s", ip_to_str(dest));

	memset(&areq, 0, sizeof(areq));

	areq.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct kaodv_rt_msg));
	areq.n.nlmsg_type = KAODVM_DELROUTE;
	areq.n.nlmsg_flags = NLM_F_REQUEST;

	areq.m.dst = dest.s_addr;
	areq.m.nhop = next_hop.s_addr;
	areq.m.time = 0;
	areq.m.flags = 0;

	if (nl_send(&aodvnl, &areq.n) < 0) {
		DEBUG(LOG_DEBUG, 0, "Failed to send netlink message");
		return -1;
	}
#ifdef DEBUG_NETLINK
	DEBUG(LOG_DEBUG, 0, "Sending del route");
#endif
	return nl_kern_route(RTM_DELROUTE, 0, AF_INET, index, &dest, &next_hop,
			     NULL, metric);
}

int nl_send_conf_msg(void)
{
	struct {
		struct nlmsghdr n;
		kaodv_conf_msg_t cm;
	} areq;

	memset(&areq, 0, sizeof(areq));

	areq.n.nlmsg_len = NLMSG_LENGTH(sizeof(kaodv_conf_msg_t));
	areq.n.nlmsg_type = KAODVM_CONFIG;
	areq.n.nlmsg_flags = NLM_F_REQUEST;

	areq.cm.qual_th = qual_threshold;
	areq.cm.active_route_timeout = active_route_timeout;
	areq.cm.is_gateway = internet_gw_mode;

#ifdef DEBUG_NETLINK
	DEBUG(LOG_DEBUG, 0, "Sending aodv conf msg");
#endif
	return nl_send(&aodvnl, &areq.n);
}
