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

#include "inet/common/PatternMatcher.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/ipv4/IPv4Route.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/RoutingTableParser.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

using namespace utils;

Define_Module(IPv4RoutingTable);

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

IPv4RoutingTable::~IPv4RoutingTable()
{
    for (auto & elem : routes)
        delete elem;
    for (auto & elem : multicastRoutes)
        delete elem;
}

void IPv4RoutingTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // get a pointer to the host module and IInterfaceTable
        cModule *host = getContainingNode(this);
        host->subscribe(NF_INTERFACE_CREATED, this);
        host->subscribe(NF_INTERFACE_DELETED, this);
        host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
        host->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
        host->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);

        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        netmaskRoutes = par("netmaskRoutes");
        forwarding = par("forwarding").boolValue();
        multicastForward = par("multicastForwarding");
        useAdminDist = par("useAdminDist");

        WATCH_PTRVECTOR(routes);
        WATCH_PTRVECTOR(multicastRoutes);
        WATCH(netmaskRoutes);
        WATCH(forwarding);
        WATCH(multicastForward);
        WATCH(routerId);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isNodeUp = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
        if (isNodeUp) {
            // set routerId if param is not "" (==no routerId) or "auto" (in which case we'll
            // do it later in a later stage, after network configurators configured the interfaces)
            const char *routerIdStr = par("routerId").stringValue();
            if (strcmp(routerIdStr, "") && strcmp(routerIdStr, "auto"))
                routerId = IPv4Address(routerIdStr);
        }
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {
        if (isNodeUp) {
            // read routing table file (and interface configuration)
            const char *filename = par("routingFile");
            RoutingTableParser parser(ift, this);
            if (*filename && parser.readRoutingTableFromFile(filename) == -1)
                throw cRuntimeError("Error reading routing table file %s", filename);
        }

        // routerID selection must be after network autoconfiguration assigned interface addresses
        if (isNodeUp)
            configureRouterId();

        // we don't use notifications during initialize(), so we do it manually.
        updateNetmaskRoutes();
    }
}

void IPv4RoutingTable::configureRouterId()
{
    if (routerId.isUnspecified()) {    // not yet configured
        const char *routerIdStr = par("routerId").stringValue();
        if (!strcmp(routerIdStr, "auto")) {    // non-"auto" cases already handled earlier
            // choose highest interface address as routerId
            for (int i = 0; i < ift->getNumInterfaces(); ++i) {
                InterfaceEntry *ie = ift->getInterface(i);
                if (!ie->isLoopback() && ie->ipv4Data() && ie->ipv4Data()->getIPAddress().getInt() > routerId.getInt())
                    routerId = ie->ipv4Data()->getIPAddress();
            }
        }
    }
    else {    // already configured
              // if there is no interface with routerId yet, assign it to the loopback address;
              // TODO find out if this is a good practice, in which situations it is useful etc.
        if (getInterfaceByAddress(routerId) == nullptr) {
            InterfaceEntry *lo0 = ift->getFirstLoopbackInterface();
            ASSERT(lo0);
            lo0->ipv4Data()->setIPAddress(routerId);
            lo0->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);
        }
    }
}

void IPv4RoutingTable::updateDisplayString()
{
    if (!hasGUI())
        return;

    char buf[80];
    if (routerId.isUnspecified())
        sprintf(buf, "%d+%d routes", (int)routes.size(), (int)multicastRoutes.size());
    else
        sprintf(buf, "routerId: %s\n%d+%d routes", routerId.str().c_str(), (int)routes.size(), (int)multicastRoutes.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void IPv4RoutingTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void IPv4RoutingTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    if (getSimulation()->getContextType() == CTX_INITIALIZE)
        return; // ignore notifications during initialize

    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);

    if (signalID == NF_INTERFACE_CREATED) {
        // add netmask route for the new interface
        updateNetmaskRoutes();
    }
    else if (signalID == NF_INTERFACE_DELETED) {
        // remove all routes that point to that interface
        const InterfaceEntry *entry = check_and_cast<const InterfaceEntry *>(obj);
        deleteInterfaceRoutes(entry);
    }
    else if (signalID == NF_INTERFACE_STATE_CHANGED) {
        invalidateCache();
    }
    else if (signalID == NF_INTERFACE_CONFIG_CHANGED) {
        invalidateCache();
    }
    else if (signalID == NF_INTERFACE_IPv4CONFIG_CHANGED) {
        // if anything IPv4-related changes in the interfaces, interface netmask
        // based routes have to be re-built.
        updateNetmaskRoutes();
    }
}

