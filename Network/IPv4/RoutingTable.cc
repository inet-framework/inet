//
// Copyright (C) 2004 Andras Varga
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

#include <algorithm>
#include <sstream>

#include "stlwatch.h"
#include "RoutingTable.h"
#include "RoutingTableParser.h"



InterfaceEntry::InterfaceEntry()
{
    index = -1;
    outputPort = -1;

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
    out << "  addr:" << inetAddr << "  mask:" << mask;
    out << "  MTU:" << mtu << "  Metric:" << metric;
    strcpy(buf, out.str().c_str());
}

std::string InterfaceEntry::detailedInfo() const
{
    std::stringstream out;
    out << "name:" << (!name.empty() ? name.c_str() : "*");
    out << "\tinet addr:" << inetAddr << "\tMask: " << mask << "\n";

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

//================================================================

RoutingEntry::RoutingEntry()
{
    interfaceNo = -1;

    metric = 0;
    type = DIRECT;
    source = MANUAL;

    age = -1;
}

void RoutingEntry::info(char *buf)
{
    std::stringstream out;
    if (host.isNull()) out << "*  "; else out << host << "  ";
    if (gateway.isNull()) out << "*  "; else out << gateway << "  ";
    if (netmask.isNull()) out << "*  "; else out << netmask << "  ";
    if (interfaceName.empty()) out << "*  "; else out << interfaceName.c_str() << "  ";
    out << (type==DIRECT ? "DIRECT" : "REMOTE");
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

    defaultRoute = NULL;

    // FIXME todo: add loopback interface

    // Read routing table file
    RoutingTableParser parser(this);
    if (parser.readRoutingTableFromFile(filename) == -1)
        error("Error reading routing table file %s", filename);

    WATCH_vector(interfaces);
    WATCH_vector(routes);
    WATCH_vector(multicastRoutes);
    //FIXME WATCH(defaultRoute)

    //printIfconfig();
    //printRoutingTable();
}


void RoutingTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}


void RoutingTable::printIfconfig()
{
    ev << "---- IF config ----\n";
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        ev << (*i)->detailedInfo() << "\n";
    ev << "\n";
}

void RoutingTable::printRoutingTable()
{
    ev << "-- Routing table --\n";
    ev.printf("%-16s %-16s %-16s %-3s %s\n",
              "Destination", "Gateway", "Netmask", "Iface");

    for (int i=0; i<numRoutingEntries(); i++)
        ev << routingEntry(i)->detailedInfo() << "\n";
    ev << "\n";
}

//---

InterfaceEntry *RoutingTable::interfaceByIndex(int index)
{
    if (index<0 || index>=interfaces.size())
        opp_error("interfaceById(): nonexistent interface %d", index);
    return interfaces[index];
}

void RoutingTable::addInterface(InterfaceEntry *entry)
{
    entry->index = interfaces.size();
    interfaces.push_back(entry);
}

bool RoutingTable::deleteInterface(InterfaceEntry *entry)
{
    InterfaceVector::iterator i = std::find(interfaces.begin(), interfaces.end(), entry);
    if (i==interfaces.end())
        return false;

    interfaces.erase(i);
    delete entry;

    for (i=interfaces.begin(); i!=interfaces.end(); ++i)
        (*i)->index = i-interfaces.begin();
    return true;
}

InterfaceEntry *RoutingTable::interfaceByPortNo(int portNo)
{
    // TBD change this to a port-to-interface table (more efficient)
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if ((*i)->outputPort==portNo)
            return *i;
    return NULL;
}

InterfaceEntry *RoutingTable::interfaceByName(const char *name)
{
    Enter_Method("interfaceByName(%s)=?", name);
    if (!name)
        return NULL;
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if (!strcmp(name, (*i)->name.c_str()))
            return *i;
    return NULL;
}

InterfaceEntry *RoutingTable::interfaceByAddress(const IPAddress& addr)
{
    Enter_Method("interfaceByAddress(%s)=?", addr.str().c_str());
    // FIXME this is rather interfaceBy_Network_Address() -- is this what's intended? --AV
    if (addr.isNull())
        return NULL;
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if (IPAddress::maskedAddrAreEqual(addr,(*i)->inetAddr,(*i)->mask))
            return *i;
    return NULL;
}

