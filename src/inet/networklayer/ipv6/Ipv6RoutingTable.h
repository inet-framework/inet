//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6ROUTINGTABLE_H
#define __INET_IPV6ROUTINGTABLE_H

#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"

namespace inet {

class IInterfaceTable;
class NetworkInterface;
class Ipv6RoutingTable;

/**
 * Represents the Ipv6 routing table and neighbour discovery data structures.
 * This object has one instance per host or router.
 *
 * See the NED documentation for general overview.
 *
 * This is a simple module without gates, it requires function calls to it
 * (message handling does nothing). Methods are provided for reading and
 * updating the interface table and the route table, as well as for unicast
 * and multicast routing.
 *
 * The route table is read from a file. The route table can also
 * be read and modified during simulation, typically by routing protocol
 * implementations.
 */
class INET_API Ipv6RoutingTable : public cSimpleModule, public IRoutingTable, protected cListener, public ILifecycle
{
  protected:
    ModuleRefByPar<IInterfaceTable> ift; // cached pointer

    bool isrouter = false;
    bool multicastForward = false; // If node is forwarding multicast info
    bool useAdminDist = false; // Use Cisco like administrative distances

#ifdef INET_WITH_xMIPv6
    bool ishome_agent = false; // added by Zarrar Yousaf @ CNI, UniDortmund on 20.02.07
    bool ismobile_node = false; // added by Zarrar Yousaf @ CNI, UniDortmund on 25.02.07
    bool mipv6Support = false; // 4.9.07 - CB
#endif /* INET_WITH_xMIPv6 */

    // Destination Cache maps dest address to next hop and interfaceId.
    // NOTE: nextHop might be a link-local address from which interfaceId cannot be deduced
    struct DestCacheEntry {
        int interfaceId = -1;
        Ipv6Address nextHopAddr;
        simtime_t expiryTime;
        // more destination specific data may be added here, e.g. path MTU
    };
    friend std::ostream& operator<<(std::ostream& os, const DestCacheEntry& e);
    typedef std::map<Ipv6Address, DestCacheEntry> DestCache;
    DestCache destCache;

    // RouteList contains local prefixes, and (for routers)
    // static, OSPF, RIP etc routes as well
    typedef std::vector<Ipv6Route *> RouteList;
    RouteList routeList;

  protected:
    // creates a new empty route, factory method overriden in subclasses that use custom routes
    virtual Ipv6Route *createNewRoute(Ipv6Address destPrefix, int prefixLength, IRoute::SourceType src);

  public:
    // internal: routes of different type can only be added via well-defined functions
    virtual void addRoute(Ipv6Route *route);

  protected:
    // helper for addRoute()
    class INET_API RouteLessThan {
        const Ipv6RoutingTable& c;

      public:
        RouteLessThan(const Ipv6RoutingTable& c) : c(c) {}
        bool operator()(const Ipv6Route *a, const Ipv6Route *b) { return c.routeLessThan(a, b); }
    };
    bool routeLessThan(const Ipv6Route *a, const Ipv6Route *b) const;

    // internal
    virtual void configureInterfaceForIpv6(NetworkInterface *ie);
    /**
     *  RFC 3513: Section 2.8 A Node's Required Address
     *  Assign the various addresses to the node's respective interface. This
     *  should be done when the Ipv6 Protocol stack is created.
     */
    virtual void assignRequiredNodeAddresses(NetworkInterface *ie);
    // internal
    virtual void configureInterfaceFromXml(NetworkInterface *ie, cXMLElement *cfg);

    // internal
    virtual void configureTunnelFromXml(cXMLElement *cfg);

    void internalAddRoute(Ipv6Route *route);
    Ipv6Route *internalRemoveRoute(Ipv6Route *route);
    RouteList::iterator internalDeleteRoute(RouteList::iterator it);

  protected:
    // displays summary above the icon
    virtual void refreshDisplay() const override;