cModule *IPv4RoutingTable::getHostModule()
{
    return findContainingNode(this);
}

void IPv4RoutingTable::deleteInterfaceRoutes(const InterfaceEntry *entry)
{
    bool changed = false;

    // delete unicast routes using this interface
    for (auto it = routes.begin(); it != routes.end(); ) {
        IPv4Route *route = *it;
        if (route->getInterface() == entry) {
            it = routes.erase(it);
            ASSERT(route->getRoutingTable() == this);    // still filled in, for the listeners' benefit
            emit(NF_ROUTE_DELETED, route);
            delete route;
            changed = true;
        }
        else
            ++it;
    }

    // delete or update multicast routes:
    //   1. delete routes has entry as input interface
    //   2. remove entry from output interface list
    for (auto it = multicastRoutes.begin(); it != multicastRoutes.end(); ) {
        IPv4MulticastRoute *route = *it;
        if (route->getInInterface() && route->getInInterface()->getInterface() == entry) {
            it = multicastRoutes.erase(it);
            ASSERT(route->getRoutingTable() == this);    // still filled in, for the listeners' benefit
            emit(NF_MROUTE_DELETED, route);
            delete route;
            changed = true;
        }
        else {
            bool removed = route->removeOutInterface(entry);
            if (removed) {
                emit(NF_MROUTE_CHANGED, route);
                changed = true;
            }
            ++it;
        }
    }

    if (changed) {
        invalidateCache();
        updateDisplayString();
    }
}

void IPv4RoutingTable::invalidateCache()
{
    routingCache.clear();
    localAddresses.clear();
    localBroadcastAddresses.clear();
}

void IPv4RoutingTable::printRoutingTable() const
{
    EV << "-- Routing table --\n";
    EV << stringf("%-16s %-16s %-16s %-4s %-16s %s\n",
            "Destination", "Netmask", "Gateway", "Iface", "", "Metric");

    for (int i = 0; i < getNumRoutes(); i++) {
        IPv4Route *route = getRoute(i);
        InterfaceEntry *interfacePtr = route->getInterface();
        EV << stringf("%-16s %-16s %-16s %-4s (%s) %d\n",
                route->getDestination().isUnspecified() ? "*" : route->getDestination().str().c_str(),
                route->getNetmask().isUnspecified() ? "*" : route->getNetmask().str().c_str(),
                route->getGateway().isUnspecified() ? "*" : route->getGateway().str().c_str(),
                !interfacePtr ? "*" : interfacePtr->getName(),
                !interfacePtr ? "*  " : interfacePtr->ipv4Data()->getIPAddress().str().c_str(),
                route->getMetric());
    }
    EV << "\n";
}

void IPv4RoutingTable::printMulticastRoutingTable() const
{
    EV << "-- Multicast routing table --\n";
    EV << stringf("%-16s %-16s %-16s %-6s %-6s %s\n",
            "Source", "Netmask", "Group", "Metric", "In", "Outs");

    for (int i = 0; i < getNumMulticastRoutes(); i++) {
        IPv4MulticastRoute *route = getMulticastRoute(i);
        EV << stringf("%-16s %-16s %-16s %-6d %-6s ",
                route->getOrigin().isUnspecified() ? "*" : route->getOrigin().str().c_str(),
                route->getOriginNetmask().isUnspecified() ? "*" : route->getOriginNetmask().str().c_str(),
                route->getMulticastGroup().isUnspecified() ? "*" : route->getMulticastGroup().str().c_str(),
                route->getMetric(),
                !route->getInInterface() ? "*" : route->getInInterface()->getInterface()->getName());
        for (unsigned int i = 0; i < route->getNumOutInterfaces(); i++) {
            if (i != 0)
                EV << ",";
            EV << route->getOutInterface(i)->getInterface()->getName();
        }
        EV << "\n";
    }
    EV << "\n";
}

