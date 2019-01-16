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

#ifndef __INET_IPV4ROUTINGTABLE_H
#define __INET_IPV4ROUTINGTABLE_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

class IInterfaceTable;
class RoutingTableParser;
class IRoutingTable;

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
 * Entries in the route table are represented by Ipv4Route objects.
 * Ipv4Route objects can be polymorphic: if a routing protocol needs
 * to store additional data, it can simply subclass from Ipv4Route,
 * and add the derived object to the table.
 *
 * Uses RoutingTableParser to read routing files (.irt, .mrt).
 *
 *
 * @see InterfaceEntry, Ipv4InterfaceData, Ipv4Route
 */
class INET_API Ipv4RoutingTable : public cSimpleModule, public IIpv4RoutingTable, protected cListener, public ILifecycle
{
  protected:
    IInterfaceTable *ift = nullptr;    // cached pointer

    Ipv4Address routerId;
    const char *netmaskRoutes = nullptr;
    bool forwarding = false;
    bool multicastForward = false;
    bool isNodeUp = false;
    bool useAdminDist = false;     // Use Cisco like administrative distances

    // for convenience
    typedef Ipv4MulticastRoute::OutInterface OutInterface;
    typedef Ipv4MulticastRoute::OutInterfaceVector OutInterfaceVector;

    // routing cache: maps destination address to the route
    typedef std::map<Ipv4Address, Ipv4Route *> RoutingCache;
    mutable RoutingCache routingCache;

    // local addresses cache (to speed up isLocalAddress())
    typedef std::set<Ipv4Address> AddressSet;
    // JcM add: to handle the local broadcast address
    mutable AddressSet localBroadcastAddresses;

  private:
    // The vectors storing routes are ordered by prefix length, administrative distance, and metric.
    // Subclasses should use internalAdd[Multicast]Route() and internalRemove[Multicast]Route() methods
    // to modify them, but they can not access them directly.

    typedef std::vector<Ipv4Route *> RouteVector;
    RouteVector routes;    // Unicast route array, sorted by netmask desc, dest asc, metric asc

    typedef std::vector<Ipv4MulticastRoute *> MulticastRouteVector;
    MulticastRouteVector multicastRoutes;    // Multicast route array, sorted by netmask desc, origin asc, metric asc

  protected:
    // set router Id
    virtual void configureRouterId();

    // adjust routes with src=IFACENETMASK to actual interface netmasks
    virtual void updateNetmaskRoutes();

    // creates a new empty route
    virtual Ipv4Route *createNewRoute();

    // displays summary above the icon
    virtual void refreshDisplay() const override;

    // delete routes for the given interface
    virtual void deleteInterfaceRoutes(const InterfaceEntry *entry);

    // invalidates routing cache and local addresses cache
    virtual void invalidateCache();

    // helper for sorting routing table, used by addRoute()
    class RouteLessThan
    {
        const Ipv4RoutingTable &c;
      public:
        RouteLessThan(const Ipv4RoutingTable& c) : c(c) {}
        bool operator () (const Ipv4Route *a, const Ipv4Route *b) { return c.routeLessThan(a, b); }
    };
    bool routeLessThan(const Ipv4Route *a, const Ipv4Route *b) const;

    // helper for sorting multicast routing table, used by addMulticastRoute()
    static bool multicastRouteLessThan(const Ipv4MulticastRoute *a, const Ipv4MulticastRoute *b);

    // helper functions:
    void internalAddRoute(Ipv4Route *entry);
    Ipv4Route *internalRemoveRoute(Ipv4Route *entry);
    void internalAddMulticastRoute(Ipv4MulticastRoute *entry);
    Ipv4MulticastRoute *internalRemoveMulticastRoute(Ipv4MulticastRoute *entry);

