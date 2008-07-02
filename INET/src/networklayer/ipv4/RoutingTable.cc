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


IPv4Route::IPv4Route()
{
    interfacePtr = NULL;

    metric = 0;
    type = DIRECT;
    source = MANUAL;
}

std::string IPv4Route::info() const
{
    std::stringstream out;
    out << "dest:"; if (host.isUnspecified()) out << "*  "; else out << host << "  ";
    out << "gw:"; if (gateway.isUnspecified()) out << "*  "; else out << gateway << "  ";
    out << "mask:"; if (netmask.isUnspecified()) out << "*  "; else out << netmask << "  ";
    out << "metric:" << metric << " ";
    out << "if:"; if (!interfacePtr) out << "*  "; else out << interfacePtr->getName() << "  ";
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

std::string IPv4Route::detailedInfo() const
{
    return std::string();
}


//==============================================================


Define_Module(RoutingTable);


std::ostream& operator<<(std::ostream& os, const IPv4Route& e)
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

        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_IPv4_INTERFACECONFIG_CHANGED);

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
        for (int i=0; i<interfaceTable->getNumInterfaces(); ++i)
            configureInterfaceForIPv4(interfaceTable->getInterface(i));
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
            for (int i=0; i<ift->getNumInterfaces(); ++i)
            {
                InterfaceEntry *ie = ift->getInterface(i);
                if (!ie->isLoopback() && ie->ipv4()->getIPAddress().getInt() > _routerId.getInt())
                    _routerId = ie->ipv4()->getIPAddress();
            }
        }
    }
    else // already configured
    {
        // if there is no interface with routerId yet, assign it to the loopback address;
        // TODO find out if this is a good practice, in which situations it is useful etc.
        if (getInterfaceByAddress(_routerId)==NULL)
        {
            InterfaceEntry *lo0 = ift->getFirstLoopbackInterface();
            lo0->ipv4()->setIPAddress(_routerId);
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
    getDisplayString().setTagArg("t",0,buf);
}

void RoutingTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void RoutingTable::receiveChangeNotification(int category, cPolymorphic *details)
{
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;  // ignore notifications during initialize

    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category==NF_INTERFACE_CREATED)
    {
        // add netmask route for the new interface
        updateNetmaskRoutes();     //FIXME only for the new one!!!
    }
    else if (category==NF_INTERFACE_DELETED)
    {
        //TODO remove all routes that point to that interface
    }
    else if (category==NF_INTERFACE_STATE_CHANGED)
    {
        //TODO invalidate routing cache
    }
    else if (category==NF_INTERFACE_CONFIG_CHANGED)
    {
        //TODO invalidate routing cache
    }
    else if (category==NF_IPv4_INTERFACECONFIG_CHANGED)
    {
        // if anything IPv4-related changes in the interfaces, interface netmask
        // based routes have to be re-built.
        updateNetmaskRoutes();
    }
}

void RoutingTable::printRoutingTable() const
{
    EV << "-- Routing table --\n";
    ev.printf("%-16s %-16s %-16s %-3s %s\n",
              "Destination", "Gateway", "Netmask", "Iface");

    for (int i=0; i<getNumRoutes(); i++)
        EV << getRoute(i)->detailedInfo() << "\n";
    EV << "\n";
}

std::vector<IPAddress> RoutingTable::gatherAddresses() const
{
    std::vector<IPAddress> addressvector;

    for (int i=0; i<ift->getNumInterfaces(); ++i)
        addressvector.push_back(ift->getInterface(i)->ipv4()->getIPAddress());
    return addressvector;
}

//---

void RoutingTable::configureInterfaceForIPv4(InterfaceEntry *ie)
{
    IPv4InterfaceData *d = new IPv4InterfaceData();
    ie->setIPv4Data(d);

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    d->setMetric((int)ceil(2e9/ie->getDatarate())); // use OSPF cost as default
}

InterfaceEntry *RoutingTable::getInterfaceByAddress(const IPAddress& addr) const
{
    Enter_Method("getInterfaceByAddress(%s)=?", addr.str().c_str());
    if (addr.isUnspecified())
        return NULL;
    for (int i=0; i<ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4()->getIPAddress()==addr)
            return ie;
    }
    return NULL;
}


void RoutingTable::configureLoopbackForIPv4()
{
    InterfaceEntry *ie = ift->getFirstLoopbackInterface();

    // add IPv4 info. Set 127.0.0.1/8 as address by default --
    // we may reconfigure later it to be the routerId
    IPv4InterfaceData *d = new IPv4InterfaceData();
    d->setIPAddress(IPAddress::LOOPBACK_ADDRESS);
    d->setNetmask(IPAddress::LOOPBACK_NETMASK);
    d->setMetric(1);
    ie->setIPv4Data(d);
}

//---

bool RoutingTable::isLocalAddress(const IPAddress& dest) const
{
    Enter_Method("isLocalAddress(%s) y/n", dest.str().c_str());

    // check if we have an interface with this address
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (dest==ie->ipv4()->getIPAddress())
            return true;
    }
    return false;
}

