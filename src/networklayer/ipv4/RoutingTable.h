//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2006 Andras Varga
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

//
//  Author:     Jochen Reber
//    Date:       18.5.00
//    On Linux:   19.5.00 - 29.5.00
//  Modified by Vincent Oberle
//    Date:       1.2.2001
//  Cleanup and rewrite: Andras Varga, 2004
//

#ifndef __ROUTINGTABLE_H
#define __ROUTINGTABLE_H

#include <vector>

#include "INETDefs.h"

#include "INotifiable.h"
#include "IPv4Address.h"
#include "IRoutingTable.h"

class IInterfaceTable;
class NotificationBoard;
class RoutingTableParser;


/**
 * Represents the routing table. This object has one instance per host
 * or router. It has methods to manage the route table and the interface table,
 * so one can achieve functionality similar to the "route" and "ifconfig" commands.
 *
 * See the NED documentation for general overview.
 *
 * This is a simple module without gates, it requires function calls to it
 * (message handling does nothing). Methods are provided for reading and
 * updating the interface table and the route table, as well as for unicast
 * and multicast routing.
 *
 * Interfaces are dynamically registered: at the start of the simulation,
 * every L2 module adds its own interface entry to the table.
 *
 * The route table is read from a file (RoutingTableParser); the file can
 * also fill in or overwrite interface settings. The route table can also
 * be read and modified during simulation, typically by routing protocol
 * implementations (e.g. OSPF).
 *
 * Entries in the route table are represented by IPv4Route objects.
 * IPv4Route objects can be polymorphic: if a routing protocol needs
 * to store additional data, it can simply subclass from IPv4Route,
 * and add the derived object to the table.
 *
 * Uses RoutingTableParser to read routing files (.irt, .mrt).
 *
 *
 * @see InterfaceEntry, IPv4InterfaceData, IPv4Route
 */
class INET_API RoutingTable: public cSimpleModule, public IRoutingTable, protected INotifiable
{
  protected:
    IInterfaceTable *ift; // cached pointer
    NotificationBoard *nb; // cached pointer

    IPv4Address routerId;
    bool IPForward;

    // DSDV parameters
    simtime_t timetolive_routing_entry;
    //
    // Routes:
    //
    typedef std::vector<IPv4Route *> RouteVector;
    RouteVector routes;          // Unicast route array
    RouteVector multicastRoutes; // Multicast route array

    // routing cache: maps destination address to the route
    typedef std::map<IPv4Address, const IPv4Route *> RoutingCache;
    mutable RoutingCache routingCache;

    typedef std::vector<IPv4RouteRule *> RoutingRule;
    RoutingRule outputRules;
    RoutingRule inputRules;


    // local addresses cache (to speed up isLocalAddress())
    typedef std::set<IPv4Address> AddressSet;
    mutable AddressSet localAddresses;
    // JcM add: to handle the local broadcast address
    mutable AddressSet localBroadcastAddresses;

  protected:
    // set IPv4 address etc on local loopback
    virtual void configureLoopbackForIPv4();

    // check if a route table entry corresponds to the following parameters
    virtual bool routeMatches(const IPv4Route *entry,
        const IPv4Address& target, const IPv4Address& nmask, const IPv4Address& gw,
        int metric, const char *dev) const;

    // set router Id
    virtual void configureRouterId();

    // adjust routes with src=IFACENETMASK to actual interface netmasks
    virtual void updateNetmaskRoutes();

    // displays summary above the icon
    virtual void updateDisplayString();

    // delete routes for the given interface
    virtual void deleteInterfaceRoutes(InterfaceEntry *entry);

    // invalidates routing cache and local addresses cache
    virtual void invalidateCache();

  public:
    RoutingTable();
    virtual ~RoutingTable();

  protected:
    virtual int numInitStages() const  {return 4;}
    virtual void initialize(int stage);

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *);

    /**
     * Called by the NotificationBoard whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

  public:
    /**
     * For debugging
     */
    virtual void printRoutingTable() const;

    /** @name Interfaces */
    //@{
    virtual void configureInterfaceForIPv4(InterfaceEntry *ie);

    /**
     * Returns an interface given by its address. Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByAddress(const IPv4Address& address) const;
    //@}

    /**
     * IPv4 forwarding on/off
     */
    virtual bool isIPForwardingEnabled()  {return IPForward;}

    /**
     * Returns routerId.
     */
    virtual IPv4Address getRouterId()  {return routerId;}

    /**
     * Sets routerId.
     */
    virtual void setRouterId(IPv4Address a)  {routerId = a;}

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const IPv4Address& dest) const;
    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local broadcast one, i.e. 192.168.0.255/24
     */
    virtual bool isLocalBroadcastAddress(const IPv4Address& dest) const;

    /**
     * The routing function.
     */
    virtual const IPv4Route *findBestMatchingRoute(const IPv4Address& dest) const;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the interface Id to send the packets with dest as
     * destination address, or -1 if destination is not in routing table.
     */
    virtual InterfaceEntry *getInterfaceForDestAddr(const IPv4Address& dest) const;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway to send the destination. Returns null address
     * if the destination is not in routing table or there is
     * no gateway (local delivery).
     */
    virtual IPv4Address getGatewayForDestAddr(const IPv4Address& dest) const;
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const IPv4Address& dest) const;

    /**
     * Returns routes for a multicast address.
     */
    virtual MulticastRoutes getMulticastRoutesFor(const IPv4Address& dest) const;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Returns the total number of routes (unicast, multicast, plus the
     * default route).
     */
    virtual int getNumRoutes() const;

    /**
     * Returns the kth route. The returned route cannot be modified;
     * you must delete and re-add it instead. This rule is emphasized
     * by returning a const pointer.
     */
    virtual const IPv4Route *getRoute(int k) const;

    /**
     * Finds the first route with the given parameters.
     */
    virtual const IPv4Route *findRoute(const IPv4Address& target, const IPv4Address& netmask,
        const IPv4Address& gw, int metric = 0, const char *dev = NULL) const;

    /**
     * Finds and returns the default route, or NULL if it doesn't exist
     */
    virtual const IPv4Route *getDefaultRoute() const;

    /**
     * Adds a route to the routing table. Note that once added, routes
     * cannot be modified; you must delete and re-add them instead.
     */
    virtual void addRoute(const IPv4Route *entry);

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted correctly, false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(const IPv4Route *entry);

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    virtual std::vector<IPv4Address> gatherAddresses() const;
    //@}
    virtual void setTimeToLiveRoutingEntry(simtime_t a){timetolive_routing_entry = a;}
    virtual simtime_t getTimeToLiveRoutingEntry(){return timetolive_routing_entry;}
    // Dsdv time to live test entry
    virtual void dsdvTestAndDelete();
    virtual const bool testValidity(const IPv4Route *entry) const;

    // IPv4 tables rules
    virtual void addRule(bool output, IPv4RouteRule *entry);
    virtual void delRule(IPv4RouteRule *entry);
    virtual const IPv4RouteRule * getRule(bool output, int index) const;
    virtual int getNumRules(bool output);
    virtual const IPv4RouteRule * findRule(bool output, int prot, int sPort, const IPv4Address &srcAddr, int dPort, const IPv4Address &destAddr, const InterfaceEntry *) const;

};

#endif

