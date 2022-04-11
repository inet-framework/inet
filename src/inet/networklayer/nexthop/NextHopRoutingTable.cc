//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/nexthop/NextHopRoutingTable.h"

#include <algorithm>
#include <sstream>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#include "inet/networklayer/nexthop/NextHopRoute.h"

namespace inet {

Define_Module(NextHopRoutingTable);

std::ostream& operator<<(std::ostream& os, const NextHopRoute& e)
{
    os << e.str();
    return os;
};

std::ostream& operator<<(std::ostream& os, const NextHopMulticastRoute& e)
{
    os << e.str();
    return os;
};

NextHopRoutingTable::NextHopRoutingTable()
{
}

NextHopRoutingTable::~NextHopRoutingTable()
{
    for (auto& elem : routes)
        delete elem;
    for (auto& elem : multicastRoutes)
        delete elem;
}

void NextHopRoutingTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // get a pointer to the IInterfaceTable
        ift.reference(this, "interfaceTableModule", true);

        const char *addressTypeString = par("addressType");
        if (!strcmp(addressTypeString, "mac"))
            addressType = L3Address::MAC;
        else if (!strcmp(addressTypeString, "modulepath"))
            addressType = L3Address::MODULEPATH;
        else if (!strcmp(addressTypeString, "moduleid"))
            addressType = L3Address::MODULEID;
        else
            throw cRuntimeError("Unknown address type");
        forwarding = par("forwarding");
        multicastForwarding = par("multicastForwarding");

        WATCH_PTRVECTOR(routes);
        WATCH_PTRVECTOR(multicastRoutes);
        WATCH(forwarding);
        WATCH(multicastForwarding);
        WATCH(routerId);

        cModule *host = getContainingNode(this);
        host->subscribe(interfaceCreatedSignal, this);
        host->subscribe(interfaceDeletedSignal, this);
        host->subscribe(interfaceStateChangedSignal, this);
        host->subscribe(interfaceConfigChangedSignal, this);
        host->subscribe(interfaceIpv4ConfigChangedSignal, this);
    }
    // TODO INITSTAGE
    else if (stage == INITSTAGE_LINK_LAYER) {
        // At this point, all L2 modules have registered themselves (added their
        // interface entries). Create the per-interface Ipv4 data structures.
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        for (int i = 0; i < interfaceTable->getNumInterfaces(); ++i)
            configureInterface(interfaceTable->getInterface(i));
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        configureLoopback();

        // read routing table file (and interface configuration)
//        RoutingTableParser parser(ift, this);
//        if (*filename && parser.readRoutingTableFromFile(filename)==-1)
//            throw cRuntimeError("Error reading routing table file %s", filename);

        // TODO
        // set routerId if param is not "" (==no routerId) or "auto" (in which case we'll
        // do it later in a later stage, after network configurators configured the interfaces)
//        const char *routerIdStr = par("routerId");
//        if (strcmp(routerIdStr, "") && strcmp(routerIdStr, "auto"))
//            routerId = Ipv4Address(routerIdStr);
        // routerID selection must be after network autoconfiguration assigned interface addresses
        configureRouterId();

        // we don't use notifications during initialize(), so we do it manually.
//        updateNetmaskRoutes();

//        printRoutingTable();
    }
}

void NextHopRoutingTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void NextHopRoutingTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    // TODO
}

void NextHopRoutingTable::routeChanged(NextHopRoute *entry, int fieldCode)
{
    if (fieldCode == IRoute::F_DESTINATION || fieldCode == IRoute::F_PREFIX_LENGTH || fieldCode == IRoute::F_METRIC) { // our data structures depend on these fields
        entry = internalRemoveRoute(entry);
        ASSERT(entry != nullptr); // failure means inconsistency: route was not found in this routing table
        internalAddRoute(entry);

//        invalidateCache();
    }
    emit(routeChangedSignal, entry); // TODO include fieldCode in the notification
}

void NextHopRoutingTable::configureRouterId()
{
    if (routerId.isUnspecified()) { // not yet configured
        const char *routerIdStr = par("routerId");
        if (!strcmp(routerIdStr, "auto")) { // non-"auto" cases already handled in earlier stage
            // choose highest interface address as routerId
            for (int i = 0; i < ift->getNumInterfaces(); ++i) {
                NetworkInterface *ie = ift->getInterface(i);
                if (!ie->isLoopback()) {
                    L3Address interfaceAddr = ie->getProtocolData<NextHopInterfaceData>()->getAddress();
                    if (routerId.isUnspecified() || routerId < interfaceAddr)
                        routerId = interfaceAddr;
                }
            }
        }
    }
//    else // already configured
//    {
//        // if there is no interface with routerId yet, assign it to the loopback address;
//        // TODO find out if this is a good practice, in which situations it is useful etc.
//        if (getInterfaceByAddress(routerId)==nullptr)
//        {
//            NetworkInterface *lo0 = ift->getFirstLoopbackInterface();
//            lo0->getNextHopProtocolData()->setAddress(routerId);
//        }
//    }
}

