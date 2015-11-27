//
// Copyright (C) 2013 Opensim Ltd.
// Author: Tamas Borbely
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

#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/common/InterfaceMatcher.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/transportlayer/udp/UDP.h"
#include "inet/common/ModuleAccess.h"

#include "inet/routing/rip/RIPPacket_m.h"
#include "inet/routing/rip/RIPRouting.h"

namespace inet {

Define_Module(RIPRouting);

std::ostream& operator<<(std::ostream& os, const RIPRoute& e)
{
    os << e.info();
    return os;
}

RIPRoute::RIPRoute(IRoute *route, RouteType type, int metric, uint16 routeTag)
    : type(type), route(route), metric(metric), changed(false), lastUpdateTime(0)
{
    dest = route->getDestinationAsGeneric();
    prefixLength = route->getPrefixLength();
    nextHop = route->getNextHopAsGeneric();
    ie = route->getInterface();
    tag = routeTag;
}

std::string RIPRoute::info() const
{
    std::stringstream out;

    out << "dest:";
    if (dest.isUnspecified())
        out << "*  ";
    else
        out << dest << "  ";
    out << "prefix:" << prefixLength << "  ";
    out << "gw:";
    if (nextHop.isUnspecified())
        out << "*  ";
    else
        out << nextHop << "  ";
    out << "metric:" << metric << " ";
    out << "if:";
    if (!ie)
        out << "*  ";
    else
        out << ie->getName() << "  ";
    out << "tag:" << tag << " ";
    out << "upd:" << lastUpdateTime << "s  ";
    switch (type) {
        case RIP_ROUTE_INTERFACE:
            out << "INTERFACE";
            break;

        case RIP_ROUTE_STATIC:
            out << "STATIC";
            break;

        case RIP_ROUTE_DEFAULT:
            out << "DEFAULT";
            break;

        case RIP_ROUTE_RTE:
            out << "RTE";
            break;

        case RIP_ROUTE_REDISTRIBUTE:
            out << "REDISTRIBUTE";
            break;
    }

    return out.str();
}

RIPInterfaceEntry::RIPInterfaceEntry(const InterfaceEntry *ie)
    : ie(ie), metric(1), mode(NO_RIP)
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

    if (metricAttr) {
        int metric = atoi(metricAttr);
        if (metric < 1 || metric >= RIP_INFINITE_METRIC)
            throw cRuntimeError("RIP: invalid metric in <interface> element at %s: %s", config->getSourceLocation(), metricAttr);
        this->metric = metric;
    }

    const char *ripModeAttr = config->getAttribute("mode");
    RIPMode mode = !ripModeAttr ? SPLIT_HORIZON_POISONED_REVERSE :
        strcmp(ripModeAttr, "NoRIP") == 0 ? NO_RIP :
        strcmp(ripModeAttr, "NoSplitHorizon") == 0 ? NO_SPLIT_HORIZON :
        strcmp(ripModeAttr, "SplitHorizon") == 0 ? SPLIT_HORIZON :
        strcmp(ripModeAttr, "SplitHorizonPoisonedReverse") == 0 ? SPLIT_HORIZON_POISONED_REVERSE :
                static_cast<RIPMode>(-1);
    if (mode == static_cast<RIPMode>(-1))
        throw cRuntimeError("RIP: invalid split-horizon-mode attribute in <interface> element at %s: %s",
                config->getSourceLocation(), ripModeAttr);
    this->mode = mode;
}

std::ostream& operator<<(std::ostream& os, const RIPInterfaceEntry& e)
{
    os << "if:" << e.ie->getName() << "  ";
    os << "metric:" << e.metric << "  ";
    os << "mode: ";
    switch (e.mode) {
        case NO_RIP:
            os << "NoRIP";
            break;

        case NO_SPLIT_HORIZON:
            os << "NoSplitHorizon";
            break;

        case SPLIT_HORIZON:
            os << "SplitHorizon";
            break;

        case SPLIT_HORIZON_POISONED_REVERSE:
            os << "SplitHorizonPoisenedReverse";
            break;

        default:
            os << "<unknown>";
            break;
    }
    return os;
}

RIPRouting::RIPRouting()
{
}

RIPRouting::~RIPRouting()
{
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        delete *it;
    cancelAndDelete(updateTimer);
    cancelAndDelete(triggeredUpdateTimer);
    cancelAndDelete(startupTimer);
    cancelAndDelete(shutdownTimer);
}

