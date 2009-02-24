//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __INET_IPADDRESSRESOLVER_H
#define __INET_IPADDRESSRESOLVER_H

#include <omnetpp.h>
#include "IPvXAddress.h"

class IInterfaceTable;
class InterfaceEntry;
class IRoutingTable;
class RoutingTable6;
class NotificationBoard;


/**
 * Utility class for finding IPv4 or IPv6 address of a host or router.
 *
 * Syntax variations understood:
 *    - literal IPv4 address: "186.54.66.2"
 *    - literal IPv6 address: "3011:7cd6:750b:5fd6:aba3:c231:e9f9:6a43"
 *    - module name: "server", "subnet.server[3]"
 *    - interface of a host or router: "server/eth0", "subnet.server[3]/eth0"
 *    - IPv4 or IPv6 address of a host or router: "server(ipv4)",
 *      "subnet.server[3](ipv6)"
 *    - IPv4 or IPv6 address of an interface of a host or router:
 *      "server/eth0(ipv4)", "subnet.server[3]/eth0(ipv6)"
 *    - routerId: "router1/routerId", "R1/routerId"
 */
class INET_API IPAddressResolver
{
  protected:
    // internal
    virtual IPAddress getIPv4AddressFrom(IInterfaceTable *ift);
    // internal
    virtual IPv6Address getIPv6AddressFrom(IInterfaceTable *ift);
    // internal
    //virtual IPv6Address getIPv6AddressFrom(IInterfaceTable *ift, int scope);
    // internal
    virtual IPv6Address getInterfaceIPv6Address(InterfaceEntry *ie);

  public:
    enum {
        ADDR_PREFER_IPv4,
        ADDR_PREFER_IPv6,
        ADDR_IPv4,
        ADDR_IPv6
    };

  public:
    IPAddressResolver() {}
    virtual ~IPAddressResolver() {}

    /**
     * Accepts dotted decimal notation ("127.0.0.1"), module name of the host
     * or router ("host[2]"), and empty string (""). For the latter, it returns
     * the null address. If module name is specified, the module will be
     * looked up using <tt>simulation.getModuleByPath()</tt>, and then
     * addressOf() will be called to determine its IP address.
     */
    virtual IPvXAddress resolve(const char *str, int addrType=ADDR_PREFER_IPv6);

    /**
     * Similar to resolve(), but returns false (instead of throwing an error)
     * if the address cannot be resolved because the given host (or interface)
     * doesn't have an address assigned yet. (It still throws an error
     * on any other error condition).
     */
    virtual bool tryResolve(const char *str, IPvXAddress& result, int addrType=ADDR_PREFER_IPv6);

    /** @name Utility functions supporting resolve() */
    //@{
    /**
     * Returns IPv4 or IPv6 address of the given host or router.
     *
     * This function uses routingTableOf() to find the IRoutingTable module,
     * then invokes getAddressFrom() to extract the IP address.
     */
    virtual IPvXAddress addressOf(cModule *host, int addrType=ADDR_PREFER_IPv6);

    /**
     * Similar to addressOf(), but only looks at the given interface
     */
    virtual IPvXAddress addressOf(cModule *host, const char *ifname, int addrType=ADDR_PREFER_IPv6);

    /**
     * Returns the router Id of the given router. Router Id is obtained from
     * the getRouterId() method of the IRoutingTable submodule.
     */
    virtual IPAddress routerIdOf(cModule *host);

    /**
     * Returns the IPv4 or IPv6 address of the given host or router, given its IInterfaceTable
     * module. For IPv4, the first usable interface address is chosen.
     */
    virtual IPvXAddress getAddressFrom(IInterfaceTable *ift, int addrType=ADDR_PREFER_IPv6);

    /**
     * Returns the IPv4 or IPv6 address of the given interface (of a host or router).
     */
    virtual IPvXAddress getAddressFrom(InterfaceEntry *ie, int addrType=ADDR_PREFER_IPv6);

    /**
     * The function tries to look up the IInterfaceTable module as submodule
     * <tt>"interfaceTable"</tt> or <tt>"networkLayer.interfaceTable"</tt> within
     * the host/router module. Throws an error if not found.
     */
    virtual IInterfaceTable *interfaceTableOf(cModule *host);

    /**
     * The function tries to look up the IRoutingTable module as submodule
     * <tt>"routingTable"</tt> or <tt>"networkLayer.routingTable"</tt> within
     * the host/router module. Throws an error if not found.
     */
    virtual IRoutingTable *routingTableOf(cModule *host);

#ifndef WITHOUT_IPv6
    /**
     * The function tries to look up the RoutingTable6 module as submodule
     * <tt>"routingTable6"</tt> or <tt>"networkLayer.routingTable6"</tt> within
     * the host/router module. Throws an error if not found.
     */
    virtual RoutingTable6 *routingTable6Of(cModule *host);
#endif

    /**
     * The function tries to look up the NotificationBoard module as submodule
     * <tt>"notificationBoard"</tt> within the host/router module. Throws an error
     * if not found.
     */
    virtual NotificationBoard *notificationBoardOf(cModule *host);

    /**
     * Like interfaceTableOf(), but doesn't throw error if not found.
     */
    virtual IInterfaceTable *findInterfaceTableOf(cModule *host);

    /**
     * Like routingTableOf(), but doesn't throw error if not found.
     */
    virtual IRoutingTable *findRoutingTableOf(cModule *host);

#ifndef WITHOUT_IPv6
    /**
     * Like interfaceTableOf(), but doesn't throw error if not found.
     */
    virtual RoutingTable6 *findRoutingTable6Of(cModule *host);
#endif

    /**
     * Like notificationBoardOf(), but doesn't throw error if not found.
     */
    virtual NotificationBoard *findNotificationBoardOf(cModule *host);
    //@}
};


#endif


