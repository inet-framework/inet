//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "IPAddressResolver.h"

IPAddress IPAddressResolver::resolve(const char *str)
{
    // handle empty address and dotted decimal notation
    if (!*str)
        return IPAddress();
    if (isdigit(*str))
        return IPAddress(str);

    // handle module name
    cModule *mod = simulation.moduleByPath(str);
    if (!mod)
        opp_error("IPAddressResolver: module `%s' not found", str);

    return addressOf(mod);
}

IPAddress IPAddressResolver::addressOf(cModule *host)
{
    RoutingTable *rt = routingTableOf(host);
    return getAddressFrom(rt);
}

IPAddress IPAddressResolver::getAddressFrom(RoutingTable *rt)
{
    // browse interfaces: for the purposes of this function, all of them should
    // share the same IP address
    IPAddress addr;
    if (rt->numInterfaces()==0)
        opp_error("IPAddressResolver: routing table `%s' has no interface registered "
                  "(yet? try in a later init stage!)", rt->fullPath().c_str());

    for (int i=0; i<rt->numInterfaces(); i++)
    {
        InterfaceEntry *e = rt->interfaceById(i);
        if (!e->inetAddr.isNull() && !e->loopback)
        {
            if (!addr.isNull() && e->inetAddr!=addr)
                opp_error("IPAddressResolver: IP address is ambiguous: different "
                          "interfaces in `%s' have different IP addresses",
                          rt->fullPath().c_str());
            addr = e->inetAddr;
        }
    }

    if (addr.isNull())
        opp_error("IPAddressResolver: no interface in `%s' has an IP address "
                  "assigned (yet? try in a later init stage!)", rt->fullPath().c_str());

    return addr;
}

RoutingTable *IPAddressResolver::routingTableOf(cModule *host)
{
    // find RoutingTable
    cModule *rtmod = host->submodule("routingTable");
    if (!rtmod)
        rtmod = host->moduleByRelativePath("networkLayer.routingTable");
    if (!rtmod)
        opp_error("IPAddressResolver: RoutingTable not found as `routingTable' or "
                  "`networkLayer.routingTable' within host/router module `%s'",
                  host->fullPath().c_str());
    return check_and_cast<RoutingTable *>(rtmod);
}

