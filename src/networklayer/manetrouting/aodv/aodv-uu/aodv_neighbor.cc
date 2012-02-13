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
 * Authors: Erik Nordström, <erik.nordstrom@it.uu.se>
 *
 *****************************************************************************/
#define NS_PORT
#define OMNETPP

#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/aodv-uu.h"
#else
#include "../aodv_uu_omnet.h"
#endif
#else
#include "aodv_neighbor.h"
#include "aodv_rerr.h"
#include "aodv_hello.h"
#include "aodv_socket.h"
#include "routing_table.h"
#include "params.h"
#include "defs_aodv.h"
#include "debug_aodv.h"

extern int llfeedback;
#endif              /* NS_PORT */


/* Add/Update neighbor from a non HELLO AODV control message... */
void NS_CLASS neighbor_add(AODV_msg * aodv_msg, struct in_addr source,
                           unsigned int ifindex)
{
    struct timeval now;
    rt_table_t *rt = NULL;
    u_int32_t seqno = 0;
    uint32_t cost;
    uint8_t fixhop;

    gettimeofday(&now, NULL);

    rt = rt_table_find(source);

    cost = costMobile;
    if (aodv_msg->prevFix)
    {
        fixhop=1;
        cost =  costStatic;
    }

    if (this->isStaticNode())
        fixhop++;


    if (!rt)
    {
        DEBUG(LOG_DEBUG, 0, "%s new NEIGHBOR!", ip_to_str(source));
        rt = rt_table_insert(source, source, 1, 0,
                             ACTIVE_ROUTE_TIMEOUT, VALID, 0, ifindex, cost, fixhop);
    }
    else
    {
        /* Don't update anything if this is a uni-directional link... */
        if (rt->flags & RT_UNIDIR)
            return;

        if (rt->dest_seqno != 0)
            seqno = rt->dest_seqno;

        rt_table_update(rt, source, 1, seqno, ACTIVE_ROUTE_TIMEOUT,
                        VALID, rt->flags, ifindex, cost, fixhop);
    }

    if (!llfeedback && rt->hello_timer.used)
        hello_update_timeout(rt, &now, ALLOWED_HELLO_LOSS * HELLO_INTERVAL);

    return;
}

#ifdef AODV_USE_STL_RT


