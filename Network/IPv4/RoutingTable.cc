//
// Copyright (C) 2004-2006 Andras Varga
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
#include "IPv4InterfaceData.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "NotifierConsts.h"


RoutingEntry::RoutingEntry()
{
    interfacePtr = NULL;

    metric = 0;
    type = DIRECT;
    source = MANUAL;
}

std::string RoutingEntry::info() const
{
    std::stringstream out;
    out << "dest:"; if (host.isUnspecified()) out << "*  "; else out << host << "  ";
    out << "gw:"; if (gateway.isUnspecified()) out << "*  "; else out << gateway << "  ";
    out << "mask:"; if (netmask.isUnspecified()) out << "*  "; else out << netmask << "  ";
    out << "metric:" << metric << " ";
    out << "if:"; if (interfaceName.empty()) out << "*  "; else out << interfaceName.c_str() << "  ";
    out << (type==DIRECT ? "DIRECT" : "REMOTE");
    switch (source)
    {
        case MANUAL:       out << " MANUAL"; break;
        case IFACENETMASK: out << " IFACENETMASK"; break;
        case RIP:          out << " RIP"; break;
        case OSPF:         out << " OSPF"; break;
        case BGP:          out << " BGP"; break;
        case ZEBRA:        out << " ZEBRA"; break;
        default:           out << " ???"; break;
    }
    return out.str();
}

std::string RoutingEntry::detailedInfo() const
{
    return std::string();
}


//==============================================================


Define_Module( RoutingTable );


std::ostream& operator<<(std::ostream& os, const RoutingEntry& e)
{
    os << e.info();
    return os;
};

RoutingTable::RoutingTable()
{
}

RoutingTable::~RoutingTable()
{
    for (unsigned int i=0; i<routes.size(); i++)
        delete routes[i];
    for (unsigned int i=0; i<multicastRoutes.size(); i++)
        delete multicastRoutes[i];
}

void RoutingTable::initialize(int stage)
{
    if (stage==0)
    {
        // get a pointer to the NotificationBoard module and InterfaceTable
        nb = NotificationBoardAccess().get();
        ift = InterfaceTableAccess().get();

        IPForward = par("IPForward").boolValue();

        WATCH_PTRVECTOR(routes);
        WATCH_PTRVECTOR(multicastRoutes);
        WATCH(IPForward);
        WATCH(_routerId);
    }
    else if (stage==1)
    {
        // L2 modules register themselves in stage 0, so we can only configure
        // the interfaces in stage 1.
        const char *filename = par("routingFile");

        // At this point, all L2 modules have registered themselves (added their
        // interface entries). Create the per-interface IPv4 data structures.
        InterfaceTable *interfaceTable = InterfaceTableAccess().get();
        for (int i=0; i<interfaceTable->numInterfaces(); ++i)
            configureInterfaceForIPv4(interfaceTable->interfaceAt(i));
        configureLoopbackForIPv4();

        // read routing table file (and interface configuration)
        RoutingTableParser parser(ift, this);
        if (*filename && parser.readRoutingTableFromFile(filename)==-1)
            error("Error reading routing table file %s", filename);

        // set routerId if param is not "" (==no routerId) or "auto" (in which case we'll
        // do it later in stage 3, after network configurators configured the interfaces)
        const char *routerIdStr = par("routerId").stringValue();
        if (strcmp(routerIdStr, "") && strcmp(routerIdStr, "auto"))
            _routerId = IPAddress(routerIdStr);
    }
    else if (stage==3)
    {
        // routerID selection must be after stage==2 when network autoconfiguration
        // assigns interface addresses
        autoconfigRouterId();

        // we don't use notifications during initialize(), so we do it manually.
        // Should be in stage=3 because autoconfigurator runs in stage=2.
        updateNetmaskRoutes();

        //printIfconfig();
        //printRoutingTable();
    }
}

