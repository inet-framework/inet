//
// Copyright (C) 2001  Vincent Oberle (vincent@oberle.com)
// Institute of Telematics, University of Karlsruhe, Germany.
// University Comillas, Madrid, Spain.
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


#ifndef __IPADDRESSRESOLVER_H
#define __IPADDRESSRESOLVER_H

#include <omnetpp.h>
#include "RoutingTable.h"
#include "IPAddress.h"

/**
 * Utility class for finding IP address of a host/router.
 */
class IPAddressResolver
{
  public:
    IPAddressResolver() {}
    ~IPAddressResolver() {}

    /**
     * Accepts dotted decimal notation ("127.0.0.1"), module name of the host
     * or router ("host[2]"), and empty string (""). For the latter, it returns
     * the null address. If module name is specified, the module will be
     * looked up using <tt>simulation.moduleByPath()</tt>, and then
     * addressOf() will be called to determine its IP address.
     */
    IPAddress resolve(const char *str);

    /** @name Utility functions supporting resolve() */
    //@{
    /**
     * Returns IP address of the given host or router. If different interfaces
     * of the host/router have different IP addresses, the function throws
     * an error.
     *
     * This function uses routingTableOf() to find the RoutingTable module,
     * then invokes getAddressFrom() to extract the IP address.
     */
    IPAddress addressOf(cModule *host);

    /**
     * Returns the IP address of the given host or router, given its RoutingTable
     * module. If different interfaces have different IP addresses, the function
     * throws an error.
     */
    IPAddress getAddressFrom(RoutingTable *rt);

    /**
     * The function tries to look up the RoutingTable module as submodule
     * <tt>"routingTable"</tt> or <tt>"networkLayer.routingTable"</tt> within
     * the host/router module. Throws an error if not found.
     */
    RoutingTable *routingTableOf(cModule *host);
    //@}
};


#endif


