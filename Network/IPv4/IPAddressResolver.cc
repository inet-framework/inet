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


#include "RoutingTable.h"
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
    cModule *rtmod = mod->moduleByRelativePath("networkLayer.routingTable");
    if (!rtmod)
        opp_error("IPAddressResolver: RoutingTable not found as networkLayer.routingTable in module `%s'", str);
    RoutingTable *rt = check_and_cast<RoutingTable *>(rtmod);

    InterfaceEntry *e = rt->interfaceByIndex(0);
    if (!e)
        opp_error("IPAddressResolver: module `%s' has no interface registered (try in a later init stage!)", str);
    if (e->inetAddr.isNull())
        opp_error("IPAddressResolver: first interface of module `%s' has no IP address assigned yet (try in a later init stage!)", str);

    // got it, finally!
    return e->inetAddr;
}


