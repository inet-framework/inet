//
// Copyright (C) 2012 Opensim Ltd.
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

#include <algorithm>
#include <sstream>

#include "inet/networklayer/generic/GenericRoutingTable.h"

#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/generic/GenericRoute.h"
#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(GenericRoutingTable);

GenericRoutingTable::GenericRoutingTable()
{
    ift = NULL;
}

GenericRoutingTable::~GenericRoutingTable()
{
    for (unsigned int i = 0; i < routes.size(); i++)
        delete routes[i];
    for (unsigned int i = 0; i < multicastRoutes.size(); i++)
        delete multicastRoutes[i];
}

void GenericRoutingTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // get a pointer to the IInterfaceTable
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        const char *addressTypeString = par("addressType");
        if (!strcmp(addressTypeString, "mac"))
            addressType = L3Address::MAC;
        else if (!strcmp(addressTypeString, "modulepath"))
            addressType = L3Address::MODULEPATH;
        else if (!strcmp(addressTypeString, "moduleid"))
            addressType = L3Address::MODULEID;
        else
            throw cRuntimeError("Unknown address type");
        forwarding = par("forwarding").boolValue();
        multicastForwarding = par("multicastForwarding");

//TODO        WATCH_PTRVECTOR(routes);
//TODO        WATCH_PTRVECTOR(multicastRoutes);
        WATCH(forwarding);
        WATCH(multicastForwarding);
        WATCH(routerId);

        cModule *host = getContainingNode(this);
        host->subscribe(NF_INTERFACE_CREATED, this);
        host->subscribe(NF_INTERFACE_DELETED, this);
        host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
        host->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
        host->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        // At this point, all L2 modules have registered themselves (added their
        // interface entries). Create the per-interface IPv4 data structures.
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        for (int i = 0; i < interfaceTable->getNumInterfaces(); ++i)
            configureInterface(interfaceTable->getInterface(i));
        configureLoopback();

//        // read routing table file (and interface configuration)
//        RoutingTableParser parser(ift, this);
//        if (*filename && parser.readRoutingTableFromFile(filename)==-1)
//            throw cRuntimeError("Error reading routing table file %s", filename);

//TODO
//        // set routerId if param is not "" (==no routerId) or "auto" (in which case we'll
//        // do it later in a later stage, after network configurators configured the interfaces)
//        const char *routerIdStr = par("routerId").stringValue();
//        if (strcmp(routerIdStr, "") && strcmp(routerIdStr, "auto"))
//            routerId = IPv4Address(routerIdStr);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {
        // routerID selection must be after network autoconfiguration assigned interface addresses
        configureRouterId();

//        // we don't use notifications during initialize(), so we do it manually.
//        updateNetmaskRoutes();

        //printRoutingTable();
    }
}

void GenericRoutingTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void GenericRoutingTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    // TODO:
}

void GenericRoutingTable::routeChanged(GenericRoute *entry, int fieldCode)
{
    if (fieldCode == IRoute::F_DESTINATION || fieldCode == IRoute::F_PREFIX_LENGTH || fieldCode == IRoute::F_METRIC) {    // our data structures depend on these fields
        entry = internalRemoveRoute(entry);
        ASSERT(entry != NULL);    // failure means inconsistency: route was not found in this routing table
        internalAddRoute(entry);

        //invalidateCache();
        updateDisplayString();
    }
    emit(NF_ROUTE_CHANGED, entry);    // TODO include fieldCode in the notification
}

