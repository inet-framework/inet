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
 *
 *****************************************************************************/
#define NS_PORT
#define OMNETPP

#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/aodv-uu.h"
#else
#include "../aodv_uu_omnet.h"
#include "compatibility.h"
#endif
#else
#include <netinet/in.h>
#include "aodv_rrep.h"
#include "aodv_neighbor.h"
#include "aodv_hello.h"
#include "routing_table.h"
#include "aodv_timeout.h"
#include "timer_queue_aodv.h"
#include "aodv_socket.h"
#include "defs_aodv.h"
#include "debug_aodv.h"
#include "params.h"

extern int unidir_hack, optimized_hellos, llfeedback;

#endif


RREP *NS_CLASS rrep_create(u_int8_t flags,
                           u_int8_t prefix,
                           u_int8_t hcnt,
                           struct in_addr dest_addr,
                           u_int32_t dest_seqno,
                           struct in_addr orig_addr, u_int32_t life)
{
    RREP *rrep;
#ifndef OMNETPP
    rrep = (RREP *) aodv_socket_new_msg();
#else
    rrep =  new RREP("RouteReply");
#endif
    rrep->type = AODV_RREP;
    rrep->res1 = 0;
    rrep->res2 = 0;
    rrep->prefix = prefix;
    rrep->hcnt = hcnt;
    rrep->dest_addr = dest_addr.s_addr;
    rrep->dest_seqno = htonl(dest_seqno);
    rrep->orig_addr = orig_addr.s_addr;
    rrep->lifetime = htonl(life);

    if (flags & RREP_REPAIR)
        rrep->r = 1;
    if (flags & RREP_ACK)
        rrep->a = 1;

    /* Don't print information about hello messages... */
#ifdef DEBUG_OUTPUT
    if (rrep->dest_addr != rrep->orig_addr)
    {
        DEBUG(LOG_DEBUG, 0, "Assembled RREP:");
        log_pkt_fields((AODV_msg *) rrep);
    }
#endif

    return rrep;
}

RREP_ack *NS_CLASS rrep_ack_create()
{
    RREP_ack *rrep_ack;
#ifndef OMNETPP
    rrep_ack = (RREP_ack *) aodv_socket_new_msg();
#else
    rrep_ack = new RREP_ack("RouteReplyAck");
#endif
    rrep_ack->type = AODV_RREP_ACK;

    DEBUG(LOG_DEBUG, 0, "Assembled RREP_ack");
    return rrep_ack;
}

void NS_CLASS rrep_ack_process(RREP_ack * rrep_ack, int rrep_acklen,
                               struct in_addr ip_src, struct in_addr ip_dst)
{
    rt_table_t *rt;

    rt = rt_table_find(ip_src);
#ifdef OMNETPP
    totalRrepAckRec++;
#endif
    if (rt == NULL)
    {
        DEBUG(LOG_WARNING, 0, "No RREP_ACK expected for %s", ip_to_str(ip_src));

        return;
    }
    DEBUG(LOG_DEBUG, 0, "Received RREP_ACK from %s", ip_to_str(ip_src));

    /* Remove unexpired timer for this RREP_ACK */
    timer_remove(&rt->ack_timer);
}

AODV_ext *NS_CLASS rrep_add_ext(RREP * rrep, int type, unsigned int offset,
                                int len, char *data)
{
    AODV_ext *ext = NULL;
#ifndef OMNETPP
    if (offset < RREP_SIZE)
        return NULL;

    ext = (AODV_ext *) ((char *) rrep + offset);

    ext->type = type;
    ext->length = len;

    memcpy(AODV_EXT_DATA(ext), data, len);
#else
    ext = rrep->addExtension(type,len,data);
#endif
    return ext;
}

