//
// Copyright (C) 2013 Opensim Ltd.
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

#include "IAddressType.h"
#include "InterfaceTableAccess.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "UDP.h"

#include "RIPPacket_m.h"
#include "RIPRouting.h"

Define_Module(RIPRouting);

#define RIP_EV EV << "RIP at " << getHostName() << " "

// XXX temporarily
inline Address netmask(const Address &addrType, int prefixLength)
{
    return IPv4Address::makeNetmask(prefixLength); // XXX IPv4 only
}

inline Address unspecifiedAddress(const Address &addrType)
{
    return addrType.getType() == Address::IPv4 ? Address(IPv4Address::UNSPECIFIED_ADDRESS) : Address(IPv6Address::UNSPECIFIED_ADDRESS); // XXX
}

inline int prefixLength(const Address &netmask)
{
    return netmask.getType() == Address::IPv4 ? netmask.toIPv4().getNetmaskLength() : 32; // XXX IPv4 only
}

bool RIPRouting::isNeighbourAddress(const Address &address)
{
    return true; // TODO
}

std::ostream& operator<<(std::ostream& os, const RIPRoute& e)
{
    os << e.info();
    return os;
}

std::string RIPRoute::info() const
{
    std::stringstream out;

    if (route)
    {
        const Address &dest = route->getDestinationAsGeneric();
        int prefixLength = route->getPrefixLength();
        const Address &gateway = route->getNextHopAsGeneric();
        InterfaceEntry *interfacePtr = route->getInterface();
        out << "dest:"; if (dest.isUnspecified()) out << "*  "; else out << dest << "  ";
        out << "prefix:" << prefixLength << "  ";
        out << "gw:"; if (gateway.isUnspecified()) out << "*  "; else out << gateway << "  ";
        out << "metric:" << metric << " ";
        out << "if:"; if (!interfacePtr) out << "*  "; else out << interfacePtr->getName() << "  ";
        switch (type)
        {
            case RIP_ROUTE_INTERFACE: out << "INTERFACE"; break;
            case RIP_ROUTE_STATIC: out << "STATIC"; break;
            case RIP_ROUTE_DEFAULT: out << "DEFAULT"; break;
            case RIP_ROUTE_RTE: out << "RTE"; break;
            case RIP_ROUTE_REDISTRIBUTE: out << "REDISTRIBUTE"; break;
        }
    }

    return out.str();
}

std::ostream& operator<<(std::ostream &os, const RIPInterfaceEntry &e)
{
    os << "if:" << e.ie->getName() << "  ";
    os << "metric:" << e.metric;
    return os;
}

RIPRouting::RIPRouting()
    : ift(NULL), rt(NULL), updateTimer(NULL), triggeredUpdateTimer(NULL)
{
}

RIPRouting::~RIPRouting()
{
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        delete *it;
    delete updateTimer;
    delete triggeredUpdateTimer;
}

void RIPRouting::initialize(int stage)
{
    if (stage == 0) {
        usePoisonedSplitHorizon = par("usePoisonedSplitHorizon");
        host = findContainingNode(this);
        ift = InterfaceTableAccess().get();
        rt = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTableModule")));
        updateTimer = new cMessage("RIP-timer");
        triggeredUpdateTimer = new cMessage("RIP-trigger");
        socket.setOutputGate(gate("udpOut"));

        WATCH_VECTOR(ripInterfaces);
        WATCH_PTRVECTOR(ripRoutes);
    }
    else if (stage == 3) {
        // initialize RIP interfaces
        // TODO now each multicast interface is added, with metric 1; use configuration instead
        for (int i = 0; i < ift->getNumInterfaces(); ++i)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast())
                addInterface(ie, 1 /*ie->ipv4Data()->getMetric()???*/);
        }
    }
    else if (stage == 4) { // interfaces and static routes are already initialized
        allRipRoutersGroup = rt->getRouterIdAsGeneric().getAddressType()->getLinkLocalRIPRoutersMulticastAddress();
        configureInitialRoutes();

        // configure socket
        socket.setMulticastLoop(false);
        socket.bind(RIP_UDP_PORT);
        for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
            socket.joinMulticastGroup(allRipRoutersGroup, it->ie->getInterfaceId());

        // subscribe to notifications
        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_ROUTE_DELETED);
        nb->subscribe(this, NF_ROUTE_ADDED);

        sendInitialRequests();

        // set update timer
        scheduleAt(RIP_UPDATE_INTERVAL, updateTimer);
    }
}

