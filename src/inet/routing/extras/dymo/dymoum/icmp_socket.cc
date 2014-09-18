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
#define NS_PORT
#define OMNETPP
#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/dymo_um.h"
#else
#include "../dymo_um_omnet.h"
#endif
#else
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"
#include "inet/routing/extras/dymo/dymoum/icmp_socket.h"
#include "inet/routing/extras/dymo/dymoum/blacklist.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>


/* Sending and receiving buffers */
static char icmp_send_buf[ICMP_SEND_BUF_SIZE];
static char icmp_recv_buf[ICMP_RECV_BUF_SIZE];

static void icmp_socket_read(int fd);

#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

void NS_CLASS icmp_socket_init(void)
{
#ifndef NS_PORT
    char ifname[IFNAMSIZ];
    struct icmp *icmp;
    int i;
    int tos = IPTOS_LOWDELAY;

    // Check if there are no interfaces
    if (this_host.nif == 0)
    {
        dlog(LOG_ERR, 0, __FUNCTION__, "No interfaces configured\n");
        exit(EXIT_FAILURE);
    }

    // For each interface...
    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
    {
        if (!DEV_NR(i).enabled)
            continue;

        // Create a raw socket
        if ((DEV_NR(i).icmp_sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        {
            dlog(LOG_ERR, errno, __FUNCTION__, "getSocket() failed");
            exit(EXIT_FAILURE);
        }

        // Make the socket only process packets received from an interface
        strncpy(ifname, DEV_NR(i).ifname, sizeof(ifname));

        if (setsockopt(DEV_NR(i).icmp_sock,
                       SOL_SOCKET,
                       SO_BINDTODEVICE,
                       &ifname,
                       sizeof(ifname)) < 0)
        {
            dlog(LOG_ERR, errno, __FUNCTION__, "setsockopt(BINDTODEVICE)"
                 " failed for %s", DEV_NR(i).ifname);
            exit(EXIT_FAILURE);
        }

        // Set priority of IP datagrams
        if (setsockopt(DEV_NR(i).icmp_sock,
                       SOL_SOCKET,
                       SO_PRIORITY,
                       &tos,
                       sizeof(int)) < 0)
        {
            dlog(LOG_ERR, errno, __FUNCTION__, "setsockopt(PRIORITY)"
                 " failed for LOWDELAY");
            exit(EXIT_FAILURE);
        }

        // Attach callback function
        if (attach_callback_func(DEV_NR(i).icmp_sock, icmp_socket_read) < 0)
        {
            dlog(LOG_ERR, 0, __FUNCTION__, "could not register input handler");
            exit(EXIT_FAILURE);
        }
    }

    // Prepare the send buffer
    memset(icmp_send_buf, '\0', ICMP_SEND_BUF_SIZE);
    icmp            = (struct icmp *) icmp_send_buf;
    icmp->icmp_type     = ICMP_ECHOREPLY;
    icmp->icmp_code     = 0;
    icmp->icmp_cksum    = in_cksum((u_short *) icmp, ICMP_ECHOREPLY_SIZE);
    icmp->icmp_id       = 0;
    icmp->icmp_seq      = 0;

#endif  /* NS_PORT */
}

void NS_CLASS icmp_socket_fini(void)
{
#ifndef NS_PORT
    int i;

    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
    {
        if (!DEV_NR(i).enabled)
            continue;
        close(DEV_NR(i).icmp_sock);
    }
#endif  /* NS_PORT */
}

u_short NS_CLASS in_cksum(u_short *icmp, int len)
{
    int nleft   = len;
    u_short *w  = icmp;
    int sum     = 0;
    u_short cksum;

    while (nleft > 1)
    {
        sum += *w++;
        nleft   -= 2;
    }

    if (nleft == 1)
    {
        u_short u = 0;

        *(u_char *) (&u) = *(u_char *) w;
        sum += u;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    cksum   = ~sum;

    return cksum;
}

void NS_CLASS icmp_reply_send(struct in_addr dest_addr, struct dev_info *dev)
{
#ifndef OMNETPP
#ifndef NS_PORT
    struct sockaddr_in icmp_sockaddr;
    u_int8_t ttl;

    icmp_sockaddr.sin_family    = AF_INET;
    icmp_sockaddr.sin_addr      = dest_addr;
    icmp_sockaddr.sin_port      = 0;

    // Set TTL
    ttl = 1;
    if (setsockopt(dev->icmp_sock,
                   SOL_IP,
                   IP_TTL,
                   &ttl,
                   sizeof(ttl)) < 0)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "setsockopt(IP_TTL) failed");
        exit(EXIT_FAILURE);
    }

    if (sendto(dev->icmp_sock, icmp_send_buf, ICMP_ECHOREPLY_SIZE, 0,
               (struct sockaddr *) &icmp_sockaddr, sizeof(icmp_sockaddr)) < 0)
    {
        dlog(LOG_WARNING, errno, __FUNCTION__,
             "failed send ICMP ECHOREPLY to %s",
             ip2str(dest_addr.s_addr));
    }
#else
    Packet *p       = allocpkt();
    struct hdr_cmn *ch  = HDR_CMN(p);
    struct hdr_ip *ih   = HDR_IP(p);
    hdr_dymoum *dh      = HDR_DYMOUM(p);

    memset(dh, '\0', DYMO_MSG_MAX_SIZE);
    dh->type    = DYMO_ECHOREPLY_TYPE;
    dh->len     = ICMP_ECHOREPLY_SIZE;
    dh->ttl     = 1;

    ch->ptype() = PT_DYMOUM;
    ch->direction() = hdr_cmn::DOWN;
    ch->size()  = IP_HDR_LEN + ICMP_ECHOREPLY_SIZE;
    ch->getBitErrorRate()   = 0;
    ch->next_hop_   = (nsaddr_t) dest_addr.s_addr;
    ch->prev_hop_   = (nsaddr_t) dev->ipaddr.s_addr;
    ch->addr_type() = NS_AF_INET;

    ih->saddr() = (nsaddr_t) dev->ipaddr.s_addr;
    ih->daddr() = (nsaddr_t) dest_addr.s_addr;
    ih->sport() = RT_PORT;
    ih->dport() = RT_PORT;
    ih->ttl()   = DYMO_IPTTL;

    Scheduler::getInstance().schedule(target_, p, 0.0);

#endif  /* NS_PORT */

    dlog(LOG_DEBUG, 0, __FUNCTION__, "sending ICMP msg to %s",
         ip2str(dest_addr.s_addr));
#endif
}


#ifdef NS_PORT
void NS_CLASS icmp_process(struct in_addr ip_src)
{
    blacklist_remove(blacklist_find(ip_src));
}
#else
static void icmp_socket_read(int fd)
{
    int i, len;
    struct ip *ip;
    struct dev_info *dev;
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // Receive message
    if ((len = recvfrom(fd, icmp_recv_buf, ICMP_RECV_BUF_SIZE, 0,
                        (struct sockaddr *) &sender_addr, &addr_len)) < 0)
    {
        dlog(LOG_WARNING, errno, __FUNCTION__, "could not receive message");
        return;
    }

    // Ignore messages generated locally
    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
        if (DEV_NR(i).enabled && DEV_NR(i).ipaddr.s_addr ==
                sender_addr.sin_addr.s_addr)
            return;

    dev = devfromicmpsock(fd);
    if (!dev)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__, "could not get device info");
        return;
    }

    /* If this is an ICMP message, remove it from the blacklist.
       NOTE: obviously we can receive ICMP messages which aren't sent from
           a neighbor, but we don't check this because in that case the address
           isn't in the blacklist and therefore we can safely invoke the
           procedures below. */
    ip = (struct ip *) icmp_recv_buf;
    if (ip->ip_p != IPPROTO_ICMP)
        return;

    blacklist_remove(blacklist_find(sender_addr.sin_addr));

    dlog(LOG_DEBUG, 0, __FUNCTION__, "ICMP msg received in %s from %s",
         dev->ifname, ip2str(sender_addr.sin_addr.s_addr));
}
#endif  /* NS_PORT */

} // namespace inetmanet

} // namespace inet

