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


//  Cleanup and rewrite: Andras Varga, 2004

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sstream>

#include "RoutingTable.h"
#include "RoutingTableParser.h"


//#define PRINTF  printf
#define PRINTF  ev.printf



InterfaceEntry::InterfaceEntry()
{
    mtu = 0;
    metric = 0;

    broadcast = false;
    multicast = false;
    pointToPoint= false;
    loopback = false;

    multicastGroupCtr = 0;
    multicastGroup = NULL;
}

void InterfaceEntry::info(char *buf)
{
    std::stringstream out;
    out << (!name.empty() ? name.c_str() : "*");
    out << "  e:" << (!encap.empty() ? encap.c_str() : "*");
    out << "  HW:" << (!hwAddrStr.empty() ? hwAddrStr.c_str() : "*");
    out << "  ia:" << inetAddr << "  M:" << mask;
    out << "...";
    strcpy(buf, out.str().c_str());
}

std::string InterfaceEntry::detailedInfo() const
{
    std::stringstream out;
    out << "name:" << (!name.empty() ? name.c_str() : "*");
    out << "\tencap:" << (!encap.empty() ? encap.c_str() : "*");
    out << "\tHWaddr:" << (!hwAddrStr.empty() ? hwAddrStr.c_str() : "*");
    out << "\n";

    if (!inetAddr.isNull())
        out << "inet addr:" << inetAddr;
    if (!bcastAddr.isNull())
        out << "\tBcast: " << bcastAddr;
    if (!mask.isNull())
        out << "\tMask: " << mask;
    out << "\n";

    out << "MTU: " << mtu << " \tMetric: " << metric << "\n";

    out << "Groups:";
    for (int j=0; j<multicastGroupCtr; j++)
        if (!multicastGroup[j].isNull())
            out << "  " << multicastGroup[j];
    out << "\n";

    if (broadcast) out << "BROADCAST ";
    if (multicast) out << "MULTICAST ";
    if (pointToPoint) out << "POINTTOPOINT ";
    if (loopback) out << "LOOPBACK ";
    out << "\n";

    return out.str();
}

void InterfaceEntry::print()
{
    PRINTF("%s", detailedInfo().c_str());
}

//================================================================

RoutingEntry::RoutingEntry()
{
    ref = 0;

    interfaceNo = -1;

    metric = 0;
    type = DIRECT;
    source = MANUAL;

    age = -1;
    routeInfo = NULL;
}

void RoutingEntry::info(char *buf)
{
    std::stringstream out;
    if (host.isNull()) out << "*  "; else out << host << "  ";
    if (gateway.isNull()) out << "*  "; else out << gateway << "  ";
    if (netmask.isNull()) out << "*  "; else out << netmask << "  ";
    out << ref << "  ";
    if (interfaceName.empty()) out << "*  "; else out << interfaceName.c_str() << "  ";
    out << (type==DIRECT ? "DIRECT" : "REMOTE") << "  ";
    out << (routeInfo ? routeInfo : NULL);
    strcpy(buf, out.str().c_str());
}

std::string RoutingEntry::detailedInfo() const
{
    return std::string();
}


//==============================================================


Define_Module( RoutingTable );


void RoutingTable::initialize()
{
    IPForward = par("IPForward").boolValue();
    const char *filename = par("routingTableFileName");

    numIntrfaces = 0;
    intrface = new InterfaceEntry *[MAX_INTERFACE_NO];

    route = new cArray("routes");
    mcRoute = new cArray("multicast routes");

    defaultRoute = NULL;
    loopbackInterface = NULL;

    // Read routing table file
    // Abort simulation if no routing file could be read
    RoutingTableParser parser(this);
    if (parser.readRoutingTableFromFile(filename) == -1)
        error("Error reading routing table file %s", filename);

    //printIfconfig();
    //printRoutingTable();
}


void RoutingTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}


// --------------
//  Print tables
// --------------

void RoutingTable::printIfconfig()
{
    InterfaceEntry *e;

    PRINTF("\n---- IF config ----");
    for (int i = -1; i < numIntrfaces; i++) {
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

bool RoutingTable::localDeliver(const IPAddress& dest)
{
    Enter_Method("localDeliver(%s) y/n", dest.str().c_str());

    for (int i = 0; i < numIntrfaces; i++) {
        if (dest.equals(intrface[i]->inetAddr)) {
            return true;
        }
    }

    if (dest.equals(loopbackInterface->inetAddr)) {
        ev << "LOCAL LOOPBACK INVOKED\n";
        return true;
    }

    return false;
}

bool RoutingTable::multicastLocalDeliver(const IPAddress& dest)
{
    Enter_Method("multicastLocalDeliver(%s) y/n", dest.str().c_str());

    for (int i = 0; i < numIntrfaces; i++) {
        for (int j = 0; j < intrface[i]->multicastGroupCtr; j++) {
            if (dest.equals(intrface[i]->multicastGroup[j])) {
                return true;
            }
        }
    }

    return false;
}


int RoutingTable::outputPortNo(const IPAddress& dest)
{
    Enter_Method("outputPortNo(%s)=?", dest.str().c_str());

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
            //
            // FIXME shouldn't we do *best* match here? This looks like first match.
            // -- Andras
            //
            e = (RoutingEntry*)route->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask)) {
                return findInterfaceByName(e->interfaceName.c_str());
            }
        }
    }

    // Is it gateway here?
    if (defaultRoute) {
        return findInterfaceByName(defaultRoute->interfaceName.c_str());
    }

    return -1;
}


