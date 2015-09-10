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
#include "inet/routing/extras/dymo/dymoum/dymo_generic.h"
#include "inet/routing/extras/dymo/dymoum/dymo_re.h"
#include "inet/routing/extras/dymo/dymoum/dymo_uerr.h"
#include "inet/routing/extras/dymo/dymoum/dymo_rerr.h"
#include "inet/routing/extras/dymo/dymoum/dymo_hello.h"
#include "inet/routing/extras/dymo/dymoum/defs_dymo.h"
#include "inet/routing/extras/dymo/dymoum/debug_dymo.h"
#ifndef OMNETPP
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include "inet/routing/extras/base/compatibility.h"
#endif
#endif  /* NS_PORT */

namespace inet {

namespace inetmanet {

void NS_CLASS generic_process_message(DYMO_element *e,struct in_addr src, u_int32_t ifindex)
{
    // Generic preprocessing
    generic_preprocess(e);

    switch (e->type)
    {
    case DYMO_RE_TYPE:
        if (((RE *) e)->a)
        {
            dlog(LOG_DEBUG, 0, __FUNCTION__,
                 "RREQ received in %s from %s",
                 DEV_IFINDEX(ifindex).ifname,
                 ip2str(src.s_addr));
        }
        else
            dlog(LOG_DEBUG, 0, __FUNCTION__,
                 "RREP received in %s from %s",
                 DEV_IFINDEX(ifindex).ifname,
                 ip2str(src.s_addr));
        re_process((RE *) e, src, ifindex);
        break;

    case DYMO_RERR_TYPE:
        dlog(LOG_DEBUG, 0, __FUNCTION__,
             "RERR received in %s from %s",
             DEV_IFINDEX(ifindex).ifname,
             ip2str(src.s_addr));
        rerr_process((RERR *) e, src, ifindex);
        break;

    case DYMO_UERR_TYPE:
        dlog(LOG_DEBUG, 0, __FUNCTION__,
             "UERR received in %s from %s",
             DEV_IFINDEX(ifindex).ifname,
             ip2str(src.s_addr));
        uerr_process((UERR *) e, src, ifindex);
        break;

    case DYMO_HELLO_TYPE:
        dlog(LOG_DEBUG, 0, __FUNCTION__,
             "HELLO received in %s from %s",
             DEV_IFINDEX(ifindex).ifname,
             ip2str(src.s_addr));
        hello_process((HELLO *) e, src, ifindex);
        break;
#ifdef NS_PORT
#ifndef OMNETPP
    case DYMO_ECHOREPLY_TYPE:
        dlog(LOG_DEBUG, 0, __FUNCTION__,
             "ICMP ECHOREPLY received in %s from %s",
             DEV_IFINDEX(ifindex).ifname,
             ip2str(src.s_addr));
        icmp_process(src);
        break;
#endif
#endif  /* NS_PORT */

    default:
        dlog(LOG_DEBUG, 0, __FUNCTION__,
             "unknown msg type %u received in %s from %s",
             e->type,
             DEV_IFINDEX(ifindex).ifname,
             ip2str(src.s_addr));

        if (e->m)
            uerr_send(e, ifindex);
        switch (e->h)
        {
        case 0: // H = 00
            // TODO: we should skip this element from the
            // packet and continue the packet processing,
            // but we are only dealing with 1-element
            // packets. Thus the packet is dropped
            break;

        case 1: // H = 01
            // TODO: we should remove this element from the
            // packet and continue the processing
            break;

        case 2: // H = 10: set I bit
            e->i = 1;
            // TODO: continue processing
            break;

        case 3: // H = 11: drop the packet
            break;
        }
#ifdef OMNETPP
        delete e;
        e=nullptr;
#endif
    }
}

} // namespace inetmanet

} // namespace inet

