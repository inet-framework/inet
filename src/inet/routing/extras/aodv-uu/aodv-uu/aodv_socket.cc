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
 * Authors: Erik Nordstr�, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/

#define NS_PORT
#define OMNETPP

#include <sys/types.h>

#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/aodv-uu.h"
#else
#include "../aodv_uu_omnet.h"
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/udp.h>
#include "inet/routing/extras/aodv-uu/aodv-uu/aodv_socket.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/timer_queue_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/aodv_rreq.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/aodv_rerr.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/aodv_rrep.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/params.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/aodv_hello.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/aodv_neighbor.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/debug_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/defs_aodv.h"

#endif              /* NS_PORT */

namespace inet {

namespace inetmanet {

#ifndef NS_PORT
#define SO_RECVBUF_SIZE 256*1024

static char recv_buf[RECV_BUF_SIZE];
static char send_buf[SEND_BUF_SIZE];

extern int wait_on_reboot, hello_qual_threshold, ratelimit;

static void aodv_socket_read(int fd);

/* Seems that some libc (for example ulibc) has a bug in the provided
 * CMSG_NXTHDR() routine... redefining it here */

static struct cmsghdr *__cmsg_nxthdr_fix(void *__ctl, size_t __size,
        struct cmsghdr *__cmsg)
{
    struct cmsghdr *__ptr;

    __ptr = (struct cmsghdr *) (((unsigned char *) __cmsg) +
                                CMSG_ALIGN(__cmsg->cmsg_len));
    if ((unsigned long) ((char *) (__ptr + 1) - (char *) __ctl) > __size)
        return nullptr;

    return __ptr;
}

struct cmsghdr *cmsg_nxthdr_fix(struct msghdr *__msg, struct cmsghdr *__cmsg)
{
    return __cmsg_nxthdr_fix(__msg->msg_control, __msg->msg_controllen, __cmsg);
}

#endif              /* NS_PORT */


void NS_CLASS aodv_socket_init()
{
#ifndef NS_PORT
    struct sockaddr_in aodv_addr;
    struct ifreq ifr;
    int i, retval = 0;
    int on = 1;
    int tos = IPTOS_LOWDELAY;
    int bufsize = SO_RECVBUF_SIZE;
    socklen_t optlen = sizeof(bufsize);

    /* Create a UDP socket */

    if (this_host.nif == 0)
    {
        fprintf(stderr, "No interfaces configured\n");
        exit(-1);
    }

    /* Open a socket for every AODV enabled interface */
    for (i = 0; i < MAX_NR_INTERFACES; i++)
    {
        if (!DEV_NR(i).enabled)
            continue;

        /* AODV socket */
        DEV_NR(i).sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (DEV_NR(i).sock < 0)
        {
            perror("");
            exit(-1);
        }
#ifdef CONFIG_GATEWAY
        /* Data packet send socket */
        DEV_NR(i).psock = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);

        if (DEV_NR(i).psock < 0)
        {
            perror("");
            exit(-1);
        }
#endif
        /* Bind the socket to the AODV port number */
        memset(&aodv_addr, 0, sizeof(aodv_addr));
        aodv_addr.sin_family = AF_INET;
        aodv_addr.sin_port = htons(AODV_PORT);
        aodv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        retval = bind(DEV_NR(i).sock, (struct sockaddr *) &aodv_addr,
                      sizeof(struct sockaddr));

        if (retval < 0)
        {
            perror("Bind failed ");
            exit(-1);
        }
        if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_BROADCAST,
                       &on, sizeof(int)) < 0)
        {
            perror("SO_BROADCAST failed ");
            exit(-1);
        }

        memset(&ifr, 0, sizeof(struct ifreq));
        strcpy(ifr.ifr_name, DEV_NR(i).ifname);

