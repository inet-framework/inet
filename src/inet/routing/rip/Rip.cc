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

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/ipv6/Ipv6Consts.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/InterfaceMatcher.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/routing/rip/Rip.h"
#include "inet/routing/rip/RipPacket_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

Define_Module(Rip);

std::ostream& operator<<(std::ostream& os, const RipRoute& e)
{
    os << e.str();
    return os;
}

std::ostream& operator<<(std::ostream& os, const RipInterfaceEntry& e)
{
    os << "if:" << e.ie->getInterfaceName() << "  ";
    os << "metric:" << e.metric << "  ";
    os << "mode: ";
    switch (e.mode) {
        case NO_RIP:
            os << "NoRIP";
            break;

        case PASSIVE:
            os << "PASSIVE";
            break;

        case NO_SPLIT_HORIZON:
            os << "NoSplitHorizon";
            break;

        case SPLIT_HORIZON:
            os << "SplitHorizon";
            break;

        case SPLIT_HORIZON_POISON_REVERSE:
            os << "SplitHorizonPoisonedReverse";
            break;

        default:
            os << "<unknown>";
            break;
    }
    return os;
}

Rip::Rip()
{
}

Rip::~Rip()
{
    for (auto & elem : ripRoutingTable)
        delete elem;
    ripRoutingTable.clear();
    cancelAndDelete(updateTimer);
    cancelAndDelete(triggeredUpdateTimer);
    cancelAndDelete(startupTimer);
    cancelAndDelete(shutdownTimer);
}

simsignal_t Rip::sentRequestSignal = registerSignal("sentRequest");
simsignal_t Rip::sentUpdateSignal = registerSignal("sentUpdate");
simsignal_t Rip::rcvdResponseSignal = registerSignal("rcvdResponse");
simsignal_t Rip::badResponseSignal = registerSignal("badResponse");
simsignal_t Rip::numRoutesSignal = registerSignal("numRoutes");

void Rip::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        socket.setOutputGate(gate("socketOut"));

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
        updateInterval = par("updateInterval");
        routeExpiryTime = par("routeExpiryTime");
        routePurgeTime = par("routePurgeTime");
        holdDownTime = par("holdDownTime");
        shutdownTime = par("shutdownTime");
        triggeredUpdate = par("triggeredUpdate");

        updateTimer = new cMessage("RIP-timer");
        triggeredUpdateTimer = new cMessage("RIP-trigger");
        startupTimer = new cMessage("RIP-startup");
        shutdownTimer = new cMessage("RIP-shutdown");

        WATCH_VECTOR(ripInterfaces);
        WATCH_PTRVECTOR(ripRoutingTable);
    }
}

void Rip::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == updateTimer) {
            processUpdate(false);
            scheduleAfter(updateInterval, msg);
        }
        else if (msg == triggeredUpdateTimer) {
            processUpdate(true);
        }
        else if (msg == startupTimer) {
            startRIPRouting();
        }
        else if (msg == shutdownTimer) {
            ASSERT(operationalState == State::STOPPING_OPERATION);
            finishActiveOperation();
        }
        else
            throw cRuntimeError("unknown self message");
    }
    else if (msg->getKind() == UDP_I_DATA) {
        Packet *pk = check_and_cast<Packet *>(msg);
        unsigned char command = pk->peekAtFront<RipPacket>()->getCommand();
        if (command == RIP_REQUEST)
            processRequest(pk);
        else if (command == RIP_RESPONSE)
            processResponse(pk);
        else
            throw cRuntimeError("RIP: unknown command (%d)", (int)command);
    }
    else if (msg->getKind() == UDP_I_ERROR) {
        EV_DETAIL << "Ignoring UDP error report\n";
        delete msg;
    }
}

