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
    File: 		RoutingTable.cc
    Purpose: 	Read in the routing table from a file;
				manage requests to the routing table
    Version 2: 	Seperates interfaces and routing information
				most options of ifconfig and route implemented
    Comment: 	OMNeT++ simple module without any gates;
				Requires function calls to it (message handling does nothing)
	Author: Jochen Reber
	Modified by	Vincent Oberle
	Date:		1.2.2001
*/

/*
 * Comment: add seperate loopback address from interface address
 * array, so that there is a perfect match between the interface
 * number and the output port.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "RoutingTable.h"


/*
 * Print on the console (first choice) or in the OMNeT window
 * (second choice).
 */
#define PRINTF  printf
//#define PRINTF  ev.printf


/*
 * Tokens that mark the route file.
 */
const char 	*IFCONFIG_START_TOKEN = "ifconfig:",
			*IFCONFIG_END_TOKEN = "ifconfigend.",
			*ROUTE_START_TOKEN = "route:",
			*ROUTE_END_TOKEN = "routeend.";


// --------------------------------------------------
//  InterfaceEntry and RoutingEntry classes function
// --------------------------------------------------

Register_Class( InterfaceEntry );

InterfaceEntry::InterfaceEntry()
{
	mtu = 0;
	metric = 0;

	name = NULL;
	encap = NULL;
	hwAddrStr = NULL;
	inetAddr = NULL;
	bcastAddr = NULL;
	mask = NULL;

	broadcast = false;
	multicast = false;
	pointToPoint= false;
	loopback = false;

	multicastGroupCtr = 0;
	multicastGroup = NULL;
}

InterfaceEntry::InterfaceEntry(const InterfaceEntry& obj) : cObject()
{
	setName(obj.cObject::name());
	operator=(obj);
}

InterfaceEntry& InterfaceEntry::operator=(const InterfaceEntry& obj)
{
	cObject::operator=(obj);
	mtu = obj.mtu;
	metric = obj.metric;
	strcpy(name, obj.name);
	strcpy(encap, obj.encap);
	strcpy(hwAddrStr, obj.hwAddrStr);
	inetAddr = obj.inetAddr;
	bcastAddr = obj.bcastAddr;
	mask = obj.mask;
	broadcast = obj.broadcast;
	multicast = obj.multicast;
	pointToPoint = obj.pointToPoint;
	loopback = obj.loopback;
	multicastGroupCtr = obj.multicastGroupCtr;
	for(int i = 0; i < multicastGroupCtr; i++)
		multicastGroup[i] = obj.multicastGroup[i];
	return *this;
}

void InterfaceEntry::print()
{
	PRINTF("name: %s\tencap: %s\t HWaddr: %s",
		   (name) ? name : "*",
		   (encap) ? encap : "*",
		   (hwAddrStr) ? hwAddrStr : "*");
	PRINTF("\n");
	if (inetAddr)
		PRINTF("inet addr:%s", inetAddr->getString());
	if (bcastAddr)
		PRINTF("\tBcast: %s", bcastAddr->getString());
	if (mask)
		PRINTF("\tMask: %s", mask->getString());
	PRINTF("\nMTU: %i\tMetric: %i", mtu, metric);
	PRINTF("\nGroups: ");
	for (int j = 0; j < multicastGroupCtr; j++) {
		if (multicastGroup[j]) {
			PRINTF("%s  ", multicastGroup[j]->getString());
		}
	}
	PRINTF("\n\t%s%s%s%s",
		   broadcast ? "  BROADCAST" : "",
		   multicast ? "  MULTICAST" : "",
		   pointToPoint ? "  POINTTOPOINT" : "",
		   loopback ? "  LOOPBACK" : "");
}

Register_Class( RoutingEntry );

RoutingEntry::RoutingEntry()
{
	ref = 0;

	host = NULL;
	gateway = NULL;
	netmask = NULL;

	interfaceName = NULL;
	interfaceNo = -1;

	metric = 0;
	type = DIRECT;
	source = MANUAL;

	age = -1;
	info = NULL;
}