  public:
    Ipv6RoutingTable();
    virtual ~Ipv6RoutingTable();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void parseXmlConfigFile();

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
    /** @name Interfaces */
    //@{
    /**
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual NetworkInterface *getInterfaceByAddress(const Ipv6Address& address) const;
    //@}

    /**
     * IP forwarding on/off
     */
    virtual bool isRouter() const { return isrouter; }

    virtual bool isMulticastForwardingEnabled() { return multicastForward; }

    /**
     * To be called from route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void routeChanged(Ipv6Route *entry, int fieldCode);

#ifdef INET_WITH_xMIPv6
    /**
     * Determine whether normal Router or Home Agent
     */
    bool isHomeAgent() const { return ishome_agent; }

    /**
     * Define whether normal Router or Home Agent.
     */
    void setIsHomeAgent(bool value) { ishome_agent = value; }

    /**
     * Determine whether a node is a Mobile Node or Correspondent Node:
     * MN if TRUE or else a CN
     */
    bool isMobileNode() const { return ismobile_node; }

    /**
     * Define whether a node is a Mobile Node or Correspondent Node:
     * MN if TRUE or else a CN
     */
    void setIsMobileNode(bool value) { ismobile_node = value; }
#endif /* INET_WITH_xMIPv6 */

    /** @name Routing functions */
    //@{
    /**
     * Checks if the address is one of the host's addresses, i.e.
     * assigned to one of its interfaces (tentatively or not).
     */
    virtual bool isLocalAddress(const Ipv6Address& dest) const;

    /**
     * Looks up the given destination address in the Destination Cache,
     * then returns the next-hop address and the interface in the outInterfaceId
     * variable. If the destination is not in the cache, outInterfaceId is set to
     * -1 and the unspecified address is returned. The caller should check
     * for interfaceId==-1, because unspecified address is also returned
     * if the link layer doesn't use addresses at all (e.g. PPP).
     *
     * NOTE: outInterfaceId is an OUTPUT parameter -- its initial value is ignored,
     * and the lookupDestCache() sets it to the correct value instead.
     */
    const Ipv6Address& lookupDestCache(const Ipv6Address& dest, int& outInterfaceId);

    /**
     * Performs longest prefix match in the routing table and returns
     * the resulting route, or nullptr if there was no match.
     */
    const Ipv6Route *doLongestPrefixMatch(const Ipv6Address& dest);

    /**
     * Checks if the given prefix already exists in the routing table (prefix list)
     */
    virtual bool isPrefixPresent(const Ipv6Address& prefix) const;

    // TODO multicast delivery
    //@}

    /** @name Managing the destination cache */
    //@{
    /**
     * Add or update a destination cache entry.
     */
    virtual void updateDestCache(const Ipv6Address& dest, const Ipv6Address& nextHopAddr, int interfaceId, simtime_t expiryTime);

    /**
     * Discard all entries in destination cache
     */
    virtual void purgeDestCache();

    /**
     * Discard all entries in destination cache where next hop is the given
     * address on the given interface. This is typically called when a router
     * becomes unreachable, and all destinations going via that router have to
     * go though router selection again.
     */
    virtual void purgeDestCacheEntriesToNeighbour(const Ipv6Address& nextHopAddr, int interfaceId);

    /**
     * Removes all destination cache entries for the specified interface
     */
    void purgeDestCacheForInterfaceId(int interfaceId);

    //@}

    /** @name Managing prefixes and the route table */
    //@{
    /**
     * Add on-link prefix (route of type FROM_RA), or update existing one.
     * To be called from code processing on-link prefixes in Router Advertisements.
     * Expiry time can be derived from the Valid Lifetime field
     * in the Router Advertisements.
     *
     * NOTE: This method does NOT update the lifetime of matching addresses
     * in the IInterfaceTable (see Ipv6InterfaceData); that has to be done
     * separately.
     */
    virtual void addOrUpdateOnLinkPrefix(const Ipv6Address& destPrefix, int prefixLength,
            int interfaceId, simtime_t expiryTime);

