// -*- C++ -*-
// $Header$
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

/*
	File: 		RoutingTable.h
	Purpose: 	Read in the interfaces and routing table from a file;
				Manage requests to the routing table and the interface table
				-- simulating the "route" and "ifconfig" commands
	Comment: 	OMNeT++ simple module without any gates;
		 		Requires function calls to it (message handling does nothing)
	Author: 	Jochen Reber
	Date: 		18.5.00
	On Linux: 	19.5.00 - 29.5.00
	Modified by	Vincent Oberle
	Date:		1.2.2001
*/

#ifndef __ROUTINGTABLE_H
#define __ROUTINGTABLE_H

#include <omnetpp.h>

// required for IPAddress typdef
#include "ip_address.h"

/*  ----------------------------------------------------------
        Constants
    ----------------------------------------------------------  */
const int 	MAX_FILESIZE = 5000;
const int 	MAX_INTERFACE_NO = 30;
const int 	MAX_ENTRY_STRING_SIZE = 20;
const int 	MAX_GROUP_STRING_SIZE = 160;

/* Route type */
enum RouteType {
	DIRECT,  // Directly attached to the router
	REMOTE   // Reached through another router
};

/* How the route was discovered */
enum RouteSource {
	MANUAL,
	RIP,
	OSPF,
	BGP
};


/*
 * Interface entry for the interface table.
 */
class InterfaceEntry : public cObject
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

/*
 * Routing entry.
 */
class RoutingEntry : public cObject
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


/*
 * Routing Table
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
	int readRoutingTableFromFile (const char *filename);
	char *createFilteredFile (char *file,
							  int &charpointer, 
							  const char *endtoken);
	void parseInterfaces(char *ifconfigFile);
	void parseRouting(char *routeFile);
	void addLocalLoopback();

	char *parseInterfaceEntry (char *ifconfigFile,
							   const char *tokenStr,
							   int &charpointer,
							   char* destStr);
	void parseMulticastGroups (char *groupStr, InterfaceEntry*);

	static int streq(const char *str1, const char *str2);
	static void skipBlanks (char *str, int &charptr);
	static int strcpyword (char *dest, const char *src);

  public:
	Module_Class_Members(RoutingTable, cSimpleModule, 0);

	void initialize();
	void handleMessage(cMessage *);

	void printIfconfig();
	void printRoutingTable();

	// Access interface/routing table

	// Look if the address is a local one, ie one of the host
	bool localDeliverNew(const IPAddress& dest);
	bool localDeliver(const char *dest) { // TO DEPRECATE
		return localDeliverNew(IPAddress(dest));
	}

	// Return the port nb to send the packets with dest as
	// destination address, or -1 if destination is not in routing table.
	int outputPortNoNew(const IPAddress& dest);
	int outputPortNo(const char *dest) { // TO DEPRECATE
		return outputPortNoNew(IPAddress(dest));
	}

	// Return the InterfaceEntry specified by its index.
	// TODO: this function is dirty... should be done differently
	// Take care, returns a pointer to InterfaceEntry now
	InterfaceEntry *getInterfaceByIndex(int index);
	
	// Search the index of an interface given by its name.
	int interfaceNameToNo(const char *name);

	// Search the index of an interface given by its address.
	int interfaceAddressToNoNew(const IPAddress& address);
	int interfaceAddressToNo(const char *address) {
		return interfaceAddressToNoNew(IPAddress(address));
	}

	// Returns the gateway to send the destination,
	IPAddress* nextGateway(const IPAddress *dest);

	// Return the number of interfaces
	int totalInterface() {
		return ifEntryCtr;
	}
	int interfaceNo() { // TO DEPRECATE
		return totalInterface();
	}


	// Following are new route table manip functions

	// The add function adds a route to the routing table.
	bool add(IPAddress *target = NULL,
			 IPAddress *netmask = NULL,
			 IPAddress *gw = NULL,
			 int metric = 0,
			 char *dev = NULL);

	// The del function deletes one or more routes from the routing table.
	bool del(IPAddress *target = NULL,
			 IPAddress *netmask = NULL,
			 IPAddress *gw = NULL,
			 int metric = 0,
			 char *dev = NULL);

	// Returns the unicast route array
	cArray *getRouteTable() {
		return route; 
	}
	// Return the default route entry
	RoutingEntry *getDefaultRoute() { 
		return defaultRoute; 
	}


	// Multicast functions

	// Look if the address is in one of the local multicast group address list.
	bool multicastLocalDeliverNew(const IPAddress& dest);
	bool multicastLocalDeliver(const char *dest) { // TO DEPRECATE
		return multicastLocalDeliverNew(IPAddress(dest));
	}

	// Return the port nb to send the packets with dest as
	// multicast destination address, or -1 if destination is not in routing table.
	int mcOutputPortNoNew(const IPAddress& dest, int index);
	int mcOutputPortNo(const char *dest, int index) { // TO DEPRECATE
		return mcOutputPortNoNew(IPAddress(dest), index);
	}

	// Return the number of multicast destinations.
	int totalMulticastDest(const IPAddress& dest);
	int multicastDestinations(const char *dest) { // TO DEPRECATE
		return totalMulticastDest(IPAddress(dest));
	}

	// To deprecate when IPAddress class will be used everywhere
	static bool isMulticastAddr(const char *dest) { // TO DEPRECATE
		return (IPAddress(dest).isMulticast());
	}

};

#endif

