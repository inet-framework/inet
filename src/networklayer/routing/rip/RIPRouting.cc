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
#include <functional>

#include "IAddressType.h"
#include "InterfaceTableAccess.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "UDP.h"
#include "InterfaceMatcher.h"

#include "RIPPacket_m.h"
#include "RIPRouting.h"

Define_Module(RIPRouting);

#define RIP_EV EV << "RIP at " << getHostName() << " "

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
        out << "upd:" << lastUpdateTime << "s  ";
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

RIPInterfaceEntry::RIPInterfaceEntry(const InterfaceEntry *ie)
    : ie(ie), metric(1), splitHorizonMode(SPLIT_HORIZON_POISONED_REVERSE)
{
    ASSERT(!ie->isLoopback());
    ASSERT(ie->isMulticast());
}

/**
 * Fills in the parameters of the interface from the matching <interface>
 * element of the configuration.
 */
void RIPInterfaceEntry::configure(cXMLElement *config)
{
    const char *metricAttr = config->getAttribute("metric");

    if (metricAttr)
    {
        int metric = atoi(metricAttr);
        if (metric == 0)
            throw cRuntimeError("RIP: invalid metric in <interface> element at %s: %s", config->getSourceLocation(), metricAttr);
        this->metric = metric;
    }

    const char *splitHorizonModeAttr = config->getAttribute("split-horizon-mode");
    SplitHorizonMode mode = !splitHorizonModeAttr ? SPLIT_HORIZON_POISONED_REVERSE :
                            strcmp(splitHorizonModeAttr, "NoSplitHorizon") == 0 ? NO_SPLIT_HORIZON :
                            strcmp(splitHorizonModeAttr, "SplitHorizon") == 0 ? SPLIT_HORIZON :
                            strcmp(splitHorizonModeAttr, "SplitHorizonPoisonedReverse") == 0 ? SPLIT_HORIZON_POISONED_REVERSE :
                            (SplitHorizonMode)-1;
    if (mode == -1)
        throw cRuntimeError("RIP: invalid split-horizon-mode attribute in <interface> element at %s: %s",
                config->getSourceLocation(), splitHorizonModeAttr);
    this->splitHorizonMode = mode;
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
    cancelAndDelete(updateTimer);
    cancelAndDelete(triggeredUpdateTimer);
}

simsignal_t RIPRouting::sentRequestSignal = SIMSIGNAL_NULL;
simsignal_t RIPRouting::sentUpdateSignal = SIMSIGNAL_NULL;
simsignal_t RIPRouting::rcvdResponseSignal = SIMSIGNAL_NULL;
simsignal_t RIPRouting::badResponseSignal = SIMSIGNAL_NULL;
simsignal_t RIPRouting::numRoutesSignal = SIMSIGNAL_NULL;

void RIPRouting::initialize(int stage)
{
    if (stage == 0) {
        host = findContainingNode(this);
        ift = InterfaceTableAccess().get();
        rt = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTableModule")));

        ripUdpPort = par("udpPort");
        updateInterval = par("updateInterval").doubleValue();
        routeExpiryTime = par("routeExpiryTime").doubleValue();
        routePurgeTime = par("routePurgeTime").doubleValue();

        updateTimer = new cMessage("RIP-timer");
        triggeredUpdateTimer = new cMessage("RIP-trigger");
        socket.setOutputGate(gate("udpOut"));

        WATCH_VECTOR(ripInterfaces);
        WATCH_PTRVECTOR(ripRoutes);

        sentRequestSignal = registerSignal("sentRequest");
        sentUpdateSignal = registerSignal("sentUpdate");
        rcvdResponseSignal = registerSignal("rcvdResponse");
        badResponseSignal = registerSignal("badResponse");
        numRoutesSignal = registerSignal("numRoutes");
    }
    else if (stage == 3) {
        configureInterfaces(par("ripConfig").xmlValue());
    }
    else if (stage == 4) { // interfaces and static routes are already initialized
        addressType = rt->getRouterIdAsGeneric().getAddressType();
        configureInitialRoutes();

        // configure socket
        socket.setMulticastLoop(false);
        socket.bind(ripUdpPort);
        for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
            socket.joinMulticastGroup(addressType->getLinkLocalRIPRoutersMulticastAddress(), it->ie->getInterfaceId());

        // subscribe to notifications
        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_ROUTE_DELETED);
        nb->subscribe(this, NF_ROUTE_ADDED);

        sendInitialRequests();

        // set update timer
        scheduleAt(updateInterval, updateTimer);
    }
}