RoutingEntry::RoutingEntry(const RoutingEntry& obj) : cObject()
{
	setName(obj.name());
	operator=(obj);
}

RoutingEntry& RoutingEntry::operator=(const RoutingEntry& obj)
{
	cObject::operator=(obj);
	ref = obj.ref;
	host = obj.host;
	gateway = obj.gateway;
	netmask = obj.netmask;
	strcpy(interfaceName, obj.interfaceName);
	interfaceNo = obj.interfaceNo;
	metric = obj.metric;
	type = obj.type;
	source = obj.source;
	age = obj.age;
	info = obj.info;
	return *this;
}

void RoutingEntry::print()
{
	PRINTF("\n%-16s %-16s %-16s %-3i %s %s %p",
		   (host) ? host->getString() : "*",
		   (gateway) ? gateway->getString() : "*", 
		   (netmask) ? netmask->getString() : "*", 
		   ref, 
		   (interfaceName) ? interfaceName : "*",
		   (type == DIRECT) ? "DIRECT" : "REMOTE",
		   (info) ? info : NULL);
}

/*
 * Indicates if a routing entry corresponds to the other parameters
 * (which can be null).
 */
bool RoutingEntry::correspondTo(IPAddress *target,
								IPAddress *netmask,
								IPAddress *gw,
								int metric,
								char *dev)
{
	if ( (target) && (!(target->isEqualTo(*host))) )
		return false;
	if ( (netmask) && (!(netmask->isEqualTo(*netmask))) )
		return false;
	if ( (gw) && (!(gw->isEqualTo(*gateway))) )
		return false;
	if ( (metric) && (metric != metric) )
		return false;
	if ( (dev) && (strcmp(dev, interfaceName)) )
		return false;

	return true;
}

// ------------------
//  Module Functions
// ------------------

Define_Module( RoutingTable );

void RoutingTable::initialize()
{
	IPForward = par("IPForward").boolValue();
	const char *filename = par("routingTableFileName");

	ifEntryCtr = 0;
	intrface = new InterfaceEntry *[MAX_INTERFACE_NO];

	route = new cArray("Route",  20);
	mcRoute = new cArray("MC Route", 30);

        // Read routing table file
        // Abort simulation if no routing file could be read
        if (readRoutingTableFromFile(filename) == -1)
          error("Error reading routing table file %s", filename);

	//printIfconfig();
	//printRoutingTable();
}

/* handleMessage just throws the message away */
void RoutingTable::handleMessage(cMessage *msg)
{
	// Nothing
}


// --------------
//  Print tables
// --------------

void RoutingTable::printIfconfig()
{
	InterfaceEntry *e;

    PRINTF("\n---- IF config ----");
    for (int i = -1; i < ifEntryCtr; i++) {
		// trick to add loopback interface in front
		e = (i == -1) ? loopbackInterface : (InterfaceEntry*)(intrface[i]);
		PRINTF("\n%d\t", i);
		e->print();
	}
    PRINTF("\n");
}

void RoutingTable::printRoutingTable()
{
    int i;

    PRINTF("\n-- Routing table --");
	PRINTF("\n%-16s %-16s %-16s %-3s %s",
		   "Destination", "Gateway", "Genmask", "Ref", "Iface");
    for (i = 0; i < route->items(); i++) {
		if (route->get(i)) {
			((RoutingEntry*)route->get(i))->print();
		}
    }

	if (defaultRoute) {
		defaultRoute->print();
	}

	PRINTF("\n");

    for (i = 0; i < mcRoute->items(); i++) {
		if (mcRoute->get(i)) {
			((RoutingEntry*)mcRoute->get(i))->print();
		}
    }
    PRINTF("\n");
}



// -----------------------------------
//  Access of interface/routing table
// -----------------------------------

/*
 * Look if the address is a local one, ie one of the host
 * TODO: remove the "New" when all IP-Suite is converted to class IPAddress
 */