void RoutingTable::addLocalLoopback()
{
    InterfaceEntry *loopbackInterface = new InterfaceEntry();

    loopbackInterface->name = "lo0";

    //loopbackInterface->inetAddr = IPAddress("127.0.0.1");
    //loopbackInterface->mask = IPAddress("255.0.0.0");
// BCH Andras -- code from UTS MPLS model
    IPAddress loopbackIP = IPAddress("127.0.0.1");
    for (cModule *curmod=parentModule(); curmod != NULL;curmod = curmod->parentModule())
    {
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        // the following line is a terrible hack. For some unknown reason,
        // the MPLS models use the host's "local_addr" parameter (string)
        // as loopback address (and also change its netmask to 255.255.255.255).
        // But this conflicts with the IPSuite which also has "local_addr" parameters,
        // numeric and not intended for use as loopback address. So until we
        // figure out why exactly the MPLS models do this, we just patch up
        // the thing and only regard "local_addr" parameters that are strings....
        // Horrible hacking.  --Andras
        if (curmod->hasPar("local_addr") && curmod->par("local_addr").type()=='S')
        {
            loopbackIP = IPAddress(curmod->par("local_addr").stringValue());
            break;
        }

    }
    ev << "My loopback address is : " << loopbackIP << "\n";
    loopbackInterface->inetAddr = loopbackIP;
    loopbackInterface->mask = IPAddress("255.255.255.255");  // ????? -- Andras
// ECH

    loopbackInterface->mtu = 3924;
    loopbackInterface->metric = 1;
    loopbackInterface->loopback = true;

    loopbackInterface->multicastGroupCtr = 0;
    loopbackInterface->multicastGroup = NULL;

    // add interface to table
    addInterface(loopbackInterface);
}

//---

bool RoutingTable::localDeliver(const IPAddress& dest)
{
    Enter_Method("localDeliver(%s) y/n", dest.str().c_str());
    // check if we have an interface with this address
    return interfaceByAddress(dest)!=NULL;
}

bool RoutingTable::multicastLocalDeliver(const IPAddress& dest)
{
    Enter_Method("multicastLocalDeliver(%s) y/n", dest.str().c_str());

    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        for (int j=0; j < (*i)->multicastGroupCtr; j++)
            if (dest.equals((*i)->multicastGroup[j]))
                return true;
    return false;
}


RoutingEntry *RoutingTable::selectBestMatchingRoute(const IPAddress& dest)
{
    // find best match (one with longest prefix)
    RoutingEntry *bestRoute = NULL;
    uint32 longestNetmask = 0;
    for (RouteVector::iterator i=routes.begin(); i!=routes.end(); ++i)
    {
        RoutingEntry *e = *i;
        if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask) &&  // match
            e->netmask.getInt()>longestNetmask)  // longest so far
        {
            bestRoute = e;
            longestNetmask = e->netmask.getInt();
        }
    }
    return bestRoute ? bestRoute : defaultRoute;
}

int RoutingTable::outputPortNo(const IPAddress& dest)
{
    Enter_Method("outputPortNo(%s)=?", dest.str().c_str());

    // FIXME join with the next function...
    RoutingEntry *e = selectBestMatchingRoute(dest);
    if (!e) return -1;
    return interfaceByName(e->interfaceName.c_str())->outputPort; // Ughhhh
}

IPAddress RoutingTable::nextGatewayAddress(const IPAddress& dest)
{
    Enter_Method("nextGatewayAddress(%s)=?", dest.str().c_str());

    RoutingEntry *e = selectBestMatchingRoute(dest);
    if (!e) return IPAddress();
    return e->gateway;
}


MulticastRoutes RoutingTable::multicastRoutesFor(const IPAddress& dest)
{
    Enter_Method("multicastOutputPortNo(%s, %d)=?", dest.str().c_str(), index);

    MulticastRoutes res;
    res.reserve(16);
    for (RouteVector::iterator i=multicastRoutes.begin(); i!=multicastRoutes.end(); ++i)
    {
        RoutingEntry *e = *i;
        if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask))
        {
            MulticastRoute r;
            r.interf = interfaceByName(e->interfaceName.c_str()); // Ughhhh
            r.gateway = e->gateway;
            res.push_back(r);
        }
    }
    return res;

}

// int RoutingTable::multicastOutputPortNo(const IPAddress& dest, int index)
// int RoutingTable::numMulticastDestinations(const IPAddress& dest)


int RoutingTable::numRoutingEntries()
{
    return routes.size()+multicastRoutes.size()+(defaultRoute?1:0);
}

RoutingEntry *RoutingTable::routingEntry(int k)
{
    if (k==0 && defaultRoute)
        return defaultRoute;
    k -= (defaultRoute?1:0);
    if (k < routes.size())
        return routes[k];
    k -= routes.size();
    if (k < multicastRoutes.size())
        return multicastRoutes[k];
    return NULL;
}

RoutingEntry *RoutingTable::findRoutingEntry(const IPAddress& target,
                                             const IPAddress& netmask,
                                             const IPAddress& gw,
                                             int metric,
                                             char *dev)
{
    int n = numRoutingEntries();
    for (int i=0; i<n; i++)
        if (routingEntryMatches(routingEntry(i), target, netmask, gw, metric, dev))
            return routingEntry(i);
    return NULL;
}

void RoutingTable::addRoutingEntry(RoutingEntry *entry)
{
    Enter_Method("addRoutingEntry(...)");

    if (entry->host.isNull())
    {
        delete defaultRoute;
        defaultRoute = entry;
    }
    else if (!entry->host.isMulticast())
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

    RouteVector::iterator i = std::find(routes.begin(), routes.end(), entry);
    if (i!=routes.end())
    {
        routes.erase(i);
        delete entry;
        return true;
    }
    i = std::find(multicastRoutes.begin(), multicastRoutes.end(), entry);
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