std::vector<IPv4Address> IPv4RoutingTable::gatherAddresses() const
{
    std::vector<IPv4Address> addressvector;

    for (int i = 0; i < ift->getNumInterfaces(); ++i)
        addressvector.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
    return addressvector;
}

//---

void IPv4RoutingTable::configureInterfaceForIPv4(InterfaceEntry *ie)
{
    IPv4InterfaceData *d = new IPv4InterfaceData();
    ie->setIPv4Data(d);

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    d->setMetric((int)ceil(2e9 / ie->getDatarate()));    // use OSPF cost as default

    // join "224.0.0.1" and "224.0.0.2" (if router) multicast groups automatically
    if (ie->isMulticast()) {
        d->joinMulticastGroup(IPv4Address::ALL_HOSTS_MCAST);
        if (forwarding)
            d->joinMulticastGroup(IPv4Address::ALL_ROUTERS_MCAST);
    }
}

InterfaceEntry *IPv4RoutingTable::getInterfaceByAddress(const IPv4Address& addr) const
{
    Enter_Method("getInterfaceByAddress(%u.%u.%u.%u)", addr.getDByte(0), addr.getDByte(1), addr.getDByte(2), addr.getDByte(3));    // note: str().c_str() too slow here

    if (addr.isUnspecified())
        return nullptr;
    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->getIPAddress() == addr)
            return ie;
    }
    return nullptr;
}

void IPv4RoutingTable::configureLoopbackForIPv4()
{
    InterfaceEntry *ie = ift->getFirstLoopbackInterface();
    if (ie) {
        // add IPv4 info. Set 127.0.0.1/8 as address by default --
        // we may reconfigure later it to be the routerId
        IPv4InterfaceData *d = new IPv4InterfaceData();
        d->setIPAddress(IPv4Address::LOOPBACK_ADDRESS);
        d->setNetmask(IPv4Address::LOOPBACK_NETMASK);
        d->setMetric(1);
        ie->setIPv4Data(d);
    }
}

//---

bool IPv4RoutingTable::isLocalAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3));    // note: str().c_str() too slow here

    if (localAddresses.empty()) {
        // collect interface addresses if not yet done
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            IPv4Address interfaceAddr = ift->getInterface(i)->ipv4Data()->getIPAddress();
            localAddresses.insert(interfaceAddr);
        }
    }

    auto it = localAddresses.find(dest);
    return it != localAddresses.end();
}

// JcM add: check if the dest addr is local network broadcast
bool IPv4RoutingTable::isLocalBroadcastAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalBroadcastAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3));    // note: str().c_str() too slow here

    if (localBroadcastAddresses.empty()) {
        // collect interface addresses if not yet done
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            IPv4Address interfaceAddr = ift->getInterface(i)->ipv4Data()->getIPAddress();
            IPv4Address broadcastAddr = interfaceAddr.makeBroadcastAddress(ift->getInterface(i)->ipv4Data()->getNetmask());
            if (!broadcastAddr.isUnspecified()) {
                localBroadcastAddresses.insert(broadcastAddr);
            }
        }
    }

    auto it = localBroadcastAddresses.find(dest);
    return it != localBroadcastAddresses.end();
}

InterfaceEntry *IPv4RoutingTable::findInterfaceByLocalBroadcastAddress(const IPv4Address& dest) const
{
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        IPv4Address interfaceAddr = ie->ipv4Data()->getIPAddress();
        IPv4Address broadcastAddr = interfaceAddr.makeBroadcastAddress(ie->ipv4Data()->getNetmask());
        if (broadcastAddr == dest)
            return ie;
    }
    return nullptr;
}