void RIPRouting::configureInitialRoutes()
{
    for (int i = 0; i < rt->getNumRoutes(); ++i)
    {
        IRoute *route = rt->getRoute(i);
        if (isLoopbackInterfaceRoute(route))
            /*ignore*/;
        else if (isLocalInterfaceRoute(route))
            addLocalInterfaceRoute(route);
        else if (isDefaultRoute(route))
            addDefaultRoute(route);
        else if (!route->getDestinationAsGeneric().isMulticast())
            addStaticRoute(route);
    }
}

// keep our data structures consistent with interface table and routing table
void RIPRouting::receiveChangeNotification(int category, const cObject *details)
{
//    if (simulation.getContextType()==CTX_INITIALIZE)
//        return;  // ignore notifications during initialize

    IRoute *route;
    InterfaceEntry *ie;

    switch (category)
    {
        case NF_INTERFACE_CREATED:
            // use RIP by default on multicast interfaces
            // TODO configure RIP interfaces and their metrics
            ie = const_cast<InterfaceEntry*>(check_and_cast<const InterfaceEntry*>(details));
            if (ie->isMulticast())
                addInterface(ie, 1);
            break;
        case NF_INTERFACE_DELETED:
            // delete interfaces and routes referencing the deleted interface
            ie = const_cast<InterfaceEntry*>(check_and_cast<const InterfaceEntry*>(details));
            deleteInterface(ie);
            break;
        case NF_INTERFACE_STATE_CHANGED:
            // if the interface is down, invalidate routes via that interface
            ie = const_cast<InterfaceEntry*>(check_and_cast<const InterfaceEntry*>(details));
            if (!ie->isUp())
                invalidateRoutes(ie);
            break;
        case NF_ROUTE_DELETED:
            // remove references to the deleted route and invalidate the RIP route
            route = const_cast<IRoute*>(check_and_cast<const IRoute*>(details));
            deleteRoute(route);
            break;
        case NF_ROUTE_ADDED:
            // add or update the RIP route
            route = const_cast<IRoute*>(check_and_cast<const IRoute*>(details));
            if (isLoopbackInterfaceRoute(route))
                /*ignore*/;
            else if (isLocalInterfaceRoute(route))
            {
                RIPRoute *ripRoute = findLocalInterfaceRoute(route);
                if (ripRoute)
                {
                    ripRoute->route = route;
                    ripRoute->metric = 1;
                    ripRoute->changed = true;
                    triggerUpdate();
                }
                else
                    addLocalInterfaceRoute(route);
            }
            else
            {
                // TODO import external routes from other routing daemons
            }
            break;
    }
}

RIPRoute *RIPRouting::findLocalInterfaceRoute(IRoute *route)
{
    InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(route->getSource());
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->type == RIPRoute::RIP_ROUTE_INTERFACE && (*it)->ie == ie)
            return *it;
    return NULL;
}

RIPInterfaceEntry *RIPRouting::findInterfaceEntryById(int interfaceId)
{
    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        if (it->ie->getInterfaceId() == interfaceId)
            return &(*it);
    return NULL;
}

void RIPRouting::sendInitialRequests()
{
    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
    {
        RIPPacket *packet = new RIPPacket("RIP request");
        packet->setCommand(RIP_REQUEST);
        packet->setEntryArraySize(1);
        RIPEntry &entry = packet->getEntry(0);
        entry.addressFamilyId = RIP_AF_NONE;
        entry.metric = RIP_INFINITE_METRIC;
        sendPacket(packet, allRipRoutersGroup, RIP_UDP_PORT, it->ie);
    }
}