        if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_BINDTODEVICE,
                       &ifr, sizeof(ifr)) < 0)
        {
            fprintf(stderr, "SO_BINDTODEVICE failed for %s", DEV_NR(i).ifname);
            perror(" ");
            exit(-1);
        }

        if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_PRIORITY,
                       &tos, sizeof(int)) < 0)
        {
            perror("Setsockopt SO_PRIORITY failed ");
            exit(-1);
        }

        if (setsockopt(DEV_NR(i).sock, SOL_IP, IP_RECVTTL,
                       &on, sizeof(int)) < 0)
        {
            perror("Setsockopt IP_RECVTTL failed ");
            exit(-1);
        }

        if (setsockopt(DEV_NR(i).sock, SOL_IP, IP_PKTINFO,
                       &on, sizeof(int)) < 0)
        {
            perror("Setsockopt IP_PKTINFO failed ");
            exit(-1);
        }
#ifdef CONFIG_GATEWAY
        if (setsockopt(DEV_NR(i).psock, SOL_SOCKET, SO_BINDTODEVICE,
                       &ifr, sizeof(ifr)) < 0)
        {
            fprintf(stderr, "SO_BINDTODEVICE failed for %s", DEV_NR(i).ifname);
            perror(" ");
            exit(-1);
        }

        bufsize = 4 * 65535;

        if (setsockopt(DEV_NR(i).psock, SOL_SOCKET, SO_SNDBUF,
                       (char *) &bufsize, optlen) < 0)
        {
            DEBUG(LOG_NOTICE, 0, "Could not set send socket buffer size");
        }
        if (getsockopt(DEV_NR(i).psock, SOL_SOCKET, SO_SNDBUF,
                       (char *) &bufsize, &optlen) == 0)
        {
            alog(LOG_NOTICE, 0, __FUNCTION__,
                 "RAW send socket buffer size set to %d", bufsize);
        }
#endif
        /* Set max allowable receive buffer size... */
        for (;; bufsize -= 1024)
        {
            if (setsockopt(DEV_NR(i).sock, SOL_SOCKET, SO_RCVBUF,
                           (char *) &bufsize, optlen) == 0)
            {
                alog(LOG_NOTICE, 0, __FUNCTION__,
                     "Receive buffer size set to %d", bufsize);
                break;
            }
            if (bufsize < RECV_BUF_SIZE)
            {
                alog(LOG_ERR, 0, __FUNCTION__,
                     "Could not set receive buffer size");
                exit(-1);
            }
        }

        retval = attach_callback_func(DEV_NR(i).sock, aodv_socket_read);

        if (retval < 0)
        {
            perror("register input handler failed ");
            exit(-1);
        }
    }
#endif              /* NS_PORT */

    num_rreq = 0;
    num_rerr = 0;
}

void NS_CLASS aodv_socket_process_packet(Packet * pkt, int len,
        struct in_addr src,
        struct in_addr dst,
        int ttl, unsigned int ifindex)
{
    /* If this was a HELLO message... Process as HELLO. */

    auto aodv_msg = pkt->peekAtFront<AODV_msg>();

    if ((aodv_msg->type == AODV_RREP && ttl == 0 && // ttl is decremented for ip layer before send to aodv
            dst.s_addr == L3Address(Ipv4Address(AODV_BROADCAST))))
    {
        auto rrep = constPtrCast<RREP> (pkt->peekAtFront<RREP>());

        hello_process(rrep, len, ifindex);
        return;
    }
    /* Make sure we add/update neighbors */

    neighbor_add(aodv_msg, src, ifindex);

    /* Check what type of msg we received and call the corresponding
       function to handle the msg... */
    pkt->trimFront();
    switch (aodv_msg->type) {
        case AODV_RREQ:
        {
            auto rreq = pkt->removeAtFront<RREQ>();
            rreq_process(rreq, len, src, dst, ttl, ifindex);
            pkt->insertAtFront(rreq);
        }
        break;
        case AODV_RREP:
        {
            DEBUG(LOG_DEBUG, 0, "Received RREP");
            auto rrep = pkt->removeAtFront<RREP>();
            rrep_process(rrep, len, src, dst, ttl, ifindex);
            pkt->insertAtFront(rrep);
        }
        break;
        case AODV_RERR:
        {
            DEBUG(LOG_DEBUG, 0, "Received RERR");
            auto rerr = pkt->removeAtFront<RERR>();
            rerr_process(rerr, len, src, dst);
            pkt->insertAtFront(rerr);
        }
        break;
        case AODV_RREP_ACK:
        {
            DEBUG(LOG_DEBUG, 0, "Received RREP_ACK");
            auto rrep_ack = pkt->removeAtFront<RREP_ack>();
            rrep_ack_process(rrep_ack, len, src, dst);
            pkt->insertAtFront(rrep_ack);
        }
        break;
        default:
            alog(LOG_WARNING, 0, __FUNCTION__,
             "Unknown msg type %u rcvd from %s to %s", aodv_msg->type,
             ip_to_str(src), ip_to_str(dst));
        break;
    }
}

