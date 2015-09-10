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
 * Authors: Erik Nordstr�, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/

#include "inet/routing/extras/dymo/dymo_um_omnet.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

namespace inetmanet {

//#define GARBAGE_COLLECT
#ifndef DYMO_USE_STL
void NS_CLASS packet_queue_init(void)
{

    INIT_DLIST_HEAD(&PQ.head);
    PQ.len = 0;

#ifdef GARBAGE_COLLECT
    /* Set up garbage collector */
    timer_init(&PQ.garbage_collect_timer, &NS_CLASS packet_queue_timeout, &PQ);

    timer_set_timeout(&PQ.garbage_collect_timer, GARBAGE_COLLECT_TIME);
#endif
}


void NS_CLASS packet_queue_destroy(void)
{
    int count = 0;
    dlist_head_t *pos, *tmp;
    if (!dlist_empty(&PQ.head))
    {
       dlist_for_each_safe(pos, tmp, &PQ.head)
       {
           struct q_pkt *qp = (struct q_pkt *)pos;
           dlist_del(pos);

           delete  qp->p;

           free(qp);
           count++;
           PQ.len--;
       }
    }
    dlog(LOG_INFO, 0, __FUNCTION__, "Dropped %d buffered packets", count);
    //  DEBUG(LOG_INFO, 0, "Destroyed %d buffered packets!", count);
}

/* Garbage collect packets which have been queued for too long... */
int NS_CLASS packet_queue_garbage_collect(void)
{
    int count = 0;
    dlist_head_t *pos, *tmp;
    struct timeval now;

    gettimeofday(&now, nullptr);

    dlist_for_each_safe(pos, tmp, &PQ.head)
    {
        struct q_pkt *qp = (struct q_pkt *)pos;
        if (timeval_diff(&now, &qp->q_time) > MAX_QUEUE_TIME)
        {

            dlist_del(pos);
            //icmpAccess.get()->sendErrorMessage(qp->p, ICMP_DESTINATION_UNREACHABLE, 0);
            //drop(qp->p);
            sendICMP(qp->p);
            free(qp);
            count++;
            PQ.len--;
        }
    }

    if (count)
    {
        dlog(LOG_DEBUG, 0, __FUNCTION__, "Dropped %d buffered packets", count);
    }

    return count;
}
/* Buffer a packet in a FIFO queue. Implemented as a linked list,
   where we add elements at the end and remove at the beginning.... */

void NS_CLASS packet_queue_add(cPacket * p, struct in_addr dest_addr)
{
    struct q_pkt *qp;
    cPacket *dgram;

    if (p->getControlInfo())
        delete p->removeControlInfo();

    if (PQ.len >= MAX_QUEUE_LENGTH)
    {
        dlog(LOG_DEBUG, 0, __FUNCTION__, "Max queue length reached,"
             " removing first packet");
        if (!dlist_empty(&PQ.head))
        {
            qp = (struct q_pkt *)PQ.head.next;

            dlist_del(PQ.head.next);
            dgram = qp->p;
            sendICMP(dgram);
            //drop(dgram);
            //icmpAccess.get()->sendErrorMessage(dgram, ICMP_DESTINATION_UNREACHABLE, ICMP_AODV_QUEUE_FULL);
            free(qp);
            PQ.len--;
        }
    }

    qp = (struct q_pkt *) malloc(sizeof(struct q_pkt));

    if (qp == nullptr)
    {
        throw cRuntimeError("Dymo packet queue, Malloc failed!\n");
        exit(-1);
    }

    qp->p = p;

    qp->dest_addr = dest_addr;
    qp->inTransit = false;

    gettimeofday(&qp->q_time, nullptr);
    INIT_DLIST_ELEM(&qp->l);
    dlist_add_tail(&qp->l, &PQ.head);
    PQ.len++;
}

int NS_CLASS packet_queue_set_verdict(struct in_addr dest_addr, int verdict)
{
    int count = 0;
    rtable_entry_t *rt;
    dlist_head_t *pos, *tmp;
    struct in_addr gw_addr;

    double delay = 0;
#define ARP_DELAY 0.005
    if (verdict == PQ_ENC_SEND)
    {
        gw_addr.s_addr = gateWayAddress->getInt();
        rt = rtable_find(gw_addr);
    }
    else
        rt = rtable_find(dest_addr);

    dlist_for_each_safe(pos, tmp, &PQ.head)
    {
        struct q_pkt *qp = (struct q_pkt *)pos;
        if (qp->dest_addr.s_addr == dest_addr.s_addr)
        {
            dlist_del(pos);
            switch (verdict)
            {
            case PQ_ENC_SEND:
                if (isInMacLayer())
                {
                    //drop(qp->p);
                    sendICMP(qp->p);
                }
                else if (dynamic_cast <IPv4Datagram *> (qp->p))
                {
                    /* Apparently, the link layer implementation can't handle
                        * a burst of packets. So to keep ARP happy, buffered
                        * packets are sent with ARP_DELAY seconds between
                        * sends. */
                    qp->p = pkt_encapsulate(static_cast <IPv4Datagram *> (qp->p), *gateWayAddress);
                    // now Ip layer decremented again
                    // sendDelayed(qp->p, delay, "to_ip_from_network");
                    sendDelayed(qp->p, delay, "to_ip");
                    delay += ARP_DELAY;
                }
                else
                {
                    //drop(qp->p);
                    sendICMP(qp->p);
                }
                break;
            case PQ_SEND:
                if (!rt)
                    return -1;
                if (qp->inTransit)
                {
                    // drop(qp->p);
                    sendICMP(qp->p);
                }
                else
                {
                    /* Apparently, the link layer implementation can't handle
                     * a burst of packets. So to keep ARP happy, buffered
                     * packets are sent with ARP_DELAY seconds between
                     * sends. */
                    // now Ip layer decremented again
                    if (isInMacLayer())
                    {
                        Ieee802Ctrl *ctrl = new Ieee802Ctrl();
                        L3Address nextHop;
                        int iface;
                        double cost;
                        getNextHop(dest_addr.s_addr, nextHop, iface, cost);
                        ctrl->setDest(MACAddress(nextHop.getLo()));
                        //TODO ctrl->setEtherType(...);
                        qp->p->setControlInfo(ctrl);
                    }
                    sendDelayed(qp->p, delay, "to_ip");
                    delay += ARP_DELAY;
                }
                break;
            case PQ_DROP:
                // drop(qp->p);
                sendICMP(qp->p);
                // icmpAccess.get()->sendErrorMessage(qp->p, ICMP_DESTINATION_UNREACHABLE, 0);
                break;
            }
            free(qp);
            count++;
            PQ.len--;
        }
    }
    /* Update rt timeouts. must be in Dymo?*/
    /*
    rtable_entry_t *next_hop_rt, *inet_rt = nullptr;
    if (rt && rt->state == VALID && (verdict == PQ_SEND || verdict == PQ_ENC_SEND)) {
        if (dest_addr.s_addr != DEV_IFINDEX(rt->ifindex).ipaddr.s_addr) {
            if (verdict == PQ_ENC_SEND && inet_rt)
                rtable_update_timeout(inet_rt);

            rtable_update_timeout(rt);
            next_hop_rt = rtable_find(rt->next_hop);
            if (next_hop_rt && next_hop_rt->state == VALID &&
                next_hop_rt->dest_addr.s_addr != rt->dest_addr.s_addr)
                rtable_update_timeout(next_hop_rt, ACTIVE_ROUTE_TIMEOUT);
        }
    }
    */
    return count;
}
#else
void NS_CLASS packet_queue_init(void)
{

    PQ.pkQueue.clear();


#ifdef GARBAGE_COLLECT
    /* Set up garbage collector */
    timer_init(&PQ.garbage_collect_timer, &NS_CLASS packet_queue_timeout, &PQ);

    timer_set_timeout(&PQ.garbage_collect_timer, GARBAGE_COLLECT_TIME);
#endif
}


void NS_CLASS packet_queue_destroy(void)
{
    int count = 0;
    while (!PQ.pkQueue.empty())
    {
        struct q_pkt *qp = PQ.pkQueue.back();
        PQ.pkQueue.pop_back();
        delete  qp->p;
        free(qp);
        count++;
    }
    dlog(LOG_INFO, 0, __FUNCTION__, "Dropped %d buffered packets", count);
    //  DEBUG(LOG_INFO, 0, "Destroyed %d buffered packets!", count);
}

/* Garbage collect packets which have been queued for too long... */
int NS_CLASS packet_queue_garbage_collect(void)
{
    int count = 0;
    struct timeval now;
    gettimeofday(&now, nullptr);
    for (unsigned int i = 0; i < PQ.pkQueue.size();)
    {
        struct q_pkt *qp = PQ.pkQueue[i];
        if (timeval_diff(&now, &qp->q_time) > MAX_QUEUE_TIME)
        {

            PQ.pkQueue.erase(PQ.pkQueue.begin()+i);
            sendICMP(qp->p);
            free(qp);
            count++;
        }
        else
            i++;
    }

    if (count)
    {
        dlog(LOG_DEBUG, 0, __FUNCTION__, "Dropped %d buffered packets", count);
    }

    return count;
}
/* Buffer a packet in a FIFO queue. Implemented as a linked list,
   where we add elements at the end and remove at the beginning.... */

void NS_CLASS packet_queue_add(cPacket * p, struct in_addr dest_addr)
{
    struct q_pkt *qp;
    cPacket *dgram;

    if (p->getControlInfo())
        delete p->removeControlInfo();

    if (PQ.pkQueue.size() >= MAX_QUEUE_LENGTH)
    {
        dlog(LOG_DEBUG, 0, __FUNCTION__, "Max queue length reached,"
             " removing first packet");
        qp = PQ.pkQueue.front();
        PQ.pkQueue.erase(PQ.pkQueue.begin());
        dgram = qp->p;
        sendICMP(dgram);
        free(qp);
    }

    qp = (struct q_pkt *) malloc(sizeof(struct q_pkt));

    if (qp == nullptr)
    {
        throw cRuntimeError("Dymo packet queue, Malloc failed!\n");
    }

    qp->p = p;

    qp->dest_addr = dest_addr;
    qp->inTransit = false;

    gettimeofday(&qp->q_time, nullptr);
    PQ.pkQueue.push_back(qp);

}

int NS_CLASS packet_queue_set_verdict(struct in_addr dest_addr, int verdict)
{
    int count = 0;
    rtable_entry_t *rt;
    struct in_addr gw_addr;

    double delay = 0;
#define ARP_DELAY 0.005
    if (verdict == PQ_ENC_SEND)
    {
        gw_addr.s_addr = L3Address(*gateWayAddress);
        rt = rtable_find(gw_addr);
    }
    else
        rt = rtable_find(dest_addr);

    std::vector<L3Address> list;
    if (isInMacLayer())
    {
        std::vector<MACAddress> listMac;
        getApList(dest_addr.s_addr.toMAC(), listMac);
        while (!listMac.empty())
        {
            list.push_back(L3Address(listMac.back()));
            listMac.pop_back();
        }
    }
    else
    {
        std::vector<IPv4Address> listIp;
        getApListIp(dest_addr.s_addr.toIPv4(), listIp);
        while (!listIp.empty())
        {
            list.push_back(L3Address(listIp.back()));
            listIp.pop_back();
        }
    }
    while (!list.empty())
    {
        struct in_addr dest_addr;
        dest_addr.s_addr = list.back();
        list.pop_back();
        for (unsigned int i=0; i < PQ.pkQueue.size();)
        {
            struct q_pkt *qp = PQ.pkQueue[i];
            if (qp->dest_addr.s_addr == dest_addr.s_addr)
            {
                PQ.pkQueue.erase(PQ.pkQueue.begin()+i);
                switch (verdict)
                {
                    case PQ_ENC_SEND:
                        if (isInMacLayer())
                        {
                            //drop(qp->p);
                            sendICMP(qp->p);
                        }
                        else if (dynamic_cast <IPv4Datagram *> (qp->p))
                        {
                            /* Apparently, the link layer implementation can't handle
                             * a burst of packets. So to keep ARP happy, buffered
                             * packets are sent with ARP_DELAY seconds between
                             * sends. */
                            qp->p = pkt_encapsulate(static_cast <IPv4Datagram *> (qp->p), *gateWayAddress);
                            // now Ip layer decremented again
                            // sendDelayed(qp->p, delay, "to_ip_from_network");
                            sendDelayed(qp->p, delay, "to_ip");
                            delay += ARP_DELAY;
                        }
                        else
                        {
                            //drop(qp->p);
                            sendICMP(qp->p);
                        }
                        break;
                    case PQ_SEND:
                        if (!rt)
                            return -1;
                        if (qp->inTransit)
                        {
                            // drop(qp->p);
                            sendICMP(qp->p);
                        }
                        else
                        {
                            /* Apparently, the link layer implementation can't handle
                             * a burst of packets. So to keep ARP happy, buffered
                             * packets are sent with ARP_DELAY seconds between
                             * sends. */
                            // now Ip layer decremented again
                            if (isInMacLayer())
                            {
                                Ieee802Ctrl *ctrl = new Ieee802Ctrl();
                                L3Address nextHop;
                                int iface;
                                double cost;
                                getNextHop(dest_addr.s_addr, nextHop, iface, cost);
                                ctrl->setDest(nextHop.toMAC());
                                //TODO ctrl->setEtherType(...);
                                qp->p->setControlInfo(ctrl);
                            }
                            sendDelayed(qp->p, delay, "to_ip");
                            delay += ARP_DELAY;
                        }
                        break;
                    case PQ_DROP:
                        // drop(qp->p);
                        sendICMP(qp->p);
                        // icmpAccess.get()->sendErrorMessage(qp->p, ICMP_DESTINATION_UNREACHABLE, 0);
                        break;
                }
                free(qp);
                count++;
            }
            else
                i++;
        }
    }
        /* Update rt timeouts. must be in Dymo?*/
    /*
    rtable_entry_t *next_hop_rt, *inet_rt = nullptr;
    if (rt && rt->state == VALID && (verdict == PQ_SEND || verdict == PQ_ENC_SEND)) {
        if (dest_addr.s_addr != DEV_IFINDEX(rt->ifindex).ipaddr.s_addr) {
            if (verdict == PQ_ENC_SEND && inet_rt)
                rtable_update_timeout(inet_rt);

            rtable_update_timeout(rt);
            next_hop_rt = rtable_find(rt->next_hop);
            if (next_hop_rt && next_hop_rt->state == VALID &&
                next_hop_rt->dest_addr.s_addr != rt->dest_addr.s_addr)
                rtable_update_timeout(next_hop_rt, ACTIVE_ROUTE_TIMEOUT);
        }
    }
    */
    return count;
}

#endif

} // namespace inetmanet

} // namespace inet