bool RoutingTable::localDeliverNew(const IPAddress& dest)
{
	for (int i = 0; i < ifEntryCtr; i++) {
		if (dest.isEqualTo(*(intrface[i]->inetAddr))) {
			return true;
		}
	}

	if (dest.isEqualTo(*(loopbackInterface->inetAddr))) {
    	ev << "LOCAL LOOPBACK INVOKED\n";
		return true;
	}

	return false;
}

/*
 * Look if the address is in one of the local multicast group
 * address list.
 * TODO: remove the "New" when all IP-Suite is converted to class IPAddress
 */
bool RoutingTable::multicastLocalDeliverNew(const IPAddress& dest)
{
	int i, j;

	for (i = 0; i < ifEntryCtr; i++) {
		for (j = 0; j < intrface[i]->multicastGroupCtr; j++) {
			if (dest.isEqualTo(*(intrface[i]->multicastGroup[j]))) {
				return true;
			}
		}
	}

	return false;
}



/*
 * Return the port nb to send the packets with dest as
 * destination address, or -1 if destination is not in routing table.
 * TODO: remove the "New" when all IP-Suite is converted to class IPAddress
 */
int RoutingTable::outputPortNoNew(const IPAddress& dest)
{
	RoutingEntry *e;

	for (int i = 0; i < route->items(); i++) {
		if (route->get(i)) {
			// The destination in the datagram should /always/ be
			// compared against the destination-address of the interface,
			// and the gateway will be used on Layer 2.
			// Theoretically, there should be a differentiation between
			// Host and Network (rather than between Host and Gateway),
			// but none is made here.
			// -- Jochen Reber, 27.10.00
			e = (RoutingEntry*)route->get(i);
			if (IPAddress::maskedAddrAreEqual(&dest, e->host, e->netmask)) {
				return interfaceNameToNo(e->interfaceName);
			}
		}
	}

	// Is it gateway here?
	if (defaultRoute) {
		return interfaceNameToNo(defaultRoute->interfaceName);
	}

	return -1;
}


/*
 * Return the port nb to send the packets with dest as
 * multicast destination address, or -1 if destination is not in routing table.
 * TODO: remove the "New" when all IP-Suite is converted to class IPAddress
 */
int RoutingTable::mcOutputPortNoNew(const IPAddress& dest, int index)
{
	int i, mcDestCtr;
	RoutingEntry *e;

	if (index >= mcRoute->items()) {
		ev << "ERROR: wrong multicast port index\n";
		return -1;
	}

	mcDestCtr = 0;
	i = -1;
	while (mcDestCtr < index + 1) {
		i++;

		// error
		if (i == mcRoute->items()) {
			ev << "ERROR: wrong multicast port index\n";
			return -1;
		}

		if (mcRoute->get(i)) {
			e = (RoutingEntry*)mcRoute->get(i);
			if (IPAddress::maskedAddrAreEqual(&dest, e->host, e->netmask)) {
				mcDestCtr++;
			}
		} 
	}
	
	return interfaceNameToNo(e->interfaceName);
}

// Confusing name
//bool RoutingTable::routingEntryExists(const char *dest)
//{
//	return (outputPortNo(dest) == -1 ? false : true);
//}


/*
 * Return the number of multicast destinations.
 */
int RoutingTable::totalMulticastDest(const IPAddress& dest)
{
	int mcDestCtr = 0;
	RoutingEntry *e;

	for (int i = 0; i < mcRoute->items(); i++) {
		if (mcRoute->get(i)) {
			e = (RoutingEntry*)mcRoute->get(i);
			if (IPAddress::maskedAddrAreEqual(&dest, e->host, e->netmask)) {
				mcDestCtr++;
			}
		}
	}
	return mcDestCtr;
}


/*
 * Return the InterfaceEntry specified by its index.
 * TODO: this function is dirty... should be done differently
 */
InterfaceEntry *RoutingTable::getInterfaceByIndex(int index)
{
	InterfaceEntry *interf;

	if (index >= ifEntryCtr) {
		ev <<"\nNon existant interface asked (with getInterfaceByIndex)\n";
		interf = new InterfaceEntry();
		interf->name = "nonexistant";
	} else {
		interf = intrface[index];
	}
	return interf;
}