void NS_CLASS aodv_socket_send(Packet *aodvPkt, struct in_addr dst,
                               int len, u_int8_t ttl, struct dev_info *dev,double delay)
{

    struct timeval now;
    /* Rate limit stuff: */

    if (ttl<=0)
    {
        delete aodvPkt;
        return;
    }

    /*
       NS_PORT: Sending of AODV_msg messages to other AODV-UU routing agents
       by encapsulating them in a Packet.

       Note: This method is _only_ for sending AODV packets to other routing
       agents, _not_ for forwarding "regular" IP packets!
     */

    /* If we are in waiting phase after reboot, don't send any RREPs */
    auto aodv_msgAux = aodvPkt->peekAtFront<AODV_msg>();
    if (aodv_msgAux == nullptr)
        throw cRuntimeError("Header is incorrect, must be of time AODV_msg");

    if (wait_on_reboot && aodv_msgAux->type == AODV_RREP)
    {
        delete aodvPkt;
        return;
    }

    /* If rate limiting is enabled, check if we are sending either a
       RREQ or a RERR. In that case, drop the outgoing control packet
       if the time since last transmit of that type of packet is less
       than the allowed RATE LIMIT time... */

    if (ratelimit)
    {

        gettimeofday(&now, nullptr);

        switch (aodv_msgAux->type)
        {
        case AODV_RREQ:
            if (num_rreq == (RREQ_RATELIMIT - 1))
            {
                if (timeval_diff(&now, &rreq_ratel[0]) < 1000)
                {
                    DEBUG(LOG_DEBUG, 0, "RATELIMIT: Dropping RREQ %ld ms",
                          timeval_diff(&now, &rreq_ratel[0]));
                    delete aodvPkt;

                    return;
                }
                else
                {
                    memmove(rreq_ratel, &rreq_ratel[1],
                            sizeof(struct timeval) * (num_rreq - 1));
                    memcpy(&rreq_ratel[num_rreq - 1], &now,
                           sizeof(struct timeval));
                }
            }
            else
            {
                memcpy(&rreq_ratel[num_rreq], &now, sizeof(struct timeval));
                num_rreq++;
            }
            break;
        case AODV_RERR:
            if (num_rerr == (RERR_RATELIMIT - 1))
            {
                if (timeval_diff(&now, &rerr_ratel[0]) < 1000)
                {
                    DEBUG(LOG_DEBUG, 0, "RATELIMIT: Dropping RERR %ld ms",
                          timeval_diff(&now, &rerr_ratel[0]));
                    delete aodvPkt;
                    return;
                }
                else
                {
                    memmove(rerr_ratel, &rerr_ratel[1],
                            sizeof(struct timeval) * (num_rerr - 1));
                    memcpy(&rerr_ratel[num_rerr - 1], &now,
                           sizeof(struct timeval));
                }
            }
            else
            {
                memcpy(&rerr_ratel[num_rerr], &now, sizeof(struct timeval));
                num_rerr++;
            }
            break;
        }
    }

    auto aodv_msg = aodvPkt->removeAtFront<AODV_msg>();

    aodv_msg->prevFix = this->isStaticNode();
    if (this->isStaticNode())
    {
        if (dynamicPtrCast<RREP>(aodv_msg))
        {
            dynamicPtrCast<RREP>(aodv_msg)->cost += costStatic;
        }
        else if (dynamicPtrCast<RREQ> (aodv_msg))
        {
            dynamicPtrCast<RREQ>(aodv_msg)->cost += costStatic;
        }
    }
    else
    {
        if (dynamicPtrCast<RREP>(aodv_msg))
        {
            dynamicPtrCast<RREP>(aodv_msg)->cost += costMobile;
        }
        else if (dynamicPtrCast<RREQ>(aodv_msg))
        {
            dynamicPtrCast<RREQ>(aodv_msg)->cost += costMobile;
        }
    }
    L3Address destAdd;
    if (dst.s_addr == L3Address(Ipv4Address(AODV_BROADCAST)))
    {
        gettimeofday(&this_host.bcast_time, nullptr);
        if (!this->isInMacLayer())
            destAdd = L3Address(Ipv4Address::ALLONES_ADDRESS);
        else
            destAdd = L3Address(MacAddress::BROADCAST_ADDRESS);
    }
    else
    {
        destAdd = dst.s_addr;
    }

    // if delay is lower than 0 compute the delay using the distributions in the configuration
    if (delay < 0)
    {
        if (dst.s_addr == L3Address(Ipv4Address(AODV_BROADCAST)))
            delay = par ("broadcastDelay").doubleValue();
        else
            delay = par ("unicastDelay").doubleValue();
    }

    /* Do not print hello msgs... */
    if (!(aodv_msg->type == AODV_RREP && (dst.s_addr == L3Address(Ipv4Address(AODV_BROADCAST)))))
        DEBUG(LOG_INFO, 0, "AODV msg to %s ttl=%d retval=%u size=%u",
              ip_to_str(dst), ttl, retval, len);

    aodvPkt->insertAtFront(aodv_msg);
    if (useIndex)
        sendToIp(aodvPkt, par("UdpPort").intValue(), destAdd, par("UdpPort").intValue(), ttl, delay, dev->ifindex);
    else
        sendToIp(aodvPkt, par("UdpPort").intValue(), destAdd, 654, ttl, delay, dev->ipaddr.s_addr);
    totalSend++;

    return;
}