void NextHopRoutingTable::configureInterface(NetworkInterface *ie)
{
    int metric = ie->getDatarate() != 0 ? (int)(ceil(2e9 / ie->getDatarate())) : 1; // use OSPF cost as default
    int interfaceModuleId = ie->getId();
    // mac
    auto d = ie->addProtocolData<NextHopInterfaceData>();
    d->setMetric(metric);
    if (addressType == L3Address::MAC)
        d->setAddress(ie->getMacAddress());
    else if (ie && addressType == L3Address::MODULEPATH)
        d->setAddress(ModulePathAddress(interfaceModuleId));
    else if (ie && addressType == L3Address::MODULEID)
        d->setAddress(ModuleIdAddress(interfaceModuleId));
}

void NextHopRoutingTable::configureLoopback()
{
//TODO needed???
//    NetworkInterface *ie = ift->getFirstLoopbackInterface()
//    // add Ipv4 info. Set 127.0.0.1/8 as address by default --
//    // we may reconfigure later it to be the routerId
//    Ipv4InterfaceData *d = new Ipv4InterfaceData();
//    d->setIPAddress(Ipv4Address::LOOPBACK_ADDRESS);
//    d->setNetmask(Ipv4Address::LOOPBACK_NETMASK);
//    d->setMetric(1);
//    ie->setIPv4Data(d);
}

void NextHopRoutingTable::refreshDisplay() const
{
// TODO
//    char buf[80];
//    if (routerId.isUnspecified())
//        sprintf(buf, "%d+%d routes", (int)routes.size(), (int)multicastRoutes.size());
//    else
//        sprintf(buf, "routerId: %s\n%d+%d routes", routerId.str().c_str(), (int)routes.size(), (int)multicastRoutes.size());
//    getDisplayString().setTagArg("t", 0, buf);
}

bool NextHopRoutingTable::routeLessThan(const NextHopRoute *a, const NextHopRoute *b)
{
    // helper for sort() in addRoute(). We want routes with longer
    // prefixes to be at front, so we compare them as "less".
    // For metric, a smaller value is better (we report that as "less").
    if (a->getPrefixLength() != b->getPrefixLength())
        return a->getPrefixLength() > b->getPrefixLength();

    if (a->getDestinationAsGeneric() != b->getDestinationAsGeneric())
        return a->getDestinationAsGeneric() < b->getDestinationAsGeneric();

    return a->getMetric() < b->getMetric();
}

bool NextHopRoutingTable::isForwardingEnabled() const
{
    return forwarding;
}

bool NextHopRoutingTable::isMulticastForwardingEnabled() const
{
    return multicastForwarding;
}

L3Address NextHopRoutingTable::getRouterIdAsGeneric() const
{
    return routerId;
}

bool NextHopRoutingTable::isLocalAddress(const L3Address& dest) const
{
    Enter_Method("isLocalAddress(%s)", dest.str().c_str());

    return ift->isLocalAddress(dest);
}

NetworkInterface *NextHopRoutingTable::getInterfaceByAddress(const L3Address& address) const
{
    Enter_Method("getInterfaceByAddress(%s)", address.str().c_str());

    return ift->findInterfaceByAddress(address);
}

NextHopRoute *NextHopRoutingTable::findBestMatchingRoute(const L3Address& dest) const
{
    Enter_Method("findBestMatchingRoute(%s)", dest.str().c_str());

    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    NextHopRoute *bestRoute = nullptr;
    for (auto e : routes) {

        if (dest.matches(e->getDestinationAsGeneric(), e->getPrefixLength())) {
            bestRoute = const_cast<NextHopRoute *>(e);
            break;
        }
    }
    return bestRoute;
}

NetworkInterface *NextHopRoutingTable::getOutputInterfaceForDestination(const L3Address& dest) const
{
    Enter_Method("getInterfaceForDestAddr(%s)", dest.str().c_str());

    const IRoute *e = findBestMatchingRoute(dest);
    return e ? e->getInterface() : nullptr;
}