    /**
     * Remove an on-link prefix. To be called when the prefix gets advertised
     * with zero lifetime, or to purge an expired prefix.
     *
     * NOTE: This method does NOT remove the matching addresses from the
     * IInterfaceTable (see Ipv6InterfaceData); that has to be done separately.
     */
    virtual void deleteOnLinkPrefix(const Ipv6Address& destPrefix, int prefixLength);

    /**
     * Add route of type OWN_ADV_PREFIX. This is a prefix that *this* router
     * advertises on this interface.
     */
    virtual void addOrUpdateOwnAdvPrefix(const Ipv6Address& destPrefix, int prefixLength,
            int interfaceId, simtime_t expiryTime);

    /**
     * Creates a static route. If metric is omitted, it gets initialized
     * to the interface's metric value.
     */
    virtual void addStaticRoute(const Ipv6Address& destPrefix, int prefixLength,
            unsigned int interfaceId, const Ipv6Address& nextHop,
            int metric = 0);

    /**
     *  Adds a default route for a host. This method requires the RA's source
     *  address and the router expiry time plus the simTime().
     */
    virtual void addDefaultRoute(const Ipv6Address& raSrcAddr, unsigned int ifID,
            simtime_t routerLifetime);

    /**
     * Adds the given getRoute(which can be OSPF, BGP, RIP or any other route)
     * with src==ROUTING_PROT. To store additional information with the route,
     * one can subclass from Ipv6Route and add more fields.
     */
    virtual void addRoutingProtocolRoute(Ipv6Route *route);

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned if the route was not in the routing table.
     */
    virtual Ipv6Route *removeRoute(Ipv6Route *route);

    /**
     * Deletes the given route from the route table.
     * Returns true, if it was deleted, false if it was not found in the routing table.
     */
    virtual bool deleteRoute(Ipv6Route *route);

    /**
     * Deletes the routes that are using the specified interface.
     */
    virtual void deleteInterfaceRoutes(const NetworkInterface *entry);

    /**
     * Return the number of routes.
     */
    virtual int getNumRoutes() const override;

    /**
     * Return the ith route.
     */
    virtual Ipv6Route *getRoute(int i) const override;
    //@}

#ifdef INET_WITH_xMIPv6
    //================Added by Zarrar Yousaf ===================================

    //void updateHomeNetworkInfo(const Ipv6Address& hoa, const Ipv6Address& ha);//10.07.07 This updates the struct HomeNetwork Info{} with the MN's Home Address(HoA) and the global scope address of the MNs Home Agent (ha).
    //const Ipv6Address& getHomeAgentAddress() { return homeInfo.homeAgentAddr; } // Zarrar 15.07.07 // return by reference - CB
    //const Ipv6Address& getMNHomeAddress() { return homeInfo.HoA; } // Zarrar 15.07.07 // return by reference - CB
    const Ipv6Address& getHomeAddress(); // NEW, 14.01.08 - CB

    /**
     * Check whether provided address is a HoA
     */
    bool isHomeAddress(const Ipv6Address& addr);

    /**
     * Deletes the current default routes for the given interface.
     */
    void deleteDefaultRoutes(int interfaceID);

    /**
     * Deletes all routes from the routing table.
     */
    void deleteAllRoutes();

    /**
     * Deletes all prefixes registered for the given interface.
     */
    void deletePrefixes(int interfaceID);

    /**
     * Can be used to check whether this node supports MIPv6 or not
     * (MN, MR, HA or CN).
     */
    bool hasMipv6Support() { return mipv6Support; }

    /**
     * This method is used to define whether the node support MIPv6 or
     * not (MN, MR, HA or CN).
     */
    void setMipv6Support(bool value) { mipv6Support = value; }

    /**
     * Checks whether the provided address is in an on-link address
     * with respect to the prefix advertisement list.
     */
    bool isOnLinkAddress(const Ipv6Address& address); // update 11.9.07 - CB
#endif /* INET_WITH_xMIPv6 */