bool RoutingTable::isLocalMulticastAddress(const IPAddress& dest) const
{
    Enter_Method("isLocalMulticastAddress(%s) y/n", dest.str().c_str());

    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        for (unsigned int j=0; j < ie->ipv4()->getMulticastGroups().size(); j++)
            if (dest.equals(ie->ipv4()->getMulticastGroups()[j]))
                return true;
    }
    return false;
}


const IPv4Route *RoutingTable::findBestMatchingRoute(const IPAddress& dest) const
{
    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    const IPv4Route *bestRoute = NULL;
    uint32 longestNetmask = 0;
    for (RouteVector::const_iterator i=routes.begin(); i!=routes.end(); ++i)
    {
        const IPv4Route *e = *i;
        if (IPAddress::maskedAddrAreEqual(dest, e->getHost(), e->getNetmask()) &&  // match
            (!bestRoute || e->getNetmask().getInt() > longestNetmask))  // longest so far
        {
            bestRoute = e;
            longestNetmask = e->getNetmask().getInt();
        }
    }
    return bestRoute;
}

InterfaceEntry *RoutingTable::getInterfaceForDestAddr(const IPAddress& dest) const
{
    Enter_Method("getInterfaceForDestAddr(%s)=?", dest.str().c_str());

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getInterface() : NULL;
}

IPAddress RoutingTable::getGatewayForDestAddr(const IPAddress& dest) const
{
    Enter_Method("getGatewayForDestAddr(%s)=?", dest.str().c_str());

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getGateway() : IPAddress();
}


MulticastRoutes RoutingTable::getMulticastRoutesFor(const IPAddress& dest) const
{
    Enter_Method("getMulticastRoutesFor(%s)=?", dest.str().c_str());

    MulticastRoutes res;
    res.reserve(16);
    for (RouteVector::const_iterator i=multicastRoutes.begin(); i!=multicastRoutes.end(); ++i)
    {
        const IPv4Route *e = *i;
        if (IPAddress::maskedAddrAreEqual(dest, e->getHost(), e->getNetmask()))
        {
            MulticastRoute r;
            r.interf = e->getInterface();
            r.gateway = e->getGateway();
            res.push_back(r);
        }
    }
    return res;
}


int RoutingTable::getNumRoutes() const
{
    return routes.size()+multicastRoutes.size();
}

const IPv4Route *RoutingTable::getRoute(int k) const
{
    if (k < (int)routes.size())
        return routes[k];
    k -= routes.size();
    if (k < (int)multicastRoutes.size())
        return multicastRoutes[k];
    return NULL;
}

const IPv4Route *RoutingTable::findRoute(const IPAddress& target, const IPAddress& netmask,
    const IPAddress& gw, int metric, const char *dev) const
{
    int n = getNumRoutes();
    for (int i=0; i<n; i++)
        if (routeMatches(getRoute(i), target, netmask, gw, metric, dev))
            return getRoute(i);
    return NULL;
}

void RoutingTable::addRoute(const IPv4Route *entry)
{
    Enter_Method("addRoute(...)");

    // check for null address and default route
    if ((entry->getHost().isUnspecified() || entry->getNetmask().isUnspecified()) &&
        (!entry->getHost().isUnspecified() || !entry->getNetmask().isUnspecified()))
        error("addRoute(): to add a default route, set both host and netmask to zero");

    // check that the interface exists
    if (!entry->getInterface())
        error("addRoute(): interface cannot be NULL");

    // add to tables
    if (!entry->getHost().isMulticast())
        routes.push_back(const_cast<IPv4Route*>(entry));
    else
        multicastRoutes.push_back(const_cast<IPv4Route*>(entry));

    updateDisplayString();
}


bool RoutingTable::deleteRoute(const IPv4Route *entry)
{
    Enter_Method("deleteRoute(...)");

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


bool RoutingTable::routeMatches(const IPv4Route *entry,
    const IPAddress& target, const IPAddress& nmask,
    const IPAddress& gw, int metric, const char *dev) const
{
    if (!target.isUnspecified() && !target.equals(entry->getHost()))
        return false;
    if (!nmask.isUnspecified() && !nmask.equals(entry->getNetmask()))
        return false;
    if (!gw.isUnspecified() && !gw.equals(entry->getGateway()))
        return false;
    if (metric && metric!=entry->getMetric())
        return false;
    if (dev && strcmp(dev, entry->getInterfaceName()))
        return false;

    return true;
}

void RoutingTable::updateNetmaskRoutes()
{
    // first, delete all routes with src=IFACENETMASK
    for (unsigned int k=0; k<routes.size(); k++)
        if (routes[k]->getSource()==IPv4Route::IFACENETMASK)
            routes.erase(routes.begin()+(k--));  // '--' is necessary because indices shift down

    // then re-add them, according to actual interface configuration
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4()->getNetmask()!=IPAddress::ALLONES_ADDRESS)
        {
            IPv4Route *route = new IPv4Route();
            route->setType(IPv4Route::DIRECT);
            route->setSource(IPv4Route::IFACENETMASK);
            route->setHost(ie->ipv4()->getIPAddress());
            route->setNetmask(ie->ipv4()->getNetmask());
            route->setGateway(IPAddress());
            route->setMetric(ie->ipv4()->getMetric());
            route->setInterface(ie);
            routes.push_back(route);
        }
    }

    updateDisplayString();
}