void Rip::startRIPRouting()
{
    addressType = rt->getRouterIdAsGeneric().getAddressType();

    cXMLElementList interfaceElements = par("ripConfig").xmlValue()->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);

    // Creates a RipInterfaceEntry for each interface found in the 'interface table'.
    for (int k = 0; k < ift->getNumInterfaces(); ++k) {
        InterfaceEntry *ie = ift->getInterface(k);
        if (ie->isMulticast() && !ie->isLoopback()) {
            int i = matcher.findMatchingSelector(ie);
            addRipInterface(ie, i >= 0 ? interfaceElements[i] : nullptr);
        }
    }

    // Import interface/static/default routes from the 'routing table'.
    for (int i = 0; i < rt->getNumRoutes(); ++i) {
        IRoute *route = rt->getRoute(i);
        if (isLoopbackInterfaceRoute(route)) {
            /*ignore*/
            ;
        }
        else if (isLocalInterfaceRoute(route)) {
            InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(route->getSource());
            RipInterfaceEntry *ripIe = findRipInterfaceById(ie->getInterfaceId());
            if(!ripIe || ripIe->mode != NO_RIP)
                importRoute(route, RipRoute::RIP_ROUTE_INTERFACE, getInterfaceMetric(ie));
        }
        else if (isDefaultRoute(route))
            importRoute(route, RipRoute::RIP_ROUTE_DEFAULT);
        else {
            const L3Address& destAddr = route->getDestinationAsGeneric();
            if (!destAddr.isMulticast() && !destAddr.isLinkLocal())
                importRoute(route, RipRoute::RIP_ROUTE_STATIC);
        }
    }

    // subscribe to interface created/deleted/changed notifications
    host->subscribe(interfaceCreatedSignal, this);
    host->subscribe(interfaceDeletedSignal, this);
    host->subscribe(interfaceStateChangedSignal, this);

    // subscribe to route added/deleted/changed notifications
    host->subscribe(routeAddedSignal, this);
    host->subscribe(routeDeletedSignal, this);
    host->subscribe(routeChangedSignal, this);

    // configure socket
    socket.setMulticastLoop(false);
    socket.bind(ripUdpPort);

    for (auto & elem : ripInterfaces)
        if (elem.mode != NO_RIP && elem.mode != PASSIVE)
            socket.joinMulticastGroup(addressType->getLinkLocalRIPRoutersMulticastAddress(), elem.ie->getInterfaceId());

    for (auto & elem : ripInterfaces)
        if (elem.mode != NO_RIP && elem.mode != PASSIVE)
            sendRIPRequest(elem);

    // set update timer
    scheduleAfter(updateInterval, updateTimer);
}

void Rip::stopRIPRouting()
{
    if (startupTimer->isScheduled())
        cancelEvent(startupTimer);
    else {
        socket.close();

        // unsubscribe to notifications
        host->unsubscribe(interfaceCreatedSignal, this);
        host->unsubscribe(interfaceDeletedSignal, this);
        host->unsubscribe(interfaceStateChangedSignal, this);
        host->unsubscribe(routeDeletedSignal, this);
        host->unsubscribe(routeAddedSignal, this);
        host->unsubscribe(routeChangedSignal, this);
    }

    // cancel timers
    cancelEvent(updateTimer);
    cancelEvent(triggeredUpdateTimer);

    // clear data
    for (auto& elem : ripRoutingTable)
        delete elem;
    ripRoutingTable.clear();
    ripInterfaces.clear();
}

/**
 * Adds a new route the RIP routing table for an existing IRoute.
 * This route will be advertised with the specified metric and routeTag fields.
 */
RipRoute *Rip::importRoute(IRoute *route, RipRoute::RouteType type, int metric, uint16 routeTag)
{
    ASSERT(metric < RIP_INFINITE_METRIC);

    RipRoute *ripRoute = new RipRoute(route, type, metric, routeTag);
    if (type == RipRoute::RIP_ROUTE_INTERFACE) {
        InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(route->getSource());
        ripRoute->setInterface(ie);
    }

    ripRoutingTable.push_back(ripRoute);
    emit(numRoutesSignal, (unsigned long)ripRoutingTable.size());
    return ripRoute;
}

/**
 * Sends a RIP request to routers on the specified link.
 */
void Rip::sendRIPRequest(const RipInterfaceEntry& ripInterface)
{
    const  auto& packet = makeShared<RipPacket>();
    packet->setCommand(RIP_REQUEST);
    packet->setEntryArraySize(1);
    RipEntry& entry = packet->getEntryForUpdate(0);
    entry.addressFamilyId = RIP_AF_NONE;
    entry.metric = RIP_INFINITE_METRIC;
    packet->setChunkLength(RIP_HEADER_SIZE + RIP_RTE_SIZE * packet->getEntryArraySize());

    EV_INFO << "sending RIP request from " << ripInterface.ie->getInterfaceName() << "\n";

    Packet *pk = new Packet("RIP request");
    pk->insertAtBack(packet);
    emit(sentRequestSignal, pk);
    sendPacket(pk, addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, ripInterface.ie);
}

/**
 * Listen on interface/route changes and update private data structures.
 */
