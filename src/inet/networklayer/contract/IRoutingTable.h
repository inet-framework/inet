//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IROUTINGTABLE_H
#define __INET_IROUTINGTABLE_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IRoute.h"

namespace inet {

/**
 * A C++ interface to abstract the functionality of a routing table, regardless of address type.
 */
class INET_API IRoutingTable
{
  public:
    virtual ~IRoutingTable() {}

    /** @name Miscellaneous functions */
    //@{
    /**
     * Forwarding on/off
     */
    virtual bool isForwardingEnabled() const = 0; // TODO IP modulba?

    /**
     * Administrative distance on/off
     */
    virtual bool isAdminDistEnabled() const = 0;

    /**
     * Multicast forwarding on/off
     */
    virtual bool isMulticastForwardingEnabled() const = 0; // TODO IP modulba?

    /**
     * Returns routerId.
     */
    virtual L3Address getRouterIdAsGeneric() const = 0;

    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const L3Address& dest) const = 0; // TODO maybe into InterfaceTable?

    /**
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual NetworkInterface *getInterfaceByAddress(const L3Address& address) const = 0; // TODO should be find..., see next one

    /**
     * Prints the routing table.
     */
    virtual void printRoutingTable() const = 0;
    //@}

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * The routing function. Performs longest prefix match for the given
     * destination address, and returns the resulting route. Returns nullptr
     * if there is no matching route.
     */
    virtual IRoute *findBestMatchingRoute(const L3Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the output interface for the packets with dest as destination
     * address, or nullptr if the destination is not in routing table.
     */
    virtual NetworkInterface *getOutputInterfaceForDestination(const L3Address& dest) const = 0; // TODO redundant

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway for the destination address. Returns the unspecified
     * address if the destination is not in routing table or the gateway field
     * is not filled in in the route.
     */
    virtual L3Address getNextHopForDestination(const L3Address& dest) const = 0; // TODO redundant AND unused
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const L3Address& dest) const = 0;

    /**
     * Returns route for a multicast origin and group.
     */
    virtual IMulticastRoute *findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const = 0;
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
    virtual IRoute *getRoute(int k) const = 0;

    /**
     * Finds and returns the default route, or nullptr if it doesn't exist
     */
    virtual IRoute *getDefaultRoute() const = 0; // TODO is this a universal concept?

    /**
     * Adds a route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addRoute(IRoute *entry) = 0;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned if the route was not in the routing table.
     */
    virtual IRoute *removeRoute(IRoute *entry) = 0;

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(IRoute *entry) = 0;

    /**
     * Returns the total number of multicast routes.
     */
    virtual int getNumMulticastRoutes() const = 0;

    /**
     * Returns the kth multicast route.
     */
    virtual IMulticastRoute *getMulticastRoute(int k) const = 0;

    /**
     * Adds a multicast route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addMulticastRoute(IMulticastRoute *entry) = 0;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned of the route was not in the routing table.
     */
    virtual IMulticastRoute *removeMulticastRoute(IMulticastRoute *entry) = 0;

    /**
     * Deletes the given multicast route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteMulticastRoute(IMulticastRoute *entry) = 0;
    //@}

    virtual IRoute *createRoute() = 0;
};

} // namespace inet

#endif