/*
 * Search the index of an interface given by its name.
 * Return -1 on error.
 */
int RoutingTable::interfaceNameToNo(const char *name)
{
	if (name == NULL) return -1;

	for (int i = 0; i < ifEntryCtr; i++) {
		if (!strcmp(name, intrface[i]->name)) {
			return i;
		}
	}
	// loopback interface has no number
	return -1;
}

/*
 * Search the index of an interface given by its address.
 * Return -1 on error.
 */
int RoutingTable::interfaceAddressToNoNew(const IPAddress& addr)
{
	if (&addr == NULL) return -1;

	for (int i = 0; i < ifEntryCtr; i++) {
		//if (addr->isEqualTo(interface[i]->inetAddr)) {
		if (IPAddress::maskedAddrAreEqual(&addr,
										  intrface[i]->inetAddr,
										  intrface[i]->mask)) {
			return i;
		}
	}

// BCH Andras -- code from UTS MPLS model
    // Add number for loopback
    if((loopbackInterface->inetAddr->getInt()) == (addr.getInt()))
        return 0;
// ECH
	return -1;
}

/*
 * Returns the gateway to send the destination,
 * NULL if the destination is not in routing table or there is 
 * no gateway (local delivery)
 */
IPAddress* RoutingTable::nextGateway(const IPAddress *dest)
{
	RoutingEntry *e;

	for (int i = 0; i < route->items(); i++) {
		if (route->get(i)) {
			e = (RoutingEntry*)route->get(i);
			if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask)) {
				return e->gateway;
			}
		}
	}

	if (defaultRoute) {
		return defaultRoute->gateway;
	}

	return NULL;
}


/*
 * The add function adds a route to the routing table.
 * Returns true if the route could have been added correctly,
 * false if not.
 *
 * Parameters:
 * target	The destination network or host.
 *			The address is checked for being a multicast address,
 *			and the right routing table is modified.
 *			NULL indicates it is the default route.
 * netmask	The netmask of the route to be added.
 *			If NULL, the netmask is determined using the
 *			class of the destination address (0.0.0.0 for 
 *			the default route).
 * gw		Any IP packets for the target network/host will
 *			be routed through the specified gateway.
 *			NB: The specified gateway must be reachable first.
 *			NULL means the target is a host, not NULL a gateway.
 * metric	Metric field in the routing table
 *			(used by routing daemons). Default is 0 (ie not used).
 * dev		Device to associate the route to.
 *			If NULL, the device will be tried to be determined alone
 *			(looking the existing interface and routing table).
 */
bool RoutingTable::add(IPAddress *target,
					   IPAddress *netmask,
					   IPAddress *gw,
					   int metric,
					   char *dev)
{
	RoutingEntry *e = new RoutingEntry();
	int dev_nb;

	e->host = target;
	// We add at the end, if everything is alright

	if (netmask) {
		e->netmask = netmask;
	} else {
		// If class D or E, the netmask is NULL here
		e->netmask = target->getNetworkMask();
	}

	if (gw) {
		e->gateway = gw;
		e->type = REMOTE;
	} else {
		e->gateway = new IPAddress();
		e->type = DIRECT;
	}

	e->metric = metric;

	if (dev) {
		dev_nb = interfaceNameToNo(dev);
		if (dev_nb != -1) {
			// The interface exists
			e->interfaceName = dev;
			e->interfaceNo = dev_nb;
		} else {
			// The interface doesn't exist
			ev <<"Adding a route failed: unexistant interface\n";
			return false;
		}
	} else {
		// Trying to find the interface
		if (e->type == REMOTE) {
			// If it's a gateway entry, looking in the routing table
			// for existing route for the gateway value
			dev_nb = outputPortNoNew(*gw);
			if (dev_nb != -1) {
				// Route for the gateway exists
				e->interfaceNo = dev_nb;
				e->interfaceName = intrface[dev_nb]->name;
			} else {
				// No route for the gateway, looking in the interface table
				dev_nb = interfaceAddressToNoNew(*gw);
				if (dev_nb != -1) {
					// The interface for the gateway address exists
					e->interfaceNo = dev_nb;
					e->interfaceName = intrface[dev_nb]->name;
				} else {
					// The interface doesn't exist
					ev <<"Adding a route failed: no interface found\n";
					return false;
				}
			}
		} else {
			// If it's a host entry, looking in the interface table
			// for the target address
			dev_nb = interfaceAddressToNoNew(*(e->host));
			if (dev_nb != -1) {
				// The interface for the gateway address exists
				e->interfaceNo = dev_nb;
				e->interfaceName = intrface[dev_nb]->name;
			} else {
				// The interface doesn't exist
				ev <<"Adding a route failed: no interface found\n";
				return false;
			}
		}
	}

	// Adding...
	// If not the default route
	if (target) {
		// Check if entry is for multicast address
		if (!(e->host->isMulticast())) {
			route->add(e);
		} else {
			mcRoute->add(e);
		}
	} else {
		defaultRoute = e;
	}

	return true;
}