void RIPRouting::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (msg == updateTimer)
        {
            processRegularUpdate();
            scheduleAt(simTime() + RIP_UPDATE_INTERVAL, msg);
        }
        else if (msg == triggeredUpdateTimer)
        {
            processTriggeredUpdate();
        }
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        RIPPacket *packet = check_and_cast<RIPPacket*>(msg);
        unsigned char command = packet->getCommand();
        if (command == RIP_REQUEST)
            processRequest(packet);
        else if (command == RIP_RESPONSE)
            processResponse(packet);
        else
            delete packet;
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        RIP_EV << "Ignoring UDP error report\n";
        delete msg;
    }
}

void RIPRouting::processRegularUpdate()
{
    RIP_EV << "sending updates on all interfaces\n";
    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        sendRoutes(allRipRoutersGroup, RIP_UDP_PORT, it->ie, false);

    // XXX clear route changed flags?
}

void RIPRouting::processTriggeredUpdate()
{
    RIP_EV << "sending triggered updates on all interfaces.\n";
    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        sendRoutes(allRipRoutersGroup, RIP_UDP_PORT, it->ie, true);

    // clear changed flags
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        (*it)->changed = false;
}

// RFC 2453 3.9.1
void RIPRouting::processRequest(RIPPacket *packet)
{
    int numEntries = packet->getEntryArraySize();
    if (numEntries == 0)
    {
        RIP_EV << "received empty request, ignoring.\n";
        delete packet;
        return;
    }

    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication*>(packet->removeControlInfo());
    RIP_EV << "received request from " << ctrlInfo->getSrcAddr() << "\n";

    for (int i = 0; i < numEntries; ++i)
    {
        RIPEntry &entry = packet->getEntry(i);
        switch (entry.addressFamilyId)
        {
            case RIP_AF_NONE:
                if (numEntries == 1 && entry.metric == RIP_INFINITE_METRIC)
                {
                    InterfaceEntry *ie = ift->getInterfaceById(ctrlInfo->getInterfaceId());
                    sendRoutes(ctrlInfo->getSrcAddr(), ctrlInfo->getSrcPort(), ie, false);
                    delete ctrlInfo;
                    delete packet;
                    return;
                }
                break;
            case RIP_AF_AUTH:
                // TODO ?
                break;
            case RIP_AF_INET:
                RIPRoute *ripRoute = findRoute(entry.address, entry.subnetMask);
                entry.metric = ripRoute ? ripRoute->metric : RIP_INFINITE_METRIC;
                // entry.nextHop, entry.routeTag?
                break;
        }
    }

    packet->setCommand(RIP_RESPONSE);
    packet->setName("RIP response");
    socket.sendTo(packet, ctrlInfo->getSrcAddr(), ctrlInfo->getSrcPort());

    delete ctrlInfo;
}

/**
 * Send all or changed part of the routing table to address/port on the specified interface.
 * This method is called by regular updates (every 30s), triggered updates (when some route changed),
 * and when RIP requests are processed.
 */
void RIPRouting::sendRoutes(const Address &address, int port, InterfaceEntry *ie, bool changedOnly)
{
    RIPPacket *packet = new RIPPacket("RIP response");
    packet->setCommand(RIP_RESPONSE);
    packet->setEntryArraySize(MAX_RIP_ENTRIES);
    int k = 0; // index into RIP entries

    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
    {
        RIPRoute *ripRoute = *it;
        IRoute *route = ripRoute->route;
        ASSERT(route != NULL);

        if (changedOnly && !ripRoute->changed)
            continue;

        // Split Horizon check:
        //   Omit routes learned from one neighbor in updates sent to that neighbor.
        //   In the case of a broadcast network, all routes learned from any neighbor on
        //   that network are omitted from updates sent on that network.
        // Split Horizon with Poisoned Reverse:
        //   Do include such routes in updates, but sets their metrics to infinity.
        int metric = ripRoute->metric;
        if (route->getInterface() == ie)
        {
            if (!usePoisonedSplitHorizon)
                continue;
            else
                metric = RIP_INFINITE_METRIC;
        }

        // fill next entry
        RIPEntry &entry = packet->getEntry(k++);
        entry.addressFamilyId = RIP_AF_INET;
        entry.address = route->getDestinationAsGeneric();
        entry.subnetMask = netmask(entry.address, route->getPrefixLength());
        entry.nextHop = unspecifiedAddress(allRipRoutersGroup); //route->getNextHop() if local ?
        entry.routeTag = ripRoute->tag;
        entry.metric = metric;

        // if packet is full, then send it and allocate a new one
        if (k >= MAX_RIP_ENTRIES)
        {
            sendPacket(packet, address, port, ie);
            packet = new RIPPacket("RIP response");
            packet->setCommand(RIP_RESPONSE);
            packet->setEntryArraySize(MAX_RIP_ENTRIES);
            k = 0;
        }
    }

    // send last packet if it has entries
    if (k > 0)
    {
        packet->setEntryArraySize(k);
        sendPacket(packet, address, port, ie);
    }
    else
        delete packet;
}

