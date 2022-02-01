//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NEXTHOPROUTINGTABLE_H
#define __INET_NEXTHOPROUTINGTABLE_H

#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/nexthop/NextHopRoute.h"

namespace inet {

class IInterfaceTable;

/**
 * A C++ interface to abstract the functionality of a routing table, regardless of address type.
 */
class INET_API NextHopRoutingTable : public cSimpleModule, public IRoutingTable, public cListener
{
  private:
    ModuleRefByPar<IInterfaceTable> ift;

    L3Address routerId;
    L3Address::AddressType addressType = L3Address::NONE;
    bool forwarding = false;
    bool multicastForwarding = false;

    typedef std::vector<NextHopRoute *> RouteVector;
    RouteVector routes; // unicast route table, sorted by prefix match order

    typedef std::vector<NextHopMulticastRoute *> MulticastRouteVector;
    MulticastRouteVector multicastRoutes; // multicast route table, sorted by prefix match order

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

    virtual void configureRouterId();

    virtual void refreshDisplay() const override;

    virtual void configureInterface(NetworkInterface *ie);

    virtual void configureLoopback();

    static bool routeLessThan(const NextHopRoute *a, const NextHopRoute *b);

    void internalAddRoute(NextHopRoute *route);
    NextHopRoute *internalRemoveRoute(NextHopRoute *route);

  public:
    NextHopRoutingTable();
    virtual ~NextHopRoutingTable();

    /** @name Miscellaneous functions */
    //@{
    /**
     * Forwarding on/off
     */
    virtual bool isForwardingEnabled() const override; // TODO IP modulba?

    /**
     * Administrative distance on/off
     */
    virtual bool isAdminDistEnabled() const override { return false; }

    /**
     * Multicast forwarding on/off
     */
    virtual bool isMulticastForwardingEnabled() const override; // TODO IP modulba?

    /**
     * Returns routerId.
     */
    virtual L3Address getRouterIdAsGeneric() const override;

    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const L3Address& dest) const override; // TODO maybe into InterfaceTable?

    /**
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual NetworkInterface *getInterfaceByAddress(const L3Address& address) const override; // TODO should be find..., see next one

    /**
     * To be called from route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void routeChanged(NextHopRoute *entry, int fieldCode);
    //@}

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * The routing function. Performs longest prefix match for the given
     * destination address, and returns the resulting route. Returns nullptr
     * if there is no matching route.
     */
    virtual NextHopRoute *findBestMatchingRoute(const L3Address& dest) const override; // TODO make coveriant return types everywhere

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the output interface for the packets with dest as destination
     * address, or nullptr if the destination is not in routing table.
     */
    virtual NetworkInterface *getOutputInterfaceForDestination(const L3Address& dest) const override; // TODO redundant

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway for the destination address. Returns the unspecified
     * address if the destination is not in routing table or the gateway field
     * is not filled in in the route.
     */
    virtual L3Address getNextHopForDestination(const L3Address& dest) const override; // TODO redundant AND unused
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const L3Address& dest) const override;

    /**
     * Returns route for a multicast origin and group.
     */
    virtual IMulticastRoute *findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const override;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Returns the total number of unicast routes.
     */
    virtual int getNumRoutes() const override;

    /**
     * Returns the kth route.
     */
    virtual IRoute *getRoute(int k) const override;

    /**
     * Finds and returns the default route, or nullptr if it doesn't exist
     */
    virtual IRoute *getDefaultRoute() const override; // TODO is this a universal concept?

    /**
     * Adds a route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addRoute(IRoute *entry) override;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned if the route was not in the routing table.
     */
    virtual IRoute *removeRoute(IRoute *entry) override;

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(IRoute *entry) override;

    /**
     * Returns the total number of multicast routes.
     */
    virtual int getNumMulticastRoutes() const override;

    /**
     * Returns the kth multicast route.
     */
    virtual IMulticastRoute *getMulticastRoute(int k) const override;

    /**
     * Adds a multicast route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addMulticastRoute(IMulticastRoute *entry) override;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned of the route was not in the routing table.
     */
    virtual IMulticastRoute *removeMulticastRoute(IMulticastRoute *entry) override;

    /**
     * Deletes the given multicast route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteMulticastRoute(IMulticastRoute *entry) override;
    //@}

    virtual IRoute *createRoute() override;

    /**
     * Prints routing table.
     */
    virtual void printRoutingTable() const override;
};

} // namespace inet

#endif