/*
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
bool RoutingTable::del(IPAddress *target,
					   IPAddress *netmask,
					   IPAddress *gw,
					   int metric,
					   char *dev)
{
	bool res = false;

	if (target) {
		// A target is given => not default route
		RoutingEntry *e;
		for (int i = 0; i < route->items(); i++) {
			if (route->get(i)) {
				e = (RoutingEntry*)route->get(i);
				if (e->correspondTo(target, netmask, gw, metric, dev)) {
					if (!(e->host->isMulticast())) {
						route->remove(e);
					} else {
						mcRoute->remove(e);
					}
					res = true;
				}
			}
		}
	} else {
		// Delete default entry
		if (defaultRoute->correspondTo(target, netmask, gw, metric, dev)) {
			defaultRoute = NULL;
			res = true;
		}
	}

	return res;
}


// ------------------------------------------------
//  Private Functions: main file reading functions
// ------------------------------------------------

/*
 * Return 1 if beginning of str1 and str2 is equal up to str2-len, 
 * otherwise 0.
 */
int RoutingTable::streq(const char *str1, const char *str2)
{
	return (strncmp(str1, str2, strlen(str2)) == 0);
}

/*
 * Copies the first word of src up to a space-char into dest
 * and appends \0, returns position of next space-char in src
 */
int RoutingTable::strcpyword(char *dest, const char *src)
{
	int i;
	for(i = 0; !isspace(dest[i] = src[i]); i++) ;
	dest[i] = '\0';
	return i;
}

/* Skip blanks in string */
void RoutingTable::skipBlanks (char *str, int &charptr)
{
	for(;isspace(str[charptr]); charptr++) ;
}			

/*
 * Read Routing Table file
 * return 0 on success, -1 on error
 */
int RoutingTable::readRoutingTableFromFile (const char *filename)
{
    FILE *fp;
	int charpointer;
	char *file = new char[MAX_FILESIZE];
	char *ifconfigFile;
	char *routeFile;

    fp = fopen(filename, "r");
    if (fp == NULL)
		error("Error on opening routing table file.");

    // read the whole into the file[] char-array
    for (charpointer = 0;
		 (file[charpointer] = getc(fp)) != EOF;
		 charpointer++) ;

	charpointer++;
    for (; charpointer < MAX_FILESIZE; charpointer++)
		file[charpointer] = '\0';
	//    file[++charpointer] = '\0';

	fclose(fp);


	// copy file into specialized, filtered char arrays
	for (charpointer = 0;
		 (charpointer < MAX_FILESIZE) && (file[charpointer] != EOF);
		 charpointer++) {
		// check for tokens at beginning of file or line
		if (charpointer == 0 || file[charpointer - 1] == '\n') {
			// copy into ifconfig filtered chararray
			if (streq(file + charpointer, IFCONFIG_START_TOKEN)) {
				ifconfigFile = createFilteredFile(file,
												  charpointer,
												  IFCONFIG_END_TOKEN);
				//PRINTF("Filtered File 1 created:\n%s\n", ifconfigFile);
			}

			// copy into route filtered chararray
			if (streq(file + charpointer, ROUTE_START_TOKEN)) {
				routeFile = createFilteredFile(file,
											   charpointer,
											   ROUTE_END_TOKEN);
				//PRINTF("Filtered File 2 created:\n%s\n", routeFile);
			}
		}
	}

	delete file;

	// parse filtered files
	parseInterfaces(ifconfigFile);
	addLocalLoopback();
	parseRouting(routeFile);

	delete ifconfigFile;
	delete routeFile;
	
	return 0;

}

