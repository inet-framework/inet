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


/*  ----------------------------------------------------------
        Constants
    ----------------------------------------------------------  */
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
class InterfaceEntry : public cObject // FIXME why cObject
{
  public:
    int mtu;
    int metric;
    char *name;
    char *encap;
    char *hwAddrStr;
    IPAddress *inetAddr;
    IPAddress *bcastAddr;
    IPAddress *mask;
    bool broadcast, multicast, pointToPoint, loopback;

    int multicastGroupCtr;
    IPAddress **multicastGroup;

  public:
    InterfaceEntry();

    InterfaceEntry(const InterfaceEntry& obj);
    virtual ~InterfaceEntry() { }
    InterfaceEntry& operator=(const InterfaceEntry& obj);

    void print();
};

/**
 * Routing entry.
 */
class RoutingEntry : public cObject // FIXME why cObject
{
  public:
    int ref; // TO DEPRECATE

    // Destination
    IPAddress *host;

    // Route mask (replace it with a prefix?)
    IPAddress *netmask;

    // Next hop
    IPAddress *gateway;

    // Interface name and nb
    char *interfaceName;
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
    void *info;

  public:
    RoutingEntry();

    RoutingEntry(const RoutingEntry& obj);
    virtual ~RoutingEntry() { }
    RoutingEntry& operator=(const RoutingEntry& obj);

    void print();

    bool correspondTo(IPAddress *target, IPAddress *netmask,
                      IPAddress *gw, int metric, char *dev);
};


/**
 * Read in the interfaces and routing table from a file; manages requests
 * to the routing table and the interface table, simulating the "route"
 * and "ifconfig" commands.
 *
 * This is a simple module without gates, it requires function calls to it
 * (message handling does nothing).
 */
class RoutingTable: public cSimpleModule
{
  private:

    // Interfaces:

    // Number of interfaces
    int ifEntryCtr;
    // Interface array
    InterfaceEntry **intrface;
    // Loopback interface
    InterfaceEntry *loopbackInterface;

    // Routes:

    // Unicast route array
    cArray *route;
    // Default route
    RoutingEntry *defaultRoute;
    // Multicast route array
    cArray *mcRoute;

    bool IPForward;

  private:
    // Parsing functions

    // Read Routing Table file; return 0 on success, -1 on error
    int readRoutingTableFromFile (const char *filename);

    // Used to create specific "files" char arrays without comments or blanks
    // from original file.
    char *createFilteredFile (char *file,
                              int &charpointer,
                              const char *endtoken);

    // Go through the ifconfigFile char array, parse all entries and
    // write them into the interface table.
    // Loopback interface is not part of the file.
    void parseInterfaces(char *ifconfigFile);

    // Go through the routeFile char array, parse all entries line by line and
    // write them into the routing table.
    void parseRouting(char *routeFile);

    // Add the entry of the local loopback interface automatically
    void addLocalLoopback();

    char *parseInterfaceEntry (char *ifconfigFile,
                               const char *tokenStr,
                               int &charpointer,
                               char* destStr);

    // Convert string separated by ':' into dynamic string array.
    void parseMulticastGroups (char *groupStr, InterfaceEntry*);

    // Return 1 if beginning of str1 and str2 is equal up to str2-len,
    // otherwise 0.
    static int streq(const char *str1, const char *str2);

    // Skip blanks in string
    static void skipBlanks (char *str, int &charptr);

    // Copies the first word of src up to a space-char into dest
    // and appends \0, returns position of next space-char in src
    static int strcpyword (char *dest, const char *src);

  public:
    Module_Class_Members(RoutingTable, cSimpleModule, 0);

    void initialize();

    /**
     * Raises an error.
     */
    void handleMessage(cMessage *);

    void printIfconfig();
    void printRoutingTable();

    /** name Access interface/routing table */
    //@{

    /**
     * Look if the address is a local one, ie one of the host.
     */
    bool localDeliver(const IPAddress& dest);

    /**
     * Return the port nb to send the packets with dest as
     * destination address, or -1 if destination is not in routing table.
     */
    int outputPortNo(const IPAddress& dest);

    /**
     * Return the InterfaceEntry specified by its index.
     * Take care, returns a pointer to InterfaceEntry now!
     */
    InterfaceEntry *getInterfaceByIndex(int index);

    /**
     * Search the index of an interface given by its name.
     * Return -1 on error.
     */
    int interfaceNameToNo(const char *name);

    /**
     * Search the index of an interface given by its address.
     * Return -1 on error.
     */
    int interfaceAddressToNo(const IPAddress& address);

    /**
     * Returns the gateway to send the destination,
     * NULL if the destination is not in routing table or there is
     * no gateway (local delivery).
     */
    IPAddress* nextGateway(const IPAddress *dest);

    /**
     * Return the number of interfaces.
     */
    int numInterfaces()  {return ifEntryCtr;}
    //@}

    /** name Route table manipulation functions */
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
    bool add(IPAddress *target = NULL,
             IPAddress *netmask = NULL,
             IPAddress *gw = NULL,
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
    bool del(IPAddress *target = NULL,
             IPAddress *netmask = NULL,
             IPAddress *gw = NULL,
             int metric = 0,
             char *dev = NULL);

    /**
     * Returns the unicast route array
     */
    cArray *getRouteTable()  {return route;}

    /**
     * Return the default route entry
     */
    RoutingEntry *getDefaultRoute()  {return defaultRoute;}
    //@}

    /** name Multicast functions */
    //@{

    /**
     * Look if the address is in one of the local multicast group
     * address list.
     */
    bool multicastLocalDeliver(const IPAddress& dest);

    /**
     * Return the port nb to send the packets with dest as
     * multicast destination address, or -1 if destination is not in routing table.
     */
    int multicastOutputPortNo(const IPAddress& dest, int index);

    /**
     * Return the number of multicast destinations.
     */
    int numMulticastDestinations(const IPAddress& dest);
    //@}
};

#endif

