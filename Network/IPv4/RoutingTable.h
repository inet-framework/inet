//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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
#include "ipsuite_defs.h"
#include "IPAddress.h"

class RoutingTableParser;


/*
 * Constants
 */
const int   MAX_FILESIZE = 5000;
const int   MAX_INTERFACE_NO = 30;
const int   MAX_ENTRY_STRING_SIZE = 20;
const int   MAX_GROUP_STRING_SIZE = 160;

/**
 * Interface entry for the interface table.
 */
class InterfaceEntry : public cPolymorphic
{
  public:
    int index;  // index in interfaces[] (!= outputPort!!!)
    int outputPort;  // FIXME fill this in!!!!
    int mtu;
    int metric;
    opp_string name;
    opp_string encap;
    opp_string hwAddrStr;
    IPAddress inetAddr;
    IPAddress bcastAddr;
    IPAddress mask;
    bool broadcast, multicast, pointToPoint, loopback;

    int multicastGroupCtr; // table size
    IPAddress *multicastGroup;  // dynamically allocated IPAddress table

  public:
    InterfaceEntry();
    virtual ~InterfaceEntry() {}
    virtual void info(char *buf);
    virtual std::string detailedInfo() const;

    // copy not supported: declare the following but leave them undefined
    InterfaceEntry(const InterfaceEntry& obj);
    InterfaceEntry& operator=(const InterfaceEntry& obj);
};


/**
 * Routing entry.
 */
class RoutingEntry : public cPolymorphic
{
  public:
    /**
     * Route type
     */
    enum RouteType
    {
        DIRECT,  // Directly attached to the router
        REMOTE   // Reached through another router
    };

    /**
     * How the route was discovered
     */
    enum RouteSource
    {
        MANUAL,
        RIP,
        OSPF,
        BGP
    };

    // Destination
    IPAddress host;

    // Route mask (replace it with a prefix?)
    IPAddress netmask;

    // Next hop
    IPAddress gateway;

    // Interface name and nb
    opp_string interfaceName;
    int interfaceNo;

    // Route type: Direct or Remote
    RouteType type;

    // Source of route, MANUAL by reading a file,
    // routing protocol name otherwise
    RouteSource source;

    // Metric, "cost" to reach the destination
    int metric;

    // Route age (in seconds, since the route was last updated)
    // Not implemented
    int age;

  public:
    RoutingEntry();
    virtual ~RoutingEntry() {}
    virtual void info(char *buf);
    virtual std::string detailedInfo() const;

    // copy not supported: declare the following but leave them undefined
    RoutingEntry(const RoutingEntry& obj);
    RoutingEntry& operator=(const RoutingEntry& obj);
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
 * or router. It has methods to manage the routing table and the interface table,
 * simulating the "route" and "ifconfig" commands.
 *
 * This is a simple module without gates, it requires function calls to it
 * (message handling does nothing).
 *
 * Uses RoutingTableParser to read routing files (.irt, .mrt).
 */
class RoutingTable: public cSimpleModule
{
  private:
    //
    // Interfaces:
    //
    typedef std::vector<InterfaceEntry *> InterfaceVector;
    InterfaceVector interfaces;

    //
    // Routes:
    //
    typedef std::vector<RoutingEntry *> RouteVector;
    RouteVector routes;          // Unicast route array
    RouteVector multicastRoutes; // Multicast route array
    RoutingEntry *defaultRoute;  // Default route

    bool IPForward;

  protected:
    bool routingEntryMatches(RoutingEntry *entry,
                             const IPAddress& target,
                             const IPAddress& nmask,
                             const IPAddress& gw,
                             int metric,
                             const char *dev);

    // the routing function
    RoutingEntry *selectBestMatchingRoute(const IPAddress& dest);

  public:
    Module_Class_Members(RoutingTable, cSimpleModule, 0);

    void initialize();

    /**
     * Raises an error.
     */
    void handleMessage(cMessage *);

    /** @name Debug/utility */
    //@{
    void printIfconfig();
    void printRoutingTable();
    //@}

    /** @name Interfaces */
    //@{
    /**
     * Returns the number of interfaces.
     */
    int numInterfaces()  {return interfaces.size();}

    /**
     * Returns the InterfaceEntry specified by its index (0..numInterfaces-1).
     */
    InterfaceEntry *interfaceByIndex(int index);

    /**
     * Add an interface.
     */
    void addInterface(InterfaceEntry *entry);

    /**
     * Delete an interface. Returns false if the interface was not in the
     * interface table. Indices of interfaces above this one will change!
     */
    bool deleteInterface(InterfaceEntry *entry);

    /**
     * Returns an interface given by its port number (gate index).
     * Returns NULL if not found.
     */
    InterfaceEntry *interfaceByPortNo(int portNo);

    /**
     * Returns an interface given by its name. Returns NULL if not found.
     */
    InterfaceEntry *interfaceByName(const char *name);

    /**
     * Returns an interface given by its address. Returns NULL if not found.
     */
    InterfaceEntry *interfaceByAddress(const IPAddress& address);
    //@}

    /** @name Routing functions (query the route table) */
    //@{

    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    bool localDeliver(const IPAddress& dest);

    /**
     * Returns the port number to send the packets with dest as
     * destination address, or -1 if destination is not in routing table.
     */
    int outputPortNo(const IPAddress& dest);

    /**
     * Returns the gateway to send the destination,
     * address if the destination is not in routing table or there is
     * no gateway (local delivery).
     */
    IPAddress nextGatewayAddress(const IPAddress& dest); //FIXME join with outputPortNo()
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
     * Returns the default route entry.
     */
    RoutingEntry *getDefaultRoute()  {return defaultRoute;}

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
    //@}

};

#endif