/*
 * Used to create specific "files" char arrays
 * without comments or blanks from original file.
 */
char *RoutingTable::createFilteredFile (char *file,
										int &charpointer,
										const char *endtoken)
{
	int i = 0;
	char *filterFile = new char[MAX_FILESIZE];
    filterFile[0] = '\0';
	
	while(true) {
		// skip blank lines and comments
		while ( !isalnum(file[charpointer]) ) {
			while (file[charpointer++] != '\n') ;
		}
		
		// check for endtoken:
		if (streq(file + charpointer, endtoken)) {
			filterFile[i] = '\0';
			break;
		}

		// copy whole line to filterFile
		while ((filterFile[i++] = file[charpointer++]) != '\n') ;
	}

	return filterFile;
}

/*
 * Go through the ifconfigFile char array, parse all entries and
 * write them into the interface table.
 * Loopback interface is not part of the file.
 */
void RoutingTable::parseInterfaces(char *ifconfigFile)
{
	int charpointer = 0;
	InterfaceEntry *e;

	// parsing of entries in interface definition
	while(ifconfigFile[charpointer] != '\0') {
		// name entry 
		if (streq(ifconfigFile + charpointer, "name:")) {
			e = new InterfaceEntry();
			intrface[ifEntryCtr] = e;
			ifEntryCtr++; // ready for the next one
			e->name = parseInterfaceEntry(ifconfigFile, "name:", charpointer,
										  new char[MAX_ENTRY_STRING_SIZE]);
			continue;
		}

		// encap entry
		if (streq(ifconfigFile + charpointer, "encap:")) {
			e->encap = parseInterfaceEntry(ifconfigFile, "encap:", charpointer,
										   new char[MAX_ENTRY_STRING_SIZE]);
			continue;
		}

		// HWaddr entry
		if (streq(ifconfigFile + charpointer, "HWaddr:")) {
			e->hwAddrStr = parseInterfaceEntry(ifconfigFile, "HWaddr:", charpointer,
											   new char[MAX_ENTRY_STRING_SIZE]);
			continue;
		}

		// inet_addr entry
		if (streq(ifconfigFile + charpointer, "inet_addr:")) {
			e->inetAddr = new IPAddress(
				parseInterfaceEntry(ifconfigFile, "inet_addr:", charpointer,
									new char[MAX_ENTRY_STRING_SIZE]));
			continue;
		}

		// Broadcast address entry
		if (streq(ifconfigFile + charpointer, "Bcast:")) {
			e->bcastAddr = new IPAddress(
				parseInterfaceEntry(ifconfigFile, "Bcast:", charpointer,
									new char[MAX_ENTRY_STRING_SIZE]));
			continue;
		}

		// Mask entry
		if (streq(ifconfigFile + charpointer, "Mask:")) {
			e->mask = new IPAddress(
				parseInterfaceEntry(ifconfigFile, "Mask:", charpointer,
									new char[MAX_ENTRY_STRING_SIZE]));
			continue;
		}

		// Multicast groups entry
		if (streq(ifconfigFile + charpointer, "Groups:")) {
			char *grStr = parseInterfaceEntry(ifconfigFile, "Groups:",
											  charpointer,
											  new char[MAX_GROUP_STRING_SIZE]);
			//PRINTF("\nMulticast gr str: %s\n", grStr);
			parseMulticastGroups(grStr, e);
			continue;
		}

		// MTU entry
		if (streq(ifconfigFile + charpointer, "MTU:")) {
			e->mtu = atoi(
				parseInterfaceEntry(ifconfigFile, "MTU:", charpointer,
									new char[MAX_ENTRY_STRING_SIZE]));
			continue;
		}

		// Metric entry
		if (streq(ifconfigFile + charpointer, "Metric:")) {
			e->metric = atoi(
				parseInterfaceEntry(ifconfigFile, "Metric:", charpointer,
									new char[MAX_ENTRY_STRING_SIZE]));
			continue;
		}

		// BROADCAST Flag
		if (streq(ifconfigFile + charpointer, "BROADCAST")) {
			e->broadcast = true;
			charpointer += strlen("BROADCAST");
			skipBlanks(ifconfigFile, charpointer);
			continue;
		}

		// MULTICAST Flag
		if (streq(ifconfigFile + charpointer, "MULTICAST")) {
			e->multicast = true;
			charpointer += strlen("MULTICAST");
			skipBlanks(ifconfigFile, charpointer);
			continue;
		}

		// POINTTOPOINT Flag
		if (streq(ifconfigFile + charpointer, "POINTTOPOINT")) {
			e->pointToPoint= true;
			charpointer += strlen("POINTTOPOINT");
			skipBlanks(ifconfigFile, charpointer);
			continue;
		}

		// no entry discovered: move charpointer on
		charpointer++;
	}
	
	// add default multicast groups to all interfaces without
	// set multicast group field
	for (int i = 0; i < ifEntryCtr; i++) {
		InterfaceEntry *interf = (InterfaceEntry*)intrface[i];
		if (interf->multicastGroupCtr == 0) {
			char emptyGroupStr[MAX_GROUP_STRING_SIZE];
			strcpy(emptyGroupStr, "");
			parseMulticastGroups(emptyGroupStr, interf);
		}
	}
}