int RoutingTable::multicastOutputPortNo(const IPAddress& dest, int index)
{
    Enter_Method("multicastOutputPortNo(%s, %d)=?", dest.str().c_str(), index);

    if (index >= mcRoute->items())
        opp_error("wrong multicast port index");

    int mcDestCtr = 0;
    int i = -1;
    RoutingEntry *e = NULL;
    while (mcDestCtr < index + 1) {
        i++;

        if (i == mcRoute->items())
            opp_error("wrong multicast port index");

        if (mcRoute->get(i)) {
            e = (RoutingEntry*)mcRoute->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask))
                mcDestCtr++;
        }
    }

    // FIXME what if e==NULL?
    return findInterfaceByName(e->interfaceName.c_str());
}

int RoutingTable::numMulticastDestinations(const IPAddress& dest)
{
    int mcDestCtr = 0;
    RoutingEntry *e;

    for (int i = 0; i < mcRoute->items(); i++) {
        if (mcRoute->get(i)) {
            e = (RoutingEntry*)mcRoute->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask)) {
                mcDestCtr++;
            }
        }
    }
    return mcDestCtr;
}


InterfaceEntry *RoutingTable::getInterfaceByIndex(int index)
{
    if (index<0 || index>=numIntrfaces)
        opp_error("getInterfaceByIndex(): nonexistent interface %d", index);

    InterfaceEntry *interf = intrface[index];
    return interf;
}


int RoutingTable::findInterfaceByName(const char *name)
{
    Enter_Method("findInterfaceByName(%s)=?", name);
    if (!name)
        return -1;

    for (int i=0; i<numIntrfaces; i++)
        if (!strcmp(name, intrface[i]->name.c_str()))
            return i;

    // loopback interface has no number
    return -1;
}


int RoutingTable::findInterfaceByAddress(const IPAddress& addr)
{
    Enter_Method("findInterfaceByAddress(%s)=?", addr.str().c_str());
    if (addr.isNull())
        return -1;

    for (int i = 0; i < numIntrfaces; i++)
        if (IPAddress::maskedAddrAreEqual(addr,intrface[i]->inetAddr,intrface[i]->mask))
            return i;

// BCH Andras -- code from UTS MPLS model
    // FIXME this seems to be a bloody hack. Why do we return 0 when intrface[0]
    // is obviously NOT what we were looking for???  --Andras

    // Add number for loopback
    if((loopbackInterface->inetAddr.getInt()) == (addr.getInt()))
        return 0;
// ECH
    return -1;
}


IPAddress RoutingTable::nextGatewayAddress(const IPAddress& dest)
{
    Enter_Method("nextGatewayAddress(%s)=?", dest.str().c_str());

    for (int i = 0; i < route->items(); i++) {
        if (route->get(i)) {
            RoutingEntry *e = (RoutingEntry*)route->get(i);
            if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask)) {
                return e->gateway;
            }
        }
    }

    if (defaultRoute)
        return defaultRoute->gateway;

    return IPAddress();
}

int RoutingTable::numRoutingEntries()
{
    return routes.size()+multicastRoutes.size()+(defaultRoute?1:0);
}

RoutingEntry *routingEntry(int k)
{
    if (k==0 && defaultRoute)
        return defaultRoute;
    k -= (defaultRoute?1:0);
    if (k < routes.size())
        return routes[k];
    k -= routes.size()
    if (k < multicastRoutes.size())
        return multicastRoutes[k];
    return NULL;
}

RoutingEntry *RoutingTable::findRoutingEntry(const IPAddress& target,
                                             const IPAddress& netmask,
                                             const IPAddress& gw,
                                             int metric = 0,
                                             char *dev = NULL)
{
    int n = numRoutingEntries();
    for (int i=0; i<n; i++)
        if (routingEntryMatches(routingEntry(i), target, netmask, gw, metric, dev))
            return routingEntry(i);
}

void RoutingTable::addRoutingEntry(RoutingEntry *entry)
{
    Enter_Method("addRoutingEntry(...)");

    if (target.isNull())
    {
        delete defaultRoute;
        defaultRoute = entry;
    }
    else if (!e->host.isMulticast())
    {
        routes.push_back(entry);
    }
    else
    {
        multicastRoutes.push_back(entry);
    }
}


bool RoutingTable::deleteRoutingEntry(RoutingEntry *entry)
{
    Enter_Method("deleteRoutingEntry(...)");

    RouteVector::iterator i = find(routes.begin(), routes.end(), entry);
    if (i!=routes.end())
    {
        routes.erase(i);
        delete entry;
        return true;
    }
    i = find(multicastRoutes.begin(), multicastRoutes.end(), entry);
    if (i!=multicastRoutes.end())
    {
        multicastRoutes.erase(i);
        delete entry;
        return true;
    }
    if (entry==defaultRoute)
    {
        delete defaultRoute;
        defaultRoute = NULL;
        return true;
    }
    return false;
}


bool RoutingTable::routingEntryMatches(RoutingEntry *entry,
                                const IPAddress& target,
                                const IPAddress& nmask,
                                const IPAddress& gw,
                                int metric,
                                const char *dev)
{
    if (!target.isNull() && !target.equals(entry->host))
        return false;
    if (!nmask.isNull() && !nmask.equals(entry->netmask))
        return false;
    if (!gw.isNull() && !gw.equals(entry->gateway))
        return false;
    if (metric && metric!=entry->metric)
        return false;
    if (dev && strcmp(dev, entry->interfaceName.c_str()))
        return false;

    return true;
}

