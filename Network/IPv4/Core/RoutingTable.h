//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

/*
    Author:     Jochen Reber
    Date:       18.5.00
    On Linux:   19.5.00 - 29.5.00
    Modified by Vincent Oberle
    Date:       1.2.2001
*/

#ifndef __ROUTINGTABLE_H
#define __ROUTINGTABLE_H

#include <omnetpp.h>
#include "ipsuite_defs.h"

// required for IPAddress typdef
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


/**
 * Interface entry for the interface table.
 */
class InterfaceEntry : public cObject // FIXME only cObject so that cArray can by used
{
  public:
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

    void print();
};


/**
 * Routing entry.
 */
class RoutingEntry : public cObject // FIXME only cObject so that cArray can by used
{
  public:
    int ref; // TO DEPRECATE

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

    // Miscellaneaous route information
    void *routeInfo;

  public:
    RoutingEntry();
    virtual ~RoutingEntry() {}
    virtual void info(char *buf);
    virtual std::string detailedInfo() const;

    // copy not supported: declare the following but leave them undefined
    RoutingEntry(const RoutingEntry& obj);
    RoutingEntry& operator=(const RoutingEntry& obj);

    void print();

    // Indicates if a routing entry corresponds to the other parameters
    // (which can be null).
    bool correspondTo(const IPAddress& target, const IPAddress& netmask,
                      const IPAddress& gw, int metric, const char *dev);
};


/**
 * Read in the interfaces and routing table from a file; manages requests
 * to the routing table and the interface table, simulating the "route"
 * and "ifconfig" commands.
 *
 * This is a simple module without gates, it requires function calls to it
 * (message handling does nothing).
 *
 * Uses RoutingTableParser to read routing files (.irt, .mrt).
 */
class RoutingTable: public cSimpleModule
{
  private:
    friend class RoutingTableParser;   // FIXME a bit ugly, we should have enough public functions...

    //
    // Interfaces:
    //

    // Number of interfaces
    int numIntrfaces;

    // Interface array
    InterfaceEntry **intrface;   // FIXME replace with std::vector

    // Loopback interface
    InterfaceEntry *loopbackInterface;

    //
    // Routes:
    //

    // Unicast route array
    cArray *route;           // FIXME replace with std::vector

    // Default route
    RoutingEntry *defaultRoute;

    // Multicast route array
    cArray *mcRoute;        // FIXME replace with std::vector

    bool IPForward;

  public:
    Module_Class_Members(RoutingTable, cSimpleModule, 0);

    void initialize();

    /**
     * Raises an error.
     */
    void handleMessage(cMessage *);

    void printIfconfig();
    void printRoutingTable();

    /** @name Accessing the interfaces and the routing table */
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
     * Returns the InterfaceEntry specified by its index.
     */
    InterfaceEntry *getInterfaceByIndex(int index);

    /**
     * Returns the index of an interface given by its name.
     * Return -1 on error.
     */
    int findInterfaceByName(const char *name);

    /**
     * Returns the index of an interface given by its address.
     * Returns -1 on error.
     */
    int findInterfaceByAddress(const IPAddress& address);

    /**
     * Returns the gateway to send the destination,
     * address if the destination is not in routing table or there is
     * no gateway (local delivery).
     */
    IPAddress nextGatewayAddress(const IPAddress& dest);

    /**
     * Returns the number of interfaces.
     */
    int numInterfaces()  {return numIntrfaces;}
    //@}

    /** @name Route table manipulation functions */
    //@{

    /**
     * The add function adds a route to the routing table.
     * Returns true if the route could have been added correctly,
     * false if not.
     * FIXME should throw exception on error!
     *
     * @param target
     *          The destination network or host.
     *          The address is checked for being a multicast address,
     *          and the right routing table is modified.
     *          NULL indicates it is the default route.
     *
     * @param netmask
     *          The netmask of the route to be added.
     *          If NULL, the netmask is determined using the
     *          class of the destination address (0.0.0.0 for
     *          the default route).
     *
     * @param gw
     *          Any IP packets for the target network/host will
     *          be routed through the specified gateway.
     *          NB: The specified gateway must be reachable first.
     *          NULL means the target is a host, not NULL a gateway.
     *
     * @param metric
     *          Metric field in the routing table (used by routing daemons).
     *          Default is 0 (ie not used).
     *
     * @param dev
     *          Device to associate the route to.
     *          If NULL, the device will be tried to be determined alone
     *          (looking the existing interface and routing table).
     */
    bool add(const IPAddress& target,
             const IPAddress& netmask,
             const IPAddress& gw,
             int metric = 0,
             char *dev = NULL);

    /**
     * The del function deletes one or more routes from the routing table.
     * Returns true if the route could have been deleted correctly,
     * false if no routes have been found.
     *
     * If no target address of the routes to delete is given, the default route
     * is deleted. The target address is checked for being a multicast address,
     * and the right routing table is modified.
     *
     * If additional parameters are given, they are used to distinct the route
     * with the right target to delete.
     */
    bool del(const IPAddress& target,
             const IPAddress& netmask,
             const IPAddress& gw,
             int metric = 0,
             char *dev = NULL);

    /**
     * Returns the unicast route array.
     */
    cArray *getRouteTable()  {return route;}

    /**
     * Returns the default route entry.
     */
    RoutingEntry *getDefaultRoute()  {return defaultRoute;}
    //@}

    /** @name Multicast functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    bool multicastLocalDeliver(const IPAddress& dest);

    /**
     * Returns the port number to send the packets with dest as
     * multicast destination address, or -1 if destination is not in routing table.
     */
    int multicastOutputPortNo(const IPAddress& dest, int index);

    /**
     * Returns the number of multicast destinations.
     */
    int numMulticastDestinations(const IPAddress& dest);
    //@}
};

#endif