char *RoutingTable::parseInterfaceEntry (char *ifconfigFile,
										 const char *tokenStr,
										 int &charpointer,
										 char* destStr)
{
	int temp = 0;

	charpointer += strlen(tokenStr);
	skipBlanks(ifconfigFile, charpointer);
	temp = strcpyword(destStr, ifconfigFile + charpointer);
	charpointer += temp;

	skipBlanks(ifconfigFile, charpointer);

	return destStr;
}

/* Convert string separated by ':' into dynamic string array. */
void RoutingTable::parseMulticastGroups (char *groupStr, 
										 InterfaceEntry *itf)
{
	int i, j, groupNo;

	itf->multicastGroupCtr = 1;

	// add "224.0.0.1" automatically,
	// use ":"-separator only if string not empty
	if (!strcmp(groupStr, "")) {
		strcat(groupStr, "224.0.0.1");
	} else { // string not empty, use seperator
		strcat(groupStr, ":224.0.0.1");
	}

	// add 224.0.0.2" only if Router (IPForward == true)
	if (IPForward) {
		strcat(groupStr, ":224.0.0.2");
	}

	// count number of group entries
	for (i = 0; groupStr[i] != '\0'; i++) {
		if (groupStr[i] == ':')
			itf->multicastGroupCtr++;
	}

	char *str = new char[ADDRESS_STRING_SIZE];
	itf->multicastGroup = new IPAddress *[itf->multicastGroupCtr];

	// Create the different IPAddress
	for (i = 0, j = 0, groupNo = 0; groupStr[i] != '\0'; i++, j++) {
		// Skip to next multicast group, if separator found
		// it's a bit a ugly...
		if (groupStr[i] == ':') {
			str[j] = '\0';
			itf->multicastGroup[groupNo] = new IPAddress(str);
			j = -1;
			groupNo++;
			continue;
		}		
		if (groupStr[i + 1] == '\0') {
			str[j] = groupStr[i];
			str[j + 1] = '\0';
			itf->multicastGroup[groupNo] = new IPAddress(str);
			break;
		}		
		str[j] = groupStr[i];
	}
}


/*
 * Add the entry of the local loopback interface automatically
 */