#ifndef OMNETPP
AODV_msg *NS_CLASS aodv_socket_new_msg(void)
{
    memset(send_buf, '\0', SEND_BUF_SIZE);
    return (AODV_msg *) (send_buf);
}

/* Copy an existing AODV message to the send buffer */
AODV_msg *NS_CLASS aodv_socket_queue_msg(AODV_msg * aodv_msg, int size)
{
    memcpy((char *) send_buf, aodv_msg, size);
    return (AODV_msg *) send_buf;
}
#endif
void aodv_socket_cleanup(void)
{
#ifndef NS_PORT
    int i;

    for (i = 0; i < MAX_NR_INTERFACES; i++)
    {
        if (!DEV_NR(i).enabled)
            continue;
        close(DEV_NR(i).sock);
    }
#endif              /* NS_PORT */
}



void NS_CLASS aodv_socket_send_delayed (Packet * pkt, struct in_addr dst,
                                   int len, u_int8_t ttl, struct dev_info *dev,double delay)
{
    if (delay > 0)
    {
        DelayInfo *info = new DelayInfo;
        info->dst = dst;
        info->len = len;
        info->ttl = ttl;
        info->dev = dev;
        pkt->setControlInfo(info);
        scheduleAt(simTime()+delay,pkt);
    }
    else
        aodv_socket_send(pkt, dst, len, ttl, dev);

}

} // namespace inetmanet

} // namespace inet