bool IPv4RoutingTable::isLocalMulticastAddress(const IPv4Address& dest) const
{
    Enter_Method("isLocalMulticastAddress(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3));    // note: str().c_str() too slow here

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data()->isMemberOfMulticastGroup(dest))
            return true;
    }
    return false;
}

void IPv4RoutingTable::purge()
{
    bool deleted = false;

    // purge unicast routes
    for (auto it = routes.begin(); it != routes.end(); ) {
        IPv4Route *route = *it;
        if (route->isValid())
            ++it;
        else {
            it = routes.erase(it);
            ASSERT(route->getRoutingTable() == this);    // still filled in, for the listeners' benefit
            emit(NF_ROUTE_DELETED, route);
            delete route;
            deleted = true;
        }
    }

    // purge multicast routes
    for (auto it = multicastRoutes.begin(); it != multicastRoutes.end(); ) {
        IPv4MulticastRoute *route = *it;
        if (route->isValid())
            ++it;
        else {
            it = multicastRoutes.erase(it);
            ASSERT(route->getRoutingTable() == this);    // still filled in, for the listeners' benefit
            emit(NF_MROUTE_DELETED, route);
            delete route;
            deleted = true;
        }
    }

    if (deleted) {
        invalidateCache();
        updateDisplayString();
    }
}