void RoutingTable::addLocalLoopback()
{
	loopbackInterface = new InterfaceEntry();

	loopbackInterface->name = "lo";
	loopbackInterface->encap = "Local Loopback";
	//loopbackInterface->inetAddr = new IPAddress("127.0.0.1");
	//loopbackInterface->mask = new IPAddress("255.0.0.0");
	
// BCH Andras -- code from UTS MPLS model
    cModule *curmod = this;
    IPAddress* loopbackIP = new IPAddress("127.0.0.1");

    for (curmod = parentModule(); curmod != NULL;curmod = curmod->parentModule())
    {
        if (curmod->hasPar("local_addr"))
        {
                 loopbackIP = new
                IPAddress(curmod->par("local_addr").stringValue());

            break;
        }

    }
    ev << "My loopback Address is : " << loopbackIP->getString() << "\n";
    
    loopbackInterface->inetAddr = loopbackIP;
    loopbackInterface->mask = new IPAddress("255.255.255.255");
// ECH
	
	loopbackInterface->mtu = 3924;
	loopbackInterface->metric = 1;
	loopbackInterface->loopback = true;

	// add default multicast groups
	char emptyGroupStr[MAX_GROUP_STRING_SIZE];
	strcpy(emptyGroupStr, "");
	parseMulticastGroups(emptyGroupStr, loopbackInterface);
}

/*
 * Go through the routeFile char array, parse all entries line by line and
 * write them into the routing table.
 */
void RoutingTable::parseRouting(char *routeFile)
{
	int i, charpointer = 0;
	RoutingEntry *e;
	char *str = new char[MAX_ENTRY_STRING_SIZE];
	InterfaceEntry *interf;

	charpointer += strlen(ROUTE_START_TOKEN);
	skipBlanks(routeFile, charpointer);
	while (routeFile[charpointer] != '\0') {
		// 1st entry: Host
		charpointer += strcpyword(str, routeFile + charpointer);
		skipBlanks(routeFile, charpointer);
		e = new RoutingEntry();
		// if entry is not the default entry
		if (strcmp(str, "default:")) {
			e->host = new IPAddress(str);
			// check if entry is for multicast address
			if (!(e->host->isMulticast())) {
				route->add(e);
			} else {
				mcRoute->add(e);
			}
		}
		// default entry
		else {
			defaultRoute = e;
		}

		// 2nd entry: Gateway
		charpointer += strcpyword(str, routeFile + charpointer);
		skipBlanks(routeFile, charpointer);
		if (!strcmp(str, "*") || !strcmp(str, "0.0.0.0")) {
			e->gateway = new IPAddress("0.0.0.0");
		} else {
			e->gateway = new IPAddress(str);
		}

		// 3rd entry: Netmask
		charpointer += strcpyword(str, routeFile + charpointer);
		skipBlanks(routeFile, charpointer);
		e->netmask = new IPAddress(str);
		
		// 4th entry: flags
		charpointer += strcpyword(str, routeFile + charpointer);
		skipBlanks(routeFile, charpointer);
		// parse flag-String to set flags
		for(i = 0; str[i]; i++) {
			if (str[i] == 'H') {
				e->type = DIRECT;
			} else { // if (str[i] == 'G') {
				e->type = REMOTE;
			}
		}

		// 5th entry: references (unsupported by Linux)
		charpointer += strcpyword(str, routeFile + charpointer);
		skipBlanks(routeFile, charpointer);
		e->ref = atoi(str);

		// 6th entry: interfaceName
		e->interfaceName = new char[MAX_ENTRY_STRING_SIZE];
		charpointer += strcpyword(e->interfaceName, routeFile + charpointer);
		skipBlanks(routeFile, charpointer);
		for (i = 0; i < ifEntryCtr; i++) {
			InterfaceEntry *interf = (InterfaceEntry*)intrface[i];
			//PRINTF("\ne->interfaceName %s", e->interfaceName);fflush(stdout);
			//PRINTF("\ninterf->name %s", interf->name);fflush(stdout);
			if (!strcmp(e->interfaceName, interf->name)) {
				e->interfaceNo = i;
				break;
			}
		}
	}
}