L3Address NextHopRoutingTable::getNextHopForDestination(const L3Address& dest) const
{
    Enter_Method("getGatewayForDestAddr(%s)", dest.str().c_str());

    const IRoute *e = findBestMatchingRoute(dest);
    return e ? e->getNextHopAsGeneric() : L3Address();
}

bool NextHopRoutingTable::isLocalMulticastAddress(const L3Address& dest) const
{
    Enter_Method("isLocalMulticastAddress(%s)", dest.str().c_str());

    return dest.isMulticast(); // TODO
}

IMulticastRoute *NextHopRoutingTable::findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const
{
    Enter_Method("findBestMatchingMulticastRoute(%s, %s)", origin.str().c_str(), group.str().c_str());

    return nullptr; // TODO
}

int NextHopRoutingTable::getNumRoutes() const
{
    return routes.size();
}

IRoute *NextHopRoutingTable::getRoute(int k) const
{
    ASSERT(k >= 0 && (unsigned int)k < routes.size());
    return routes[k];
}

IRoute *NextHopRoutingTable::getDefaultRoute() const
{
    // if there is a default route entry, it is the last valid entry
    auto i = routes.rbegin();
    if (i != routes.rend() && (*i)->getPrefixLength() == 0)
        return *i;
    return nullptr;
}

void NextHopRoutingTable::addRoute(IRoute *route)
{
    Enter_Method("addRoute(...)");

    NextHopRoute *entry = check_and_cast<NextHopRoute *>(route);

    // check that the interface exists
    if (!entry->getInterface())
        throw cRuntimeError("addRoute(): interface cannot be nullptr");

    internalAddRoute(entry);

    emit(routeAddedSignal, entry);
}

IRoute *NextHopRoutingTable::removeRoute(IRoute *route)
{
    Enter_Method("removeRoute(...)");

    NextHopRoute *entry = internalRemoveRoute(check_and_cast<NextHopRoute *>(route));
    if (entry != nullptr) {
        EV_INFO << "remove route " << entry->str() << "\n";
        emit(routeDeletedSignal, entry);
        entry->setRoutingTable(nullptr);
    }

    return entry;
}

bool NextHopRoutingTable::deleteRoute(IRoute *route)
{
    Enter_Method("deleteRoute(...)");

    NextHopRoute *entry = internalRemoveRoute(check_and_cast<NextHopRoute *>(route));
    if (entry != nullptr) {
        EV_INFO << "remove route " << entry->str() << "\n";
        emit(routeDeletedSignal, entry);
        delete entry;
    }
    return entry != nullptr;
}

void NextHopRoutingTable::internalAddRoute(NextHopRoute *route)
{
    ASSERT(route->getRoutingTableAsGeneric() == nullptr);

    // add to tables
    // we keep entries sorted, so that we can stop at the first match when doing the longest prefix matching
    auto pos = upper_bound(routes.begin(), routes.end(), route, routeLessThan);
    routes.insert(pos, route);

    route->setRoutingTable(this);
}

NextHopRoute *NextHopRoutingTable::internalRemoveRoute(NextHopRoute *route)
{
    auto i = find(routes, route);
    if (i != routes.end()) {
        ASSERT(route->getRoutingTableAsGeneric() == this);
        routes.erase(i);
        return route;
    }
    return nullptr;
}

int NextHopRoutingTable::getNumMulticastRoutes() const
{
    return 0; // TODO
}

IMulticastRoute *NextHopRoutingTable::getMulticastRoute(int k) const
{
    return nullptr; // TODO
}

void NextHopRoutingTable::addMulticastRoute(IMulticastRoute *entry)
{
    Enter_Method("addMulticastRoute(...)");

    // TODO
}

IMulticastRoute *NextHopRoutingTable::removeMulticastRoute(IMulticastRoute *entry)
{
    Enter_Method("removeMulticastRoute(...)");

    return nullptr; // TODO
}

bool NextHopRoutingTable::deleteMulticastRoute(IMulticastRoute *entry)
{
    Enter_Method("deleteMulticastRoute(...)");

    return false; // TODO
}

IRoute *NextHopRoutingTable::createRoute()
{
    return new NextHopRoute();
}

void NextHopRoutingTable::printRoutingTable() const
{
    for (const auto& elem : routes)
        EV_INFO << (elem)->getInterface()->getInterfaceFullPath() << " -> " << (elem)->getDestinationAsGeneric().str() << " as " << (elem)->str() << endl;
}

} // namespace inet