void Rip::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent("Rip::receiveChangeNotification(%s)", cComponent::getSignalName(signalID));

    const InterfaceEntry *ie;
    const InterfaceEntryChangeDetails *change;

    if (signalID == interfaceCreatedSignal) {
        // configure interface for RIP
        ie = check_and_cast<const InterfaceEntry *>(obj);
        if (ie->isMulticast() && !ie->isLoopback()) {
            cXMLElementList config = par("ripConfig").xmlValue()->getChildrenByTagName("interface");
            int i = InterfaceMatcher(config).findMatchingSelector(ie);
            if (i >= 0)
                addRipInterface(ie, config[i]);
        }
    }
    else if (signalID == interfaceDeletedSignal) {
        // delete interfaces and routes referencing the deleted interface
        ie = check_and_cast<const InterfaceEntry *>(obj);
        deleteRipInterface(ie);
    }
    else if (signalID == interfaceStateChangedSignal) {
        change = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        auto fieldId = change->getFieldId();
        if (fieldId == InterfaceEntry::F_STATE || fieldId == InterfaceEntry::F_CARRIER) {
            ie = change->getInterfaceEntry();
            if (!ie->isUp()) {
                for (auto & elem : ripRoutingTable)
                    if ((elem)->getInterface() == ie) {
                        invalidateRoute(elem);
                    }
            }
            else {
                RipInterfaceEntry *ripInterfacePtr = findRipInterfaceById(ie->getInterfaceId());
                if (ripInterfacePtr && ripInterfacePtr->mode != NO_RIP && ripInterfacePtr->mode != PASSIVE)
                    sendRIPRequest(*ripInterfacePtr);
            }
        }
    }
    else if (signalID == routeDeletedSignal) {
        // remove references to the deleted route and invalidate the RIP route
        const IRoute *route = check_and_cast<const IRoute *>(obj);
        for (auto & elem : ripRoutingTable) {
            if ((elem)->getRoute() == route) {
                (elem)->setRoute(nullptr);
                if (route->getSource() != this) {
                    invalidateRoute(elem);
                }
            }
        }
    }
    else if (signalID == routeAddedSignal) {
        // add or update the RIP route
        IRoute *route = const_cast<IRoute *>(check_and_cast<const IRoute *>(obj));
        if (route->getSource() != this) {
            if (isLoopbackInterfaceRoute(route)) {
                /*ignore*/
                ;
            }
            else if (isLocalInterfaceRoute(route)) {
                InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(route->getSource());
                RipRoute *ripRoute = findRipRoute(ie, RipRoute::RIP_ROUTE_INTERFACE);
                if (ripRoute) {    // readded
                    RipInterfaceEntry *ripIe = findRipInterfaceById(ie->getInterfaceId());
                    ripRoute->setRoute(route);
                    ripRoute->setMetric(ripIe ? ripIe->metric : 1);
                    ripRoute->setChanged(true);
                    triggerUpdate();
                }
                else
                {
                    RipInterfaceEntry *ripIe = findRipInterfaceById(ie->getInterfaceId());
                    if(!ripIe || ripIe->mode != NO_RIP)
                        importRoute(route, RipRoute::RIP_ROUTE_INTERFACE, getInterfaceMetric(ie));
                }
            }
            else {
                // TODO import external routes from other routing daemons
            }
        }
    }
    else if (signalID == routeChangedSignal) {
        const IRoute *route = check_and_cast<const IRoute *>(obj);
        if (route->getSource() != this) {
            RipRoute *ripRoute = findRipRoute(route);
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
                    ripRoute->setChanged(true);
                    triggerUpdate();
                }
            }
        }
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

void Rip::handleStartOperation(LifecycleOperation *operation)
{
    cancelEvent(startupTimer);
    scheduleAfter(par("startupTime"), startupTimer);
}

