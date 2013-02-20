//
// Copyright (C) 2004-2006 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


//  Cleanup and rewrite: Andras Varga, 2004

#include <algorithm>
#include <sstream>

#include "RoutingTable.h"

#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "IPv4Route.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "RoutingTableParser.h"


Define_Module(RoutingTable);


std::ostream& operator<<(std::ostream& os, const IPv4Route& e)
{
    os << e.info();
    return os;
};

std::ostream& operator<<(std::ostream& os, const IPv4MulticastRoute& e)
{
    os << e.info();
    return os;
};

RoutingTable::RoutingTable()
{
    ift = NULL;
    nb = NULL;
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
        // get a pointer to the NotificationBoard module and IInterfaceTable
        nb = NotificationBoardAccess().get();
        ift = InterfaceTableAccess().get();

        IPForward = par("IPForward").boolValue();
        multicastForward = par("forwardMulticast");

        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_INTERFACE_IPv4CONFIG_CHANGED);

        WATCH_PTRVECTOR(routes);
        WATCH_PTRVECTOR(multicastRoutes);
        WATCH(IPForward);
        WATCH(routerId);
    }
    else if (stage==1)
    {
        // L2 modules register themselves in stage 0, so we can only configure
        // the interfaces in stage 1.
        const char *filename = par("routingFile");

        // At this point, all L2 modules have registered themselves (added their
        // interface entries). Create the per-interface IPv4 data structures.
        IInterfaceTable *interfaceTable = InterfaceTableAccess().get();
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
            routerId = IPv4Address(routerIdStr);
    }
    else if (stage==3)
    {
        // routerID selection must be after stage==2 when network autoconfiguration
        // assigns interface addresses
        configureRouterId();

        // we don't use notifications during initialize(), so we do it manually.
        // Should be in stage=3 because autoconfigurator runs in stage=2.
        updateNetmaskRoutes();

        //printRoutingTable();
    }
}

void RoutingTable::configureRouterId()
{
    if (routerId.isUnspecified())  // not yet configured
    {
        const char *routerIdStr = par("routerId").stringValue();
        if (!strcmp(routerIdStr, "auto"))  // non-"auto" cases already handled in stage 1
        {
            // choose highest interface address as routerId
            for (int i=0; i<ift->getNumInterfaces(); ++i)
            {
                InterfaceEntry *ie = ift->getInterface(i);
                if (!ie->isLoopback() && ie->ipv4Data()->getIPAddress().getInt() > routerId.getInt())
                    routerId = ie->ipv4Data()->getIPAddress();
            }
        }
    }
    else // already configured
    {
        // if there is no interface with routerId yet, assign it to the loopback address;
        // TODO find out if this is a good practice, in which situations it is useful etc.
        if (getInterfaceByAddress(routerId)==NULL)
        {
            InterfaceEntry *lo0 = ift->getFirstLoopbackInterface();
            lo0->ipv4Data()->setIPAddress(routerId);
            lo0->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);
        }
    }
}

void RoutingTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    if (routerId.isUnspecified())
        sprintf(buf, "%d+%d routes", (int)routes.size(), (int)multicastRoutes.size());
    else
        sprintf(buf, "routerId: %s\n%d+%d routes", routerId.str().c_str(), (int)routes.size(), (int)multicastRoutes.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void RoutingTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void RoutingTable::receiveChangeNotification(int category, const cObject *details)
{
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;  // ignore notifications during initialize

    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category==NF_INTERFACE_CREATED)
    {
        // add netmask route for the new interface
        updateNetmaskRoutes();
    }
    else if (category==NF_INTERFACE_DELETED)
    {
        // remove all routes that point to that interface
        InterfaceEntry *entry = const_cast<InterfaceEntry*>(check_and_cast<const InterfaceEntry*>(details));
        deleteInterfaceRoutes(entry);
    }
    else if (category==NF_INTERFACE_STATE_CHANGED)
    {
        invalidateCache();
    }
    else if (category==NF_INTERFACE_CONFIG_CHANGED)
    {
        invalidateCache();
    }
    else if (category==NF_INTERFACE_IPv4CONFIG_CHANGED)
    {
        // if anything IPv4-related changes in the interfaces, interface netmask
        // based routes have to be re-built.
        updateNetmaskRoutes();
    }
}