void NS_CLASS neighbor_link_break(rt_table_t * rt)
{
    /* If hopcount = 1, this is a direct neighbor and a link break has
       occured. Send a RERR with the incremented sequence number */
    RERR *rerr = NULL;
    rt_table_t *rt_u;
    struct in_addr rerr_unicast_dest;
    int i;

    rerr_unicast_dest.s_addr = 0;

    if (!rt)
        return;

    if (rt->hcnt != 1)
    {
        DEBUG(LOG_DEBUG, 0, "%s is not a neighbor, hcnt=%d!!!",
              ip_to_str(rt->dest_addr), rt->hcnt);
        return;
    }

    DEBUG(LOG_DEBUG, 0, "Link %s down!", ip_to_str(rt->dest_addr));

    /* Invalidate the entry of the route that broke or timed out... */
    rt_table_invalidate(rt);

    /* Create a route error msg, unless the route is to be repaired */
    if (rt->nprec && !(rt->flags & RT_REPAIR))
    {
        rerr = rerr_create(0, rt->dest_addr, rt->dest_seqno);
        DEBUG(LOG_DEBUG, 0, "Added %s as unreachable, seqno=%lu",
              ip_to_str(rt->dest_addr), rt->dest_seqno);

        if (rt->nprec == 1)
            rerr_unicast_dest = rt->precursors[0].neighbor;
    }

    /* Purge precursor list: */
    if (!(rt->flags & RT_REPAIR))
        precursor_list_destroy(rt);

    /* Check the routing table for entries which have the unreachable
       destination (dest) as next hop. These entries (destinations)
       cannot be reached either since dest is down. They should
       therefore also be included in the RERR. */
    for (AodvRtTableMap::iterator it = aodvRtTableMap.begin(); it != aodvRtTableMap.end(); it++)
    {
        rt_table_t *rt_u = it->second;;
        if (rt_u->state == VALID &&
                rt_u->next_hop.s_addr == rt->dest_addr.s_addr &&
                rt_u->dest_addr.s_addr != rt->dest_addr.s_addr)
        {
            /* If the link that broke are marked for repair,
             *                    then do the same for all additional unreachable
             *                                       destinations. */

            if ((rt->flags & RT_REPAIR) && rt_u->hcnt <= MAX_REPAIR_TTL)
            {
                rt_u->flags |= RT_REPAIR;
                DEBUG(LOG_DEBUG, 0, "Marking %s for REPAIR",
                        ip_to_str(rt_u->dest_addr));
                rt_table_invalidate(rt_u);
                continue;
            }
            rt_table_invalidate(rt_u);
            if (!rt_u->precursors.empty())
            {
                if (!rerr)
                {
                    rerr = rerr_create(0, rt_u->dest_addr, rt_u->dest_seqno);
                    if (rt_u->precursors.size() == 1)
                        rerr_unicast_dest = rt_u->precursors[0].neighbor;
                        DEBUG(LOG_DEBUG, 0,
                              "Added %s as unreachable, seqno=%lu",
                              ip_to_str(rt_u->dest_addr), rt_u->dest_seqno);
                }
                else
                {
                    /* Decide whether new precursors make this a non unicast RERR */
                    rerr_add_udest(rerr, rt_u->dest_addr, rt_u->dest_seqno);
                    if (rerr_unicast_dest.s_addr)
                    {
                        for (unsigned int i = 0; i< rt_u->precursors.size(); i++)
                        {
                            precursor_t pr = rt_u->precursors[i];
                            if (pr.neighbor.s_addr != rerr_unicast_dest.s_addr)
                            {
                                rerr_unicast_dest.s_addr = 0;
                                break;
                            }
                        }
                    }
                    DEBUG(LOG_DEBUG, 0, "Added %s as unreachable, seqno=%lu",ip_to_str(rt_u->dest_addr), rt_u->dest_seqno);
                }
            }
            precursor_list_destroy(rt_u);
        }
    }

    if (rerr)
    {
        DEBUG(LOG_DEBUG, 0, "RERR created, %d bytes.", RERR_CALC_SIZE(rerr));

        rt_u = rt_table_find(rerr_unicast_dest);

        if (rt_u && rerr->dest_count == 1 && (rerr_unicast_dest.s_addr!=0))
            aodv_socket_send((AODV_msg *) rerr,
                             rerr_unicast_dest,
                             RERR_CALC_SIZE(rerr), 1,
                             &DEV_IFINDEX(rt_u->ifindex));

        else if (rerr->dest_count > 0)
        {
            /* FIXME: Should only transmit RERR on those interfaces
             *       * which have precursor nodes for the broken route */
            double delay = -1;
            if (par("EqualDelay"))
                delay = par("broadcastDelay");
            int cont = getNumWlanInterfaces();
            for (i = 0; i < MAX_NR_INTERFACES; i++)
            {
                struct in_addr dest;
                if (!DEV_NR(i).enabled)
                    continue;
                dest.s_addr = AODV_BROADCAST;
                if (cont>1)
                    aodv_socket_send((AODV_msg *) rerr->dup(), dest,
                                     RERR_CALC_SIZE(rerr), 1, &DEV_NR(i),delay);
                else
                    aodv_socket_send((AODV_msg *) rerr, dest,
                                     RERR_CALC_SIZE(rerr), 1, &DEV_NR(i),delay);
                cont--;
            }
        }
        else
            delete rerr;
    }
}
#else