void Rip::handleStopOperation(LifecycleOperation *operation)
{
    // invalidate routes
    for (auto & elem : ripRoutingTable)
        invalidateRoute(elem);
    // send updates to neighbors
    for (auto & elem : ripInterfaces)
        sendRoutes(addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, elem, false);

    stopRIPRouting();

    // wait a few seconds before calling doneCallback, so that UDP can send the messages
    scheduleAfter(shutdownTime, shutdownTimer);
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void Rip::handleCrashOperation(LifecycleOperation *operation)
{
    stopRIPRouting();
}

/**
 * This method called when a triggered or regular update timer expired.
 * It either sends the changed/all routes to neighbors.
 */
void Rip::processUpdate(bool triggered)
{
    if (triggered)
        EV_INFO << "sending triggered updates on all interfaces.\n";
    else
        EV_INFO << "sending regular updates on all interfaces\n";

    for (auto &ripInterface : ripInterfaces)
        if (ripInterface.ie->isUp())
            sendRoutes(addressType->getLinkLocalRIPRoutersMulticastAddress(), ripUdpPort, ripInterface, triggered);

    // clear changed flags
    for (auto &ripRoute : ripRoutingTable)
        ripRoute->setChanged(false);
}

/**
 * Processes a request received from a RIP router or a monitoring process.
 * The request processing follows the guidelines described in RFC 2453 3.9.1.
 *
 * There are two cases:
 * - the request enumerates the requested prefixes
 *     There is an RipEntry for each requested route in the packet.
 *     The RIP module simply looks up the prefix in its table, and
 *     if it sets the metric field of the entry to the metric of the
 *     found route, or to infinity (16) if not found. Once all entries
 *     are have been filled in, change the command from Request to Response,
 *     and sent the packet back to the requester. If there are no
 *     entries in the request, then no response is sent; the request is
 *     silently discarded.
 * - the whole routing table is requested
 *     In this case the RipPacket contains only one entry, with addressFamilyId 0,
 *     and metric 16 (infinity). In this case the whole routing table is sent,
 *     using the normal output process (sendRoutes() method).
 */
void Rip::processRequest(Packet *packet)
{
    const auto& ripPacket = dynamicPtrCast<RipPacket>(packet->peekAtFront<RipPacket>()->dupShared());

    int numEntries = ripPacket->getEntryArraySize();
    if (numEntries == 0) {
        EV_INFO << "received empty request, ignoring.\n";
        delete packet;
        return;
    }

    L3Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
    int srcPort = packet->getTag<L4PortInd>()->getSrcPort();
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();

    EV_INFO << "received request from " << srcAddr << "\n";

    for (int i = 0; i < numEntries; ++i) {
        RipEntry& entry = ripPacket->getEntryForUpdate(i);
        switch (entry.addressFamilyId) {
            case RIP_AF_NONE:
                if (numEntries == 1 && entry.metric == RIP_INFINITE_METRIC) {
                    RipInterfaceEntry *ripInterface = findRipInterfaceById(interfaceId);
                    if (ripInterface)
                        sendRoutes(srcAddr, srcPort, *ripInterface, false);
                    delete packet;
                    return;
                }
                else {
                    throw cRuntimeError("RIP: invalid request.");
                }
                break;

            case RIP_AF_INET: {
                RipRoute *ripRoute = findRipRoute(entry.address, entry.prefixLength);
                entry.metric = ripRoute ? ripRoute->getMetric() : RIP_INFINITE_METRIC;
                // entry.nextHop, entry.routeTag?
                break;
            }

            default:
                throw cRuntimeError("RIP: request has invalid addressFamilyId: %d.", (int)entry.addressFamilyId);
        }
    }

    ripPacket->setCommand(RIP_RESPONSE);
    Packet *outPacket = new Packet("RIP response");
    outPacket->insertAtBack(ripPacket);
    socket.sendTo(outPacket, srcAddr, srcPort);
}

/**
 * Send all or changed part of the routing table to address/port on the specified interface.
 * This method is called by regular updates (every 30s), triggered updates (when some route changed),
 * and when RIP requests are processed.
 */
void Rip::sendRoutes(const L3Address& address, int port, const RipInterfaceEntry& ripInterface, bool changedOnly)
{
    if(ripInterface.mode == NO_RIP)
        return;

    if(ripInterface.mode == PASSIVE)
    {
        EV_DEBUG << "No update is sent from passive interface " << ripInterface.ie->getFullName() << std::endl;
        checkExpiredRoutes();
        return;
    }

    EV_DEBUG << "Sending " << (changedOnly ? "changed" : "all") << " routes on " << ripInterface.ie->getFullName() << std::endl;

    int maxEntries = mode == RIPv2 ? 25 : B(B(ripInterface.ie->getMtu()) - IPv6_HEADER_BYTES - UDP_HEADER_LENGTH - RIP_HEADER_SIZE).get() / RIP_RTE_SIZE.get();

    Packet *pk = new Packet("RIP response");
    auto packet = makeShared<RipPacket>();
    packet->setCommand(RIP_RESPONSE);
    packet->setEntryArraySize(maxEntries);
    int k = 0;    // index into RIP entries

    checkExpiredRoutes();

    for (auto &ripRoute : ripRoutingTable) {
        // this is a triggered update
        if (changedOnly) {

            // make sure triggered update is active
            ASSERT(triggeredUpdate);

            if (!ripRoute->isChanged())
                continue;
        }

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
            else if (ripInterface.mode == SPLIT_HORIZON_POISON_REVERSE)
                metric = RIP_INFINITE_METRIC;
        }

        EV_DEBUG << "Add entry for " << ripRoute->getDestination() << "/" << ripRoute->getPrefixLength() << ": "
                 << " metric=" << metric << std::endl;

        // fill next entry
        RipEntry& entry = packet->getEntryForUpdate(k++);
        entry.addressFamilyId = RIP_AF_INET;
        entry.address = ripRoute->getDestination();
        entry.prefixLength = ripRoute->getPrefixLength();
        entry.nextHop = addressType->getUnspecifiedAddress();    //route->getNextHop() if local ?
        entry.routeTag = ripRoute->getRouteTag();
        entry.metric = metric;

        // if packet is full, then send it and allocate a new one
        if (k >= maxEntries) {
            packet->setChunkLength(RIP_HEADER_SIZE + RIP_RTE_SIZE * packet->getEntryArraySize());
            pk->insertAtBack(packet);

            emit(sentUpdateSignal, pk);
            sendPacket(pk, address, port, ripInterface.ie);
            pk = new Packet("RIP response");
            packet = makeShared<RipPacket>();
            packet->setCommand(RIP_RESPONSE);
            packet->setEntryArraySize(maxEntries);
            k = 0;
        }
    }

    // send last packet if it has entries
    if (k > 0) {
        packet->setEntryArraySize(k);
        packet->setChunkLength(RIP_HEADER_SIZE + RIP_RTE_SIZE * packet->getEntryArraySize());
        pk->insertAtBack(packet);

        emit(sentUpdateSignal, pk);
        sendPacket(pk, address, port, ripInterface.ie);
    }
    else
        delete pk;
}