IPv4Route *IPv4RoutingTable::findBestMatchingRoute(const IPv4Address& dest) const
{
    Enter_Method("findBestMatchingRoute(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3));    // note: str().c_str() too slow here

    auto it = routingCache.find(dest);
    if (it != routingCache.end()) {
        if (it->second == nullptr || it->second->isValid())
            return it->second;
    }

    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    IPv4Route *bestRoute = nullptr;
    for (auto e : routes) {
        if (e->isValid()) {
            if (IPv4Address::maskedAddrAreEqual(dest, e->getDestination(), e->getNetmask())) {    // match
                bestRoute = const_cast<IPv4Route *>(e);
                break;
            }
        }
    }

    routingCache[dest] = bestRoute;
    return bestRoute;
}

InterfaceEntry *IPv4RoutingTable::getInterfaceForDestAddr(const IPv4Address& dest) const
{
    Enter_Method("getInterfaceForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3));    // note: str().c_str() too slow here

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getInterface() : nullptr;
}

IPv4Address IPv4RoutingTable::getGatewayForDestAddr(const IPv4Address& dest) const
{
    Enter_Method("getGatewayForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3));    // note: str().c_str() too slow here

    const IPv4Route *e = findBestMatchingRoute(dest);
    return e ? e->getGateway() : IPv4Address();
}

const IPv4MulticastRoute *IPv4RoutingTable::findBestMatchingMulticastRoute(const IPv4Address& origin, const IPv4Address& group) const
{
    Enter_Method("getMulticastRoutesFor(%u.%u.%u.%u, %u.%u.%u.%u)",
            origin.getDByte(0), origin.getDByte(1), origin.getDByte(2), origin.getDByte(3),
            group.getDByte(0), group.getDByte(1), group.getDByte(2), group.getDByte(3));    // note: str().c_str() too slow here here

    // TODO caching?

    for (auto e : multicastRoutes) {
        if (e->isValid() && e->matches(origin, group))
            return e;
    }

    return nullptr;
}

IPv4Route *IPv4RoutingTable::getRoute(int k) const
{
    if (k < (int)routes.size())
        return routes[k];
    return nullptr;
}

IPv4Route *IPv4RoutingTable::getDefaultRoute() const
{
    // if exists default route entry, it is the last valid entry
    for (RouteVector::const_reverse_iterator i = routes.rbegin(); i != routes.rend() && (*i)->getNetmask().isUnspecified(); ++i) {
        if ((*i)->isValid())
            return *i;
    }
    return nullptr;
}

// The 'routes' vector stores the routes in this order.
// The best matching route should precede the other matching routes,
// so the method should return true if a is better the b.
bool IPv4RoutingTable::routeLessThan(const IPv4Route *a, const IPv4Route *b) const
{
    // longer prefixes are better, because they are more specific
    if (a->getNetmask() != b->getNetmask())
        return a->getNetmask() > b->getNetmask();

    if (a->getDestination() != b->getDestination())
        return a->getDestination() < b->getDestination();

    // for the same destination/netmask:

    // smaller administration distance is better (if useAdminDist)
    if (useAdminDist && (a->getAdminDist() != b->getAdminDist()))
        return a->getAdminDist() < b->getAdminDist();

    // smaller metric is better
    return a->getMetric() < b->getMetric();
}

void IPv4RoutingTable::setRouterId(IPv4Address a)
{
    routerId = a;
}

void IPv4RoutingTable::internalAddRoute(IPv4Route *entry)
{
    if (!entry->getNetmask().isValidNetmask())
        throw cRuntimeError("addRoute(): wrong netmask %s in route", entry->getNetmask().str().c_str());

    if (entry->getNetmask().getInt() != 0 && (entry->getDestination().getInt() & entry->getNetmask().getInt()) == 0)
        throw cRuntimeError("addRoute(): all bits of destination address %s is 0 inside non zero netmask %s",
                entry->getDestination().str().c_str(), entry->getNetmask().str().c_str());

    if ((entry->getDestination().getInt() & ~entry->getNetmask().getInt()) != 0)
        throw cRuntimeError("addRoute(): suspicious route: destination IP address %s has bits set outside netmask %s",
                entry->getDestination().str().c_str(), entry->getNetmask().str().c_str());

    // check that the interface exists
    if (!entry->getInterface())
        throw cRuntimeError("addRoute(): interface cannot be nullptr");

    // if this is a default route, remove old default route (we're replacing it)
    if (entry->getNetmask().isUnspecified()) {
        IPv4Route *oldDefaultRoute = getDefaultRoute();
        if (oldDefaultRoute != nullptr)
            deleteRoute(oldDefaultRoute);
    }

    // The 'routes' vector may contain multiple routes with the same destination/netmask.
    // Routes are stored in descending netmask length and ascending administrative_distance/metric order,
    // so the first matching is the best one.
    // XXX Should only the route with the best metic be stored? Then the worse route should be deleted and
    //     internalAddRoute() should return a bool indicating if it was successful.

    // add to tables
    // we keep entries sorted by netmask desc, metric asc in routeList, so that we can
    // stop at the first match when doing the longest netmask matching
    auto pos = upper_bound(routes.begin(), routes.end(), entry, RouteLessThan(*this));
    routes.insert(pos, entry);

    entry->setRoutingTable(this);
}

void IPv4RoutingTable::addRoute(IPv4Route *entry)
{
    Enter_Method("addRoute(...)");

    internalAddRoute(entry);

    invalidateCache();
    updateDisplayString();

    emit(NF_ROUTE_ADDED, entry);
}

IPv4Route *IPv4RoutingTable::internalRemoveRoute(IPv4Route *entry)
{
    auto i = std::find(routes.begin(), routes.end(), entry);
    if (i != routes.end()) {
        routes.erase(i);
        return entry;
    }
    return nullptr;
}

IPv4Route *IPv4RoutingTable::removeRoute(IPv4Route *entry)
{
    Enter_Method("removeRoute(...)");

    entry = internalRemoveRoute(entry);

    if (entry != nullptr) {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this);    // still filled in, for the listeners' benefit
        emit(NF_ROUTE_DELETED, entry);
        entry->setRoutingTable(nullptr);
    }
    return entry;
}

bool IPv4RoutingTable::deleteRoute(IPv4Route *entry)    //TODO this is almost duplicate of removeRoute()
{
    Enter_Method("deleteRoute(...)");

    entry = internalRemoveRoute(entry);

    if (entry != nullptr) {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this);    // still filled in, for the listeners' benefit
        emit(NF_ROUTE_DELETED, entry);
        delete entry;
    }
    return entry != nullptr;
}

bool IPv4RoutingTable::multicastRouteLessThan(const IPv4MulticastRoute *a, const IPv4MulticastRoute *b)
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

void IPv4RoutingTable::internalAddMulticastRoute(IPv4MulticastRoute *entry)
{
    if (!entry->getOriginNetmask().isValidNetmask())
        throw cRuntimeError("addMulticastRoute(): wrong netmask %s in multicast route", entry->getOriginNetmask().str().c_str());

    if ((entry->getOrigin().getInt() & ~entry->getOriginNetmask().getInt()) != 0)
        throw cRuntimeError("addMulticastRoute(): suspicious route: origin IP address %s has bits set outside netmask %s",
                entry->getOrigin().str().c_str(), entry->getOriginNetmask().str().c_str());

    if (!entry->getMulticastGroup().isUnspecified() && !entry->getMulticastGroup().isMulticast())
        throw cRuntimeError("addMulticastRoute(): group address (%s) is not a multicast address",
                entry->getMulticastGroup().str().c_str());

    // check that the interface exists
    if (entry->getInInterface() && !entry->getInInterface()->getInterface()->isMulticast())
        throw cRuntimeError("addMulticastRoute(): input interface must be multicast capable");

    for (unsigned int i = 0; i < entry->getNumOutInterfaces(); i++) {
        IPv4MulticastRoute::OutInterface *outInterface = entry->getOutInterface(i);
        if (!outInterface)
            throw cRuntimeError("addMulticastRoute(): output interface cannot be nullptr");
        else if (!outInterface->getInterface()->isMulticast())
            throw cRuntimeError("addMulticastRoute(): output interface must be multicast capable");
        else if (entry->getInInterface() && outInterface->getInterface() == entry->getInInterface()->getInterface())
            throw cRuntimeError("addMulticastRoute(): output interface cannot be the same as the input interface");
    }

    // add to tables
    // we keep entries sorted by netmask desc, metric asc in routeList, so that we can
    // stop at the first match when doing the longest netmask matching
    auto pos =
        upper_bound(multicastRoutes.begin(), multicastRoutes.end(), entry, multicastRouteLessThan);
    multicastRoutes.insert(pos, entry);

    entry->setRoutingTable(this);
}

void IPv4RoutingTable::addMulticastRoute(IPv4MulticastRoute *entry)
{
    Enter_Method("addMulticastRoute(...)");

    internalAddMulticastRoute(entry);

    invalidateCache();
    updateDisplayString();

    emit(NF_MROUTE_ADDED, entry);
}

IPv4MulticastRoute *IPv4RoutingTable::internalRemoveMulticastRoute(IPv4MulticastRoute *entry)
{
    auto i = std::find(multicastRoutes.begin(), multicastRoutes.end(), entry);
    if (i != multicastRoutes.end()) {
        multicastRoutes.erase(i);
        return entry;
    }
    return nullptr;
}

IPv4MulticastRoute *IPv4RoutingTable::removeMulticastRoute(IPv4MulticastRoute *entry)
{
    Enter_Method("removeMulticastRoute(...)");

    entry = internalRemoveMulticastRoute(entry);

    if (entry != nullptr) {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this);    // still filled in, for the listeners' benefit
        emit(NF_MROUTE_DELETED, entry);
        entry->setRoutingTable(nullptr);
    }
    return entry;
}