void RoutingTable::autoconfigRouterId()
{
    if (_routerId.isUnspecified())  // not yet configured
    {
        const char *routerIdStr = par("routerId").stringValue();
        if (!strcmp(routerIdStr, "auto"))  // non-"auto" cases already handled in stage 1
        {
            // choose highest interface address as routerId
            for (int i=0; i<ift->numInterfaces(); ++i)
            {
                InterfaceEntry *ie = ift->interfaceAt(i);
                if (!ie->isLoopback() && ie->ipv4()->inetAddress().getInt() > _routerId.getInt())
                    _routerId = ie->ipv4()->inetAddress();
            }
        }
    }
    else // already configured
    {
        // if there is no interface with routerId yet, assign it to the loopback address;
        // TODO find out if this is a good practice, in which situations it is useful etc.
        if (interfaceByAddress(_routerId)==NULL)
        {
            InterfaceEntry *lo0 = ift->firstLoopbackInterface();
            lo0->ipv4()->setInetAddress(_routerId);
            lo0->ipv4()->setNetmask(IPAddress::ALLONES_ADDRESS);
        }
    }
}

void RoutingTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    if (_routerId.isUnspecified())
        sprintf(buf, "%d+%d routes", routes.size(), multicastRoutes.size());
    else
        sprintf(buf, "routerId: %s\n%d+%d routes", _routerId.str().c_str(), routes.size(), multicastRoutes.size());
    displayString().setTagArg("t",0,buf);
}

void RoutingTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void RoutingTable::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category==NF_IPv4_INTERFACECONFIG_CHANGED)
    {
        // if anything IPv4-related changes in the interfaces, interface netmask
        // based routes have to be re-built.
        updateNetmaskRoutes();
    }
}

void RoutingTable::printRoutingTable()
{
    EV << "-- Routing table --\n";
    ev.printf("%-16s %-16s %-16s %-3s %s\n",
              "Destination", "Gateway", "Netmask", "Iface");

    for (int i=0; i<numRoutingEntries(); i++)
        EV << routingEntry(i)->detailedInfo() << "\n";
    EV << "\n";
}

std::vector<IPAddress> RoutingTable::gatherAddresses()
{
    std::vector<IPAddress> addressvector;

    for (int i=0; i<ift->numInterfaces(); ++i)
        addressvector.push_back(ift->interfaceAt(i)->ipv4()->inetAddress());
    return addressvector;
}

//---

void RoutingTable::configureInterfaceForIPv4(InterfaceEntry *ie)
{
    IPv4InterfaceData *d = new IPv4InterfaceData();
    ie->setIPv4Data(d);

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    d->setMetric((int)ceil(2e9/ie->datarate())); // use OSPF cost as default
}

InterfaceEntry *RoutingTable::interfaceByAddress(const IPAddress& addr)
{
    Enter_Method("interfaceByAddress(%s)=?", addr.str().c_str());
    if (addr.isUnspecified())
        return NULL;
    for (int i=0; i<ift->numInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);
        if (ie->ipv4()->inetAddress()==addr)
            return ie;
    }
    return NULL;
}


void RoutingTable::configureLoopbackForIPv4()
{
    InterfaceEntry *ie = ift->firstLoopbackInterface();

    // add IPv4 info. Set 127.0.0.1/8 as address by default --
    // we may reconfigure later it to be the routerId
    IPv4InterfaceData *d = new IPv4InterfaceData();
    d->setInetAddress(IPAddress::LOOPBACK_ADDRESS);
    d->setNetmask(IPAddress::LOOPBACK_NETMASK);
    d->setMetric(1);
    ie->setIPv4Data(d);
}

//---

bool RoutingTable::localDeliver(const IPAddress& dest)
{
    Enter_Method("localDeliver(%s) y/n", dest.str().c_str());

    // check if we have an interface with this address
    for (int i=0; i<ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);
        if (dest==ie->ipv4()->inetAddress())
            return true;
    }
    return false;
}

bool RoutingTable::multicastLocalDeliver(const IPAddress& dest)
{
    Enter_Method("multicastLocalDeliver(%s) y/n", dest.str().c_str());

    for (int i=0; i<ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);
        for (unsigned int j=0; j < ie->ipv4()->multicastGroups().size(); j++)
            if (dest.equals(ie->ipv4()->multicastGroups()[j]))
                return true;
    }
    return false;
}


RoutingEntry *RoutingTable::findBestMatchingRoute(const IPAddress& dest)
{
    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    RoutingEntry *bestRoute = NULL;
    uint32 longestNetmask = 0;
    for (RouteVector::iterator i=routes.begin(); i!=routes.end(); ++i)
    {
        RoutingEntry *e = *i;
        if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask) &&  // match
            (!bestRoute || e->netmask.getInt()>longestNetmask))  // longest so far
        {
            bestRoute = e;
            longestNetmask = e->netmask.getInt();
        }
    }
    return bestRoute;
}