/**
 * Processes the RIP response and updates the routing table.
 *
 * First it validates the packet to avoid corrupting the routing
 * table with a wrong packet. Valid responses must come from a neighboring
 * RIP router.
 *
 * Next each RipEntry is processed one by one. Check that destination address
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
void Rip::processResponse(Packet *packet)
{
    emit(rcvdResponseSignal, packet);

    bool isValid = isValidResponse(packet);
    if (!isValid) {
        EV_INFO << "dropping invalid response.\n";
        emit(badResponseSignal, packet);
        delete packet;
        return;
    }

    L3Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    packet->clearTags();

    RipInterfaceEntry *incomingIe = findRipInterfaceById(interfaceId);
    if (!incomingIe) {
        EV_INFO << "dropping unexpected RIP response.\n";
        emit(badResponseSignal, packet);
        delete packet;
        return;
    }

    const auto& ripPacket = packet->peekAtFront<RipPacket>();

    EV_INFO << "response received from " << srcAddr << "\n";
    int numEntries = ripPacket->getEntryArraySize();
    for (int i = 0; i < numEntries; ++i) {
        const RipEntry& entry = ripPacket->getEntry(i);
        int metric = std::min((int)entry.metric + std::max(incomingIe->metric, 1), RIP_INFINITE_METRIC);
        L3Address nextHop = entry.nextHop.isUnspecified() ? srcAddr : entry.nextHop;

        RipRoute *ripRoute = findRipRoute(entry.address, entry.prefixLength);
        if (ripRoute) {
            RipRoute::RouteType routeType = ripRoute->getType();
            int routeMetric = ripRoute->getMetric();
            L3Address fromAddr = ripRoute->getFrom();

            if ((routeType == RipRoute::RIP_ROUTE_STATIC || routeType == RipRoute::RIP_ROUTE_DEFAULT) && routeMetric != RIP_INFINITE_METRIC)
                continue;

            if (fromAddr == srcAddr)
                ripRoute->setLastUpdateTime(simTime());

            if (metric < routeMetric || (fromAddr == srcAddr && routeMetric != metric)) {
                bool preventRouteUpdate = false;
                // we receive a route update that shows the unreachable route is now reachable
                if(routeMetric == RIP_INFINITE_METRIC && metric < RIP_INFINITE_METRIC) {
                    if(holdDownTime > 0 && simTime() < ripRoute->getLastInvalidationTime() + holdDownTime) {
                        EV_DEBUG << "hold-down timer prevents update to route " << ripRoute->getDestination() << std::endl;
                        preventRouteUpdate = true;
                    }
                    else {
                        IRoute *route = ripRoute->getRoute();
                        if(route && route->getMetric() <= metric) {
                            EV_DEBUG << "existing route " << ripRoute->getDestination() << " has a better metric" << std::endl;
                            preventRouteUpdate = true;
                        }
                    }
                }

                if(!preventRouteUpdate)
                    updateRoute(ripRoute, incomingIe->ie, nextHop, metric, entry.routeTag, srcAddr);
            }

            // TODO RIPng: if the metric is the same as the old one, and the old route is about to expire (i.e. at least halfway to the expiration point)
            //             then update the old route with the new RTE
        }
        else {
            if (metric != RIP_INFINITE_METRIC)
                addRoute(entry.address, entry.prefixLength, incomingIe->ie, nextHop, metric, entry.routeTag, srcAddr);
        }
    }

    delete packet;
}

bool Rip::isValidResponse(Packet *packet)
{
    // check that received from ripUdpPort
    if (packet->getTag<L4PortInd>()->getSrcPort() != ripUdpPort) {
        EV_WARN << "source port is not " << ripUdpPort << "\n";
        return false;
    }

    L3Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();

    // check that it is not our response (received own multicast message)
    if (rt->isLocalAddress(srcAddr)) {
        EV_WARN << "received own response\n";
        return false;
    }

    if (mode == RIPng) {
        if (!srcAddr.isLinkLocal()) {
            EV_WARN << "source address is not link-local: " << srcAddr << "\n";
            return false;
        }
        if (packet->getTag<HopLimitInd>()->getHopLimit() != 255) {
            EV_WARN << "ttl is not 255";
            return false;
        }
    }
    else {
        // check that source is on a directly connected network
        if (!ift->isNeighborAddress(srcAddr)) {
            EV_WARN << "source is not directly connected " << srcAddr << "\n";
            return false;
        }
    }

    const auto& ripPacket = packet->peekAtFront<RipPacket>();
    // validate entries
    int numEntries = ripPacket->getEntryArraySize();
    for (int i = 0; i < numEntries; ++i) {
        const RipEntry& entry = ripPacket->getEntry(i);

        // check that metric is in range [0,16]
        if (entry.metric < 0 || entry.metric > RIP_INFINITE_METRIC) {
            EV_WARN << "received metric is not in the [0," << RIP_INFINITE_METRIC << "] range.\n";
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
void Rip::addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop,
        int metric, uint16 routeTag, const L3Address& from)
{
    EV_DEBUG << "Add route to " << dest << "/" << prefixLength << ": "
             << "nextHop=" << nextHop << " metric=" << metric << std::endl;

    IRoute *route = createRoute(dest, prefixLength, ie, nextHop, metric);

    RipRoute *ripRoute = new RipRoute(route, RipRoute::RIP_ROUTE_RTE, metric, routeTag);
    ripRoute->setFrom(from);
    ripRoute->setLastUpdateTime(simTime());
    ripRoute->setChanged(true);
    ripRoutingTable.push_back(ripRoute);

    emit(numRoutesSignal, (unsigned long)ripRoutingTable.size());
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
void Rip::updateRoute(RipRoute *ripRoute, const InterfaceEntry *ie, const L3Address& nextHop, int metric, uint16 routeTag, const L3Address& from)
{
    EV_DEBUG << "Updating route to " << ripRoute->getDestination() << "/" << ripRoute->getPrefixLength() << ": "
             << "nextHop=" << nextHop << " metric=" << metric << std::endl;

    int oldMetric = ripRoute->getMetric();

    ripRoute->setInterface(const_cast<InterfaceEntry *>(ie));
    ripRoute->setMetric(metric);
    ripRoute->setFrom(from);
    ripRoute->setRouteTag(routeTag);

    if (oldMetric == RIP_INFINITE_METRIC && metric < RIP_INFINITE_METRIC) {
        IRoute *route = ripRoute->getRoute();
        if(route)
            rt->deleteRoute(route);
        ripRoute->setType(RipRoute::RIP_ROUTE_RTE);
        ripRoute->setNextHop(nextHop);
        IRoute *newRoute = createRoute(ripRoute->getDestination(), ripRoute->getPrefixLength(), ie, nextHop, metric);
        ripRoute->setRoute(newRoute);
    }

    if (oldMetric != RIP_INFINITE_METRIC) {
        IRoute *route = ripRoute->getRoute();
        ASSERT(route);
        rt->deleteRoute(route);

        ripRoute->setRoute(nullptr);
        ripRoute->setNextHop(nextHop);

        if (metric < RIP_INFINITE_METRIC) {
            IRoute *newRoute = createRoute(ripRoute->getDestination(), ripRoute->getPrefixLength(), ie, nextHop, metric);
            ripRoute->setRoute(newRoute);
        }
    }

    ripRoute->setChanged(true);
    triggerUpdate();

    if (oldMetric != RIP_INFINITE_METRIC && metric == RIP_INFINITE_METRIC)
        invalidateRoute(ripRoute);
    else
        ripRoute->setLastUpdateTime(simTime());
}

/**
 * Sets the update timer to trigger an update in the [1s,5s] interval.
 * If the update is already scheduled, it does nothing.
 */