bool IPv4RoutingTable::deleteMulticastRoute(IPv4MulticastRoute *entry)
{
    Enter_Method("deleteMulticastRoute(...)");

    entry = internalRemoveMulticastRoute(entry);

    if (entry != nullptr) {
        invalidateCache();
        updateDisplayString();
        ASSERT(entry->getRoutingTable() == this);    // still filled in, for the listeners' benefit
        emit(NF_MROUTE_DELETED, entry);
        delete entry;
    }
    return entry != nullptr;
}

void IPv4RoutingTable::routeChanged(IPv4Route *entry, int fieldCode)
{
    if (fieldCode == IPv4Route::F_DESTINATION || fieldCode == IPv4Route::F_PREFIX_LENGTH || fieldCode == IPv4Route::F_METRIC) {    // our data structures depend on these fields
        entry = internalRemoveRoute(entry);
        ASSERT(entry != nullptr);    // failure means inconsistency: route was not found in this routing table
        internalAddRoute(entry);

        invalidateCache();
        updateDisplayString();
    }
    emit(NF_ROUTE_CHANGED, entry);    // TODO include fieldCode in the notification
}

void IPv4RoutingTable::multicastRouteChanged(IPv4MulticastRoute *entry, int fieldCode)
{
    if (fieldCode == IPv4MulticastRoute::F_ORIGIN || fieldCode == IPv4MulticastRoute::F_ORIGINMASK ||
        fieldCode == IPv4MulticastRoute::F_MULTICASTGROUP || fieldCode == IPv4MulticastRoute::F_METRIC)    // our data structures depend on these fields
    {
        entry = internalRemoveMulticastRoute(entry);
        ASSERT(entry != nullptr);    // failure means inconsistency: route was not found in this routing table
        internalAddMulticastRoute(entry);

        invalidateCache();
        updateDisplayString();
    }
    emit(NF_MROUTE_CHANGED, entry);    // TODO include fieldCode in the notification
}