    /**
     * ILifecycle method
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;

    // IRoutingTable methods:
    virtual bool isAdminDistEnabled() const override { return useAdminDist; }
    virtual bool isForwardingEnabled() const override { return isRouter(); } // TODO inconsistent names
    virtual bool isMulticastForwardingEnabled() const override { return true; /*TODO isMulticastForwardingEnabled();*/ }
    virtual L3Address getRouterIdAsGeneric() const override { return L3Address(Ipv6Address()); /*TODO getRouterId();*/ }
    virtual bool isLocalAddress(const L3Address& dest) const override { return isLocalAddress(dest.toIpv6()); }
    virtual bool isLocalBroadcastAddress(const L3Address& dest) const { return false; /*TODO isLocalBroadcastAddress(dest.toIPv6());*/ }
    virtual NetworkInterface *getInterfaceByAddress(const L3Address& address) const override;
    virtual NetworkInterface *findInterfaceByLocalBroadcastAddress(const L3Address& dest) const { return nullptr; /*TODO findInterfaceByLocalBroadcastAddress(dest.toIPv6());*/ }
    virtual IRoute *findBestMatchingRoute(const L3Address& dest) const override { return const_cast<Ipv6Route *>((const_cast<Ipv6RoutingTable *>(this))->doLongestPrefixMatch(dest.toIpv6())); } // FIXME what a name??!! also: remove const; ALSO: THIS DOES NOT UPDATE DESTCACHE LIKE METHODS BUILT ON IT!
    virtual NetworkInterface *getOutputInterfaceForDestination(const L3Address& dest) const override { const Ipv6Route *e = (const_cast<Ipv6RoutingTable *>(this))->doLongestPrefixMatch(dest.toIpv6()); return e ? e->getInterface() : nullptr; }
    virtual L3Address getNextHopForDestination(const L3Address& dest) const override { const Ipv6Route *e = (const_cast<Ipv6RoutingTable *>(this))->doLongestPrefixMatch(dest.toIpv6()); return e ? e->getNextHopAsGeneric() : L3Address(); }
    virtual bool isLocalMulticastAddress(const L3Address& dest) const override { return false; /*TODO isLocalMulticastAddress(dest.toIPv6());*/ }
    virtual IMulticastRoute *findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const override { return nullptr; /*TODO findBestMatchingMulticastRoute(origin.toIPv6(), group.toIPv6());*/ }
    virtual IRoute *getDefaultRoute() const override { return nullptr; /*TODO getDefaultRoute();*/ }
    virtual void addRoute(IRoute *entry) override { addRoutingProtocolRoute(check_and_cast<Ipv6Route *>(entry)); } // TODO contrast that with addStaticRoute()!
    virtual IRoute *removeRoute(IRoute *entry) override { return removeRoute(check_and_cast<Ipv6Route *>(entry)); }
    virtual bool deleteRoute(IRoute *entry) override { return deleteRoute(check_and_cast<Ipv6Route *>(entry)); }
    virtual IMulticastRoute *getMulticastRoute(int i) const override { return nullptr; /*TODO*/ }
    virtual int getNumMulticastRoutes() const override { return 0; /*TODO getNumMulticastRoutes();*/ }
    virtual void addMulticastRoute(IMulticastRoute *entry) override { /*TODO addMulticastRoute(entry);*/ }
    virtual IMulticastRoute *removeMulticastRoute(IMulticastRoute *entry) override { /*TODO removeMulticastRoute(entry);*/ return entry; }
    virtual bool deleteMulticastRoute(IMulticastRoute *entry) override { return false; /*TODO deleteMulticastRoute(entry);*/ }
    virtual IRoute *createRoute() override { return new Ipv6Route(Ipv6Address(), 0, IRoute::MANUAL); }

    /**
     * Prints the routing table.
     */
    virtual void printRoutingTable() const override;
};

} // namespace inet

#endif

