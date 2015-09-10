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

#ifndef __INET_L3ADDRESSRESOLVER_H
#define __INET_L3ADDRESSRESOLVER_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IRoutingTable.h"

namespace inet {

// Forward declarations:
class IInterfaceTable;
class InterfaceEntry;
class IIPv4RoutingTable;
class IPv6RoutingTable;
class GenericRoutingTable;

#define DEFAULT_ADDR_TYPE    (ADDR_IPv4 | ADDR_IPv6 | ADDR_MAC | ADDR_MODULEPATH | ADDR_MODULEID)

/**
 * Utility class for finding IPv4 or IPv6 address of a host or router.
 *
 * Syntax variations understood:
 *    - literal IPv4 address: "186.54.66.2"
 *    - literal IPv6 address: "3011:7cd6:750b:5fd6:aba3:c231:e9f9:6a43"
 *    - module name: "server", "subnet.server[3]"
 *    - interface of a host or router: "server%eth0", "subnet.server[3]%eth0"
 *    - IPv4 or IPv6 address of a host or router: "server(ipv4)",
 *      "subnet.server[3](ipv6)"
 *    - IPv4 or IPv6 address of an interface of a host or router:
 *      "server%eth0(ipv4)", "subnet.server[3]%eth0(ipv6)"
 *    - routerId: "router1%routerId", "R1%routerId"
 *    - interface of a host or router toward defined another node: "client1>router"
 */
class INET_API L3AddressResolver
{
  protected:
    // internal
    virtual bool getIPv4AddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask);
    // internal
    virtual bool getIPv6AddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask);
    // internal
    virtual bool getMACAddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask);
    // internal
    virtual bool getModulePathAddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask);
    // internal
    virtual bool getModuleIdAddressFrom(L3Address& retAddr, IInterfaceTable *ift, bool netmask);
    // internal
    virtual bool getInterfaceIPv4Address(L3Address& ret, InterfaceEntry *ie, bool mask);
    // internal
    virtual bool getInterfaceIPv6Address(L3Address& ret, InterfaceEntry *ie, bool mask);
    // internal
    virtual bool getInterfaceMACAddress(L3Address& ret, InterfaceEntry *ie, bool mask);
    // internal
    virtual bool getInterfaceModulePathAddress(L3Address& ret, InterfaceEntry *ie, bool mask);
    // internal
    virtual bool getInterfaceModuleIdAddress(L3Address& ret, InterfaceEntry *ie, bool mask);

  public:
    enum {
        ADDR_IPv4 = 1,
        ADDR_IPv6 = 2,
        ADDR_MAC = 4,
        ADDR_MODULEPATH = 8,
        ADDR_MODULEID = 16,
        ADDR_MASK = 32
    };

  public:
    L3AddressResolver() {}
    virtual ~L3AddressResolver() {}

    /**
     * Accepts dotted decimal notation ("127.0.0.1"), module name of the host
     * or router ("host[2]"), and empty string (""). For the latter, it returns
     * the null address. If module name is specified, the module will be
     * looked up using <tt>getModuleByPath()</tt>, and then
     * addressOf() will be called to determine its IP address.
     */
    virtual L3Address resolve(const char *str, int addrType = DEFAULT_ADDR_TYPE);

    /**
     * Utility function: Calls resolve() for each item in the string vector, and
     * returns the result in an address vector. The string vector may come e.g.
     * from cStringTokenizer::asVector().
     */
    virtual std::vector<L3Address> resolve(std::vector<std::string> strs, int addrType = DEFAULT_ADDR_TYPE);

    /**
     * Similar to resolve(), but returns false (instead of throwing an error)
     * if the address cannot be resolved because the given host (or interface)
     * doesn't have an address assigned yet. (It still throws an error
     * on any other error condition).
     */
    virtual bool tryResolve(const char *str, L3Address& result, int addrType = DEFAULT_ADDR_TYPE);

    /** @name Utility functions supporting resolve() */
    //@{
    /**
     * Returns IPv4 or IPv6 address of the given host or router.
     *
     * This function uses routingTableOf() to find the IIPv4RoutingTable module,
     * then invokes getAddressFrom() to extract the IP address.
     */
    virtual L3Address addressOf(cModule *host, int addrType = DEFAULT_ADDR_TYPE);

    /**
     * Similar to addressOf(), but only looks at the given interface
     */
    virtual L3Address addressOf(cModule *host, const char *ifname, int addrType = DEFAULT_ADDR_TYPE);

    /**
     * Returns IPv4 or IPv6 address of the given host or router.
     *
     * This function find an interface of host connected to destmod
     * then invokes getAddressFrom() to extract the IP address.
     */
    virtual L3Address addressOf(cModule *host, cModule *destmod, int addrType = DEFAULT_ADDR_TYPE);

    /**
     * Returns the router Id of the given router. Router Id is obtained from
     * the getRouterId() method of the IIPv4RoutingTable submodule.
     */
    virtual L3Address routerIdOf(cModule *host);

    /**
     * Returns the IPv4 or IPv6 address of the given host or router, given its IInterfaceTable
     * module. For IPv4, the first usable interface address is chosen.
     */
    virtual L3Address getAddressFrom(IInterfaceTable *ift, int addrType = DEFAULT_ADDR_TYPE);

    /**
     * Returns the IPv4 or IPv6 address of the given interface (of a host or router).
     */
    virtual L3Address getAddressFrom(InterfaceEntry *ie, int addrType = DEFAULT_ADDR_TYPE);

    /**
     * The function tries to look up the IInterfaceTable module as submodule
     * <tt>"interfaceTable"</tt> or <tt>"networkLayer.interfaceTable"</tt> within
     * the host/router module. Throws an error if not found.
     */
    virtual IInterfaceTable *interfaceTableOf(cModule *host);

    /**
     * The function tries to look up the IIPv4RoutingTable module as submodule
     * <tt>"routingTable"</tt> or <tt>"networkLayer.routingTable"</tt> within
     * the host/router module. Throws an error if not found.
     */
    virtual IIPv4RoutingTable *routingTableOf(cModule *host);

    /**
     * The function tries to look up the IPv6RoutingTable module as submodule
     * <tt>"routingTable6"</tt> or <tt>"networkLayer.routingTable6"</tt> within
     * the host/router module. Throws an error if not found.
     */
    virtual IPv6RoutingTable *routingTable6Of(cModule *host);

    /**
     * Like interfaceTableOf(), but doesn't throw error if not found.
     */
    virtual IInterfaceTable *findInterfaceTableOf(cModule *host);

    /**
     * Like routingTableOf(), but doesn't throw error if not found.
     */
    virtual IIPv4RoutingTable *findIPv4RoutingTableOf(cModule *host);

    /**
     * Like interfaceTableOf(), but doesn't throw error if not found.
     */
    virtual IPv6RoutingTable *findIPv6RoutingTableOf(cModule *host);

    /**
     * Like interfaceTableOf(), but doesn't throw error if not found.
     */
    virtual GenericRoutingTable *findGenericRoutingTableOf(cModule *host);

    /**
     * Find the Host with the specified address.
     */
    virtual cModule *findHostWithAddress(const L3Address& addr);
    //@}
};

} // namespace inet

#endif // ifndef __INET_L3ADDRESSRESOLVER_H