cModule *RoutingTable::getHostModule()
{
    return findContainingNode(this);
}

void RoutingTable::deleteInterfaceRoutes(InterfaceEntry *entry)
{
    bool changed = false;

    // delete unicast routes using this interface
    for (RouteVector::iterator it = routes.begin(); it != routes.end(); )
    {
        IPv4Route *route = *it;
        if (route->getInterface() == entry)
        {
            it = routes.erase(it);
            ASSERT(route->getRoutingTable() == this); // still filled in, for the listeners' benefit
            nb->fireChangeNotification(NF_IPv4_ROUTE_DELETED, route);
            delete route;
            changed = true;
        }
        else
            ++it;
    }

    // delete or update multicast routes:
    //   1. delete routes has entry as parent
    //   2. remove entry from children list
    for (MulticastRouteVector::iterator it = multicastRoutes.begin(); it != multicastRoutes.end(); )
    {
        IPv4MulticastRoute *route = *it;
        if (route->getParent() == entry)
        {
            it = multicastRoutes.erase(it);
            ASSERT(route->getRoutingTable() == this); // still filled in, for the listeners' benefit
            nb->fireChangeNotification(NF_IPv4_MROUTE_DELETED, route);
            delete route;
            changed = true;
        }
        else
        {
            bool removed = route->removeChild(entry);
            if (removed)
            {
                nb->fireChangeNotification(NF_IPv4_MROUTE_CHANGED, route);
                changed = true;
            }
            ++it;
        }
    }

    if (changed)
    {
        invalidateCache();
        updateDisplayString();
    }
}

void RoutingTable::invalidateCache()
{
    routingCache.clear();
    localAddresses.clear();
    localBroadcastAddresses.clear();
}

void RoutingTable::printRoutingTable() const
{
    EV << "-- Routing table --\n";
    ev.printf("%-16s %-16s %-16s %-4s %-16s %s\n",
              "Destination", "Netmask", "Gateway", "Iface", "", "Metric");

    for (int i=0; i<getNumRoutes(); i++) {
        IPv4Route *route = getRoute(i);
        InterfaceEntry *interfacePtr = route->getInterface();
        ev.printf("%-16s %-16s %-16s %-4s (%s) %d\n",
                  route->getDestination().isUnspecified() ? "*" : route->getDestination().str().c_str(),
                  route->getNetmask().isUnspecified() ? "*" : route->getNetmask().str().c_str(),
                  route->getGateway().isUnspecified() ? "*" : route->getGateway().str().c_str(),
                  !interfacePtr ? "*" : interfacePtr->getName(),
                  !interfacePtr ? "*  " : interfacePtr->ipv4Data()->getIPAddress().str().c_str(),
                  route->getMetric());
    }
    EV << "\n";
}

void RoutingTable::printMulticastRoutingTable() const
{
    EV << "-- Multicast routing table --\n";
    ev.printf("%-16s %-16s %-16s %-6s %-6s %s\n",
              "Source", "Netmask", "Group", "Metric", "In", "Outs");

    for (int i=0; i<getNumMulticastRoutes(); i++) {
        IPv4MulticastRoute *route = getMulticastRoute(i);
        ev.printf("%-16s %-16s %-16s %-6d %-6s ",
                  route->getOrigin().isUnspecified() ? "*" : route->getOrigin().str().c_str(),
                  route->getOriginNetmask().isUnspecified() ? "*" : route->getOriginNetmask().str().c_str(),
                  route->getMulticastGroup().isUnspecified() ? "*" : route->getMulticastGroup().str().c_str(),
                  route->getMetric(),
                  !route->getParent() ? "*" : route->getParent()->getName());
        const ChildInterfaceVector &children = route->getChildren();
        for (ChildInterfaceVector::const_iterator it = children.begin(); it != children.end(); ++it)
        {
            if (it != children.begin())
                EV << ",";
            EV << (*it)->getInterface()->getName();
        }
        EV << "\n";
    }
    EV << "\n";
}

std::vector<IPv4Address> RoutingTable::gatherAddresses() const
{
    std::vector<IPv4Address> addressvector;

    for (int i=0; i<ift->getNumInterfaces(); ++i)
        addressvector.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
    return addressvector;
}

//---