InterfaceEntry *RoutingTable::interfaceForDestAddr(const IPAddress& dest)
{
    Enter_Method("interfaceForDestAddr(%s)=?", dest.str().c_str());

    RoutingEntry *e = findBestMatchingRoute(dest);
    if (!e) return NULL;
    return e->interfacePtr;
}

IPAddress RoutingTable::gatewayForDestAddr(const IPAddress& dest)
{
    Enter_Method("gatewayForDestAddr(%s)=?", dest.str().c_str());

    RoutingEntry *e = findBestMatchingRoute(dest);
    if (!e) return IPAddress();
    return e->gateway;
}


MulticastRoutes RoutingTable::multicastRoutesFor(const IPAddress& dest)
{
    Enter_Method("multicastRoutesFor(%s)=?", dest.str().c_str());

    MulticastRoutes res;
    res.reserve(16);
    for (RouteVector::iterator i=multicastRoutes.begin(); i!=multicastRoutes.end(); ++i)
    {
        RoutingEntry *e = *i;
        if (IPAddress::maskedAddrAreEqual(dest, e->host, e->netmask))
        {
            MulticastRoute r;
            r.interf = ift->interfaceByName(e->interfaceName.c_str()); // Ughhhh
            r.gateway = e->gateway;
            res.push_back(r);
        }
    }
    return res;

}


int RoutingTable::numRoutingEntries()
{
    return routes.size()+multicastRoutes.size();
}

RoutingEntry *RoutingTable::routingEntry(int k)
{
    if (k < (int)routes.size())
        return routes[k];
    k -= routes.size();
    if (k < (int)multicastRoutes.size())
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

    // check for null address and default route
    if ((entry->host.isUnspecified() || entry->netmask.isUnspecified()) &&
        (!entry->host.isUnspecified() || !entry->netmask.isUnspecified()))
        error("addRoutingEntry(): to add a default route, set both host and netmask to zero");

    // fill in interface ptr from interface name
    entry->interfacePtr = ift->interfaceByName(entry->interfaceName.c_str());
    if (!entry->interfacePtr)
        error("addRoutingEntry(): interface `%s' doesn't exist", entry->interfaceName.c_str());

    // add to tables
    if (!entry->host.isMulticast())
    {
        routes.push_back(entry);
    }
    else
    {
        multicastRoutes.push_back(entry);
    }

    updateDisplayString();
}


bool RoutingTable::deleteRoutingEntry(RoutingEntry *entry)
{
    Enter_Method("deleteRoutingEntry(...)");

    RouteVector::iterator i = std::find(routes.begin(), routes.end(), entry);
    if (i!=routes.end())
    {
        routes.erase(i);
        delete entry;
        updateDisplayString();
        return true;
    }
    i = std::find(multicastRoutes.begin(), multicastRoutes.end(), entry);
    if (i!=multicastRoutes.end())
    {
        multicastRoutes.erase(i);
        delete entry;
        updateDisplayString();
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
    if (!target.isUnspecified() && !target.equals(entry->host))
        return false;
    if (!nmask.isUnspecified() && !nmask.equals(entry->netmask))
        return false;
    if (!gw.isUnspecified() && !gw.equals(entry->gateway))
        return false;
    if (metric && metric!=entry->metric)
        return false;
    if (dev && strcmp(dev, entry->interfaceName.c_str()))
        return false;

    return true;
}

void RoutingTable::updateNetmaskRoutes()
{
    // first, delete all routes with src=IFACENETMASK
    for (unsigned int k=0; k<routes.size(); k++)
        if (routes[k]->source==RoutingEntry::IFACENETMASK)
            routes.erase(routes.begin()+(k--));  // '--' is necessary because indices shift down

    // then re-add them, according to actual interface configuration
    for (int i=0; i<ift->numInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->interfaceAt(i);
        if (ie->ipv4()->netmask()!=IPAddress::ALLONES_ADDRESS)
        {
            RoutingEntry *route = new RoutingEntry();
            route->type = RoutingEntry::DIRECT;
            route->source = RoutingEntry::IFACENETMASK;
            route->host = ie->ipv4()->inetAddress();
            route->netmask = ie->ipv4()->netmask();
            route->gateway = IPAddress();
            route->metric = ie->ipv4()->metric();
            route->interfaceName = ie->name();
            route->interfacePtr = ie;
            routes.push_back(route);
        }
    }

    updateDisplayString();
}