void Rip::triggerUpdate()
{
    if (triggeredUpdate && !triggeredUpdateTimer->isScheduled()) {
        double delay = par("triggeredUpdateDelay");
        // Triggered updates may be suppressed if a regular
        // update is due by the time the triggered update would be sent.
        if (!updateTimer->isScheduled() || updateTimer->getArrivalTime() > simTime() + delay)
        {
            EV_DETAIL << "scheduling triggered update \n";
            scheduleAfter(delay, triggeredUpdateTimer);
        }
    }
}

/**
 * Should be called regularly to handle expiry and purge of routes.
 */
void Rip::checkExpiredRoutes()
{
    // iterate over each rip route and check if it has expired
    // note that the iterator becomes invalid after calling purgeRoute
    for (RouteVector::iterator iter = ripRoutingTable.begin(); iter != ripRoutingTable.end();) {
        RipRoute *ripRoute = (*iter);
        if (ripRoute->getType() == RipRoute::RIP_ROUTE_RTE) {
            simtime_t now = simTime();
            if (now >= ripRoute->getLastUpdateTime() + routeExpiryTime + routePurgeTime)
            {
                iter = purgeRoute(ripRoute);
                continue;
            }
            else if (now >= ripRoute->getLastUpdateTime() + routeExpiryTime)
                invalidateRoute(ripRoute);
        }

        iter++;
    }
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
void Rip::invalidateRoute(RipRoute *ripRoute)
{
    EV_INFO << "invalidating route dest:" << ripRoute->getDestination() << "\n";

    IRoute *route = ripRoute->getRoute();
    if (route) {
        ripRoute->setRoute(nullptr);
        rt->deleteRoute(route);
    }
    ripRoute->setMetric(RIP_INFINITE_METRIC);
    ripRoute->setChanged(true);
    ripRoute->setLastInvalidationTime(simTime());
    triggerUpdate();
}

/**
 * Removes the route from the routing table.
 */
Rip::RouteVector::iterator Rip::purgeRoute(RipRoute *ripRoute)
{
    ASSERT(ripRoute->getType() == RipRoute::RIP_ROUTE_RTE);

    EV_INFO << "purging route dest:" << ripRoute->getDestination() << "\n";

    IRoute *route = ripRoute->getRoute();
    if (route) {
        ripRoute->setRoute(nullptr);
        rt->deleteRoute(route);
    }

    // erase the ripRoute from the vector
    auto itt = ripRoutingTable.erase(std::find(ripRoutingTable.begin(), ripRoutingTable.end(), ripRoute));

    delete ripRoute;
    ripRoute = nullptr;

    emit(numRoutesSignal, (unsigned long)ripRoutingTable.size());

    // return the iterator that points to the next element after the deleted one
    return itt;
}

/**
 * Sends the packet to the specified destination.
 * If the destAddr is a multicast, then the destInterface must be specified.
 */
void Rip::sendPacket(Packet *packet, const L3Address& destAddr, int destPort, const InterfaceEntry *destInterface)
{
    if (destAddr.isMulticast()) {
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destInterface->getInterfaceId());
        if (mode == RIPng) {
            socket.setTimeToLive(255);
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(addressType->getLinkLocalAddress(destInterface));
        }
    }
    socket.sendTo(packet, destAddr, destPort);
}

