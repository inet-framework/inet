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

#ifndef __INET_IROUTINGTABLE_H
#define __INET_IROUTINGTABLE_H

#include <vector>

#include "INETDefs.h"

#include "IPv4Address.h"
#include "IPv4Route.h"  // not strictly required, but most clients will need it anyway
#include "IPv4RouteRule.h"


/** Returned by IRoutingTable as the result of multicast routing */
struct MulticastRoute
{
    InterfaceEntry *interf;
    IPv4Address gateway;
};
typedef std::vector<MulticastRoute> MulticastRoutes;


/**
 * A C++ interface to abstract the functionality of IRoutingTable.
 * Referring to IRoutingTable via this interface makes it possible to
 * transparently replace IRoutingTable with a different implementation,
 * without any change to the base INET.
 *
 * @see IRoutingTable, IPv4Route
 */
class INET_API IRoutingTable
{
  public:
    virtual ~IRoutingTable() {};

    /**
     * For debugging
     */
    virtual void printRoutingTable() const = 0;

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
    virtual bool isIPForwardingEnabled() = 0;

    /**
     * Returns routerId.
     */
    virtual IPv4Address getRouterId() = 0;

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
	 * Checks if the address is a local broadcast one, i.e. one 192.168.0.255/24
	 */
	virtual bool isLocalBroadcastAddress(const IPv4Address& dest) const = 0;

    /**
     * The routing function.
     */
    virtual const IPv4Route *findBestMatchingRoute(const IPv4Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the interface Id to send the packets with dest as
     * destination address, or -1 if destination is not in routing table.
     */
    virtual InterfaceEntry *getInterfaceForDestAddr(const IPv4Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway to send the destination. Returns null address
     * if the destination is not in routing table or there is
     * no gateway (local delivery).
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
     * Returns routes for a multicast address.
     */
    virtual MulticastRoutes getMulticastRoutesFor(const IPv4Address& dest) const = 0;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Returns the total number of routes (unicast, multicast, plus the
     * default route).
     */
    virtual int getNumRoutes() const = 0;

    /**
     * Returns the kth route. The returned route cannot be modified;
     * you must delete and re-add it instead. This rule is emphasized
     * by returning a const pointer.
     */
    virtual const IPv4Route *getRoute(int k) const = 0;

    /**
     * Finds and returns the default route, or NULL if it doesn't exist
     */
    virtual const IPv4Route *findRoute(const IPv4Address& target, const IPv4Address& netmask,
        const IPv4Address& gw, int metric = 0, const char *dev = NULL) const = 0;

    /**
     * Finds and returns the default route, or NULL if it doesn't exist
     */
    virtual const IPv4Route *getDefaultRoute() const = 0;

    /**
     * Adds a route to the routing table. Note that once added, routes
     * cannot be modified; you must delete and re-add them instead.
     */
    virtual void addRoute(const IPv4Route *entry) = 0;

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted correctly, false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(const IPv4Route *entry) = 0;

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    virtual std::vector<IPv4Address> gatherAddresses() const = 0;
    //@}
   // Dsdv time to live test entry
    virtual void setTimeToLiveRoutingEntry(simtime_t a) = 0;
    virtual simtime_t getTimeToLiveRoutingEntry()=0;
    virtual void dsdvTestAndDelete() = 0;
    virtual const bool testValidity(const IPv4Route *entry) const = 0;
    // Rules (similar to linux iptables)
    virtual void addRule(bool output,IPv4RouteRule *entry) = 0;
    virtual void delRule(IPv4RouteRule *entry) = 0;
    virtual const IPv4RouteRule * getRule(bool output,int index) const =0;
    virtual int getNumRules(bool output)=0;
    virtual const IPv4RouteRule * findRule(bool output, int prot, int sPort,
                                     const IPv4Address &srcAddr, int dPort,
                                     const IPv4Address &destAddr, const InterfaceEntry *) const = 0;
};

#endif