simsignal_t RIPRouting::sentRequestSignal = registerSignal("sentRequest");
simsignal_t RIPRouting::sentUpdateSignal = registerSignal("sentUpdate");
simsignal_t RIPRouting::rcvdResponseSignal = registerSignal("rcvdResponse");
simsignal_t RIPRouting::badResponseSignal = registerSignal("badResponse");
simsignal_t RIPRouting::numRoutesSignal = registerSignal("numRoutes");

void RIPRouting::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        socket.setOutputGate(gate("udpOut"));

        const char *m = par("mode");
        if (!m)
            throw cRuntimeError("Missing 'mode' parameter.");
        else if (!strcmp(m, "RIPv2"))
            mode = RIPv2;
        else if (!strcmp(m, "RIPng"))
            mode = RIPng;
        else
            throw cRuntimeError("Unrecognized 'mode' parameter: %s", m);

        ripUdpPort = par("udpPort");
        updateInterval = par("updateInterval").doubleValue();
        routeExpiryTime = par("routeExpiryTime").doubleValue();
        routePurgeTime = par("routePurgeTime").doubleValue();
        shutdownTime = par("shutdownTime").doubleValue();

        updateTimer = new cMessage("RIP-timer");
        triggeredUpdateTimer = new cMessage("RIP-trigger");
        startupTimer = new cMessage("RIP-startup");
        shutdownTimer = new cMessage("RIP-shutdown");

        WATCH_VECTOR(ripInterfaces);
        WATCH_PTRVECTOR(ripRoutes);
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {    // interfaces and static routes are already initialized
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
        if (isOperational)
            scheduleAt(simTime() + par("startupTime").doubleValue(), startupTimer);
    }
}

/**
 * Creates a RIPInterfaceEntry for each interface found in the interface table.
 */
void RIPRouting::configureInterfaces(cXMLElement *config)
{
    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);

    for (int k = 0; k < ift->getNumInterfaces(); ++k) {
        InterfaceEntry *ie = ift->getInterface(k);
        if (ie->isMulticast() && !ie->isLoopback()) {
            int i = matcher.findMatchingSelector(ie);
            addInterface(ie, i >= 0 ? interfaceElements[i] : nullptr);
        }
    }
}

/**
 * Import interface/static/default routes from the routing table.
 */
void RIPRouting::configureInitialRoutes()
{
    for (int i = 0; i < rt->getNumRoutes(); ++i) {
        IRoute *route = rt->getRoute(i);
        if (isLoopbackInterfaceRoute(route)) {
            /*ignore*/
            ;
        }
        else if (isLocalInterfaceRoute(route)) {
            InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(route->getSource());
            importRoute(route, RIPRoute::RIP_ROUTE_INTERFACE, getInterfaceMetric(ie));
        }
        else if (isDefaultRoute(route))
            importRoute(route, RIPRoute::RIP_ROUTE_DEFAULT);
        else {
            const L3Address& destAddr = route->getDestinationAsGeneric();
            if (!destAddr.isMulticast() && !destAddr.isLinkLocal())
                importRoute(route, RIPRoute::RIP_ROUTE_STATIC);
        }
    }
}

/**
 * Adds a new route the RIP routing table for an existing IRoute.
 * This route will be advertised with the specified metric and routeTag fields.
 */
RIPRoute *RIPRouting::importRoute(IRoute *route, RIPRoute::RouteType type, int metric, uint16 routeTag)
{
    ASSERT(metric < RIP_INFINITE_METRIC);

    RIPRoute *ripRoute = new RIPRoute(route, type, metric, routeTag);
    if (type == RIPRoute::RIP_ROUTE_INTERFACE) {
        InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(route->getSource());
        ripRoute->setInterface(ie);
    }

    ripRoutes.push_back(ripRoute);
    emit(numRoutesSignal, ripRoutes.size());
    return ripRoute;
}

/**
 * Sends a RIP request to routers on the specified link.
 */
void RIPRouting::sendRIPRequest(const RIPInterfaceEntry& ripInterface)
{
    RIPPacket *packet = new RIPPacket("RIP request");
    packet->setCommand(RIP_REQUEST);
    packet->setEntryArraySize(1);
    RIPEntry& entry = packet->getEntry(0);
    entry.addressFamilyId = RIP_AF_NONE;
    entry.metric = RIP_INFINITE_METRIC;
    emit(sentRequestSignal, packet);
    sendPacket(packet, addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, ripInterface.ie);
}