/*----------------------------------------
 *      private methods
 *----------------------------------------*/

RipInterfaceEntry *Rip::findRipInterfaceById(int interfaceId)
{
    for (auto & elem : ripInterfaces)
        if (elem.ie->getInterfaceId() == interfaceId)
            return &(elem);

    return nullptr;
}

RipRoute *Rip::findRipRoute(const L3Address& destination, int prefixLength)
{
    for (auto & elem : ripRoutingTable)
        if ((elem)->getDestination() == destination && (elem)->getPrefixLength() == prefixLength)
            return elem;

    return nullptr;
}

RipRoute *Rip::findRipRoute(const L3Address& destination, int prefixLength, RipRoute::RouteType type)
{
    for (auto & elem : ripRoutingTable)
        if ((elem)->getType() == type && (elem)->getDestination() == destination && (elem)->getPrefixLength() == prefixLength)
            return elem;

    return nullptr;
}

RipRoute *Rip::findRipRoute(const IRoute *route)
{
    for (auto & elem : ripRoutingTable)
        if ((elem)->getRoute() == route)
            return elem;

    return nullptr;
}

RipRoute *Rip::findRipRoute(const InterfaceEntry *ie, RipRoute::RouteType type)
{
    for (auto & elem : ripRoutingTable)
        if ((elem)->getType() == type && (elem)->getInterface() == ie)
            return elem;

    return nullptr;
}

