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

#include "RoutingTable.h"
#include "RoutingTableParser.h"



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
    if (addr.isNull())
        return NULL;
    for (InterfaceVector::iterator i=interfaces.begin(); i!=interfaces.end(); ++i)
        if (IPAddress::maskedAddrAreEqual(addr,(*i)->inetAddr,(*i)->mask))
            return *i;
    return NULL;
}

//---

bool RoutingTable::localDeliver(const IPAddress& dest)
{
    Enter_Method("localDeliver(%s) y/n", dest.str().c_str());
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


/*FIXME
int RoutingTable::outputPortNo(const IPAddress& dest)
{
    Enter_Method("outputPortNo(%s)=?", dest.str().c_str());

    RoutingEntry *e;
    for (int i=0; i < route->items(); i++)
    {
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
                return interfaceByName(e->interfaceName.c_str());
            }
        }
    }

    // Is it gateway here?
    if (defaultRoute) {
        return interfaceByName(defaultRoute->interfaceName.c_str());
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
    return interfaceByName(e->interfaceName.c_str());
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
*/

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