/**
 * Creates a RIPInterfaceEntry for each interface found in the interface table.
 */
void RIPRouting::configureInterfaces(cXMLElement *config)
{
    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);

    for (int i = 0; i < ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->isMulticast() && !ie->isLoopback())
        {
            int i = matcher.findMatchingSelector(ie);
            if (i >= 0)
                addInterface(ie, interfaceElements[i]);
        }
    }
}

/**
 * Import interface/static/default routes from the routing table.
 */
void RIPRouting::configureInitialRoutes()
{
    for (int i = 0; i < rt->getNumRoutes(); ++i)
    {
        IRoute *route = rt->getRoute(i);
        if (isLoopbackInterfaceRoute(route))
            /*ignore*/;
        else if (isLocalInterfaceRoute(route))
            importRoute(route, RIPRoute::RIP_ROUTE_INTERFACE);
        else if (isDefaultRoute(route))
            importRoute(route, RIPRoute::RIP_ROUTE_DEFAULT);
        else if (!route->getDestinationAsGeneric().isMulticast())
            importRoute(route, RIPRoute::RIP_ROUTE_STATIC);
    }
}

RIPRoute* RIPRouting::importRoute(IRoute *route, RIPRoute::RouteType type, int metric)
{
    RIPRoute *ripRoute = new RIPRoute(route, type, metric);
    if (type == RIPRoute::RIP_ROUTE_INTERFACE)
    {
        ripRoute->ie = check_and_cast<InterfaceEntry*>(route->getSource());
        RIPInterfaceEntry *ripIe = findInterfaceById(ripRoute->ie->getInterfaceId());
        if (ripIe)
            ripRoute->metric = ripIe->metric;
    }

    ripRoutes.push_back(ripRoute);
    emit(numRoutesSignal, ripRoutes.size());
    return ripRoute;
}

/**
 * Requests the whole routing table from all neighboring RIP routers.
 */
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
        emit(sentRequestSignal, packet);
        sendPacket(packet, addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, it->ie);
    }
}

/**
 * Listen on interface/route changes and update private data structures.
 */
void RIPRouting::receiveChangeNotification(int category, const cObject *details)
{
    IRoute *route;
    const InterfaceEntry *ie;

    switch (category)
    {
        case NF_INTERFACE_CREATED:
            // use RIP by default on multicast interfaces
            // TODO configure RIP interfaces and their metrics
            ie = check_and_cast<const InterfaceEntry*>(details);
            if (ie->isMulticast() && !ie->isLoopback())
            {
                cXMLElementList config = par("ripConfig").xmlValue()->getChildrenByTagName("interface");
                int i = InterfaceMatcher(config).findMatchingSelector(ie);
                if (i >= 0)
                    addInterface(ie, config[i]);
            }
            break;
        case NF_INTERFACE_DELETED:
            // delete interfaces and routes referencing the deleted interface
            ie = check_and_cast<const InterfaceEntry*>(details);
            deleteInterface(ie);
            break;
        case NF_INTERFACE_STATE_CHANGED:
            // if the interface is down, invalidate routes via that interface
            ie = const_cast<InterfaceEntry*>(check_and_cast<const InterfaceEntry*>(details));
            if (!ie->isUp())
            {
                invalidateRoutes(ie);
            }
            break;
        case NF_ROUTE_DELETED:
            // remove references to the deleted route and invalidate the RIP route
            route = const_cast<IRoute*>(check_and_cast<const IRoute*>(details));
            if (route->getSource() != this)
            {
                for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
                    if ((*it)->route == route)
                        (*it)->route = NULL;
            }
            break;
        case NF_ROUTE_ADDED:
            // add or update the RIP route
            route = const_cast<IRoute*>(check_and_cast<const IRoute*>(details));
            if (route->getSource() != this)
            {
                if (isLoopbackInterfaceRoute(route))
                    /*ignore*/;
                else if (isLocalInterfaceRoute(route))
                {
                    InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(route->getSource());
                    RIPRoute *ripRoute = findRoute(ie, RIPRoute::RIP_ROUTE_INTERFACE);
                    if (ripRoute)
                    {
                        ripRoute->route = route;
                        ripRoute->metric = 1;
                        ripRoute->changed = true;
                        triggerUpdate();
                    }
                    else
                        importRoute(route, RIPRoute::RIP_ROUTE_INTERFACE);
                }
                else
                {
                    // TODO import external routes from other routing daemons
                }
            }
            break;
    }
}