void GenericRoutingTable::configureRouterId()
{
    if (routerId.isUnspecified()) {    // not yet configured
        const char *routerIdStr = par("routerId").stringValue();
        if (!strcmp(routerIdStr, "auto")) {    // non-"auto" cases already handled in earlier stage
            // choose highest interface address as routerId
            for (int i = 0; i < ift->getNumInterfaces(); ++i) {
                InterfaceEntry *ie = ift->getInterface(i);
                if (!ie->isLoopback()) {
                    L3Address interfaceAddr = ie->getGenericNetworkProtocolData()->getAddress();
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
//        if (getInterfaceByAddress(routerId)==NULL)
//        {
//            InterfaceEntry *lo0 = ift->getFirstLoopbackInterface();
//            lo0->getGenericNetworkProtocolData()->setAddress(routerId);
//        }
//    }
}

void GenericRoutingTable::configureInterface(InterfaceEntry *ie)
{
    int metric = (int)(ceil(2e9 / ie->getDatarate()));    // use OSPF cost as default
    int interfaceModuleId = ie->getInterfaceModule() ? ie->getInterfaceModule()->getParentModule()->getId() : -1;
    // mac
    GenericNetworkProtocolInterfaceData *d = new GenericNetworkProtocolInterfaceData();
    d->setMetric(metric);
    if (addressType == L3Address::MAC)
        d->setAddress(ie->getMacAddress());
    else if (ie->getInterfaceModule() && addressType == L3Address::MODULEPATH)
        d->setAddress(ModulePathAddress(interfaceModuleId));
    else if (ie->getInterfaceModule() && addressType == L3Address::MODULEID)
        d->setAddress(ModuleIdAddress(interfaceModuleId));
    ie->setGenericNetworkProtocolData(d);
}

void GenericRoutingTable::configureLoopback()
{
    InterfaceEntry *ie = ift->getFirstLoopbackInterface();

//TODO needed???
//    // add IPv4 info. Set 127.0.0.1/8 as address by default --
//    // we may reconfigure later it to be the routerId
//    IPv4InterfaceData *d = new IPv4InterfaceData();
//    d->setIPAddress(IPv4Address::LOOPBACK_ADDRESS);
//    d->setNetmask(IPv4Address::LOOPBACK_NETMASK);
//    d->setMetric(1);
//    ie->setIPv4Data(d);
}

void GenericRoutingTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

//TODO
//    char buf[80];
//    if (routerId.isUnspecified())
//        sprintf(buf, "%d+%d routes", (int)routes.size(), (int)multicastRoutes.size());
//    else
//        sprintf(buf, "routerId: %s\n%d+%d routes", routerId.str().c_str(), (int)routes.size(), (int)multicastRoutes.size());
//    getDisplayString().setTagArg("t", 0, buf);
}

bool GenericRoutingTable::routeLessThan(const GenericRoute *a, const GenericRoute *b)
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

bool GenericRoutingTable::isForwardingEnabled() const
{
    return forwarding;
}

bool GenericRoutingTable::isMulticastForwardingEnabled() const
{
    return multicastForwarding;
}

L3Address GenericRoutingTable::getRouterIdAsGeneric() const
{
    return routerId;
}

bool GenericRoutingTable::isLocalAddress(const L3Address& dest) const
{
    //TODO: Enter_Method("isLocalAddress(%s)", dest.str().c_str());

    // collect interface addresses if not yet done
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        L3Address interfaceAddr = ift->getInterface(i)->getGenericNetworkProtocolData()->getAddress();
        if (interfaceAddr == dest)
            return true;
    }
    return false;
}

InterfaceEntry *GenericRoutingTable::getInterfaceByAddress(const L3Address& address) const
{
    // collect interface addresses if not yet done
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *ie = ift->getInterface(i);
        L3Address interfaceAddr = ie->getGenericNetworkProtocolData()->getAddress();
        if (interfaceAddr == address)
            return ie;
    }
    return NULL;
}

GenericRoute *GenericRoutingTable::findBestMatchingRoute(const L3Address& dest) const
{
    //TODO Enter_Method("findBestMatchingRoute(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    GenericRoute *bestRoute = NULL;
    for (RouteVector::const_iterator i = routes.begin(); i != routes.end(); ++i) {
        GenericRoute *e = *i;
        if (dest.matches(e->getDestinationAsGeneric(), e->getPrefixLength())) {
            bestRoute = const_cast<GenericRoute *>(e);
            break;
        }
    }
    return bestRoute;
}

InterfaceEntry *GenericRoutingTable::getOutputInterfaceForDestination(const L3Address& dest) const
{
    //TODO Enter_Method("getInterfaceForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    const IRoute *e = findBestMatchingRoute(dest);
    return e ? e->getInterface() : NULL;
}

L3Address GenericRoutingTable::getNextHopForDestination(const L3Address& dest) const
{
    //TODO Enter_Method("getGatewayForDestAddr(%u.%u.%u.%u)", dest.getDByte(0), dest.getDByte(1), dest.getDByte(2), dest.getDByte(3)); // note: str().c_str() too slow here

    const IRoute *e = findBestMatchingRoute(dest);
    return e ? e->getNextHopAsGeneric() : L3Address();
}

bool GenericRoutingTable::isLocalMulticastAddress(const L3Address& dest) const
{
    return dest.isMulticast();    //TODO
}

IMulticastRoute *GenericRoutingTable::findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const
{
    return NULL;    //TODO
}

int GenericRoutingTable::getNumRoutes() const
{
    return routes.size();
}

IRoute *GenericRoutingTable::getRoute(int k) const
{
    ASSERT(k >= 0 && (unsigned int)k < routes.size());
    return routes[k];
}

IRoute *GenericRoutingTable::getDefaultRoute() const
{
    // if there is a default route entry, it is the last valid entry
    for (RouteVector::const_reverse_iterator i = routes.rbegin(); i != routes.rend() && (*i)->getPrefixLength() == 0; ++i)
        return *i;
    return NULL;
}

void GenericRoutingTable::addRoute(IRoute *route)
{
    Enter_Method("addRoute(...)");

    GenericRoute *entry = dynamic_cast<GenericRoute *>(route);

    // check that the interface exists
    if (!entry->getInterface())
        throw cRuntimeError("addRoute(): interface cannot be NULL");

    internalAddRoute(entry);

    updateDisplayString();
    emit(NF_ROUTE_ADDED, entry);
}

IRoute *GenericRoutingTable::removeRoute(IRoute *route)
{
    Enter_Method("removeRoute(...)");

    GenericRoute *entry = internalRemoveRoute(check_and_cast<GenericRoute *>(route));
    if (entry) {
        updateDisplayString();
        emit(NF_ROUTE_DELETED, entry);
    }

    return entry;
}

bool GenericRoutingTable::deleteRoute(IRoute *entry)
{
    IRoute *route = removeRoute(entry);
    delete route;
    return route != NULL;
}

void GenericRoutingTable::internalAddRoute(GenericRoute *route)
{
    ASSERT(route->getRoutingTableAsGeneric() == NULL);

    // add to tables
    // we keep entries sorted, so that we can stop at the first match when doing the longest prefix matching
    RouteVector::iterator pos = upper_bound(routes.begin(), routes.end(), route, routeLessThan);
    routes.insert(pos, route);

    route->setRoutingTable(this);
}

GenericRoute *GenericRoutingTable::internalRemoveRoute(GenericRoute *route)
{
    RouteVector::iterator i = std::find(routes.begin(), routes.end(), route);
    if (i != routes.end()) {
        ASSERT(route->getRoutingTableAsGeneric() == this);
        routes.erase(i);
        route->setRoutingTable(NULL);
        return route;
    }
    return NULL;
}

int GenericRoutingTable::getNumMulticastRoutes() const
{
    return 0;    //TODO
}

IMulticastRoute *GenericRoutingTable::getMulticastRoute(int k) const
{
    return NULL;    //TODO
}

void GenericRoutingTable::addMulticastRoute(IMulticastRoute *entry)
{
    //TODO
}

IMulticastRoute *GenericRoutingTable::removeMulticastRoute(IMulticastRoute *entry)
{
    return NULL;    //TODO
}

bool GenericRoutingTable::deleteMulticastRoute(IMulticastRoute *entry)
{
    return false;    //TODO
}

IRoute *GenericRoutingTable::createRoute()
{
    return new GenericRoute();
}

} // namespace inet