void NS_CLASS rrep_send(RREP * rrep, rt_table_t * rev_rt,
                        rt_table_t * fwd_rt, int size)
{
    u_int8_t rrep_flags = 0;
    struct in_addr dest;

    if (!rev_rt)
    {
        DEBUG(LOG_WARNING, 0, "Can't send RREP, rev_rt = NULL!");
        return;
    }

    dest.s_addr = rrep->dest_addr;

    /* Check if we should request a RREP-ACK */
    if ((rev_rt->state == VALID && rev_rt->flags & RT_UNIDIR) ||
            (rev_rt->hcnt == 1 && unidir_hack))
    {
        rt_table_t *neighbor = rt_table_find(rev_rt->next_hop);

        if (neighbor && neighbor->state == VALID && !neighbor->ack_timer.used)
        {
            /* If the node we received a RREQ for is a neighbor we are
               probably facing a unidirectional link... Better request a
               RREP-ack */
            rrep_flags |= RREP_ACK;
            neighbor->flags |= RT_UNIDIR;

            /* Must remove any pending hello timeouts when we set the
               RT_UNIDIR flag, else the route may expire after we begin to
               ignore hellos... */
            timer_remove(&neighbor->hello_timer);
            neighbor_link_break(neighbor);

            DEBUG(LOG_DEBUG, 0, "Link to %s is unidirectional!",
                  ip_to_str(neighbor->dest_addr));

            timer_set_timeout(&neighbor->ack_timer, NEXT_HOP_WAIT);
        }
    }

    DEBUG(LOG_DEBUG, 0, "Sending RREP to next hop %s about %s->%s",
          ip_to_str(rev_rt->next_hop), ip_to_str(rev_rt->dest_addr),
          ip_to_str(dest));
#ifdef OMNETPP
    if (!omnet_exist_rte (rev_rt->next_hop))
    {
        struct in_addr nm;
        nm.s_addr = IPAddress((uint32_t)rev_rt->next_hop.s_addr).getNetworkMask().getInt();
        if (useIndex)
            omnet_chg_rte(rev_rt->next_hop,rev_rt->next_hop, nm, 1,false,rev_rt->ifindex);
        else
            omnet_chg_rte(rev_rt->next_hop,rev_rt->next_hop, nm, 1,false,DEV_NR(rev_rt->ifindex).ipaddr);
    }
    totalRrepSend++;
#endif
    rrep->ttl=MAXTTL;
    aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, 1,
                     &DEV_IFINDEX(rev_rt->ifindex));

    /* Update precursor lists */
    if (fwd_rt)
    {
        precursor_add(fwd_rt, rev_rt->next_hop);
        precursor_add(rev_rt, fwd_rt->next_hop);
    }

    if (!llfeedback && optimized_hellos)
        hello_start();
}

void NS_CLASS rrep_forward(RREP * rrep, int size, rt_table_t * rev_rt,
                           rt_table_t * fwd_rt, int ttl)
{
    /* Sanity checks... */
    if (!fwd_rt || !rev_rt)
    {
        DEBUG(LOG_WARNING, 0, "Could not forward RREP because of NULL route!");
        return;
    }

    if (!rrep)
    {
        DEBUG(LOG_WARNING, 0, "No RREP to forward!");
        return;
    }

    DEBUG(LOG_DEBUG, 0, "Forwarding RREP to %s", ip_to_str(rev_rt->next_hop));

    /* Here we should do a check if we should request a RREP_ACK,
       i.e we suspect a unidirectional link.. But how? */
    if (0)
    {
        rt_table_t *neighbor;

        /* If the source of the RREP is not a neighbor we must find the
           neighbor (link) entry which is the next hop towards the RREP
           source... */
        if (rev_rt->dest_addr.s_addr != rev_rt->next_hop.s_addr)
            neighbor = rt_table_find(rev_rt->next_hop);
        else
            neighbor = rev_rt;

        if (neighbor && !neighbor->ack_timer.used)
        {
            /* If the node we received a RREQ for is a neighbor we are
               probably facing a unidirectional link... Better request a
               RREP-ack */
            rrep->a = 1;
            neighbor->flags |= RT_UNIDIR;

            timer_set_timeout(&neighbor->ack_timer, NEXT_HOP_WAIT);
        }
    }
#ifndef OMNETPP
    rrep = (RREP *) aodv_socket_queue_msg((AODV_msg *) rrep, size);
    rrep->hcnt = fwd_rt->hcnt;  /* Update the hopcount */

    aodv_socket_send((AODV_msg *) rrep, rev_rt->next_hop, size, ttl,
                     &DEV_IFINDEX(rev_rt->ifindex));

#else
    RREP * rrep_new = check_and_cast <RREP *> (rrep->dup());
    rrep_new->hcnt = fwd_rt->hcnt;
    totalRrepSend++;
    rrep_new->ttl=ttl;
    aodv_socket_send((AODV_msg *) rrep_new, rev_rt->next_hop, size, 1,
                     &DEV_IFINDEX(rev_rt->ifindex));
#endif
    precursor_add(fwd_rt, rev_rt->next_hop);
    precursor_add(rev_rt, fwd_rt->next_hop);
    rt_table_update_timeout(rev_rt, ACTIVE_ROUTE_TIMEOUT);
}


