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
#include "inet/routing/extras/dymo/dymoum/dymo_socket.h"
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>


#define SO_RECVBUF_SIZE (256 * 1024)

/* Receiving and sending buffers */

#ifndef OMNETPP
static char recv_buf[RECV_BUF_SIZE];
static char send_buf[SEND_BUF_SIZE];
#endif

static void dymo_socket_read(int fd);

#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

void NS_CLASS dymo_socket_init()
{
#ifndef NS_PORT
    struct sockaddr_in dymo_addr;
    char ifname[IFNAMSIZ];
    int i;
    int on          = 1;
    int tos         = IPTOS_LOWDELAY;
    int bufsize     = SO_RECVBUF_SIZE;
    socklen_t bufoptlen = sizeof(bufsize);

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

        // Create an UDP socket
        if ((DEV_NR(i).sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            dlog(LOG_ERR, errno, __FUNCTION__, "getSocket() failed");
            exit(EXIT_FAILURE);
        }

        // Bind to DYMO port number
        memset(&dymo_addr, 0, sizeof(struct sockaddr_in));
        dymo_addr.sin_family        = AF_INET;
        dymo_addr.sin_port      = htons(DYMO_PORT);
        dymo_addr.sin_addr.s_addr   = htonl(INADDR_ANY);

        if (bind(DEV_NR(i).sock,
                 (struct sockaddr *)&dymo_addr,
                 sizeof(struct sockaddr)) < 0)
        {
            dlog(LOG_ERR, errno, __FUNCTION__, "bind() failed");
            exit(EXIT_FAILURE);
        }

        // Enable the datagram socket as a broadcast one
        if (setsockopt(DEV_NR(i).sock,
                       SOL_SOCKET,
                       SO_BROADCAST,
                       &on,
                       sizeof(int)) < 0)
        {
            dlog(LOG_ERR, errno, __FUNCTION__, "setsockopt(BROADCAST) failed");
            exit(EXIT_FAILURE);
        }

        // Make the socket only process packets received from an interface
        strncpy(ifname, DEV_NR(i).ifname, sizeof(ifname));

        if (setsockopt(DEV_NR(i).sock,
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
        if (setsockopt(DEV_NR(i).sock,
                       SOL_SOCKET,
                       SO_PRIORITY,
                       &tos,
                       sizeof(int)) < 0)
        {
            dlog(LOG_ERR, errno, __FUNCTION__, "setsockopt(PRIORITY)"
                 " failed for LOWDELAY");
            exit(EXIT_FAILURE);
        }

        // Set maximum allowable receive buffer size
        for ( ; ; bufsize -= 1024)
        {
            if (setsockopt(DEV_NR(i).sock,
                           SOL_SOCKET,
                           SO_RCVBUF,
                           (char *) &bufsize,
                           bufoptlen) == 0)
            {
                dlog(LOG_NOTICE, 0, __FUNCTION__,
                     "receive buffer size set to %d",
                     bufsize);
                break;
            }
            if (bufsize < RECV_BUF_SIZE)
            {
                dlog(LOG_ERR, 0, __FUNCTION__, "could not set receive buffer size");
                exit(EXIT_FAILURE);
            }

        }

        // Attach callback function
        if (attach_callback_func(DEV_NR(i).sock, dymo_socket_read) < 0)
        {
            dlog(LOG_ERR, 0, __FUNCTION__, "could not register input handler");
            exit(EXIT_FAILURE);
        }
    }
#endif  /* NS_PORT */
    num_dymo_msgs = 0;
}

void NS_CLASS dymo_socket_fini()
{
#ifndef NS_PORT
    int i;

    for (i = 0; i < DYMO_MAX_NR_INTERFACES; i++)
    {
        if (!DEV_NR(i).enabled)
            continue;
        close(DEV_NR(i).sock);
    }
#endif  /* NS_PORT */
}

#ifndef OMNETPP
DYMO_element *NS_CLASS dymo_socket_new_element()
{

    memset(send_buf, '\0', SEND_BUF_SIZE);
    return (DYMO_element *) send_buf;

}
#endif

DYMO_element *NS_CLASS dymo_socket_queue(DYMO_element *e)
{
#ifndef OMNETPP
    memcpy((char *) send_buf, e, e->len);
    return (DYMO_element *) send_buf;
#else
    send_buf = e;
    return send_buf;
#endif
}

void NS_CLASS dymo_socket_send(struct in_addr dest_addr, struct dev_info *dev,double delay)
{
    struct timeval now;
#ifndef NS_PORT
    struct sockaddr_in dest_sockaddr;
    u_int8_t ttl;

    dest_sockaddr.sin_family    = AF_INET;
    dest_sockaddr.sin_addr      = dest_addr;
    dest_sockaddr.sin_port      = htons(DYMO_PORT);

    // Set TTL
    ttl = DYMO_IPTTL;
    if (setsockopt(dev->sock,
                   SOL_IP,
                   IP_TTL,
                   &ttl,
                   sizeof(ttl)) < 0)
    {
        dlog(LOG_ERR, errno, __FUNCTION__, "setsockopt(IP_TTL) failed");
        exit(EXIT_FAILURE);
    }
#else
#ifndef OMNETPP
    int len = ((DYMO_element *) send_buf)->len;

    Packet *p       = allocpkt();
    struct hdr_cmn *ch  = HDR_CMN(p);
    struct hdr_ip *ih   = HDR_IP(p);
    hdr_dymoum *dh      = HDR_DYMOUM(p);

    memset(dh, '\0', DYMO_MSG_MAX_SIZE);
    memcpy(dh, send_buf, len);

    ch->ptype() = PT_DYMOUM;
    ch->direction() = hdr_cmn::DOWN;
    ch->size()  = IP_HDR_LEN + len;
    ch->getBitErrorRate()   = 0;
    ch->next_hop_   = (nsaddr_t) dest_addr.s_addr;
    ch->prev_hop_   = (nsaddr_t) dev->ipaddr.s_addr;
    ch->addr_type() = NS_AF_INET;

    ih->saddr() = (nsaddr_t) dev->ipaddr.s_addr;
    ih->daddr() = (nsaddr_t) dest_addr.s_addr;
    ih->sport() = RT_PORT;
    ih->dport() = RT_PORT;
    ih->ttl()   = DYMO_IPTTL;
#else
    if (send_buf==nullptr)
        return;
    DYMO_element *p = send_buf;
    RE * rePkt=dynamic_cast<RE*>(p);
    send_buf = nullptr;
    totalSend++;
    if (this->isStaticNode())
    {
        p->previousStatic=true;
        if (rePkt)
        {
            for (int i=0; i<rePkt->numBlocks(); i++)
                rePkt->re_blocks[i].cost += costStatic;
        }
    }
    else
    {
        p->previousStatic = false;
        if (rePkt)
        {
            for (int i=0; i<rePkt->numBlocks(); i++)
                rePkt->re_blocks[i].cost += costMobile;
        }
    }

    if (dynamic_cast<RE*>(p))
    {
        RE * re = static_cast<RE*>(p);
        if (isInMacLayer())
        {
            int numBlock = re_numblocks(re);
            re->setByteLength(re->getByteLength()+(numBlock*12)+RE_BASIC_SIZE);
        }
        else
            re->setByteLength(re->getByteLength()+re->len);


        if (re->a)
            totalRreqSend++;
        else
            totalRrepSend++;
    }
    else if (dynamic_cast<RERR *>(p))
    {
        RERR * rerr = static_cast<RERR *>(p);
        totalRerrSend++;
        if (isInMacLayer())
        {
            int numBlock = rerr_numblocks(rerr);
            // The mac layer address has 6 bytes, the ip 4
            rerr->setByteLength(rerr->getByteLength ()+(numBlock*10)+RERR_BASIC_SIZE);
        }
        else
            rerr->setByteLength(rerr->getByteLength ()+ rerr->len);
    }
#endif
#endif  /* NS_PORT */

    // Rate limit stuff
    gettimeofday(&now, nullptr);
    if (num_dymo_msgs == DYMO_RATELIMIT - 1)
    {
        if (timeval_diff(&now, &dymo_rate[0]) < 1000)
        {
            delete p;
            return; // dropping packet
        }
        else
        {
            memmove(dymo_rate,
                    &dymo_rate[1],
                    sizeof(struct timeval) * (num_dymo_msgs - 1));
            dymo_rate[num_dymo_msgs-1] = now;
        }
    }
    else
    {
        dymo_rate[num_dymo_msgs] = now;
        num_dymo_msgs++;
    }

    // Send the queued packet
#ifdef NS_PORT
#ifndef OMNETPP
    /*if (ih->daddr() == IP_BROADCAST)
    Scheduler::getInstance().schedule(target_, p, 0.02 * Random::uniform());
    else*/ // TODO: add jitter for sending broadcast messages?
    Scheduler::getInstance().schedule(target_, p, 0.0);
#else

    L3Address destAdd;
    if (dest_addr.s_addr == L3Address(IPv4Address(DYMO_BROADCAST)))
    {
        destAdd.set(IPv4Address::ALLONES_ADDRESS);
        if (delay>0)
        {
            if (useIndex)
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,delay,dev->ifindex);
            else
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,delay,dev->ipaddr.s_addr);

        }
        else
        {
            if (useIndex)
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,par("broadcastDelay"),dev->ifindex);
            else
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,par("broadcastDelay"),dev->ipaddr.s_addr);
        }
    }
    else
    {

        destAdd = dest_addr.s_addr;
        if (delay>0)
        {
            if (useIndex)
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,delay,dev->ifindex);
            else
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,delay,dev->ipaddr.s_addr);
        }
        else
        {
            if (useIndex)
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,par("unicastDelay"),dev->ifindex);
            else
                sendToIp(p, DYMO_PORT, destAdd, DYMO_PORT,DYMO_IPTTL,par("unicastDelay"),dev->ipaddr.s_addr);
        }
    }
    totalSend++;