/**
 * 1. validate packet
 * 2. for each entry:
 *      metric = MIN(p.metric + cost of if it arrived at, infinity)
 *      if there is no route for the dest address:
 *        add new route to the routing table unless the metric is infinity
 *      else:
 *        if received from the route.gateway
 *          reinitialize timeout
 *        if (received from route.gateway AND route.metric != metric) OR metric < route.metric
 *          updateRoute(route)
 */
void RIPRouting::processResponse(RIPPacket *packet)
{
    bool isValid = isValidResponse(packet);
    if (!isValid)
    {
        RIP_EV << "dropping invalid response.\n";
        delete packet;
        return;
    }

    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication*>(packet->removeControlInfo());
    RIPInterfaceEntry *incomingIe = findInterfaceEntryById(ctrlInfo->getInterfaceId());
    if (!incomingIe)
    {
        RIP_EV << "dropping unexpected RIP response.\n";
        delete packet;
        return;
    }

    RIP_EV << "response received from " << ctrlInfo->getSrcAddr() << "\n";
    int numEntries = packet->getEntryArraySize();
    for (int i = 0; i < numEntries; ++i) {
        RIPEntry &entry = packet->getEntry(i);
        int metric = std::min((int)entry.metric + incomingIe->metric, RIP_INFINITE_METRIC);
        Address from = ctrlInfo->getSrcAddr();
        Address nextHop = entry.nextHop.isUnspecified() ? from : entry.nextHop;

        RIPRoute *ripRoute = findRoute(entry.address, entry.subnetMask);
        if (ripRoute)
        {
            if (ripRoute->from == from)
                ripRoute->expiryTime = simTime() + RIP_ROUTE_EXPIRY_TIME;
            if ((ripRoute->from == from && ripRoute->metric != metric) || metric < ripRoute->metric)
                updateRoute(ripRoute, incomingIe->ie, nextHop, metric, from);
        }
        else
        {
            if (metric != RIP_INFINITE_METRIC)
                addRoute(entry.address, entry.subnetMask, incomingIe->ie, nextHop, metric, from);
        }
    }

    delete packet;
}

bool RIPRouting::isValidResponse(RIPPacket *packet)
{
    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication*>(packet->getControlInfo());

    // check that received from RIP_UDP_PORT
    if (ctrlInfo->getSrcPort() != RIP_UDP_PORT)
    {
        RIP_EV << "source port is not " << RIP_UDP_PORT << "\n";
        return false;
    }

    // check that source is on a directly connected network
    if (!isNeighbourAddress(ctrlInfo->getSrcAddr()))
    {
        RIP_EV << "source is not directly connected " << ctrlInfo->getSrcAddr() << "\n";
        return false;
    }

    // check that it is not our response (received own multicast message)
    if (rt->isLocalAddress(ctrlInfo->getSrcAddr()))
    {
        RIP_EV << "received own response\n";
        return false;
    }

    // validate entries
    int numEntries = packet->getEntryArraySize();
    for (int i = 0; i < numEntries; ++i)
    {
        RIPEntry &entry = packet->getEntry(i);

        // check that metric is in range [1,16]
        if (entry.metric < 1 || entry.metric > RIP_INFINITE_METRIC)
        {
            RIP_EV << "received metric is not in the [1," << RIP_INFINITE_METRIC << "] range.\n";
            return false;
        }

        // check that destination address is a unicast address
        // TODO exclude 0.x.x.x, 127.x.x.x too
        if (!entry.address.isUnicast())
        {
            RIP_EV << "destination address of an entry is not unicast: " << entry.address << "\n";
            return false;
        }
    }

    return true;
}