void NS_CLASS rrep_process(RREP * rrep, int rreplen, struct in_addr ip_src,
                           struct in_addr ip_dst, int ip_ttl,unsigned int ifindex)
{
    u_int32_t rrep_lifetime, rrep_seqno, rrep_new_hcnt;
    u_int8_t pre_repair_hcnt = 0, pre_repair_flags = 0;
    rt_table_t *fwd_rt, *rev_rt;
    AODV_ext *ext;
    unsigned int extlen = 0;
    int rt_flags = 0;
    struct in_addr rrep_dest, rrep_orig;
#ifdef CONFIG_GATEWAY
    struct in_addr inet_dest_addr;
    int inet_rrep = 0;
#endif

    /* Convert to correct byte order on affeected fields: */
    rrep_dest.s_addr = rrep->dest_addr;
    rrep_orig.s_addr = rrep->orig_addr;
    rrep_seqno = ntohl(rrep->dest_seqno);
    rrep_lifetime = ntohl(rrep->lifetime);
    /* Increment RREP hop count to account for intermediate node... */
    rrep_new_hcnt = rrep->hcnt + 1;

    if (rreplen < (int) RREP_SIZE)
    {
        alog(LOG_WARNING, 0, __FUNCTION__,
             "IP data field too short (%u bytes)"
             " from %s to %s", rreplen, ip_to_str(ip_src), ip_to_str(ip_dst));
        return;
    }



    /* Ignore messages which aim to a create a route to one self */

#ifndef OMNETPP
    if (rrep_dest.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)
        return;

    if (rrep_orig.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)
        DEBUG(LOG_DEBUG, 0, "rrep for us");
#else


    if (isLocalAddress (rrep_dest.s_addr))
        return;
    if (isLocalAddress(rrep_orig.s_addr))
        DEBUG(LOG_DEBUG, 0, "rrep for us");
#endif

    DEBUG(LOG_DEBUG, 0, "from %s about %s->%s",
          ip_to_str(ip_src), ip_to_str(rrep_orig), ip_to_str(rrep_dest));
#ifdef DEBUG_OUTPUT
    log_pkt_fields((AODV_msg *) rrep);
#endif