void RIPRouting::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (msg == updateTimer)
        {
            processUpdate(false);
            scheduleAt(simTime() + updateInterval, msg);
        }
        else if (msg == triggeredUpdateTimer)
        {
            processUpdate(true);
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

void RIPRouting::processUpdate(bool triggered)
{
    if (triggered)
        RIP_EV << "sending triggered updates on all interfaces.\n";
    else
        RIP_EV << "sending regular updates on all interfaces\n";

    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        sendRoutes(addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, *it, triggered);

    // clear changed flags
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        (*it)->changed = false;
}

/**
 * Processes a request received from a RIP router or a monitoring process.
 * The request processing follows the guidelines described in RFC 2453 3.9.1.
 *
 * There are two cases:
 * - the request enumerates the requested prefixes
 *     There is an RIPEntry for each requested route in the packet.
 *     The RIP module simply looks up the prefix in its table, and
 *     if it sets the metric field of the entry to the metric of the
 *     found route, or to infinity (16) if not found. Once all entries
 *     are have been filled in, change the command from Request to Response,
 *     and sent the packet back to the requestor. If there are no
 *     entries in the request, then no response is sent; the request is
 *     silently discarded.
 * - the whole routing table is requested
 *     In this case the RIPPacket contains only one entry, with addressFamilyId 0,
 *     and metric 16 (infinity). In this case the whole routing table is sent,
 *     using the normal output process (sendRoutes() method).
 */
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
    Address srcAddr = ctrlInfo->getSrcAddr();
    int srcPort = ctrlInfo->getSrcPort();
    int interfaceId = ctrlInfo->getInterfaceId();
    delete ctrlInfo;

    RIP_EV << "received request from " << srcAddr << "\n";

    RIPRoute *ripRoute;
    for (int i = 0; i < numEntries; ++i)
    {
        RIPEntry &entry = packet->getEntry(i);
        switch (entry.addressFamilyId)
        {
            case RIP_AF_NONE:
                if (numEntries == 1 && entry.metric == RIP_INFINITE_METRIC)
                {
                    RIPInterfaceEntry *ripInterface = findInterfaceById(interfaceId);
                    if (ripInterface)
                        sendRoutes(srcAddr, srcPort, *ripInterface, false);
                    delete packet;
                    return;
                }
                else
                {
                    delete packet;
                    throw cRuntimeError("RIP: invalid request.");
                }
                break;
            case RIP_AF_INET:
                ripRoute = findRoute(entry.address, entry.prefixLength);
                entry.metric = ripRoute ? ripRoute->metric : RIP_INFINITE_METRIC;
                // entry.nextHop, entry.routeTag?
                break;
            default:
                delete packet;
                throw cRuntimeError("RIP: request has invalid addressFamilyId: %d.", (int)entry.addressFamilyId);
        }
    }

    packet->setCommand(RIP_RESPONSE);
    packet->setName("RIP response");
    socket.sendTo(packet, srcAddr, srcPort);
}

/**
 * Send all or changed part of the routing table to address/port on the specified interface.
 * This method is called by regular updates (every 30s), triggered updates (when some route changed),
 * and when RIP requests are processed.
 */