  public:
    Ipv4RoutingTable() {}
    virtual ~Ipv4RoutingTable();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *) override;

    /**
     * Called by the signal handler whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  public:
    /**
     * For debugging
     */
    virtual void printRoutingTable() const override;

    /**
     * For debugging
     */
    virtual void printMulticastRoutingTable() const override;

    /**
     * Returns the host or router this routing table lives in.
     */
    virtual cModule *getHostModule() override;

    /** @name Interfaces */
    //@{
    /**
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByAddress(const Ipv4Address& address) const override;
    //@}

    /**
     * AdminDist on/off
     */
    virtual bool isAdminDistEnabled() const override { return useAdminDist; }

    /**
     * Ipv4 forwarding on/off
     */
    virtual bool isForwardingEnabled() const override { return forwarding; }

    /**
     * Ipv4 multicast forwarding on/off
     */
    virtual bool isMulticastForwardingEnabled() const override { return multicastForward; }

    /**
     * Returns routerId.
     */
    virtual Ipv4Address getRouterId() const override { return routerId; }

    /**
     * Sets routerId.
     */
    virtual void setRouterId(Ipv4Address a) override;

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const Ipv4Address& dest) const override;

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local network broadcast address, i.e. one of the
     * broadcast addresses derived from the interface addresses and netmasks.
     */
    virtual bool isLocalBroadcastAddress(const Ipv4Address& dest) const override;

    /**
     * Returns the interface entry having the specified address
     * as its local broadcast address.
     */
    virtual InterfaceEntry *findInterfaceByLocalBroadcastAddress(const Ipv4Address& dest) const override;

    /**
     * The routing function. Performs longest prefix match for the given
     * destination address, and returns the resulting route. Returns nullptr
     * if there is no matching route.
     */
    virtual Ipv4Route *findBestMatchingRoute(const Ipv4Address& dest) const override;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the output interface for the packets with dest as destination
     * address, or nullptr if the destination is not in routing table.
     */
    virtual InterfaceEntry *getInterfaceForDestAddr(const Ipv4Address& dest) const override;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway for the destination address. Returns the unspecified
     * address if the destination is not in routing table or the gateway field
     * is not filled in in the route.
     */
    virtual Ipv4Address getGatewayForDestAddr(const Ipv4Address& dest) const override;
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const Ipv4Address& dest) const override;

    /**
     * Returns route for a multicast source and multicast group.
     */
    virtual const Ipv4MulticastRoute *findBestMatchingMulticastRoute(const Ipv4Address& origin, const Ipv4Address& group) const override;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Returns the total number of routes (unicast, multicast, plus the
     * default route).
     */
    virtual int getNumRoutes() const override { return routes.size(); }

    /**
     * Returns the kth route.
     */
    virtual Ipv4Route *getRoute(int k) const override;

    /**
     * Finds and returns the default route, or nullptr if it doesn't exist
     */
    virtual Ipv4Route *getDefaultRoute() const override;

    /**
     * Adds a route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addRoute(Ipv4Route *entry) override;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned of the route was not in the routing table.
     */
    virtual Ipv4Route *removeRoute(Ipv4Route *entry) override;

    /**
     * Removes the given route from the routing table, and delete it.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(Ipv4Route *entry) override;

    /**
     * Returns the total number of multicast routes.
     */
    virtual int getNumMulticastRoutes() const override { return multicastRoutes.size(); }

    /**
     * Returns the kth multicast route.
     */
    virtual Ipv4MulticastRoute *getMulticastRoute(int k) const override { return k >= 0 && static_cast<size_t>(k) < multicastRoutes.size() ? multicastRoutes[k] : nullptr; }

    /**
     * Adds a multicast route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addMulticastRoute(Ipv4MulticastRoute *entry) override;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned of the route was not in the routing table.
     */
    virtual Ipv4MulticastRoute *removeMulticastRoute(Ipv4MulticastRoute *entry) override;

    /**
     * Deletes the given multicast route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteMulticastRoute(Ipv4MulticastRoute *entry) override;

    /**
     * Deletes invalid routes from the routing table. Invalid routes are those
     * where the isValid() method returns false.
     */
    virtual void purge() override;

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    virtual std::vector<Ipv4Address> gatherAddresses() const override;

    /**
     * To be called from route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void routeChanged(Ipv4Route *entry, int fieldCode) override;

    /**
     * To be called from multicast route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void multicastRouteChanged(Ipv4MulticastRoute *entry, int fieldCode) override;
    //@}

    /**
     * ILifecycle method
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;

    virtual L3Address getRouterIdAsGeneric() const override { return getRouterId(); }
    virtual bool isLocalAddress(const L3Address& dest) const override { return isLocalAddress(dest.toIpv4()); }
    virtual InterfaceEntry *getInterfaceByAddress(const L3Address& address) const override { return getInterfaceByAddress(address.toIpv4()); }
    virtual IRoute *findBestMatchingRoute(const L3Address& dest) const override { return findBestMatchingRoute(dest.toIpv4()); }
    virtual InterfaceEntry *getOutputInterfaceForDestination(const L3Address& dest) const override { return getInterfaceForDestAddr(dest.toIpv4()); }    //XXX inconsistent names
    virtual L3Address getNextHopForDestination(const L3Address& dest) const override { return getGatewayForDestAddr(dest.toIpv4()); }    //XXX inconsistent names
    virtual bool isLocalMulticastAddress(const L3Address& dest) const override { return isLocalMulticastAddress(dest.toIpv4()); }
    virtual IMulticastRoute *findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const override { return const_cast<Ipv4MulticastRoute *>(findBestMatchingMulticastRoute(origin.toIpv4(), group.toIpv4())); }    //XXX remove 'const' from Ipv4 method?
    virtual IRoute *createRoute() override { return new Ipv4Route(); }

  private:
    virtual void addRoute(IRoute *entry) override { addRoute(check_and_cast<Ipv4Route *>(entry)); }
    virtual IRoute *removeRoute(IRoute *entry) override { return removeRoute(check_and_cast<Ipv4Route *>(entry)); }
    virtual bool deleteRoute(IRoute *entry) override { return deleteRoute(check_and_cast<Ipv4Route *>(entry)); }

    virtual void addMulticastRoute(IMulticastRoute *entry) override { addMulticastRoute(check_and_cast<Ipv4MulticastRoute *>(entry)); }
    virtual IMulticastRoute *removeMulticastRoute(IMulticastRoute *entry) override { return removeMulticastRoute(check_and_cast<Ipv4MulticastRoute *>(entry)); }
    virtual bool deleteMulticastRoute(IMulticastRoute *entry) override { return deleteMulticastRoute(check_and_cast<Ipv4MulticastRoute *>(entry)); }
};

} // namespace inet

#endif // ifndef __INET_IPV4ROUTINGTABLE_H