/**
 * Listen on interface/route changes and update private data structures.
 */
void RIPRouting::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent("RIPRouting::receiveChangeNotification(%s)", notificationCategoryName(signalID));

    IRoute *route;
    const InterfaceEntry *ie;
    const InterfaceEntryChangeDetails *change;

    if (signalID == NF_INTERFACE_CREATED) {
        // configure interface for RIP
        ie = check_and_cast<const InterfaceEntry *>(obj);
        if (ie->isMulticast() && !ie->isLoopback()) {
            cXMLElementList config = par("ripConfig").xmlValue()->getChildrenByTagName("interface");
            int i = InterfaceMatcher(config).findMatchingSelector(ie);
            if (i >= 0)
                addInterface(ie, config[i]);
        }
    }
    else if (signalID == NF_INTERFACE_DELETED) {
        // delete interfaces and routes referencing the deleted interface
        ie = check_and_cast<const InterfaceEntry *>(obj);
        deleteInterface(ie);
    }
    else if (signalID == NF_INTERFACE_STATE_CHANGED) {
        change = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        if (change->getFieldId() == InterfaceEntry::F_CARRIER || change->getFieldId() == InterfaceEntry::F_STATE) {
            ie = change->getInterfaceEntry();
            if (!ie->isUp()) {
                invalidateRoutes(ie);
            }
            else {
                RIPInterfaceEntry *ripInterfacePtr = findInterfaceById(ie->getInterfaceId());
                if (ripInterfacePtr && ripInterfacePtr->mode != NO_RIP)
                    sendRIPRequest(*ripInterfacePtr);
            }
        }
    }
    else if (signalID == NF_ROUTE_DELETED) {
        // remove references to the deleted route and invalidate the RIP route
        route = const_cast<IRoute *>(check_and_cast<const IRoute *>(obj));
        if (route->getSource() != this) {
            for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
                if ((*it)->getRoute() == route) {
                    (*it)->setRoute(nullptr);
                    invalidateRoute(*it);
                }
        }
    }
    else if (signalID == NF_ROUTE_ADDED) {
        // add or update the RIP route
        route = const_cast<IRoute *>(check_and_cast<const IRoute *>(obj));
        if (route->getSource() != this) {
            if (isLoopbackInterfaceRoute(route)) {
                /*ignore*/
                ;
            }
            else if (isLocalInterfaceRoute(route)) {
                InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(route->getSource());
                RIPRoute *ripRoute = findRoute(ie, RIPRoute::RIP_ROUTE_INTERFACE);
                if (ripRoute) {    // readded
                    RIPInterfaceEntry *ripIe = findInterfaceById(ie->getInterfaceId());
                    ripRoute->setRoute(route);
                    ripRoute->setMetric(ripIe ? ripIe->metric : 1);
                    ripRoute->setChanged(true);
                    triggerUpdate();
                }
                else
                    importRoute(route, RIPRoute::RIP_ROUTE_INTERFACE, getInterfaceMetric(ie));
            }
            else {
                // TODO import external routes from other routing daemons
            }
        }
    }
    else if (signalID == NF_ROUTE_CHANGED) {
        route = const_cast<IRoute *>(check_and_cast<const IRoute *>(obj));
        if (route->getSource() != this) {
            RIPRoute *ripRoute = findRoute(route);
            if (ripRoute) {
                // TODO check and update tag
                bool changed = route->getDestinationAsGeneric() != ripRoute->getDestination() ||
                    route->getPrefixLength() != ripRoute->getPrefixLength() ||
                    route->getNextHopAsGeneric() != ripRoute->getNextHop() ||
                    route->getInterface() != ripRoute->getInterface();
                ripRoute->setDestination(route->getDestinationAsGeneric());
                ripRoute->setPrefixLength(route->getPrefixLength());
                ripRoute->setNextHop(route->getNextHopAsGeneric());
                ripRoute->setInterface(route->getInterface());
                if (changed) {
                    ripRoute->setChanged(changed);
                    triggerUpdate();
                }
            }
        }
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

bool RIPRouting::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_ROUTING_PROTOCOLS) {
            isOperational = true;
            cancelEvent(startupTimer);
            scheduleAt(simTime() + par("startupTime").doubleValue(), startupTimer);
            return true;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_ROUTING_PROTOCOLS) {
            // invalidate routes
            for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
                invalidateRoute(*it);
            // send updates to neighbors
            for (auto it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
                sendRoutes(addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, *it, false);

            stopRIPRouting();

            // wait a few seconds before calling doneCallback, so that UDP can send the messages
            shutdownTimer->setContextPointer(doneCallback);
            scheduleAt(simTime() + shutdownTime, shutdownTimer);

            return false;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            stopRIPRouting();
            isOperational = false;
            return true;
        }
    }

    return true;
}