void RIPRouting::sendRoutes(const Address &address, int port, const RIPInterfaceEntry &ripInterface, bool changedOnly)
{
    RIPPacket *packet = new RIPPacket("RIP response");
    packet->setCommand(RIP_RESPONSE);
    packet->setEntryArraySize(MAX_RIP_ENTRIES);
    int k = 0; // index into RIP entries

    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
    {
        RIPRoute *ripRoute = checkRouteIsExpired(*it);
        if (!ripRoute)
            continue;

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
        if (route->getInterface() == ripInterface.ie)
        {
            if (ripInterface.splitHorizonMode == SPLIT_HORIZON)
                continue;
            else if (ripInterface.splitHorizonMode == SPLIT_HORIZON_POISONED_REVERSE)
                metric = RIP_INFINITE_METRIC;
        }

        // fill next entry
        RIPEntry &entry = packet->getEntry(k++);
        entry.addressFamilyId = RIP_AF_INET;
        entry.address = route->getDestinationAsGeneric();
        entry.prefixLength = route->getPrefixLength();
        entry.nextHop = addressType->getUnspecifiedAddress(); //route->getNextHop() if local ?
        entry.routeTag = ripRoute->tag;
        entry.metric = metric;

        // if packet is full, then send it and allocate a new one
        if (k >= MAX_RIP_ENTRIES)
        {
            emit(sentUpdateSignal, packet);
            sendPacket(packet, address, port, ripInterface.ie);
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
        emit(sentUpdateSignal, packet);
        sendPacket(packet, address, port, ripInterface.ie);
    }
    else
        delete packet;
}

/**
 * Processes the RIP response and updates the routing table.
 *
 * First it validates the packet to avoid corrupting the routing
 * table with a wrong packet. Valid responses must come from a neighboring
 * RIP router.
 *
 * Next each RIPEntry is processed one by one. Check that destination address
 * and metric are valid. Then compute the new metric by adding the metric
 * of the interface to the metric found in the entry.
 *
 *   If there is no route to the destination, and the new metric is not infinity,
 *   then add a new route to the routing table.
 *
 *   If there is an existing route to the destination,
 *
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
    emit(rcvdResponseSignal, packet);

    bool isValid = isValidResponse(packet);
    if (!isValid)
    {
        RIP_EV << "dropping invalid response.\n";
        emit(badResponseSignal, packet);
        delete packet;
        return;
    }

    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication*>(packet->removeControlInfo());
    Address srcAddr = ctrlInfo->getSrcAddr();
    int interfaceId = ctrlInfo->getInterfaceId();
    delete ctrlInfo;

    RIPInterfaceEntry *incomingIe = findInterfaceById(interfaceId);
    if (!incomingIe)
    {
        RIP_EV << "dropping unexpected RIP response.\n";
        emit(badResponseSignal, packet);
        delete packet;
        return;
    }

    RIP_EV << "response received from " << srcAddr << "\n";
    int numEntries = packet->getEntryArraySize();
    for (int i = 0; i < numEntries; ++i) {
        RIPEntry &entry = packet->getEntry(i);
        int metric = std::min((int)entry.metric + incomingIe->metric, RIP_INFINITE_METRIC);
        Address nextHop = entry.nextHop.isUnspecified() ? srcAddr : entry.nextHop;

        RIPRoute *ripRoute = findRoute(entry.address, entry.prefixLength, RIPRoute::RIP_ROUTE_RTE);
        if (ripRoute)
        {
            if (ripRoute->from == srcAddr)
                ripRoute->lastUpdateTime = simTime();
            if ((ripRoute->from == srcAddr && ripRoute->metric != metric) || metric < ripRoute->metric)
                updateRoute(ripRoute, incomingIe->ie, nextHop, metric, srcAddr);
        }
        else
        {
            if (metric != RIP_INFINITE_METRIC)
                addRoute(entry.address, entry.prefixLength, incomingIe->ie, nextHop, metric, srcAddr);
        }
    }

    delete packet;
}

bool RIPRouting::isValidResponse(RIPPacket *packet)
{
    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication*>(packet->getControlInfo());

    // check that received from ripUdpPort
    if (ctrlInfo->getSrcPort() != ripUdpPort)
    {
        RIP_EV << "source port is not " << ripUdpPort << "\n";
        return false;
    }

    // check that it is not our response (received own multicast message)
    if (rt->isLocalAddress(ctrlInfo->getSrcAddr()))
    {
        RIP_EV << "received own response\n";
        return false;
    }

    // check that source is on a directly connected network
    if (!ift->isNeighborAddress(ctrlInfo->getSrcAddr()))
    {
        RIP_EV << "source is not directly connected " << ctrlInfo->getSrcAddr() << "\n";
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
void RIPRouting::addRoute(const Address &dest, int prefixLength, const InterfaceEntry *ie, const Address &nextHop, int metric, const Address &from)
{
    IRoute *route = rt->createRoute();
    route->setSource(this);
    route->setDestination(dest);
    route->setPrefixLength(prefixLength);
    route->setInterface(const_cast<InterfaceEntry*>(ie));
    route->setNextHop(nextHop);
    route->setMetric(metric);
    RIPRoute *ripRoute = new RIPRoute(route, RIPRoute::RIP_ROUTE_RTE, metric);
    ripRoute->from = from;
    ripRoute->lastUpdateTime = simTime();
    ripRoute->changed = true;
    route->setProtocolData(ripRoute);
    rt->addRoute(route);
    ripRoutes.push_back(ripRoute);
    emit(numRoutesSignal, ripRoutes.size());
    triggerUpdate();
}

/**
 * Updates an existing route with the information learned from a RIP packet.
 * If the metric is infinite (16), then the route is invalidated.
 * It triggers an update, so neighbor routers are notified about the change.
 *
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
void RIPRouting::updateRoute(RIPRoute *ripRoute, const InterfaceEntry *ie, const Address &nextHop, int metric, const Address &from)
{
    ASSERT(ripRoute && ripRoute->type == RIPRoute::RIP_ROUTE_RTE);
    ASSERT(ripRoute->route);

    ripRoute->route->setInterface(const_cast<InterfaceEntry*>(ie));
    ripRoute->route->setNextHop(nextHop);
    ripRoute->route->setMetric(metric);
    if (metric == RIP_INFINITE_METRIC)
        ripRoute->route->setEnabled(false);
    ripRoute->type = RIPRoute::RIP_ROUTE_RTE;
    ripRoute->from = from;
    ripRoute->metric = metric;
    ripRoute->changed = true;

    triggerUpdate();

    if (metric == RIP_INFINITE_METRIC)
        invalidateRoute(ripRoute);
    else
        ripRoute->lastUpdateTime = simTime();
}

/**
 * Sets the update timer to trigger an update in the [1s,5s] interval.
 * If the update is already scheduled, it does nothing.
 */
void RIPRouting::triggerUpdate()
{
    if (!triggeredUpdateTimer->isScheduled())
    {
        double delay = par("triggeredUpdateDelay");
        scheduleAt(simTime() + delay, triggeredUpdateTimer);
    }
}

/**
 * Should be called regularly to handle expiry and purge of routes.
 * Returns the route if it is valid.
 */
RIPRoute *RIPRouting::checkRouteIsExpired(RIPRoute *route)
{
    if (route->type == RIPRoute::RIP_ROUTE_RTE)
    {
        simtime_t now = simTime();
        if (now >= route->lastUpdateTime + routeExpiryTime + routePurgeTime)
        {
            purgeRoute(route);
            return NULL;
        }
        if (now >= route->lastUpdateTime + routeExpiryTime)
        {
            invalidateRoute(route);
            return NULL;
        }
    }
    return route;
}

/*
 * Invalidates the route, i.e. marks it invalid, but keeps it in the routing table for 120s,
 * so the neighbors are notified about the broken route in the next update.
 *
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
    ripRoute->changed = true;
    triggerUpdate();
}

/**
 * Removes the route from the routing table.
 */
void RIPRouting::purgeRoute(RIPRoute *ripRoute)
{
    ASSERT(ripRoute->type == RIPRoute::RIP_ROUTE_RTE);

    IRoute *route = ripRoute->route;
    if (route)
    {
        ripRoute->route = NULL;
        route->setProtocolData(NULL);
        rt->deleteRoute(route);
    }

    RouteVector::iterator end = std::remove(ripRoutes.begin(), ripRoutes.end(), ripRoute);
    if (end != ripRoutes.end())
        ripRoutes.erase(end, ripRoutes.end());
    delete ripRoute;

    emit(numRoutesSignal, ripRoutes.size());
}

/**
 * Sends the packet to the specified UDP dest/port.
 * If the dest is a multicast address, then the outgoing interface must be specified.
 */
void RIPRouting::sendPacket(RIPPacket *packet, const Address &destAddr, int destPort, const InterfaceEntry *destInterface)
{
    packet->setByteLength(4 + 20 * packet->getEntryArraySize()); // XXX compute from address lengths
// XXX it seems that setMulticastOutputInterface() has no effect
//    if (destAddr.isMulticast())
//        socket.setMulticastOutputInterface(destInterface->getInterfaceId());
//    socket.sendTo(packet, destAddress, destPort);
    if (destAddr.isMulticast())
        socket.sendTo(packet, destAddr, destPort, destInterface->getInterfaceId());
    else
        socket.sendTo(packet, destAddr, destPort);
}

/*----------------------------------------
 *      private methods
 *----------------------------------------*/

RIPInterfaceEntry *RIPRouting::findInterfaceById(int interfaceId)
{
    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        if (it->ie->getInterfaceId() == interfaceId)
            return &(*it);
    return NULL;
}


RIPRoute *RIPRouting::findRoute(const Address &destination, int prefixLength)
{
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
    {
        IRoute *route = (*it)->route;
        if (route && route->getDestinationAsGeneric() == destination && route->getPrefixLength() == prefixLength)
            return *it;
    }
    return NULL;
}

RIPRoute *RIPRouting::findRoute(const Address &destination, int prefixLength, RIPRoute::RouteType type)
{
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
    {
        IRoute *route = (*it)->route;
        if (route && route->getDestinationAsGeneric() == destination && route->getPrefixLength() == prefixLength
                && (*it)->type == type)
            return *it;
    }
    return NULL;
}

RIPRoute *RIPRouting::findRoute(const InterfaceEntry *ie, RIPRoute::RouteType type)
{
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->type == type && (*it)->ie == ie)
            return *it;
    return NULL;
}