void NS_CLASS neighbor_link_break(rt_table_t * rt)
{
    /* If hopcount = 1, this is a direct neighbor and a link break has
       occured. Send a RERR with the incremented sequence number */
    RERR *rerr = NULL;
    rt_table_t *rt_u;
    struct in_addr rerr_unicast_dest;
    int i;

    rerr_unicast_dest.s_addr = 0;

    if (!rt)
        return;

    if (rt->hcnt != 1)
    {
        DEBUG(LOG_DEBUG, 0, "%s is not a neighbor, hcnt=%d!!!",
              ip_to_str(rt->dest_addr), rt->hcnt);
        return;
    }

    DEBUG(LOG_DEBUG, 0, "Link %s down!", ip_to_str(rt->dest_addr));

    /* Invalidate the entry of the route that broke or timed out... */
    rt_table_invalidate(rt);

    /* Create a route error msg, unless the route is to be repaired */
    if (rt->nprec && !(rt->flags & RT_REPAIR))
    {
        rerr = rerr_create(0, rt->dest_addr, rt->dest_seqno);
        DEBUG(LOG_DEBUG, 0, "Added %s as unreachable, seqno=%lu",
              ip_to_str(rt->dest_addr), rt->dest_seqno);

        if (rt->nprec == 1)
            rerr_unicast_dest = FIRST_PREC(rt->precursors)->neighbor;
    }

    /* Purge precursor list: */
    if (!(rt->flags & RT_REPAIR))
        precursor_list_destroy(rt);

    /* Check the routing table for entries which have the unreachable
       destination (dest) as next hop. These entries (destinations)
       cannot be reached either since dest is down. They should
       therefore also be included in the RERR. */
    for (i = 0; i < RT_TABLESIZE; i++)
    {
        list_t *pos;
        list_foreach(pos, &rt_tbl.tbl[i])
        {
            rt_table_t *rt_u = (rt_table_t *) pos;

            if (rt_u->state == VALID &&
                    rt_u->next_hop.s_addr == rt->dest_addr.s_addr &&
                    rt_u->dest_addr.s_addr != rt->dest_addr.s_addr)
            {

                /* If the link that broke are marked for repair,
                   then do the same for all additional unreachable
                   destinations. */

                if ((rt->flags & RT_REPAIR) && rt_u->hcnt <= MAX_REPAIR_TTL)
                {

                    rt_u->flags |= RT_REPAIR;
                    DEBUG(LOG_DEBUG, 0, "Marking %s for REPAIR",
                          ip_to_str(rt_u->dest_addr));

                    rt_table_invalidate(rt_u);
                    continue;
                }

                rt_table_invalidate(rt_u);

                if (rt_u->nprec)
                {

                    if (!rerr)
                    {
                        rerr =
                            rerr_create(0, rt_u->dest_addr, rt_u->dest_seqno);

                        if (rt_u->nprec == 1)
                            rerr_unicast_dest =
                                FIRST_PREC(rt_u->precursors)->neighbor;

                        DEBUG(LOG_DEBUG, 0,
                              "Added %s as unreachable, seqno=%lu",
                              ip_to_str(rt_u->dest_addr), rt_u->dest_seqno);
                    }
                    else
                    {
                        /* Decide whether new precursors make this a non unicast
                           RERR */
                        rerr_add_udest(rerr, rt_u->dest_addr, rt_u->dest_seqno);

                        if (rerr_unicast_dest.s_addr)
                        {
                            list_t *pos2;
                            list_foreach(pos2, &rt_u->precursors)
                            {
                                precursor_t *pr = (precursor_t *) pos2;
                                if (pr->neighbor.s_addr !=
                                        rerr_unicast_dest.s_addr)
                                {
                                    rerr_unicast_dest.s_addr = 0;
                                    break;
                                }
                            }
                        }
                        DEBUG(LOG_DEBUG, 0,
                              "Added %s as unreachable, seqno=%lu",
                              ip_to_str(rt_u->dest_addr), rt_u->dest_seqno);
                    }
                }
                precursor_list_destroy(rt_u);
            }
        }
    }

    if (rerr)
    {
        DEBUG(LOG_DEBUG, 0, "RERR created, %d bytes.", RERR_CALC_SIZE(rerr));

        rt_u = rt_table_find(rerr_unicast_dest);

        if (rt_u && rerr->dest_count == 1 && (rerr_unicast_dest.s_addr!=0))
            aodv_socket_send((AODV_msg *) rerr,
                             rerr_unicast_dest,
                             RERR_CALC_SIZE(rerr), 1,
                             &DEV_IFINDEX(rt_u->ifindex));

        else if (rerr->dest_count > 0)
        {
            /* FIXME: Should only transmit RERR on those interfaces
             *       * which have precursor nodes for the broken route */
#ifdef OMNETPP
            double delay = -1;
            if (par("EqualDelay"))
                delay = par("broadcastDelay");
            int cont = getNumWlanInterfaces();
#endif
            for (i = 0; i < MAX_NR_INTERFACES; i++)
            {
                struct in_addr dest;
                if (!DEV_NR(i).enabled)
                    continue;
                dest.s_addr = AODV_BROADCAST;
#ifdef OMNETPP
                if (cont>1)
                    aodv_socket_send((AODV_msg *) rerr->dup(), dest,
                                     RERR_CALC_SIZE(rerr), 1, &DEV_NR(i),delay);
                else
                    aodv_socket_send((AODV_msg *) rerr, dest,
                                     RERR_CALC_SIZE(rerr), 1, &DEV_NR(i),delay);
                cont--;
#else
                aodv_socket_send((AODV_msg *) rerr, dest,
                                 RERR_CALC_SIZE(rerr), 1, &DEV_NR(i));
#endif
            }
        }
#ifdef OMNETPP
        else
            delete rerr;
#endif
    }
}
#endif