void RoutingTable::configureInterfaceForIPv4(InterfaceEntry *ie)
{
    IPv4InterfaceData *d = new IPv4InterfaceData();
    ie->setIPv4Data(d);

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    d->setMetric((int)ceil(2e9/ie->getDatarate())); // use OSPF cost as default

    // join "224.0.0.1" and "224.0.0.2" (if router) multicast groups automatically
    if (ie->isMulticast())
    {
        d->joinMulticastGroup(IPv4Address::ALL_HOSTS_MCAST);
        if (IPForward)
            d->joinMulticastGroup(IPv4Address::ALL_ROUTERS_MCAST);
    }
}

InterfaceEntry *RoutingTable::getInterfaceByAddress(const IPv4Address& addr) const
{
    Enter_Method("getInterfaceByAddress(%u.%u.%u.%u)", addr.getDByte(0), addr.getDByte(1), addr.getDByte(2), addr.getDByte(3)); // note: str().c_str() too slow here

    if (addr.isUnspecified())
        return NULL;
    for (int i=0; i<ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->getIPAddress()==addr)
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
    d->setIPAddress(IPv4Address::LOOPBACK_ADDRESS);
    d->setNetmask(IPv4Address::LOOPBACK_NETMASK);
    d->setMetric(1);
    ie->setIPv4Data(d);
}

//---

bool RoutingTable::isLocalAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    if (localAddresses.empty())
    {
        // collect interface addresses if not yet done
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            IPv4Address interfaceAddr = ift->getInterface(i)->ipv4Data()->getIPAddress();
            localAddresses.insert(interfaceAddr);
        }
    }

    AddressSet::iterator it = localAddresses.find(dest);
    return it!=localAddresses.end();
}

// JcM add: check if the dest addr is local network broadcast
bool RoutingTable::isLocalBroadcastAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalBroadcastAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    if (localBroadcastAddresses.empty())
    {
        // collect interface addresses if not yet done
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            IPv4Address interfaceAddr = ift->getInterface(i)->ipv4Data()->getIPAddress();
            IPv4Address broadcastAddr = interfaceAddr.getBroadcastAddress(ift->getInterface(i)->ipv4Data()->getNetmask());
            if (!broadcastAddr.isUnspecified())
            {
                 localBroadcastAddresses.insert(broadcastAddr);
            }
        }
    }

    AddressSet::iterator it = localBroadcastAddresses.find(dest);
    return it!=localBroadcastAddresses.end();
}

InterfaceEntry *RoutingTable::findInterfaceByLocalBroadcastAddress(const IPv4Address& dest) const
{
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        IPv4Address interfaceAddr = ie->ipv4Data()->getIPAddress();
        IPv4Address broadcastAddr = interfaceAddr.getBroadcastAddress(ie->ipv4Data()->getNetmask());
        if (broadcastAddr == dest)
            return ie;
    }
    return NULL;
}

bool RoutingTable::isLocalMulticastAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalMulticastAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->isMemberOfMulticastGroup(dest))
            return true;
    }
    return false;
}

void RoutingTable::purge()
{
    bool deleted = false;

    // purge unicast routes
    for (RouteVector::iterator it = routes.begin(); it != routes.end(); )
    {
        IPv4Route *route = *it;
        if (route->isValid())
            ++it;
        else
        {
            it = routes.erase(it);
            ASSERT(route->getRoutingTable() == this); // still filled in, for the listeners' benefit
            nb->fireChangeNotification(NF_IPv4_ROUTE_DELETED, route);
            delete route;
            deleted = true;
        }
    }

    // purge multicast routes
    for (MulticastRouteVector::iterator it = multicastRoutes.begin(); it != multicastRoutes.end(); )
    {
        IPv4MulticastRoute *route = *it;
        if (route->isValid())
            ++it;
        else
        {
            it = multicastRoutes.erase(it);
            ASSERT(route->getRoutingTable() == this); // still filled in, for the listeners' benefit
            nb->fireChangeNotification(NF_IPv4_MROUTE_DELETED, route);
            delete route;
            deleted = true;
        }
    }

    if (deleted)
    {
        invalidateCache();
        updateDisplayString();
    }
}

