//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#include <omnetpp.h>
#include "INETDefs.h"
#include "IPAddress.h"
#include "InterfaceTable.h"
#include "NotificationBoard.h"

class RoutingTableParser;


/**
 * Routing entry in RoutingTable.
 *
 * @see RoutingTable
 */
//TODO: make it consistent with IPv6 RoutingTable; wrap public members into
//methods; add notification mechanism
class INET_API RoutingEntry : public cPolymorphic
{
  public:
    /** Route type */
    enum RouteType
    {
        DIRECT,  ///< Directly attached to the router
        REMOTE   ///< Reached through another router
    };

    /** Specifies where the route comes from */
    enum RouteSource
    {
        MANUAL,       ///< manually added static route
        IFACENETMASK, ///< comes from an interface's netmask
        RIP,          ///< managed by the given routing protocol
        OSPF,         ///< managed by the given routing protocol
        BGP,          ///< managed by the given routing protocol
        ZEBRA,        ///< managed by the Quagga/Zebra based model
    };

    /// Destination
    IPAddress host;

    /// Route mask
    IPAddress netmask;

    /// Next hop
    IPAddress gateway;

    /// Interface name and pointer
    opp_string interfaceName;
    InterfaceEntry *interfacePtr;

    /// Route type: Direct or Remote
    RouteType type;

    /// Source of route, MANUAL by reading a file,
    /// routing protocol name otherwise
    RouteSource source;

    /// Metric ("cost" to reach the destination)
    int metric;

  private:
    // copying not supported: following are private and also left undefined
    RoutingEntry(const RoutingEntry& obj);
    RoutingEntry& operator=(const RoutingEntry& obj);

  public:
    RoutingEntry();
    virtual ~RoutingEntry() {}
    virtual std::string info() const;
    virtual std::string detailedInfo() const;
};


/** Returned as the result of multicast routing */
struct MulticastRoute
{
    InterfaceEntry *interf;
    IPAddress gateway;
};
typedef std::vector<MulticastRoute> MulticastRoutes;


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
 * Entries in the route table are represented by RoutingEntry objects.
 * RoutingEntry objects can be polymorphic: if a routing protocol needs
 * to store additional data, it can simply subclass from RoutingEntry,
 * and add the derived object to the table.
 *
 * Uses RoutingTableParser to read routing files (.irt, .mrt).
 *
 *
 * @see InterfaceEntry, IPv4InterfaceData, RoutingEntry
 */
class INET_API RoutingTable: public cSimpleModule, public INotifiable
{
  private:
    InterfaceTable *ift; // cached pointer
    NotificationBoard *nb; // cached pointer

    IPAddress _routerId;
    bool IPForward;

    //
    // Routes:
    //
    typedef std::vector<RoutingEntry *> RouteVector;
    RouteVector routes;          // Unicast route array
    RouteVector multicastRoutes; // Multicast route array

  protected:
    // set IP address etc on local loopback
    void configureLoopbackForIPv4();

    // check if a route table entry corresponds to the following parameters
    bool routingEntryMatches(RoutingEntry *entry,
                             const IPAddress& target,
                             const IPAddress& nmask,
                             const IPAddress& gw,
                             int metric,
                             const char *dev);

    // set router Id
    void autoconfigRouterId();

    // adjust routes with src=IFACENETMASK to actual interface netmasks
    void updateNetmaskRoutes();

    // displays summary above the icon
    void updateDisplayString();

  public:
    RoutingTable();
    virtual ~RoutingTable();

  protected:
    int numInitStages() const  {return 4;}
    void initialize(int stage);

    /**
     * Raises an error.
     */
    void handleMessage(cMessage *);

  public:
    /**
     * Called by the NotificationBoard whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveChangeNotification(int category, cPolymorphic *details);

    /** @name Debug/utility */
    //@{
    void printIfconfig();
    void printRoutingTable();
    //@}

    /** @name Interfaces */
    //@{
    void configureInterfaceForIPv4(InterfaceEntry *ie);

    /**
     * Returns an interface given by its address. Returns NULL if not found.
     */
    InterfaceEntry *interfaceByAddress(const IPAddress& address);
    //@}

    /**
     * IP forwarding on/off
     */
    bool ipForward()  {return IPForward;}

    /**
     * Returns routerId.
     */
    IPAddress routerId()  {return _routerId;}

    /**
     * Sets routerId.
     */
    void setRouterId(IPAddress a)  {_routerId = a;}

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    bool localDeliver(const IPAddress& dest);

    /**
     * The routing function.
     */
    RoutingEntry *findBestMatchingRoute(const IPAddress& dest);

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the interface Id to send the packets with dest as
     * destination address, or -1 if destination is not in routing table.
     */
    InterfaceEntry *interfaceForDestAddr(const IPAddress& dest);

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway to send the destination. Returns null address
     * if the destination is not in routing table or there is
     * no gateway (local delivery).
     */
    IPAddress gatewayForDestAddr(const IPAddress& dest);
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    bool multicastLocalDeliver(const IPAddress& dest);

    /**
     * Returns routes for a multicast address.
     */
    MulticastRoutes multicastRoutesFor(const IPAddress& dest);
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Total number of routing entries (unicast, multicast entries and default route).
     */
    int numRoutingEntries();

    /**
     * Return kth routing entry.
     */
    RoutingEntry *routingEntry(int k);

    /**
     * Find first routing entry with the given parameters.
     */
    RoutingEntry *findRoutingEntry(const IPAddress& target,
                                   const IPAddress& netmask,
                                   const IPAddress& gw,
                                   int metric = 0,
                                   char *dev = NULL);

    /**
     * Adds a route to the routing table.
     */
    void addRoutingEntry(RoutingEntry *entry);

    /**
     * Deletes the given routes from the routing table.
     * Returns true if the route was deleted correctly, false if it was
     * not in the routing table.
     */
    bool deleteRoutingEntry(RoutingEntry *entry);

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    std::vector<IPAddress> gatherAddresses();
    //@}

};

#endif