RIPRoute *RIPRouting::findRoute(const Address &destination, const Address &subnetMask)
{
    int prefixLen = prefixLength(subnetMask);
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
    {
        IRoute *route = (*it)->route;
        if (route && route->getDestinationAsGeneric() == destination && route->getPrefixLength() == prefixLen)
            return *it;
    }
    return NULL;
}

/**
 * RFC 2453 3.9.2:
 *
 * Adding a route to the routing table consists of:
 *
 * - Setting the destination address to the destination address in the RTE
 * - Setting the metric to the newly calculated metric
 * - Set the next hop address to be the address of the router from which
 *   the datagram came
 * - Initialize the timeout for the route.  If the garbage-collection
 *   timer is running for this route, stop it
 * - Set the route change flag
 * - Signal the output process to trigger an update
 */
void RIPRouting::addRoute(const Address &dest, const Address &subnetMask, InterfaceEntry *ie, const Address &nextHop, int metric, const Address &from)
{
    IRoute *route = rt->createRoute();
    route->setSource(this);
    route->setDestination(dest);
    route->setPrefixLength(prefixLength(subnetMask));
    route->setInterface(ie);
    route->setNextHop(nextHop);
    route->setMetric(metric);
    RIPRoute *ripRoute = new RIPRoute(route, RIPRoute::RIP_ROUTE_RTE/*XXX*/, metric);
    ripRoute->from = from;
    ripRoute->expiryTime = simTime() + RIP_ROUTE_EXPIRY_TIME;
    ripRoute->purgeTime = 0;
    ripRoute->changed = true;
    route->setProtocolData(ripRoute);
    rt->addRoute(route);
    ripRoutes.push_back(ripRoute);
    triggerUpdate();
}

/**
 * RFC 2453 3.9.2:
 *
 * Do the following actions:
 *
 *  - Adopt the route from the datagram (i.e., put the new metric in and
 *    adjust the next hop address, if necessary).
 *  - Set the route change flag and signal the output process to trigger
 *    an update
 *  - If the new metric is infinity, start the deletion process
 *    (described above); otherwise, re-initialize the timeout
 */
void RIPRouting::updateRoute(RIPRoute *ripRoute, InterfaceEntry *ie, const Address &nextHop, int metric, const Address &from)
{
    if (ripRoute->route)
    {
        ripRoute->route->setInterface(ie);
        ripRoute->route->setNextHop(nextHop);
        ripRoute->route->setMetric(metric);
    }
    else
    {
        // TODO add new route
    }

    ripRoute->type = RIPRoute::RIP_ROUTE_RTE; // XXX?
    ripRoute->from = from;
    ripRoute->metric = metric;
    ripRoute->changed = true;

    triggerUpdate();

    if (metric == RIP_INFINITE_METRIC)
        invalidateRoute(ripRoute);
    else {
        ripRoute->expiryTime = simTime() + RIP_ROUTE_EXPIRY_TIME;
        ripRoute->purgeTime = 0;
    }
}

void RIPRouting::triggerUpdate()
{
    if (!triggeredUpdateTimer->isScheduled())
    {
        double delay = uniform(RIP_TRIGGERED_UPDATE_DELAY_MIN, RIP_TRIGGERED_UPDATE_DELAY_MAX);
        scheduleAt(simTime() + delay, triggeredUpdateTimer);
    }
}

RIPRoute *RIPRouting::checkRoute(RIPRoute *route)
{
    simtime_t now = simTime();
    if (route->purgeTime > 0 && now > route->purgeTime)
    {
        purgeRoute(route);
        return NULL;
    }
    if (now > route->expiryTime)
    {
        invalidateRoute(route);
        return NULL;
    }
    return route;
}