IPv4Route *RoutingTable::findBestMatchingRoute(const IPv4Address& dest) const
{
    Enter_Method("findBestMatchingRoute(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    RoutingCache::iterator it = routingCache.find(dest);
    if (it != routingCache.end())
    {
        if (it->second==NULL || it->second->isValid())
            return it->second;
    }

    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    IPv4Route *bestRoute = NULL;
    for (RouteVector::const_iterator i=routes.begin(); i!=routes.end(); ++i)
    {
        IPv4Route *e = *i;
        if (e->isValid())
        {
            if (IPv4Address::maskedAddrAreEqual(dest, e->getDestination(), e->getNetmask())) // match
            {
                bestRoute = const_cast<IPv4Route *>(e);
                break;
            }
        }
    }

    routingCache[dest] = bestRoute;
    return bestRoute;
}

InterfaceEntry *RoutingTable::getInterfaceForDestAddr(const IPv4Address& dest) const
{
    Enter_Method("getInterfaceForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getInterface() : NULL;
}

IPv4Address RoutingTable::getGatewayForDestAddr(const IPv4Address& dest) const
{
    Enter_Method("getGatewayForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getGateway() : IPv4Address();
}


const IPv4MulticastRoute *RoutingTable::findBestMatchingMulticastRoute(const IPv4Address &origin, const IPv4Address &group) const
{
    Enter_Method("getMulticastRoutesFor(%u.%u.%u.%u, %u.%u.%u.%u)",
            origin.getDByte(0), origin.getDByte(1), origin.getDByte(2), origin.getDByte(3),
            group.getDByte(0), group.getDByte(1), group.getDByte(2), group.getDByte(3)); // note: str().c_str() too slow here here

    // TODO caching?

    for (MulticastRouteVector::const_iterator i=multicastRoutes.begin(); i!=multicastRoutes.end(); ++i)
    {
        const IPv4MulticastRoute *e = *i;
        if (e->isValid() && e->matches(origin, group))
            return e;
    }

    return NULL;
}

IPv4Route *RoutingTable::getRoute(int k) const
{
    if (k < (int)routes.size())
        return routes[k];
    return NULL;
}

IPv4Route *RoutingTable::getDefaultRoute() const
{
    // if exists default route entry, it is the last valid entry
    for (RouteVector::const_reverse_iterator i=routes.rbegin(); i!=routes.rend() && (*i)->getNetmask().isUnspecified(); ++i)
    {
        if ((*i)->isValid())
            return *i;
    }
    return NULL;
}

bool RoutingTable::routeLessThan(const IPv4Route *a, const IPv4Route *b)
{
    // helper for sort() in addRoute(). We want routes with longer
    // prefixes to be at front, so we compare them as "less".
    // For metric, a smaller value is better (we report that as "less").
    if (a->getNetmask() != b->getNetmask())
        return a->getNetmask() > b->getNetmask();

    if (a->getDestination() != b->getDestination())
        return a->getDestination() < b->getDestination();

    return a->getMetric() < b->getMetric();
}

void RoutingTable::setRouterId(IPv4Address a)
{
    routerId = a;
}

void RoutingTable::internalAddRoute(IPv4Route *entry)
{
    if (!entry->getNetmask().isValidNetmask())
        error("addRoute(): wrong netmask %s in route", entry->getNetmask().str().c_str());

    if ((entry->getDestination().getInt() & ~entry->getNetmask().getInt()) != 0)
        error("addRoute(): suspicious route: destination IP address %s has bits set outside netmask %s",
                entry->getDestination().str().c_str(), entry->getNetmask().str().c_str());

    // check that the interface exists
    if (!entry->getInterface())
        error("addRoute(): interface cannot be NULL");

    // if this is a default route, remove old default route (we're replacing it)
    if (entry->getNetmask().isUnspecified())
    {
        IPv4Route *oldDefaultRoute = getDefaultRoute();
        if (oldDefaultRoute != NULL)
            deleteRoute(oldDefaultRoute);
    }

    // add to tables
    // we keep entries sorted by netmask desc, metric asc in routeList, so that we can
    // stop at the first match when doing the longest netmask matching
    RouteVector::iterator pos = upper_bound(routes.begin(), routes.end(), entry, routeLessThan);
    routes.insert(pos, entry);

    entry->setRoutingTable(this);
}

void RoutingTable::addRoute(IPv4Route *entry)
{
    Enter_Method("addRoute(...)");

    internalAddRoute(entry);

    invalidateCache();
    updateDisplayString();

    nb->fireChangeNotification(NF_IPv4_ROUTE_ADDED, entry);
}

IPv4Route *RoutingTable::internalRemoveRoute(IPv4Route *entry)
{
    RouteVector::iterator i = std::find(routes.begin(), routes.end(), entry);
    if (i!=routes.end())
    {
        routes.erase(i);
        return entry;
    }
    return NULL;
}

IPv4Route *RoutingTable::removeRoute(IPv4Route *entry)
{
    Enter_Method("removeRoute(...)");

    entry = internalRemoveRoute(entry);

    if (entry != NULL)
    {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this); // still filled in, for the listeners' benefit
        nb->fireChangeNotification(NF_IPv4_ROUTE_DELETED, entry);
        entry->setRoutingTable(NULL);
    }
    return entry;
}

bool RoutingTable::deleteRoute(IPv4Route *entry)
{
    Enter_Method("deleteRoute(...)");

    entry = internalRemoveRoute(entry);

    if (entry != NULL)
    {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this); // still filled in, for the listeners' benefit
        nb->fireChangeNotification(NF_IPv4_ROUTE_DELETED, entry);
        delete entry;
    }
    return entry != NULL;
}

bool RoutingTable::multicastRouteLessThan(const IPv4MulticastRoute *a, const IPv4MulticastRoute *b)
{
    // We want routes with longer
    // prefixes to be at front, so we compare them as "less".
    if (a->getOriginNetmask() != b->getOriginNetmask())
        return a->getOriginNetmask() > b->getOriginNetmask();

    // For metric, a smaller value is better (we report that as "less").
    if (a->getOrigin() != b->getOrigin())
        return a->getOrigin() < b->getOrigin();

    // put the unspecified group after the specified ones
    if (a->getMulticastGroup() != b->getMulticastGroup())
        return a->getMulticastGroup() > b->getMulticastGroup();

    return a->getMetric() < b->getMetric();
}

void RoutingTable::internalAddMulticastRoute(IPv4MulticastRoute *entry)
{
    if (!entry->getOriginNetmask().isValidNetmask())
        error("addMulticastRoute(): wrong netmask %s in multicast route", entry->getOriginNetmask().str().c_str());

    if ((entry->getOrigin().getInt() & ~entry->getOriginNetmask().getInt()) != 0)
        error("addMulticastRoute(): suspicious route: origin IP address %s has bits set outside netmask %s",
                entry->getOrigin().str().c_str(), entry->getOriginNetmask().str().c_str());

    if (!entry->getMulticastGroup().isUnspecified() && !entry->getMulticastGroup().isMulticast())
        error("addMulticastRoute(): group address (%s) is not a multicast address",
                entry->getMulticastGroup().str().c_str());

    // check that the interface exists
    if (entry->getParent() && !entry->getParent()->isMulticast())
        error("addMulticastRoute(): parent interface must be multicast capable");

    const ChildInterfaceVector &children = entry->getChildren();
    for (ChildInterfaceVector::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        if (!(*it))
            error("addMulticastRoute(): child interface cannot be NULL");
        else if (!(*it)->getInterface()->isMulticast())
            error("addMulticastRoute(): child interface must be multicast capable");
        else if ((*it)->getInterface() == entry->getParent())
            error("addMulticastRoute(): child interface cannot be the same as the parent");
    }


    // add to tables
    // we keep entries sorted by netmask desc, metric asc in routeList, so that we can
    // stop at the first match when doing the longest netmask matching
    MulticastRouteVector::iterator pos =
            upper_bound(multicastRoutes.begin(), multicastRoutes.end(), entry, multicastRouteLessThan);
    multicastRoutes.insert(pos, entry);

    entry->setRoutingTable(this);
}

void RoutingTable::addMulticastRoute(IPv4MulticastRoute *entry)
{
    Enter_Method("addMulticastRoute(...)");

    internalAddMulticastRoute(entry);

    invalidateCache();
    updateDisplayString();

    nb->fireChangeNotification(NF_IPv4_MROUTE_ADDED, entry);
}

IPv4MulticastRoute *RoutingTable::internalRemoveMulticastRoute(IPv4MulticastRoute *entry)
{
    MulticastRouteVector::iterator i = std::find(multicastRoutes.begin(), multicastRoutes.end(), entry);
    if (i!=multicastRoutes.end())
    {
        multicastRoutes.erase(i);
        return entry;
    }
    return NULL;
}

IPv4MulticastRoute *RoutingTable::removeMulticastRoute(IPv4MulticastRoute *entry)
{
    Enter_Method("removeMulticastRoute(...)");

    entry = internalRemoveMulticastRoute(entry);

    if (entry != NULL)
    {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this); // still filled in, for the listeners' benefit
        nb->fireChangeNotification(NF_IPv4_MROUTE_DELETED, entry);
        entry->setRoutingTable(NULL);
    }
    return entry;
}