void IPv4RoutingTable::updateNetmaskRoutes()
{
    // first, delete all routes with src=IFACENETMASK
    for (unsigned int k = 0; k < routes.size(); k++) {
        if (routes[k]->getSourceType() == IRoute::IFACENETMASK) {
            auto it = routes.begin() + (k--);    // '--' is necessary because indices shift down
            IPv4Route *route = *it;
            routes.erase(it);
            ASSERT(route->getRoutingTable() == this);    // still filled in, for the listeners' benefit
            emit(NF_ROUTE_DELETED, route);
            delete route;
        }
    }

    // then re-add them, according to actual interface configuration
    // TODO: say there's a node somewhere in the network that belongs to the interface's subnet
    // TODO: and it is not on the same link, and the gateway does not use proxy ARP, how will packets reach that node?
    PatternMatcher interfaceNameMatcher(netmaskRoutes, false, true, true);
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (interfaceNameMatcher.matches(ie->getFullName()) && ie->ipv4Data() && ie->ipv4Data()->getNetmask() != IPv4Address::ALLONES_ADDRESS)
        {
            IPv4Route *route = createNewRoute();
            route->setSourceType(IRoute::IFACENETMASK);
            route->setSource(ie);
            route->setDestination(ie->ipv4Data()->getIPAddress().doAnd(ie->ipv4Data()->getNetmask()));
            route->setNetmask(ie->ipv4Data()->getNetmask());
            route->setGateway(IPv4Address());
            route->setAdminDist(IPv4Route::dDirectlyConnected);
            route->setMetric(ie->ipv4Data()->getMetric());
            route->setInterface(ie);
            route->setRoutingTable(this);
            auto pos = upper_bound(routes.begin(), routes.end(), route, RouteLessThan(*this));
            routes.insert(pos, route);
            emit(NF_ROUTE_ADDED, route);
        }
    }

    invalidateCache();
    updateDisplayString();
}

bool IPv4RoutingTable::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_NETWORK_LAYER) {
            // read routing table file (and interface configuration)
            const char *filename = par("routingFile");
            RoutingTableParser parser(ift, this);
            if (*filename && parser.readRoutingTableFromFile(filename) == -1)
                throw cRuntimeError("Error reading routing table file %s", filename);
        }
        else if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_TRANSPORT_LAYER) {
            configureRouterId();
            updateNetmaskRoutes();
            isNodeUp = true;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_NETWORK_LAYER) {
            while (!routes.empty())
                delete removeRoute(routes[0]);
            isNodeUp = false;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            while (!routes.empty())
                delete removeRoute(routes[0]);
            isNodeUp = false;
        }
    }
    return true;
}

IPv4Route *IPv4RoutingTable::createNewRoute()
{
    return new IPv4Route();
}

} // namespace inet