void RIPRouting::addInterface(const InterfaceEntry *ie, cXMLElement *config)
{
    RIPInterfaceEntry ripInterface(ie);
    ripInterface.configure(config);
    ripInterfaces.push_back(ripInterface);
}

void RIPRouting::deleteInterface(const InterfaceEntry *ie)
{
    // delete interfaces and routes referencing ie
    for (InterfaceVector::iterator it = ripInterfaces.begin(); it != ripInterfaces.end(); )
    {
        if (it->ie == ie)
            it = ripInterfaces.erase(it);
        else
            it++;
    }
    bool emitNumRoutesSignal = false;
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); )
    {
        if ((*it)->ie == ie)
        {
            it = ripRoutes.erase(it);
            emitNumRoutesSignal = true;
        }
        else
            it++;
    }
    if (emitNumRoutesSignal)
        emit(numRoutesSignal, ripRoutes.size());
}

void RIPRouting::invalidateRoutes(const InterfaceEntry *ie)
{
    for (RouteVector::iterator it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->route && (*it)->route->getInterface() == ie)
            invalidateRoute(*it);
}

bool RIPRouting::isLoopbackInterfaceRoute(const IRoute *route)
{
    InterfaceEntry *ie = dynamic_cast<InterfaceEntry*>(route->getSource());
    return ie && ie->isLoopback();
}

bool RIPRouting::isLocalInterfaceRoute(const IRoute *route)
{
    InterfaceEntry *ie = dynamic_cast<InterfaceEntry*>(route->getSource());
    return ie && !ie->isLoopback();
}
