//
// Copyright (C) 2008 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IIPV4ROUTINGTABLE_H
#define __INET_IIPV4ROUTINGTABLE_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"    // not strictly required, but most clients will need it anyway

namespace inet {

/**
 * A C++ interface to abstract the functionality of IIpv4RoutingTable.
 * Referring to IIpv4RoutingTable via this interface makes it possible to
 * transparently replace IIpv4RoutingTable with a different implementation,
 * without any change to the base INET.
 *
 * @see IIpv4RoutingTable, Ipv4Route
 */
class INET_API IIpv4RoutingTable : public IRoutingTable
{
  public:
    virtual ~IIpv4RoutingTable() {};

    /**
     * For debugging
     */
    virtual void printMulticastRoutingTable() const = 0;

    /**
     * Returns the host or router this routing table lives in.
     */
    virtual cModule *getHostModule() = 0;

    /** @name Interfaces */
    //@{
    /**
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByAddress(const Ipv4Address& address) const = 0;
    using IRoutingTable::getInterfaceByAddress;
    //@}

    /**
     * Returns routerId.
     */
    virtual Ipv4Address getRouterId() const = 0;

    /**
     * Sets routerId.
     */
    virtual void setRouterId(Ipv4Address a) = 0;

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const Ipv4Address& dest) const = 0;
    using IRoutingTable::isLocalAddress;

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local network broadcast address, i.e. one of the
     * broadcast addresses derived from the interface addresses and netmasks.
     */
    virtual bool isLocalBroadcastAddress(const Ipv4Address& dest) const = 0;

    /**
     * Returns the interface entry having the specified address
     * as its local broadcast address.
     */
    virtual InterfaceEntry *findInterfaceByLocalBroadcastAddress(const Ipv4Address& dest) const = 0;

    /**
     * The routing function. Performs longest prefix match for the given
     * destination address, and returns the resulting route. Returns nullptr
     * if there is no matching route.
     */
    virtual Ipv4Route *findBestMatchingRoute(const Ipv4Address& dest) const = 0;
    using IRoutingTable::findBestMatchingRoute;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the output interface for the packets with dest as destination
     * address, or nullptr if the destination is not in routing table.
     */
    virtual InterfaceEntry *getInterfaceForDestAddr(const Ipv4Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway for the destination address. Returns the unspecified
     * address if the destination is not in routing table or the gateway field
     * is not filled in in the route.
     */
    virtual Ipv4Address getGatewayForDestAddr(const Ipv4Address& dest) const = 0;
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const Ipv4Address& dest) const = 0;
    using IRoutingTable::isLocalMulticastAddress;

    /**
     * Returns route for a multicast origin and group.
     */
    virtual const Ipv4MulticastRoute *findBestMatchingMulticastRoute(const Ipv4Address& origin, const Ipv4Address& group) const = 0;
    using IRoutingTable::findBestMatchingMulticastRoute;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Returns the kth route.
     */
    virtual Ipv4Route *getRoute(int k) const override = 0;

    /**
     * Finds and returns the default route, or nullptr if it doesn't exist
     */
    virtual Ipv4Route *getDefaultRoute() const override = 0;

    /**
     * Adds a route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addRoute(Ipv4Route *entry) = 0;
    using IRoutingTable::addRoute;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned of the route was not in the routing table.
     */
    virtual Ipv4Route *removeRoute(Ipv4Route *entry) = 0;
    using IRoutingTable::removeRoute;

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(Ipv4Route *entry) = 0;
    using IRoutingTable::deleteRoute;

    /**
     * Returns the kth multicast route.
     */
    virtual Ipv4MulticastRoute *getMulticastRoute(int k) const override = 0;

    /**
     * Adds a multicast route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addMulticastRoute(Ipv4MulticastRoute *entry) = 0;
    using IRoutingTable::addMulticastRoute;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned of the route was not in the routing table.
     */
    virtual Ipv4MulticastRoute *removeMulticastRoute(Ipv4MulticastRoute *entry) = 0;
    using IRoutingTable::removeMulticastRoute;

    /**
     * Deletes the given multicast route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteMulticastRoute(Ipv4MulticastRoute *entry) = 0;
    using IRoutingTable::deleteMulticastRoute;

    /**
     * Deletes invalid routes from the routing table. Invalid routes are those
     * where the isValid() method returns false.
     */
    virtual void purge() = 0;

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    virtual std::vector<Ipv4Address> gatherAddresses() const = 0;

    /**
     * To be called from route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void routeChanged(Ipv4Route *entry, int fieldCode) = 0;

    /**
     * To be called from multicast route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void multicastRouteChanged(Ipv4MulticastRoute *entry, int fieldCode) = 0;
    //@}
};

} // namespace inet

#endif // ifndef __INET_IIPV4ROUTINGTABLE_H