bool RoutingTable::deleteMulticastRoute(IPv4MulticastRoute *entry)
{
    Enter_Method("deleteMulticastRoute(...)");

    entry = internalRemoveMulticastRoute(entry);

    if (entry != NULL)
    {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this); // still filled in, for the listeners' benefit
        nb->fireChangeNotification(NF_IPv4_MROUTE_DELETED, entry);
        delete entry;
    }
    return entry != NULL;
}

void RoutingTable::routeChanged(IPv4Route *entry, int fieldCode)
{
    if (fieldCode==IPv4Route::F_DESTINATION || fieldCode==IPv4Route::F_NETMASK || fieldCode==IPv4Route::F_METRIC) // our data structures depend on these fields
    {
        entry = internalRemoveRoute(entry);
        ASSERT(entry != NULL);  // failure means inconsistency: route was not found in this routing table
        internalAddRoute(entry);

        invalidateCache();
        updateDisplayString();
    }
    nb->fireChangeNotification(NF_IPv4_ROUTE_CHANGED, entry); // TODO include fieldCode in the notification
}

void RoutingTable::multicastRouteChanged(IPv4MulticastRoute *entry, int fieldCode)
{
    if (fieldCode==IPv4MulticastRoute::F_ORIGIN || fieldCode==IPv4MulticastRoute::F_ORIGINMASK ||
            fieldCode==IPv4MulticastRoute::F_MULTICASTGROUP || fieldCode==IPv4MulticastRoute::F_METRIC) // our data structures depend on these fields
    {
        entry = internalRemoveMulticastRoute(entry);
        ASSERT(entry != NULL);  // failure means inconsistency: route was not found in this routing table
        internalAddMulticastRoute(entry);

        invalidateCache();
        updateDisplayString();
    }
    nb->fireChangeNotification(NF_IPv4_MROUTE_CHANGED, entry); // TODO include fieldCode in the notification
}