    /* Determine whether there are any extensions */

#ifndef OMNETPP
    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE);
    while ((rreplen - extlen) > RREP_SIZE)
    {
#else
    totalRrepRec++;
    ext = rrep->getFirstExtension();
    for (int i=0; i<rrep->getNumExtension (); i++)
    {
#endif
        switch (ext->type)
        {
        case RREP_EXT:
            DEBUG(LOG_INFO, 0, "RREP include EXTENSION");
            /* Do something here */
            break;
#ifdef CONFIG_GATEWAY
        case RREP_INET_DEST_EXT:
            if (ext->length == sizeof(u_int32_t))
            {

                /* Destination address in RREP is the gateway address, while the
                 * extension holds the real destination */
                memcpy(&inet_dest_addr, AODV_EXT_DATA(ext), ext->length);
                DEBUG(LOG_DEBUG, 0, "RREP_INET_DEST_EXT: <%s>",
                      ip_to_str(inet_dest_addr));
                /* This was a RREP from a gateway */
                rt_flags |= RT_GATEWAY;
                inet_rrep = 1;
                break;
            }
#endif
        default:
            alog(LOG_WARNING, 0, __FUNCTION__, "Unknown or bad extension %d",
                 ext->type);
            break;
        }
        extlen += AODV_EXT_SIZE(ext);
        ext = AODV_EXT_NEXT(ext);
    }

    /* ---------- CHECK IF WE SHOULD MAKE A FORWARD ROUTE ------------ */

    fwd_rt = rt_table_find(rrep_dest);
    rev_rt = rt_table_find(rrep_orig);

    if (!fwd_rt)
    {
        /* We didn't have an existing entry, so we insert a new one. */
        fwd_rt = rt_table_insert(rrep_dest, ip_src, rrep_new_hcnt, rrep_seqno,
                                 rrep_lifetime, VALID, rt_flags, ifindex);
    }
    else if (fwd_rt->dest_seqno == 0 ||
             (int32_t) rrep_seqno > (int32_t) fwd_rt->dest_seqno ||
             (rrep_seqno == fwd_rt->dest_seqno &&
              (fwd_rt->state == INVALID || fwd_rt->flags & RT_UNIDIR ||
               rrep_new_hcnt < fwd_rt->hcnt)))
    {
        pre_repair_hcnt = fwd_rt->hcnt;
        pre_repair_flags = fwd_rt->flags;

        fwd_rt = rt_table_update(fwd_rt, ip_src, rrep_new_hcnt, rrep_seqno,
                                 rrep_lifetime, VALID,
                                 rt_flags | fwd_rt->flags);
    }
    else
    {
        if (fwd_rt->hcnt > 1)
        {
            DEBUG(LOG_DEBUG, 0,
                  "Dropping RREP, fwd_rt->hcnt=%d fwd_rt->seqno=%ld",
                  fwd_rt->hcnt, fwd_rt->dest_seqno);
        }
        return;
    }

    /* If the RREP_ACK flag is set we must send a RREP
       acknowledgement to the destination that replied... */
    if (rrep->a)
    {
        RREP_ack *rrep_ack;

        rrep_ack = rrep_ack_create();
        totalRrepAckSend++;
        rrep_ack->ttl=MAXTTL;
        aodv_socket_send((AODV_msg *) rrep_ack, fwd_rt->next_hop,
                         NEXT_HOP_WAIT, 1, &DEV_IFINDEX(fwd_rt->ifindex));
        /* Remove RREP_ACK flag... */
        rrep->a = 0;
    }

    /* Check if this RREP was for us (i.e. we previously made a RREQ
       for this host). */
#ifndef OMNETPP
    if (rrep_orig.s_addr == DEV_IFINDEX(ifindex).ipaddr.s_addr)
    {
#else
    if (isLocalAddress (rrep_orig.s_addr))
    {
#endif

#ifdef CONFIG_GATEWAY
        if (inet_rrep)
        {
            rt_table_t *inet_rt;
            inet_rt = rt_table_find(inet_dest_addr);

            /* Add a "fake" route indicating that this is an Internet
             * destination, thus should be encapsulated and routed through a
             * gateway... */
            if (!inet_rt)
                rt_table_insert(inet_dest_addr, rrep_dest, rrep_new_hcnt, 0,
                                rrep_lifetime, VALID, RT_INET_DEST, ifindex);
            else if (inet_rt->state == INVALID || rrep_new_hcnt < inet_rt->hcnt)
            {
                rt_table_update(inet_rt, rrep_dest, rrep_new_hcnt, 0,
                                rrep_lifetime, VALID, RT_INET_DEST |
                                inet_rt->flags);
            }
            else
            {
                DEBUG(LOG_DEBUG, 0, "INET Response, but no update %s",
                      ip_to_str(inet_dest_addr));
            }
        }
#endif              /* CONFIG_GATEWAY */

        /* If the route was previously in repair, a NO DELETE RERR should be
           sent to the source of the route, so that it may choose to reinitiate
           route discovery for the destination. Fixed a bug here that caused the
           repair flag to be unset and the RERR never being sent. Thanks to
           McWood <hjw_5@hotmail.com> for discovering this. */
        if (pre_repair_flags & RT_REPAIR)
        {
            if (fwd_rt->hcnt > pre_repair_hcnt)
            {
                RERR *rerr;
                u_int8_t rerr_flags = 0;
                struct in_addr dest;

                dest.s_addr = AODV_BROADCAST;
                rerr_flags |= RERR_NODELETE;

#ifdef OMNETPP
                if (fwd_rt->nprec)
                {
#endif
                    rerr = rerr_create(rerr_flags, fwd_rt->dest_addr,
                                       fwd_rt->dest_seqno);
                    rerr->ttl=1;
                    if (fwd_rt->nprec)
                        aodv_socket_send((AODV_msg *) rerr, dest,
                                         RERR_CALC_SIZE(rerr), 1,
                                         &DEV_IFINDEX(fwd_rt->ifindex));
#ifdef OMNETPP
                }
#endif
            }
        }
    }
    else
    {
        /* --- Here we FORWARD the RREP on the REVERSE route --- */
        if (rev_rt && rev_rt->state == VALID)
        {
#ifndef OMNETPP
            rrep_forward(rrep, rreplen, rev_rt, fwd_rt, --ip_ttl);
#else
            rrep_forward(rrep, rreplen, rev_rt, fwd_rt, ip_ttl); // the ttl is decremented for ip layer
#endif
        }
        else
        {
            DEBUG(LOG_DEBUG, 0, "Could not forward RREP - NO ROUTE!!!");
        }
    }

    if (!llfeedback && optimized_hellos)
        hello_start();
}

/************************************************************************/

/* Include a Hello Interval Extension on the RREP and return new offset */

int rrep_add_hello_ext(RREP * rrep, int offset, u_int32_t interval)
{
    AODV_ext *ext;
#ifndef OMNETPP
    ext = (AODV_ext *) ((char *) rrep + RREP_SIZE + offset);
    ext->type = RREP_HELLO_INTERVAL_EXT;
    ext->length = sizeof(interval);
    memcpy(AODV_EXT_DATA(ext), &interval, sizeof(interval));
#else
    ext = rrep->addExtension(RREP_HELLO_INTERVAL_EXT,sizeof(interval),(char*)&interval);
#endif
    return (offset + AODV_EXT_SIZE(ext));
}

