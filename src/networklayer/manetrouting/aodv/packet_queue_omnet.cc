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
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/

#include "aodv_uu_omnet.h"


#define GARBAGE_COLLECT

void NS_CLASS packet_queue_init(void)
{
    INIT_LIST_HEAD(&PQ.head);
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
    list_t *pos, *tmp;

    list_foreach_safe(pos, tmp, &PQ.head)
    {
        struct q_pkt *qp = (struct q_pkt *)pos;
        list_detach(pos);

        delete  qp->p;

        free(qp);
        count++;
        PQ.len--;
    }

    DEBUG(LOG_INFO, 0, "Destroyed %d buffered packets!", count);
}

/* Garbage collect packets which have been queued for too long... */
int NS_CLASS packet_queue_garbage_collect(void)
{
    int count = 0;
    list_t *pos, *tmp;
    struct timeval now;

    gettimeofday(&now, NULL);

    list_foreach_safe(pos, tmp, &PQ.head)
    {
        struct q_pkt *qp = (struct q_pkt *)pos;
        if (timeval_diff(&now, &qp->q_time) > MAX_QUEUE_TIME)
        {
            list_detach(pos);
            //icmpAccess.get()->sendErrorMessage(qp->p, ICMP_DESTINATION_UNREACHABLE, 0);
            //   drop (qp->p);
            sendICMP(qp->p);
            free(qp);
            count++;
            PQ.len--;
        }
    }

    if (count)
    {
        DEBUG(LOG_DEBUG, 0, "Removed %d packet(s)!", count);
    }

    return count;
}
/* Buffer a packet in a FIFO queue. Implemented as a linked list,
   where we add elements at the end and remove at the beginning.... */

void NS_CLASS packet_queue_add(cPacket * p, struct in_addr dest_addr)
{
    struct q_pkt *qp;
    cPacket *dgram;

    if (PQ.len >= MAX_QUEUE_LENGTH)
    {
        DEBUG(LOG_DEBUG, 0, "MAX Queue length! Removing first packet.");
        if (!list_empty(&PQ.head))
        {
            qp = (struct q_pkt *)PQ.head.next;
            list_detach(PQ.head.next);
            dgram =qp->p;
            // drop (dgram);
            sendICMP(dgram);
//  icmpAccess.get()->sendErrorMessage(dgram, ICMP_DESTINATION_UNREACHABLE, ICMP_AODV_QUEUE_FULL);
            free(qp);
            PQ.len--;
        }
    }

    qp = (struct q_pkt *) malloc(sizeof(struct q_pkt));

    if (qp == NULL)
    {
        fprintf(stderr, "Malloc failed!\n");
        exit(-1);
    }

    qp->p = p;
    qp->dest_addr = dest_addr;

    gettimeofday(&qp->q_time, NULL);

    list_add_tail(&PQ.head, &qp->l);

    PQ.len++;

    DEBUG(LOG_INFO, 0, "buffered pkt to %s qlen=%u",
          ip_to_str(dest_addr), PQ.len);
}

int NS_CLASS packet_queue_set_verdict(struct in_addr dest_addr, int verdict)
{
    int count = 0;
    rt_table_t *rt, *next_hop_rt, *inet_rt = NULL;
    list_t *pos, *tmp;
    struct in_addr gw_addr;

    double delay = 0;
#define ARP_DELAY 0.005

    if (verdict == PQ_ENC_SEND)
    {
        gw_addr.s_addr =   gateWayAddress->getInt();
        rt = rt_table_find(gw_addr);
    }
    else
        rt = rt_table_find(dest_addr);

    list_foreach_safe(pos, tmp, &PQ.head)
    {

        struct q_pkt *qp = (struct q_pkt *)pos;
        if (qp->dest_addr.s_addr == dest_addr.s_addr)
        {
            list_detach(pos);

            switch (verdict)
            {
            case PQ_ENC_SEND:
                if (dynamic_cast <IPDatagram *> (qp->p))
                {
                    qp->p = pkt_encapsulate(dynamic_cast <IPDatagram *> (qp->p), *gateWayAddress);
                    // now Ip layer decremented again
                    /* Apparently, the link layer implementation can't handle
                     a burst of packets. So to keep ARP happy, buffered              *                   *
                     packets are sent with ARP_DELAY seconds between sends. */
                    sendDelayed (qp->p,delay,"to_ip");
                    delay += ARP_DELAY;
                }
                else
                {
                    // drop(qp->p);
                    sendICMP(qp->p);
                }
                break;
            case PQ_SEND:
                if (!rt)
                    return -1;
                /* Apparently, the link layer implementation can't handle
                 * a burst of packets. So to keep ARP happy, buffered
                 * packets are sent with ARP_DELAY seconds between
                 * sends. */
                // now Ip layer decremented again
                sendDelayed(qp->p, delay, "to_ip");
                delay += ARP_DELAY;
                break;
            case PQ_DROP:
                //drop(qp->p);
                sendICMP(qp->p);

//              icmpAccess.get()->sendErrorMessage(qp->p, ICMP_DESTINATION_UNREACHABLE, 0);
                break;
            }
            free(qp);
            count++;
            PQ.len--;
        }
    }
    /* Update rt timeouts */
    if (rt && rt->state == VALID &&
            (verdict == PQ_SEND || verdict == PQ_ENC_SEND))
    {
        if (dest_addr.s_addr != DEV_IFINDEX(rt->ifindex).ipaddr.s_addr)
        {
            if (verdict == PQ_ENC_SEND && inet_rt)
                rt_table_update_timeout(inet_rt, ACTIVE_ROUTE_TIMEOUT);

            rt_table_update_timeout(rt, ACTIVE_ROUTE_TIMEOUT);

            next_hop_rt = rt_table_find(rt->next_hop);

            if (next_hop_rt && next_hop_rt->state == VALID &&
                    next_hop_rt->dest_addr.s_addr != rt->dest_addr.s_addr)
                rt_table_update_timeout(next_hop_rt, ACTIVE_ROUTE_TIMEOUT);
        }

        DEBUG(LOG_INFO, 0, "SENT %d packets to %s qlen=%u",
              count, ip_to_str(dest_addr), PQ.len);
    }
    else if (verdict == PQ_DROP)
    {
        DEBUG(LOG_INFO, 0, "DROPPED %d packets for %s!",
              count, ip_to_str(dest_addr));
    }

    return count;
}

