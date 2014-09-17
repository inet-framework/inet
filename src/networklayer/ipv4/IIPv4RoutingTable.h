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

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4Route.h"    // not strictly required, but most clients will need it anyway
#include "inet/networklayer/common/IRoutingTable.h"

namespace inet {

/**
 * A C++ interface to abstract the functionality of IIPv4RoutingTable.
 * Referring to IIPv4RoutingTable via this interface makes it possible to
 * transparently replace IIPv4RoutingTable with a different implementation,
 * without any change to the base INET.
 *
 * @see IIPv4RoutingTable, IPv4Route
 */
class INET_API IIPv4RoutingTable : public IRoutingTable
{
  public:
    virtual ~IIPv4RoutingTable() {};

    /**
     * For debugging
     */
    virtual void printRoutingTable() const = 0;

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
    virtual void configureInterfaceForIPv4(InterfaceEntry *ie) = 0;

    /**
     * Returns an interface given by its address. Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByAddress(const IPv4Address& address) const = 0;
    //@}

    /**
     * IPv4 forwarding on/off
     */
    virtual bool isForwardingEnabled() const = 0;

    /**
     * IPv4 multicast forwarding on/off
     */
    virtual bool isMulticastForwardingEnabled() const = 0;

    /**
     * Returns routerId.
     */
    virtual IPv4Address getRouterId() const = 0;

    /**
     * Sets routerId.
     */
    virtual void setRouterId(IPv4Address a) = 0;

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const IPv4Address& dest) const = 0;

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local network broadcast address, i.e. one of the
     * broadcast addresses derived from the interface addresses and netmasks.
     */
    virtual bool isLocalBroadcastAddress(const IPv4Address& dest) const = 0;

    /**
     * Returns the interface entry having the specified address
     * as its local broadcast address.
     */
    virtual InterfaceEntry *findInterfaceByLocalBroadcastAddress(const IPv4Address& dest) const = 0;

    /**
     * The routing function. Performs longest prefix match for the given
     * destination address, and returns the resulting route. Returns NULL
     * if there is no matching route.
     */
    virtual IPv4Route *findBestMatchingRoute(const IPv4Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the output interface for the packets with dest as destination
     * address, or NULL if the destination is not in routing table.
     */
    virtual InterfaceEntry *getInterfaceForDestAddr(const IPv4Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway for the destination address. Returns the unspecified
     * address if the destination is not in routing table or the gateway field
     * is not filled in in the route.
     */
    virtual IPv4Address getGatewayForDestAddr(const IPv4Address& dest) const = 0;
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const IPv4Address& dest) const = 0;

    /**
     * Returns route for a multicast origin and group.
     */
    virtual const IPv4MulticastRoute *findBestMatchingMulticastRoute(const IPv4Address& origin, const IPv4Address& group) const = 0;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Returns the total number of unicast routes.
     */
    virtual int getNumRoutes() const = 0;

    /**
     * Returns the kth route.
     */
    virtual IPv4Route *getRoute(int k) const = 0;

    /**
     * Finds and returns the default route, or NULL if it doesn't exist
     */
    virtual IPv4Route *getDefaultRoute() const = 0;

    /**
     * Adds a route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addRoute(IPv4Route *entry) = 0;

    /**
     * Removes the given route from the routing table, and returns it.
     * NULL is returned of the route was not in the routing table.
     */
    virtual IPv4Route *removeRoute(IPv4Route *entry) = 0;

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(IPv4Route *entry) = 0;

    /**
     * Returns the total number of multicast routes.
     */
    virtual int getNumMulticastRoutes() const = 0;

    /**
     * Returns the kth multicast route.
     */
    virtual IPv4MulticastRoute *getMulticastRoute(int k) const = 0;

    /**
     * Adds a multicast route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addMulticastRoute(IPv4MulticastRoute *entry) = 0;

    /**
     * Removes the given route from the routing table, and returns it.
     * NULL is returned of the route was not in the routing table.
     */
    virtual IPv4MulticastRoute *removeMulticastRoute(IPv4MulticastRoute *entry) = 0;

    /**
     * Deletes the given multicast route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteMulticastRoute(IPv4MulticastRoute *entry) = 0;

    /**
     * Deletes invalid routes from the routing table. Invalid routes are those
     * where the isValid() method returns false.
     */
    virtual void purge() = 0;

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    virtual std::vector<IPv4Address> gatherAddresses() const = 0;

    /**
     * To be called from route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void routeChanged(IPv4Route *entry, int fieldCode) = 0;

    /**
     * To be called from multicast route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void multicastRouteChanged(IPv4MulticastRoute *entry, int fieldCode) = 0;
    //@}
};

} // namespace inet

#endif // ifndef __INET_IIPV4ROUTINGTABLE_H