void RIPRouting::startRIPRouting()
{
    addressType = rt->getRouterIdAsGeneric().getAddressType();

    // configure interfaces
    configureInterfaces(par("ripConfig").xmlValue());

    // import interface routes
    configureInitialRoutes();

    // subscribe to notifications
    host->subscribe(NF_INTERFACE_CREATED, this);
    host->subscribe(NF_INTERFACE_DELETED, this);
    host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
    host->subscribe(NF_ROUTE_DELETED, this);
    host->subscribe(NF_ROUTE_ADDED, this);
    host->subscribe(NF_ROUTE_CHANGED, this);

    // configure socket
    socket.setMulticastLoop(false);
    socket.bind(ripUdpPort);

    for (auto it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        if (it->mode != NO_RIP)
            socket.joinMulticastGroup(addressType->getLinkLocalRIPRoutersMulticastAddress(), it->ie->getInterfaceId());


    for (auto it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        if (it->mode != NO_RIP)
            sendRIPRequest(*it);


    // set update timer
    scheduleAt(simTime() + updateInterval, updateTimer);
}

void RIPRouting::stopRIPRouting()
{
    if (startupTimer->isScheduled())
        cancelEvent(startupTimer);
    else {
        socket.close();

        // unsubscribe to notifications
        host->unsubscribe(NF_INTERFACE_CREATED, this);
        host->unsubscribe(NF_INTERFACE_DELETED, this);
        host->unsubscribe(NF_INTERFACE_STATE_CHANGED, this);
        host->unsubscribe(NF_ROUTE_DELETED, this);
        host->unsubscribe(NF_ROUTE_ADDED, this);
        host->unsubscribe(NF_ROUTE_CHANGED, this);
    }

    // cancel timers
    cancelEvent(updateTimer);
    cancelEvent(triggeredUpdateTimer);

    // clear data
    ripRoutes.clear();
    ripInterfaces.clear();
}

void RIPRouting::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Model error: self msg '%s' received when isOperational is false", msg->getName());
        EV_ERROR << "Application is turned off, dropping '" << msg->getName() << "' message\n";
        delete msg;
        return;
    }

    if (msg->isSelfMessage()) {
        if (msg == updateTimer) {
            processUpdate(false);
            scheduleAt(simTime() + updateInterval, msg);
        }
        else if (msg == triggeredUpdateTimer) {
            processUpdate(true);
        }
        else if (msg == startupTimer) {
            startRIPRouting();
        }
        else if (msg == shutdownTimer) {
            isOperational = false;
            IDoneCallback *doneCallback = (IDoneCallback *)msg->getContextPointer();
            msg->setContextPointer(nullptr);
            doneCallback->invoke();
        }
    }
    else if (msg->getKind() == UDP_I_DATA) {
        RIPPacket *packet = check_and_cast<RIPPacket *>(msg);
        unsigned char command = packet->getCommand();
        if (command == RIP_REQUEST)
            processRequest(packet);
        else if (command == RIP_RESPONSE)
            processResponse(packet);
        else
            throw cRuntimeError("RIP: unknown command (%d)", (int)command);
    }
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_DETAIL << "Ignoring UDP error report\n";
        delete msg;
    }
}

/**
 * This method called when a triggered or regular update timer expired.
 * It either sends the changed/all routes to neighbors.
 */