/*
 * Called when the timeout expires, or a metric is set to 16 because an update received from the current router.
 * It will
 * - set purgeTime to expiryTime + 120s
 * - set metric of the route to 16 (infinity)
 * - set routeChangeFlag
 * - signal the output process to trigger a response
 */
void RIPRouting::invalidateRoute(RIPRoute *ripRoute)
{
    ripRoute->route->setMetric(RIP_INFINITE_METRIC);
    ripRoute->route->setEnabled(false);
    ripRoute->metric = RIP_INFINITE_METRIC;
    ripRoute->purgeTime = ripRoute->expiryTime + RIP_ROUTE_PURGE_TIME;
    ripRoute->changed = true;
    triggerUpdate();
}

void RIPRouting::purgeRoute(RIPRoute *ripRoute)
{
    ripRoute->route->setProtocolData(NULL);
    // XXX should set isExpired() to true, and let rt->purge() to do the work
    rt->deleteRoute(ripRoute->route);

    RouteVector::iterator end = std::remove(ripRoutes.begin(), ripRoutes.end(), ripRoute);
    if (end != ripRoutes.end())
        ripRoutes.erase(end, ripRoutes.end());
    delete ripRoute;
}

void RIPRouting::sendPacket(RIPPacket *packet, const Address &address, int port, InterfaceEntry *ie)
{
    packet->setByteLength(4 + 20 * packet->getEntryArraySize()); // XXX compute from address lengths
// XXX it seems that setMulticastOutputInterface() has no effect
//    if (address.isMulticast())
//        socket.setMulticastOutputInterface(ie->getInterfaceId());
//    socket.sendTo(packet, address, port);
    if (address.isMulticast())
        socket.sendTo(packet, address, port, ie->getInterfaceId());
    else
        socket.sendTo(packet, address, port);
}

/*----------------------------------------
 *
 *----------------------------------------*/
void RIPRouting::addInterface(InterfaceEntry *ie, int metric)
{
    ripInterfaces.push_back(RIPInterfaceEntry(ie, metric));
}

void RIPRouting::deleteInterface(InterfaceEntry *ie)
{
    // delete interfaces and routes referencing ie
    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); )
    {
        if (it->ie == ie)
            it = ripInterfaces.erase(it);
        else
            it++;
    }
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); )
    {
        if ((*it)->ie == ie)
            it = ripRoutes.erase(it);
        else
            it++;
    }
}

void RIPRouting::invalidateRoutes(InterfaceEntry *ie)
{
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->route && (*it)->route->getInterface() == ie)
            invalidateRoute(*it);
}

void RIPRouting::deleteRoute(IRoute *route)
{
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->route == route)
            (*it)->route = NULL;
}

bool RIPRouting::isLoopbackInterfaceRoute(IRoute *route)
{
    InterfaceEntry *ie = dynamic_cast<InterfaceEntry*>(route->getSource());
    return ie && ie->isLoopback();
}

bool RIPRouting::isLocalInterfaceRoute(IRoute *route)
{
    InterfaceEntry *ie = dynamic_cast<InterfaceEntry*>(route->getSource());
    return ie && !ie->isLoopback();
}

bool RIPRouting::isDefaultRoute(IRoute *route)
{
    return route->getPrefixLength() == 0;
}

void RIPRouting::addLocalInterfaceRoute(IRoute *route)
{
    InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(route->getSource());
    RIPInterfaceEntry *ripIe = findInterfaceEntryById(ie->getInterfaceId());
    RIPRoute *ripRoute = new RIPRoute(route, RIPRoute::RIP_ROUTE_INTERFACE, ripIe ? ripIe->metric : 1);
    ripRoute->ie = ie;
    ripRoutes.push_back(ripRoute);
}

void RIPRouting::addDefaultRoute(IRoute *route)
{
    ripRoutes.push_back(new RIPRoute(route, RIPRoute::RIP_ROUTE_DEFAULT, 1));
}

void RIPRouting::addStaticRoute(IRoute *route)
{
    ripRoutes.push_back(new RIPRoute(route, RIPRoute::RIP_ROUTE_STATIC, 1));
}

std::string RIPRouting::getHostName() {
    return host->getFullName();
}
