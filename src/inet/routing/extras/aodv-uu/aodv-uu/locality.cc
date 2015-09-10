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
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#endif
#else
#include <netdb.h>
extern int h_errno;

#include "inet/routing/extras/aodv-uu/aodv-uu/locality.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/defs_aodv.h"
#include "inet/routing/extras/aodv-uu/aodv-uu/debug_aodv.h"

extern int gw_prefix;
#endif

namespace inet {

namespace inetmanet {

int NS_CLASS locality(struct in_addr dest, unsigned int ifindex)
{

#ifndef NS_PORT
    if (gw_prefix)
    {
        if ((dest.s_addr & DEV_IFINDEX(ifindex).netmask.s_addr) ==
                (DEV_IFINDEX(ifindex).ipaddr.s_addr & DEV_IFINDEX(ifindex).netmask.
                 s_addr))
            return HOST_ADHOC;
        else
            return HOST_INET;

    }
    else
    {
        struct hostent *hent;

        hent = gethostbyaddr(&dest, sizeof(struct in_addr), AF_INET);

        if (!hent)
        {
            switch (h_errno)
            {
            case HOST_NOT_FOUND:
                DEBUG(LOG_DEBUG, 0, "RREQ for Non-Internet dest %s",
                      ip_to_str(dest));
                return HOST_UNKNOWN;
            default:
                DEBUG(LOG_DEBUG, 0, "Unknown DNS error");
                break;

            }
        }
        else
            return HOST_INET;
    }
#else
#ifndef OMNETPP
    char *dstnet = L3Address::getInstance().get_subnetaddr(dest.s_addr);
    char *subnet =
        L3Address::getInstance().get_subnetaddr(DEV_NR(NS_DEV_NR).ipaddr.s_addr);
    DEBUG(LOG_DEBUG, 0, "myaddr=%d, dest=%d dstnet=%s subnet=%s",
          DEV_NR(NS_DEV_NR).ipaddr.s_addr, dest.s_addr, dstnet, subnet);
    if (subnet != nullptr)
    {
        if (dstnet != nullptr)
        {
            if (strcmp(dstnet, subnet) != 0)
            {
                delete[]dstnet;
                return HOST_INET;
            }
            delete[]dstnet;
        }
        delete[]subnet;
    }
    assert(dstnet == nullptr);
    return HOST_UNKNOWN;
#else
    InterfaceEntry *   ie;
    ie = getInterfaceEntry (ifindex);
    struct in_addr dstnet;
    struct in_addr subnet;
    struct in_addr interface;

    int masklen = ie->ipv4Data()->getNetmask().getNetmaskLength();
    dstnet.s_addr = L3Address(dest.s_addr).getPrefix(masklen);   // dstnet.s_addr = dest.s_addr & mask.s_addr;
    interface.s_addr = L3Address(ie->ipv4Data()->getIPAddress());
    subnet.s_addr = interface.s_addr.getPrefix(masklen);    //interface.s_addr & mask.s_addr;
    if (!subnet.s_addr.isUnspecified())
    {
        if (!dstnet.s_addr.isUnspecified())
        {
            if (subnet.s_addr==dstnet.s_addr)
                return HOST_ADHOC;
            else
                return HOST_INET;
        }
    }
    return HOST_UNKNOWN;
#endif
#endif
    return HOST_UNKNOWN;
}

} // namespace inetmanet

} // namespace inet


