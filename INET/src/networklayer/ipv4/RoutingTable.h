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
//TODO add notification mechanism
class INET_API IPv4Route : public cPolymorphic
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

  protected:
    IPAddress host;     ///< Destination
    IPAddress netmask;  ///< Route mask
    IPAddress gateway;  ///< Next hop
    InterfaceEntry *interfacePtr; ///< interface
    RouteType type;     ///< direct or remote
    RouteSource source; ///< manual, routing prot, etc.
    int metric;         ///< Metric ("cost" to reach the destination)

  private:
    // copying not supported: following are private and also left undefined
    IPv4Route(const IPv4Route& obj);
    IPv4Route& operator=(const IPv4Route& obj);

  public:
    IPv4Route();
    virtual ~IPv4Route() {}
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    void setHost(IPAddress host)  {this->host = host;}
    void setNetmask(IPAddress netmask)  {this->netmask = netmask;}
    void setGateway(IPAddress gateway)  {this->gateway = gateway;}
    void setInterface(InterfaceEntry *interfacePtr)  {this->interfacePtr = interfacePtr;}
    void setType(RouteType type)  {this->type = type;}
    void setSource(RouteSource source)  {this->source = source;}
    void setMetric(int metric)  {this->metric = metric;}

    /** Destination address prefix to match */
    IPAddress getHost() const {return host;}

    /** Represents length of prefix to match */
    IPAddress getNetmask() const {return netmask;}

    /** Next hop address */
    IPAddress getGateway() const {return gateway;}

    /** Next hop interface */
    InterfaceEntry *getInterface() const {return interfacePtr;}

    /** Convenience method */
    const char *getInterfaceName() const {return interfacePtr ? interfacePtr->getName() : "";}

    /** Route type: Direct or Remote */
    RouteType getType() const {return type;}

    /** Source of route. MANUAL (read from file), from routing protocol, etc */
    RouteSource getSource() const {return source;}

    /** "Cost" to reach the destination */
    int getMetric() const {return metric;}
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
class INET_API RoutingTable: public cSimpleModule, public INotifiable
{
  protected:
    InterfaceTable *ift; // cached pointer
    NotificationBoard *nb; // cached pointer

    IPAddress _routerId;
    bool IPForward;

    //
    // Routes:
    //
    typedef std::vector<IPv4Route *> RouteVector;
    RouteVector routes;          // Unicast route array
    RouteVector multicastRoutes; // Multicast route array

  protected:
    // set IP address etc on local loopback
    virtual void configureLoopbackForIPv4();

    // check if a route table entry corresponds to the following parameters
    virtual bool routeMatches(IPv4Route *entry,
                      const IPAddress& target,
                      const IPAddress& nmask,
                      const IPAddress& gw,
                      int metric,
                      const char *dev) const;

    // set router Id
    virtual void autoconfigRouterId();

    // adjust routes with src=IFACENETMASK to actual interface netmasks
    virtual void updateNetmaskRoutes();

    // displays summary above the icon
    virtual void updateDisplayString();

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

  public:
    /**
     * Called by the NotificationBoard whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveChangeNotification(int category, cPolymorphic *details);

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
    virtual InterfaceEntry *getInterfaceByAddress(const IPAddress& address) const;
    //@}

    /**
     * IP forwarding on/off
     */
    virtual bool isIPForwardingEnabled()  {return IPForward;}

    /**
     * Returns routerId.
     */
    virtual IPAddress getRouterId()  {return _routerId;}

    /**
     * Sets routerId.
     */
    virtual void setRouterId(IPAddress a)  {_routerId = a;}

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const IPAddress& dest) const;

    /**
     * The routing function.
     */
    virtual IPv4Route *findBestMatchingRoute(const IPAddress& dest) const;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the interface Id to send the packets with dest as
     * destination address, or -1 if destination is not in routing table.
     */
    virtual InterfaceEntry *getInterfaceForDestAddr(const IPAddress& dest) const;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway to send the destination. Returns null address
     * if the destination is not in routing table or there is
     * no gateway (local delivery).
     */
    virtual IPAddress getGatewayForDestAddr(const IPAddress& dest) const;
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const IPAddress& dest) const;

    /**
     * Returns routes for a multicast address.
     */
    virtual MulticastRoutes getMulticastRoutesFor(const IPAddress& dest) const;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Total number of routing entries (unicast, multicast entries and default route).
     */
    virtual int getNumRoutes() const;

    /**
     * Return kth routing entry.
     */
    virtual IPv4Route *getRoute(int k) const;

    /**
     * Find first routing entry with the given parameters.
     */
    virtual IPv4Route *findRoute(const IPAddress& target,
                                   const IPAddress& netmask,
                                   const IPAddress& gw,
                                   int metric = 0,
                                   char *dev = NULL) const;

    /**
     * Adds a route to the routing table.
     */
    virtual void addRoute(IPv4Route *entry);

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted correctly, false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(IPv4Route *entry);

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    virtual std::vector<IPAddress> gatherAddresses() const;
    //@}

};

#endif