void Rip::addRipInterface(const InterfaceEntry *ie, cXMLElement *config)
{
    RipInterfaceEntry ripInterface(ie);

    // Fills in the parameters of the interface from the matching <interface>
    // element of the configuration.
    if (config)
    {
        const char *metricAttr = config->getAttribute("metric");

        if (metricAttr) {
            int metric = atoi(metricAttr);
            if (metric < 0 || metric >= RIP_INFINITE_METRIC)
                throw cRuntimeError("RIP: invalid metric in <interface> element at %s: %s", config->getSourceLocation(), metricAttr);
            ripInterface.metric = metric;
        }

        const char *ripModeAttr = config->getAttribute("mode");
        RipMode mode = !ripModeAttr ? SPLIT_HORIZON_POISON_REVERSE :
            strcmp(ripModeAttr, "NoRIP") == 0 ? NO_RIP :
            strcmp(ripModeAttr, "PASSIVE") == 0 ? PASSIVE :
            strcmp(ripModeAttr, "NoSplitHorizon") == 0 ? NO_SPLIT_HORIZON :
            strcmp(ripModeAttr, "SplitHorizon") == 0 ? SPLIT_HORIZON :
            strcmp(ripModeAttr, "SplitHorizonPoisonReverse") == 0 ? SPLIT_HORIZON_POISON_REVERSE :
            strcmp(ripModeAttr, "SplitHorizonPoisonedReverse") == 0 ? SPLIT_HORIZON_POISON_REVERSE : // TODO: left here for backward compatibility, delete this line eventually
                    static_cast<RipMode>(-1);

        if (mode == static_cast<RipMode>(-1))
            throw cRuntimeError("RIP: invalid mode attribute in <interface> element at %s: %s",
                    config->getSourceLocation(), ripModeAttr);
        ripInterface.mode = mode;
    }

    ripInterfaces.push_back(ripInterface);
}

void Rip::deleteRipInterface(const InterfaceEntry *ie)
{
    // delete interfaces and routes referencing ie
    for (auto it = ripInterfaces.begin(); it != ripInterfaces.end(); ) {
        if (it->ie == ie)
            it = ripInterfaces.erase(it);
        else
            it++;
    }
    bool emitNumRoutesSignal = false;
    for (auto it = ripRoutingTable.begin(); it != ripRoutingTable.end(); ) {
        if ((*it)->getInterface() == ie) {
            delete *it;
            it = ripRoutingTable.erase(it);
            emitNumRoutesSignal = true;
        }
        else
            it++;
    }
    if (emitNumRoutesSignal)
        emit(numRoutesSignal, (unsigned long)ripRoutingTable.size());
}

int Rip::getInterfaceMetric(InterfaceEntry *ie)
{
    RipInterfaceEntry *ripIe = findRipInterfaceById(ie->getInterfaceId());
    return ripIe ? ripIe->metric : 1;
}

IRoute *Rip::createRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop, int metric)
{
    // create a new route
    IRoute *route = rt->createRoute();

    // adding generic fields
    route->setDestination(dest);
    route->setNextHop(nextHop);
    route->setPrefixLength(prefixLength);
    route->setMetric(metric);
    route->setAdminDist(IRoute::dRIP);
    route->setInterface(const_cast<InterfaceEntry *>(ie));
    route->setSourceType(IRoute::RIP);
    route->setSource(this);

    EV_DETAIL << "Adding new route " << route << endl;
    rt->addRoute(route);

    return route;
}

} // namespace inet