#endif
#else
    if (sendto(dev->sock, send_buf, ((DYMO_element *) send_buf)->len,
               0, (struct sockaddr *) &dest_sockaddr, sizeof(dest_sockaddr)) < 0)
    {
        dlog(LOG_WARNING, errno, __FUNCTION__, "failed send to %s", ip2str(dest_addr.s_addr));
        return;
    }
#endif  /* NS_PORT */
}

#ifdef NS_PORT
#ifndef OMNETPP
void NS_CLASS recv_dymoum_pkt(Packet *p)
{
    struct in_addr src_addr;
    struct hdr_cmn *ch  = HDR_CMN(p);
    struct hdr_ip *ih   = HDR_IP(p);
    hdr_dymoum *dh      = HDR_DYMOUM(p);
    DYMO_element *e     = (DYMO_element *) recv_buf;

    assert(ch->ptype() == PT_DYMOUM);
    assert(ch->direction() == hdr_cmn::UP);
    assert(ih->sport() == RT_PORT);
    assert(ih->dport() == RT_PORT);

    src_addr.s_addr = ih->saddr();
    memcpy(recv_buf, dh, RECV_BUF_SIZE);

    Packet::free(p);

    generic_process_message(e, src_addr, NS_IFINDEX);
}
#else
void NS_CLASS recv_dymoum_pkt(DYMO_element *e,const struct in_addr &src_addr,int ifIndex)
{
    generic_process_message(e, src_addr,ifIndex);
}
#endif
#else
static void dymo_socket_read(int fd)
{
    int i, len;
    DYMO_element *msg;
    struct dev_info *dev;
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // Receive message
    if ((len = recvfrom(fd, recv_buf, RECV_BUF_SIZE, 0,
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

    dev = devfromsock(fd);
    if (!dev)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__, "could not get device info");
        return;
    }

    msg = (DYMO_element *) recv_buf;
    if (msg->len != len)
    {
        dlog(LOG_WARNING, 0, __FUNCTION__,
             "actual message length did not match with stated length: %d != %d",
             len, msg->len);
        return;
    }

    generic_process_message(msg, sender_addr.sin_addr, dev->ifindex);
}
#endif  /* NS_PORT */

} // namespace inetmanet

} // namespace inet