void RIPRouting::processUpdate(bool triggered)
{
    if (triggered)
        EV_INFO << "sending triggered updates on all interfaces.\n";
    else
        EV_INFO << "sending regular updates on all interfaces\n";

    for (auto it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        if (it->mode != NO_RIP)
            sendRoutes(addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, *it, triggered);


    // clear changed flags
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        (*it)->setChanged(false);
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
    if (numEntries == 0) {
        EV_INFO << "received empty request, ignoring.\n";
        delete packet;
        return;
    }

    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication *>(packet->removeControlInfo());
    L3Address srcAddr = ctrlInfo->getSrcAddr();
    int srcPort = ctrlInfo->getSrcPort();
    int interfaceId = ctrlInfo->getInterfaceId();
    delete ctrlInfo;

    EV_INFO << "received request from " << srcAddr << "\n";

    RIPRoute *ripRoute;
    for (int i = 0; i < numEntries; ++i) {
        RIPEntry& entry = packet->getEntry(i);
        switch (entry.addressFamilyId) {
            case RIP_AF_NONE:
                if (numEntries == 1 && entry.metric == RIP_INFINITE_METRIC) {
                    RIPInterfaceEntry *ripInterface = findInterfaceById(interfaceId);
                    if (ripInterface)
                        sendRoutes(srcAddr, srcPort, *ripInterface, false);
                    delete packet;
                    return;
                }
                else {
                    delete packet;
                    throw cRuntimeError("RIP: invalid request.");
                }
                break;

            case RIP_AF_INET:
                ripRoute = findRoute(entry.address, entry.prefixLength);
                entry.metric = ripRoute ? ripRoute->getMetric() : RIP_INFINITE_METRIC;
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
void RIPRouting::sendRoutes(const L3Address& address, int port, const RIPInterfaceEntry& ripInterface, bool changedOnly)
{
    EV_DEBUG << "Sending " << (changedOnly ? "changed" : "all") << " routes on " << ripInterface.ie->getFullName() << std::endl;

    int maxEntries = mode == RIPv2 ? 25 : (ripInterface.ie->getMTU() - 40    /*IPv6_HEADER_BYTES*/ - 8    /*UDP_HEADER_BYTES*/ - RIP_HEADER_SIZE) / RIP_RTE_SIZE;

    RIPPacket *packet = new RIPPacket("RIP response");
    packet->setCommand(RIP_RESPONSE);
    packet->setEntryArraySize(maxEntries);
    int k = 0;    // index into RIP entries

    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it) {
        RIPRoute *ripRoute = checkRouteIsExpired(*it);
        if (!ripRoute)
            continue;

        if (changedOnly && !ripRoute->isChanged())
            continue;

        // Split Horizon check:
        //   Omit routes learned from one neighbor in updates sent to that neighbor.
        //   In the case of a broadcast network, all routes learned from any neighbor on
        //   that network are omitted from updates sent on that network.
        // Split Horizon with Poisoned Reverse:
        //   Do include such routes in updates, but sets their metrics to infinity.
        int metric = ripRoute->getMetric();
        if (ripRoute->getInterface() == ripInterface.ie) {
            if (ripInterface.mode == SPLIT_HORIZON)
                continue;
            else if (ripInterface.mode == SPLIT_HORIZON_POISONED_REVERSE)
                metric = RIP_INFINITE_METRIC;
        }

        EV_DEBUG << "Add entry for " << ripRoute->getDestination() << "/" << ripRoute->getPrefixLength() << ": "
                 << " metric=" << metric << std::endl;

        // fill next entry
        RIPEntry& entry = packet->getEntry(k++);
        entry.addressFamilyId = RIP_AF_INET;
        entry.address = ripRoute->getDestination();
        entry.prefixLength = ripRoute->getPrefixLength();
        entry.nextHop = addressType->getUnspecifiedAddress();    //route->getNextHop() if local ?
        entry.routeTag = ripRoute->getRouteTag();
        entry.metric = metric;

        // if packet is full, then send it and allocate a new one
        if (k >= maxEntries) {
            emit(sentUpdateSignal, packet);
            sendPacket(packet, address, port, ripInterface.ie);
            packet = new RIPPacket("RIP response");
            packet->setCommand(RIP_RESPONSE);
            packet->setEntryArraySize(maxEntries);
            k = 0;
        }
    }

    // send last packet if it has entries
    if (k > 0) {
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
    if (!isValid) {
        EV_INFO << "dropping invalid response.\n";
        emit(badResponseSignal, packet);
        delete packet;
        return;
    }

    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication *>(packet->removeControlInfo());
    L3Address srcAddr = ctrlInfo->getSrcAddr();
    int interfaceId = ctrlInfo->getInterfaceId();
    delete ctrlInfo;

    RIPInterfaceEntry *incomingIe = findInterfaceById(interfaceId);
    if (!incomingIe) {
        EV_INFO << "dropping unexpected RIP response.\n";
        emit(badResponseSignal, packet);
        delete packet;
        return;
    }

    EV_INFO << "response received from " << srcAddr << "\n";
    int numEntries = packet->getEntryArraySize();
    for (int i = 0; i < numEntries; ++i) {
        RIPEntry& entry = packet->getEntry(i);
        int metric = std::min((int)entry.metric + incomingIe->metric, RIP_INFINITE_METRIC);
        L3Address nextHop = entry.nextHop.isUnspecified() ? srcAddr : entry.nextHop;

        RIPRoute *ripRoute = findRoute(entry.address, entry.prefixLength);
        if (ripRoute) {
            RIPRoute::RouteType routeType = ripRoute->getType();
            int routeMetric = ripRoute->getMetric();
            if ((routeType == RIPRoute::RIP_ROUTE_STATIC || routeType == RIPRoute::RIP_ROUTE_DEFAULT) && routeMetric != RIP_INFINITE_METRIC)
                continue;
            if (ripRoute->getFrom() == srcAddr)
                ripRoute->setLastUpdateTime(simTime());
            if ((ripRoute->getFrom() == srcAddr && ripRoute->getMetric() != metric) || metric < ripRoute->getMetric())
                updateRoute(ripRoute, incomingIe->ie, nextHop, metric, entry.routeTag, srcAddr);
            // TODO RIPng: if the metric is the same as the old one, and the old route is aboute to expire (i.e. at least halfway to the expiration point)
            //             then update the old route with the new RTE
        }
        else {
            if (metric != RIP_INFINITE_METRIC)
                addRoute(entry.address, entry.prefixLength, incomingIe->ie, nextHop, metric, entry.routeTag, srcAddr);
        }
    }

    delete packet;
}

bool RIPRouting::isValidResponse(RIPPacket *packet)
{
    UDPDataIndication *ctrlInfo = check_and_cast<UDPDataIndication *>(packet->getControlInfo());

    // check that received from ripUdpPort
    if (ctrlInfo->getSrcPort() != ripUdpPort) {
        EV_WARN << "source port is not " << ripUdpPort << "\n";
        return false;
    }

    // check that it is not our response (received own multicast message)
    if (rt->isLocalAddress(ctrlInfo->getSrcAddr())) {
        EV_WARN << "received own response\n";
        return false;
    }

    if (mode == RIPng) {
        if (!ctrlInfo->getSrcAddr().isLinkLocal()) {
            EV_WARN << "source address is not link-local: " << ctrlInfo->getSrcAddr() << "\n";
            return false;
        }
        if (ctrlInfo->getTtl() != 255) {
            EV_WARN << "ttl is not 255";
            return false;
        }
    }
    else {
        // check that source is on a directly connected network
        if (!ift->isNeighborAddress(ctrlInfo->getSrcAddr())) {
            EV_WARN << "source is not directly connected " << ctrlInfo->getSrcAddr() << "\n";
            return false;
        }
    }

    // validate entries
    int numEntries = packet->getEntryArraySize();
    for (int i = 0; i < numEntries; ++i) {
        RIPEntry& entry = packet->getEntry(i);

        // check that metric is in range [1,16]
        if (entry.metric < 1 || entry.metric > RIP_INFINITE_METRIC) {
            EV_WARN << "received metric is not in the [1," << RIP_INFINITE_METRIC << "] range.\n";
            return false;
        }

        // check that destination address is a unicast address
        // TODO exclude 0.x.x.x, 127.x.x.x too
        if (!entry.address.isUnicast()) {
            EV_WARN << "destination address of an entry is not unicast: " << entry.address << "\n";
            return false;
        }

        if (mode == RIPng) {
            if (entry.address.isLinkLocal()) {
                EV_WARN << "destination address of an entry is link-local: " << entry.address << "\n";
                return false;
            }
            if (entry.prefixLength < 0 || entry.prefixLength > addressType->getMaxPrefixLength()) {
                EV_WARN << "prefixLength is outside of the [0," << addressType->getMaxPrefixLength() << "] interval\n";
                return false;
            }
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
void RIPRouting::addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop,
        int metric, uint16 routeTag, const L3Address& from)
{
    EV_DEBUG << "Add route to " << dest << "/" << prefixLength << ": "
             << "nextHop=" << nextHop << " metric=" << metric << std::endl;

    IRoute *route = addRoute(dest, prefixLength, ie, nextHop, metric);

    RIPRoute *ripRoute = new RIPRoute(route, RIPRoute::RIP_ROUTE_RTE, metric, routeTag);
    ripRoute->setFrom(from);
    ripRoute->setLastUpdateTime(simTime());
    ripRoute->setChanged(true);
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
void RIPRouting::updateRoute(RIPRoute *ripRoute, const InterfaceEntry *ie, const L3Address& nextHop, int metric, uint16 routeTag, const L3Address& from)
{
    //ASSERT(ripRoute && ripRoute->getType() == RIPRoute::RIP_ROUTE_RTE);
    //ASSERT(!ripRoute->getRoute() || ripRoute->getRoute()->getSource() == this);

    EV_DEBUG << "Updating route to " << ripRoute->getDestination() << "/" << ripRoute->getPrefixLength() << ": "
             << "nextHop=" << nextHop << " metric=" << metric << std::endl;

    int oldMetric = ripRoute->getMetric();
    ripRoute->setInterface(const_cast<InterfaceEntry *>(ie));
    ripRoute->setMetric(metric);
    ripRoute->setFrom(from);
    ripRoute->setRouteTag(routeTag);

    if (oldMetric == RIP_INFINITE_METRIC && metric < RIP_INFINITE_METRIC) {
        ASSERT(!ripRoute->getRoute());
        ripRoute->setType(RIPRoute::RIP_ROUTE_RTE);
        ripRoute->setNextHop(nextHop);

        IRoute *route = addRoute(ripRoute->getDestination(), ripRoute->getPrefixLength(), ie, nextHop, metric);
        ripRoute->setRoute(route);
    }
    if (oldMetric != RIP_INFINITE_METRIC) {
        IRoute *route = ripRoute->getRoute();
        ASSERT(route);

        ripRoute->setRoute(nullptr);
        deleteRoute(route);

        ripRoute->setNextHop(nextHop);
        if (metric < RIP_INFINITE_METRIC) {
            route = addRoute(ripRoute->getDestination(), ripRoute->getPrefixLength(), ie, nextHop, metric);
            ripRoute->setRoute(route);
        }
    }

    ripRoute->setChanged(true);
    triggerUpdate();

    if (metric == RIP_INFINITE_METRIC && oldMetric != RIP_INFINITE_METRIC)
        invalidateRoute(ripRoute);
    else
        ripRoute->setLastUpdateTime(simTime());
}

/**
 * Sets the update timer to trigger an update in the [1s,5s] interval.
 * If the update is already scheduled, it does nothing.
 */
void RIPRouting::triggerUpdate()
{
    if (!triggeredUpdateTimer->isScheduled()) {
        double delay = par("triggeredUpdateDelay");
        simtime_t updateTime = simTime() + delay;
        // Triggered updates may be suppressed if a regular
        // update is due by the time the triggered update would be sent.
        if (!updateTimer->isScheduled() || updateTimer->getArrivalTime() > updateTime)
            scheduleAt(updateTime, triggeredUpdateTimer);
    }
}

/**
 * Should be called regularly to handle expiry and purge of routes.
 * If the route is valid, then returns it, otherwise returns nullptr.
 */
RIPRoute *RIPRouting::checkRouteIsExpired(RIPRoute *route)
{
    if (route->getType() == RIPRoute::RIP_ROUTE_RTE) {
        simtime_t now = simTime();
        if (now >= route->getLastUpdateTime() + routeExpiryTime + routePurgeTime) {
            purgeRoute(route);
            return nullptr;
        }
        if (now >= route->getLastUpdateTime() + routeExpiryTime) {
            invalidateRoute(route);
            return nullptr;
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
    IRoute *route = ripRoute->getRoute();
    if (route) {
        ripRoute->setRoute(nullptr);
        deleteRoute(route);
    }
    ripRoute->setMetric(RIP_INFINITE_METRIC);
    ripRoute->setChanged(true);
    triggerUpdate();
}

/**
 * Removes the route from the routing table.
 */
void RIPRouting::purgeRoute(RIPRoute *ripRoute)
{
    ASSERT(ripRoute->getType() == RIPRoute::RIP_ROUTE_RTE);

    IRoute *route = ripRoute->getRoute();
    if (route) {
        ripRoute->setRoute(nullptr);
        deleteRoute(route);
    }

    auto end = std::remove(ripRoutes.begin(), ripRoutes.end(), ripRoute);
    if (end != ripRoutes.end())
        ripRoutes.erase(end, ripRoutes.end());
    delete ripRoute;

    emit(numRoutesSignal, ripRoutes.size());
}

/**
 * Sends the packet to the specified destination.
 * If the destAddr is a multicast, then the destInterface must be specified.
 */
void RIPRouting::sendPacket(RIPPacket *packet, const L3Address& destAddr, int destPort, const InterfaceEntry *destInterface)
{
    packet->setByteLength(RIP_HEADER_SIZE + RIP_RTE_SIZE * packet->getEntryArraySize());

    if (destAddr.isMulticast()) {
        UDPSocket::SendOptions options;
        options.outInterfaceId = destInterface->getInterfaceId();
        if (mode == RIPng) {
            socket.setTimeToLive(255);
            options.srcAddr = addressType->getLinkLocalAddress(destInterface);
        }
        socket.sendTo(packet, destAddr, destPort, &options);
    }
    else
        socket.sendTo(packet, destAddr, destPort);
}

/*----------------------------------------
 *      private methods
 *----------------------------------------*/

RIPInterfaceEntry *RIPRouting::findInterfaceById(int interfaceId)
{
    for (auto it = ripInterfaces.begin(); it != ripInterfaces.end(); ++it)
        if (it->ie->getInterfaceId() == interfaceId)
            return &(*it);

    return nullptr;
}

RIPRoute *RIPRouting::findRoute(const L3Address& destination, int prefixLength)
{
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength)
            return *it;

    return nullptr;
}

RIPRoute *RIPRouting::findRoute(const L3Address& destination, int prefixLength, RIPRoute::RouteType type)
{
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->getType() == type && (*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength)
            return *it;

    return nullptr;
}

RIPRoute *RIPRouting::findRoute(const IRoute *route)
{
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->getRoute() == route)
            return *it;

    return nullptr;
}

RIPRoute *RIPRouting::findRoute(const InterfaceEntry *ie, RIPRoute::RouteType type)
{
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->getType() == type && (*it)->getInterface() == ie)
            return *it;

    return nullptr;
}

void RIPRouting::addInterface(const InterfaceEntry *ie, cXMLElement *config)
{
    RIPInterfaceEntry ripInterface(ie);
    if (config)
        ripInterface.configure(config);
    ripInterfaces.push_back(ripInterface);
}

void RIPRouting::deleteInterface(const InterfaceEntry *ie)
{
    // delete interfaces and routes referencing ie
    for (auto it = ripInterfaces.begin(); it != ripInterfaces.end(); ) {
        if (it->ie == ie)
            it = ripInterfaces.erase(it);
        else
            it++;
    }
    bool emitNumRoutesSignal = false;
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ) {
        if ((*it)->getInterface() == ie) {
            it = ripRoutes.erase(it);
            emitNumRoutesSignal = true;
        }
        else
            it++;
    }
    if (emitNumRoutesSignal)
        emit(numRoutesSignal, ripRoutes.size());
}

int RIPRouting::getInterfaceMetric(InterfaceEntry *ie)
{
    RIPInterfaceEntry *ripIe = findInterfaceById(ie->getInterfaceId());
    return ripIe ? ripIe->metric : 1;
}

void RIPRouting::invalidateRoutes(const InterfaceEntry *ie)
{
    for (auto it = ripRoutes.begin(); it != ripRoutes.end(); ++it)
        if ((*it)->getInterface() == ie)
            invalidateRoute(*it);

}

IRoute *RIPRouting::addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop, int metric)
{
    IRoute *route = rt->createRoute();
    route->setSourceType(IRoute::RIP);
    route->setSource(this);
    route->setDestination(dest);
    route->setPrefixLength(prefixLength);
    route->setInterface(const_cast<InterfaceEntry *>(ie));
    route->setNextHop(nextHop);
    route->setMetric(metric);
    rt->addRoute(route);
    return route;
}

void RIPRouting::deleteRoute(IRoute *route)
{
    rt->deleteRoute(route);
}

bool RIPRouting::isLoopbackInterfaceRoute(const IRoute *route)
{
    InterfaceEntry *ie = dynamic_cast<InterfaceEntry *>(route->getSource());
    return ie && ie->isLoopback();
}

bool RIPRouting::isLocalInterfaceRoute(const IRoute *route)
{
    InterfaceEntry *ie = dynamic_cast<InterfaceEntry *>(route->getSource());
    return ie && !ie->isLoopback();
}

} // namespace inet