void RoutingTable::updateNetmaskRoutes()
{
    // first, delete all routes with src=IFACENETMASK
    for (unsigned int k=0; k<routes.size(); k++)
    {
        if (routes[k]->getSource()==IPv4Route::IFACENETMASK)
        {
            std::vector<IPv4Route *>::iterator it = routes.begin()+(k--);  // '--' is necessary because indices shift down
            IPv4Route *route = *it;
            routes.erase(it);
            ASSERT(route->getRoutingTable() == this); // still filled in, for the listeners' benefit
            nb->fireChangeNotification(NF_IPv4_ROUTE_DELETED, route);
            delete route;
        }
    }

    // then re-add them, according to actual interface configuration
    // TODO: say there's a node somewhere in the network that belongs to the interface's subnet
    // TODO: and it is not on the same link, and the gateway does not use proxy ARP, how will packets reach that node?
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->getNetmask()!=IPv4Address::ALLONES_ADDRESS)
        {
            IPv4Route *route = new IPv4Route();
            route->setSource(IPv4Route::IFACENETMASK);
            route->setDestination(ie->ipv4Data()->getIPAddress().doAnd(ie->ipv4Data()->getNetmask()));
            route->setNetmask(ie->ipv4Data()->getNetmask());
            route->setGateway(IPv4Address());
            route->setMetric(ie->ipv4Data()->getMetric());
            route->setInterface(ie);
            route->setRoutingTable(this);
            RouteVector::iterator pos = upper_bound(routes.begin(), routes.end(), route, routeLessThan);
            routes.insert(pos, route);
            nb->fireChangeNotification(NF_IPv4_ROUTE_ADDED, route);
        }
    }

    invalidateCache();
    updateDisplayString();
}

