//
// Copyright (C) 2014 OpenSim Ltd.
// Author: Benjamin Seregi
// Copyright (C) 2019 Universidad de Malaga
// Author: Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//


#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/routing/extras/LoadNg/LoadNg.h"
#include "inet/routing/extras/LoadNg/DeepFirstForwardTag_m.h"

// DONE: actualize routes using hello information,
// DONE: Fill the routing tables using the routes computes by Dijkstra

// TODO: Extract alternative routes from Dijkstra if the principal fails.
// TODO: Recalculate Dijkstra using the transmission errors
// TODO: Calculate the route to the sink using Hellos
// TODO: measure link quality metrics and to use them for routing.
// TODO: Modify the link layer that quality measures could arrive to upper layers
// TODO: Modify the link layer to force that a percentage of links could be only unidirectional
// TODO: Modify the link layer to force a predetermine lost packets in predetermined links.
// TODO: Add FFD tag to the packets stolen from network layer.

namespace inet {
namespace inetmanet {

Define_Module(LoadNg);

void LoadNg::initialize(int stage)
{
    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
        addressType = getSelfIPAddress().getAddressType();  // needed for handleStartOperation()

    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        lastBroadcastTime = SIMTIME_ZERO;
        rebootTime = SIMTIME_ZERO;
        rreqId = sequenceNum = 0;
        rreqCount = rerrCount = 0;
        host = getContainingNode(this);
        routingTable = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);

        aodvUDPPort = par("udpPort");
        askGratuitousRREP = par("askGratuitousRREP");
        useHelloMessages = par("useHelloMessages");
        destinationOnlyFlag = par("destinationOnlyFlag");
        activeRouteTimeout = par("activeRouteTimeout");
        helloInterval = par("helloInterval");
        allowedHelloLoss = par("allowedHelloLoss");
        netDiameter = par("netDiameter");
        nodeTraversalTime = par("nodeTraversalTime");
        rerrRatelimit = par("rerrRatelimit");
        rreqRetries = par("rreqRetries");
        rreqRatelimit = par("rreqRatelimit");
        timeoutBuffer = par("timeoutBuffer");
        ttlStart = par("ttlStart");
        ttlIncrement = par("ttlIncrement");
        ttlThreshold = par("ttlThreshold");
        localAddTTL = par("localAddTTL");
        jitterPar = &par("jitter");
        periodicJitter = &par("periodicJitter");

        myRouteTimeout = par("myRouteTimeout");
        deletePeriod = par("deletePeriod");
        blacklistTimeout = par("blacklistTimeout");
        netTraversalTime = par("netTraversalTime");
        nextHopWait = par("nextHopWait");
        pathDiscoveryTime = par("pathDiscoveryTime");
        expungeTimer = new cMessage("ExpungeTimer");
        counterTimer = new cMessage("CounterTimer");
        rrepAckTimer = new cMessage("RrepAckTimer");
        blacklistTimer = new cMessage("BlackListTimer");
        if (useHelloMessages)
            helloMsgTimer = new cMessage("HelloMsgTimer");

        minHelloInterval = par("minHelloInterval");
        maxHelloInterval = par("maxHelloInterval");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::manet, nullptr, gate("ipIn"));
        registerProtocol(Protocol::manet, gate("ipOut"), nullptr);
        networkProtocol->registerHook(0, this);
        host->subscribe(linkBrokenSignal, this);
        if (par("useDisjkstra")) {
            dijkstra = new Dijkstra();
            dijkstra->setRoot(this->getSelfIPAddress());
        }
    }
}

void LoadNg::handleMessageWhenUp(cMessage *msg)
{
    checkNeigList();
    if (msg->isSelfMessage()) {
        if (auto waitForRrep = dynamic_cast<WaitForRrep *>(msg))
            handleWaitForRREP(waitForRrep);
        else if (msg == helloMsgTimer)
            sendHelloMessagesIfNeeded();
        else if (msg == expungeTimer)
            expungeRoutes();
        else if (msg == counterTimer) {
            rreqCount = rerrCount = 0;
            scheduleAt(simTime() + 1, counterTimer);
        }
        else if (msg == rrepAckTimer)
            handleRREPACKTimer();
        else if (msg == blacklistTimer)
            handleBlackListTimer();
        else
            throw cRuntimeError("Unknown self message");
    }
    else {
        auto packet = check_and_cast<Packet *>(msg);
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();

        if (protocol == &Protocol::icmpv4) {
            IcmpHeader *icmpPacket = check_and_cast<IcmpHeader *>(msg);
            // ICMP packet arrived, dropped
            delete icmpPacket;
        }
        else if (protocol == &Protocol::icmpv6) {
            IcmpHeader *icmpPacket = check_and_cast<IcmpHeader *>(msg);
            // ICMP packet arrived, dropped
            delete icmpPacket;
        }
        else if (true) {  //FIXME protocol == ???
            Packet *udpPacket = check_and_cast<Packet *>(msg);
            //udpPacket->popAtFront<UdpHeader>();
            L3Address sourceAddr = udpPacket->getTag<L3AddressInd>()->getSrcAddress();
            // KLUDGE: I added this -1 after TTL decrement has been moved in Ipv4
            unsigned int arrivalPacketTTL = udpPacket->getTag<HopLimitInd>()->getHopLimit() - 1;
            const auto& ctrlPacket = udpPacket->popAtFront<LoadNgControlPacket>();
//            ctrlPacket->copyTags(*msg);

            switch (ctrlPacket->getPacketType()) {
                case RREQ:
                    handleRREQ(dynamicPtrCast<Rreq>(ctrlPacket->dupShared()), sourceAddr, arrivalPacketTTL);
                    break;

                case RREP:
                    handleRREP(dynamicPtrCast<Rrep>(ctrlPacket->dupShared()), sourceAddr);
                    break;

                case RERR:
                    handleRERR(dynamicPtrCast<const Rerr>(ctrlPacket), sourceAddr);
                    break;

                case RREPACK:
                    handleRREPACK(dynamicPtrCast<const RrepAck>(ctrlPacket), sourceAddr);
                    break;

                case HELLO:
                    handleHelloMessage(dynamicPtrCast<const Hello>(ctrlPacket));
                    break;

                default:
                    throw cRuntimeError("AODV Control Packet arrived with undefined packet type: %d", ctrlPacket->getPacketType());
            }
            delete udpPacket;
        }
    }
}

INetfilter::IHook::Result LoadNg::ensureRouteForDatagram(Packet *datagram)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    const L3Address& sourceAddr = networkHeader->getSourceAddress();

    if (destAddr.isBroadcast() || routingTable->isLocalAddress(destAddr) || destAddr.isMulticast())
        return ACCEPT;
    else {
        EV_INFO << "Finding route for source " << sourceAddr << " with destination " << destAddr << endl;
        IRoute *route = routingTable->findBestMatchingRoute(destAddr);
        LoadNgRouteData *routeData = route ? dynamic_cast<LoadNgRouteData *>(route->getProtocolData()) : nullptr;
        bool isActive = routeData && routeData->isActive();
        if (isActive && !route->getNextHopAsGeneric().isUnspecified()) {
            EV_INFO << "Active route found: " << route << endl;

            // Each time a route is used to forward a data packet, its Active Route
            // Lifetime field of the source, destination and the next hop on the
            // path to the destination is updated to be no less than the current
            // time plus ACTIVE_ROUTE_TIMEOUT.

            updateValidRouteLifeTime(destAddr, simTime() + activeRouteTimeout);
            updateValidRouteLifeTime(route->getNextHopAsGeneric(), simTime() + activeRouteTimeout);

            return ACCEPT;
        }
        else {
            bool isInactive = routeData && !routeData->isActive();
            // A node disseminates a RREQ when it determines that it needs a route
            // to a destination and does not have one available.  This can happen if
            // the destination is previously unknown to the node, or if a previously
            // valid route to the destination expires or is marked as invalid.

            EV_INFO << (isInactive ? "Inactive" : "Missing") << " route for destination " << destAddr << endl;

            delayDatagram(datagram);

            if (!hasOngoingRouteDiscovery(destAddr)) {
                // When a new route to the same destination is required at a later time
                // (e.g., upon route loss), the TTL in the RREQ IP header is initially
                // set to the Hop Count plus TTL_INCREMENT.
                if (isInactive)
                    startRouteDiscovery(destAddr, route->getMetric() + ttlIncrement);
                else
                    startRouteDiscovery(destAddr);
            }
            else
                EV_DETAIL << "Route discovery is in progress, originator " << getSelfIPAddress() << " target " << destAddr << endl;

            return QUEUE;
        }
    }
}

LoadNg::LoadNg()
{
}

bool LoadNg::hasOngoingRouteDiscovery(const L3Address& target)
{
    return waitForRREPTimers.find(target) != waitForRREPTimers.end();
}

void LoadNg::startRouteDiscovery(const L3Address& target, unsigned timeToLive)
{
    EV_INFO << "Starting route discovery with originator " << getSelfIPAddress() << " and destination " << target << endl;
    ASSERT(!hasOngoingRouteDiscovery(target));
    auto rreq = createRREQ(target);
    addressToRreqRetries[target] = 0;
    sendRREQ(rreq, addressType->getBroadcastAddress(), timeToLive);
}

L3Address LoadNg::getSelfIPAddress() const
{
    return routingTable->getRouterIdAsGeneric();
}

void LoadNg::delayDatagram(Packet *datagram)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    EV_DETAIL << "Queuing datagram, source " << networkHeader->getSourceAddress() << ", destination " << networkHeader->getDestinationAddress() << endl;
    const L3Address& target = networkHeader->getDestinationAddress();
    targetAddressToDelayedPackets.insert(std::pair<L3Address, Packet *>(target, datagram));
}

void LoadNg::sendRREQ(const Ptr<Rreq>& rreq, const L3Address& destAddr, unsigned int timeToLive)
{
    // In an expanding ring search, the originating node initially uses a TTL =
    // TTL_START in the RREQ packet IP header and sets the timeout for
    // receiving a RREP to RING_TRAVERSAL_TIME milliseconds.
    // RING_TRAVERSAL_TIME is calculated as described in section 10.  The
    // TTL_VALUE used in calculating RING_TRAVERSAL_TIME is set equal to the
    // value of the TTL field in the IP header.  If the RREQ times out
    // without a corresponding RREP, the originator broadcasts the RREQ
    // again with the TTL incremented by TTL_INCREMENT.  This continues
    // until the TTL set in the RREQ reaches TTL_THRESHOLD, beyond which a
    // TTL = NET_DIAMETER is used for each attempt.

    if (rreqCount >= rreqRatelimit) {
        EV_WARN << "A node should not originate more than RREQ_RATELIMIT RREQ messages per second. Canceling sending RREQ" << endl;
        return;
    }

    if (rreq->getHopLimit() == 0) {
        EV_WARN << "Hop limit 0. Canceling sending RERR" << endl;
        return;
    }


    auto rrepTimer = waitForRREPTimers.find(rreq->getDestAddr());
    WaitForRrep *rrepTimerMsg = nullptr;
    if (rrepTimer != waitForRREPTimers.end()) {
        rrepTimerMsg = rrepTimer->second;
        unsigned int lastTTL = rrepTimerMsg->getLastTTL();
        rrepTimerMsg->setDestAddr(rreq->getDestAddr());

        // The Hop Count stored in an invalid routing table entry indicates the
        // last known hop count to that destination in the routing table.  When
        // a new route to the same destination is required at a later time
        // (e.g., upon route loss), the TTL in the RREQ IP header is initially
        // set to the Hop Count plus TTL_INCREMENT.  Thereafter, following each
        // timeout the TTL is incremented by TTL_INCREMENT until TTL =
        // TTL_THRESHOLD is reached.  Beyond this TTL = NET_DIAMETER is used.
        // Once TTL = NET_DIAMETER, the timeout for waiting for the RREP is set
        // to NET_TRAVERSAL_TIME, as specified in section 6.3.

        if (timeToLive != 0) {
            rrepTimerMsg->setLastTTL(timeToLive);
            rrepTimerMsg->setFromInvalidEntry(true);
            cancelEvent(rrepTimerMsg);
        }
        else if (lastTTL + ttlIncrement < ttlThreshold) {
            ASSERT(!rrepTimerMsg->isScheduled());
            timeToLive = lastTTL + ttlIncrement;
            rrepTimerMsg->setLastTTL(lastTTL + ttlIncrement);
        }
        else {
            ASSERT(!rrepTimerMsg->isScheduled());
            timeToLive = netDiameter;
            rrepTimerMsg->setLastTTL(netDiameter);
        }
    }
    else {
        rrepTimerMsg = new WaitForRrep();
        waitForRREPTimers[rreq->getDestAddr()] = rrepTimerMsg;
        ASSERT(hasOngoingRouteDiscovery(rreq->getDestAddr()));

        timeToLive = ttlStart;
        rrepTimerMsg->setLastTTL(ttlStart);
        rrepTimerMsg->setFromInvalidEntry(false);
        rrepTimerMsg->setDestAddr(rreq->getDestAddr());
    }

    // Each time, the timeout for receiving a RREP is RING_TRAVERSAL_TIME.
    simtime_t ringTraversalTime = 2.0 * nodeTraversalTime * (timeToLive + timeoutBuffer);
    scheduleAt(simTime() + ringTraversalTime, rrepTimerMsg);

    EV_INFO << "Sending a Route Request with target " << rreq->getDestAddr() << " and TTL= " << timeToLive << endl;
    sendAODVPacket(rreq, destAddr, timeToLive, *jitterPar);
    rreqCount++;
}

void LoadNg::sendRREP(const Ptr<Rrep>& rrep, const L3Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Sending Route Reply to " << destAddr << endl;

    // When any node transmits a RREP, the precursor list for the
    // corresponding destination node is updated by adding to it
    // the next hop node to which the RREP is forwarded.

    if (rrep->getHopLimit() == 0) {
        EV_WARN << "Hop limit 0. Canceling sending RREP" << endl;
        return;
    }

    IRoute *destRoute = routingTable->findBestMatchingRoute(destAddr);
    const L3Address& nextHop = destRoute->getNextHopAsGeneric();
    //LoadNgRouteData *destRouteData = check_and_cast<LoadNgRouteData *>(destRoute->getProtocolData());

    // The node we received the Route Request for is our neighbor,
    // it is probably an unidirectional link
    if (destRoute->getMetric() == 1) {
        // It is possible that a RREP transmission may fail, especially if the
        // RREQ transmission triggering the RREP occurs over a unidirectional
        // link.

        auto ackReq = new LoadNgAckRrepReq();
        rrep->getTlvOptionsForUpdate().insertTlvOption(ackReq);
        //rrep->setAckRequiredFlag(true);
        // when a node detects that its transmission of a RREP message has failed,
        // it remembers the next-hop of the failed RREP in a "blacklist" set.

        failedNextHop = nextHop;

        if (rrepAckTimer->isScheduled())
            cancelEvent(rrepAckTimer);

        scheduleAt(simTime() + nextHopWait, rrepAckTimer);
    }
    sendAODVPacket(rrep, nextHop, timeToLive, 0);
}

const Ptr<Rreq> LoadNg::createRREQ(const L3Address& destAddr)
{
    auto rreqPacket = makeShared<Rreq>(); // TODO: "AODV-RREQ");
    //IRoute *lastKnownRoute = routingTable->findBestMatchingRoute(destAddr);

    rreqPacket->setPacketType(RREQ);

    // The Originator Sequence Number in the RREQ message is the
    // node's own sequence number, which is incremented prior to
    // insertion in a RREQ.
    sequenceNum++;

    rreqPacket->setHopLimits(par("maxHopLimit"));
    rreqPacket->setSeqNum(sequenceNum);

    rreqPacket->setOriginatorAddr(getSelfIPAddress());
    rreqPacket->setDestAddr(destAddr);

    if (rreqPacket->getOriginatorAddr().getType() == L3Address::IPv4)
        rreqPacket->setAddrLen(3);
    else if (rreqPacket->getOriginatorAddr().getType() == L3Address::IPv6)
        rreqPacket->setAddrLen(15);
    else if (rreqPacket->getOriginatorAddr().getType() == L3Address::MAC)
        rreqPacket->setAddrLen(5);

    // The Hop Count field is set to zero.
    rreqPacket->setHopCount(0);
// metric data
    auto metric = new LoadNgMetricOption();
    metric->setValue(0);
    rreqPacket->getTlvOptionsForUpdate().insertTlvOption(metric);


    // Before broadcasting the RREQ, the originating node buffers the RREQ
    // ID and the Originator IP address (its own address) of the RREQ for
    // PATH_DISCOVERY_TIME.
    // In this way, when the node receives the packet again from its neighbors,
    // it will not reprocess and re-forward the packet.

    RreqIdentifier rreqIdentifier(getSelfIPAddress(), rreqId);
    rreqsArrivalTime[rreqIdentifier] = simTime();
    if (rreqPacket->getAddrLen() == 3)
        rreqPacket->setChunkLength(B(30));
    else if (rreqPacket->getAddrLen() == 15)
        rreqPacket->setChunkLength(B(54));
    else if (rreqPacket->getAddrLen() == 5)
        rreqPacket->setChunkLength(B(34));
    return rreqPacket;
}

const Ptr<Rrep> LoadNg::createRREP(const Ptr<Rreq>& rreq, IRoute *destRoute, IRoute *originatorRoute, const L3Address& lastHopAddr)
{
    if (rreq->getDestAddr() != getSelfIPAddress())
        return Ptr<Rrep>();
    auto rrep = makeShared<Rrep>(); // TODO: "AODV-RREP");
    rrep->setPacketType(RREP);

    // When generating a RREP message, a node copies the Destination IP
    // Address and the Originator Sequence Number from the RREQ message into
    // the corresponding fields in the RREP message.

    rrep->setDestAddr(rreq->getOriginatorAddr());
    rrep->setOriginatorAddr(this->getSelfIPAddress());

    sequenceNum++;
    rrep->setSeqNum(sequenceNum);
    rrep->setHopLimits(par("maxHopLimit"));

    if (rrep->getOriginatorAddr().getType() == L3Address::IPv4)
        rrep->setAddrLen(3);
    else if (rrep->getOriginatorAddr().getType() == L3Address::IPv6)
        rrep->setAddrLen(15);
    else if (rrep->getOriginatorAddr().getType() == L3Address::MAC)
        rrep->setAddrLen(5);
    else
        throw cRuntimeError("Error size incorrect");
    // OriginatorAddr = The IP address of the node which originated the RREQ
    // for which the route is supplied.
    rrep->setHopCount(0);

    // The destination node copies the value MY_ROUTE_TIMEOUT
    // into the Lifetime field of the RREP.
   // rrep->setLifeTime(myRouteTimeout);

// include metrict.
    auto metric = new LoadNgMetricOption();
    metric->setValue(0);
    rrep->getTlvOptionsForUpdate().insertTlvOption(metric);

    if (rrep->getAddrLen() == 3)
        rrep->setChunkLength(B(34));
    else if (rrep->getAddrLen() == 15)
        rrep->setChunkLength(B(58));
    else if (rrep->getAddrLen() == 5)
        rrep->setChunkLength(B(38));
    return rrep;
}

void LoadNg::handleRREP(const Ptr<Rrep>& rrep, const L3Address& sourceAddr)
{
    EV_INFO << "AODV Route Reply arrived with source addr: " << sourceAddr << " originator addr: " << rrep->getOriginatorAddr()
            << " destination addr: " << rrep->getDestAddr() << endl;

    if (!rrep->getSeqField()) // bad format
        return;

    rrep->setHopLimits(rrep->getHopLimit()-1);

    // the hello is now a sparate type.
   /*if (rrep->getDestAddr().isUnspecified()) {
        EV_INFO << "This Route Reply is a Hello Message" << endl;
        handleHelloMessage(rrep);
        return;
    }
    */

    // When a node receives a RREP message, it searches (using longest-
    // prefix matching) for a route to the previous hop.

    // If needed, a route is created for the previous hop,
    // but without a valid sequence number (see section 6.2)

    int metricType = -1;
    unsigned int metric = 255;
    unsigned int metricNeg = 255;
    auto metricPos = rrep->getTlvOptionsForUpdate().findByType(METRIC);
    if (metricPos != -1) {
        auto metricTlv = check_and_cast<LoadNgMetricOption *>(rrep->getTlvOptionsForUpdate().getTlvOptionForUpdate(metricPos));
        if (metricTlv->getExtensionFlag())
            metricType = metricTlv->getExtension();
        if (metricTlv->getValueFlag())
            metric = metricTlv->getValue();
        if (metricType == HOPCOUNT) {
            if (metricType == HOPCOUNT) {
                if (metric < 255)
                    metric++;
                metricTlv->setValue(metric);
                metricNeg = 1;
            }
            else {
                // unknow metric set to hopcount
                metricTlv->setType(HOPCOUNT);
                metricTlv->setValue(255);
            }
            metricNeg = 1;
        }
    }
    IRoute *previousHopRoute = routingTable->findBestMatchingRoute(sourceAddr);
    if (!previousHopRoute || previousHopRoute->getSource() != this) {
        // create without valid sequence number
        previousHopRoute = createRoute(sourceAddr, sourceAddr, 1, -1, true, simTime() + activeRouteTimeout, metricType, metricNeg);
    }
    else {
        auto loadNgRouteData = check_and_cast<LoadNgRouteData *> (previousHopRoute->getProtocolData());
        updateRoutingTable(previousHopRoute, sourceAddr, 1, loadNgRouteData->getDestSeqNum(), true, simTime() + activeRouteTimeout, metricType, metric);
    }

    // Next, the node then increments the hop count value in the RREP by one,
    // to account for the new hop through the intermediate node
    unsigned int newHopCount = rrep->getHopCount() + 1;
    rrep->setHopCount(newHopCount);

    IRoute *originRoute = routingTable->findBestMatchingRoute(rrep->getOriginatorAddr());
    LoadNgRouteData *orgRouteData = nullptr;

    int64_t seqNum = rrep->getSeqNum();

    if (originRoute && originRoute->getSource() == this) {    // already exists
        orgRouteData = check_and_cast<LoadNgRouteData *>(originRoute->getProtocolData());

        // 11.1.  Identifying Invalid RREQ or RREP Messages,
        if (seqNum < orgRouteData->getDestSeqNum()) // invalid message
            return;

        orgRouteData->setIsBidirectiona(true);

        if (orgRouteData->getDestSeqNum() == -1) {
            updateRoutingTable(originRoute, sourceAddr, newHopCount, seqNum, true, simTime() + activeRouteTimeout, metricType, metric);
        }
           // Upon comparison, the existing entry is updated only in the following circumstances:
        else if (seqNum > orgRouteData->getDestSeqNum()) {
            updateRoutingTable(originRoute, sourceAddr, newHopCount, seqNum, true, simTime() + activeRouteTimeout, metricType, metric);

        }
        else {
            // (iii) the sequence numbers are the same, but the route is
            //       marked as inactive, or
            if (seqNum == orgRouteData->getDestSeqNum() && !orgRouteData->isActive()) {
                updateRoutingTable(originRoute, sourceAddr, newHopCount, seqNum, true, simTime() + activeRouteTimeout, metricType, metric);
            }
            // (iv) the sequence numbers are the same, and the New Hop Count is
            //      smaller than the hop count in route table entry.
            else if (seqNum == orgRouteData->getDestSeqNum() && metric < (unsigned int)orgRouteData->getMetric()) {
                updateRoutingTable(originRoute, sourceAddr, newHopCount, seqNum, true, simTime() + activeRouteTimeout, metricType, metric);
            }
            else if (seqNum == orgRouteData->getDestSeqNum() && metric == (unsigned int)orgRouteData->getMetric() && newHopCount < (unsigned int)originRoute->getMetric()) {
                updateRoutingTable(originRoute, sourceAddr, newHopCount, seqNum, true, simTime() + activeRouteTimeout, metricType, metric);
            }
        }
    }
    else {    // create forward route for the destination: this path will be used by the originator to send data packets
       // orgRouteData->setIsBidirectiona(true);
        originRoute = createRoute(rrep->getOriginatorAddr(), sourceAddr, newHopCount, seqNum, true, simTime() + activeRouteTimeout, metricType, metric);
     }

    // check ack req, Tlv
    auto flagPos = rrep->getTlvOptionsForUpdate().findByType(FLAGS);
    if (flagPos != -1) {
        auto flag = check_and_cast<LoadNgAckRrepReq *>(rrep->getTlvOptionsForUpdate().getTlvOptionForUpdate(flagPos));
        if (flag->getValue() == REQACK) {
            auto rrepACK = createRREPACK();
            sendRREPACK(rrepACK, sourceAddr);
            delete rrep->getTlvOptionsForUpdate().dropTlvOption(flag);
            // rrep->setAckRequiredFlag(false);
        }
    }



    if (hasOngoingRouteDiscovery(rrep->getOriginatorAddr())) {
        EV_INFO << "The Route Reply has arrived for our Route Request to node " << rrep->getOriginatorAddr() << endl;
        completeRouteDiscovery(rrep->getOriginatorAddr());
    }


    // Then the forward route for this destination is created if it does not
    // already exist.

    IRoute *destRoute = routingTable->findBestMatchingRoute(rrep->getDestAddr());
    LoadNgRouteData *destRouteData = nullptr;
    if (this->getSelfIPAddress() == rrep->getDestAddr()) {
       // nothing to do

    }
    else if (destRoute && destRoute->getSource() == this) {    // already exists
        destRouteData = check_and_cast<LoadNgRouteData *>(destRoute->getProtocolData());
        // Upon comparison, the existing entry is updated only in the following circumstances:
        updateRoutingTable(destRoute, rrep->getDestAddr(), destRoute->getMetric(), destRouteData->getDestSeqNum(), true, simTime() + activeRouteTimeout, destRouteData->getMetric(), destRouteData->getMetric());

        // When any node transmits a RREP, the precursor list for the
        // corresponding destination node is updated by adding to it
        // the next hop node to which the RREP is forwarded.

        if (rrep->getHopCount() > 0) {
            auto outgoingRREP = dynamicPtrCast<Rrep>(rrep->dupShared());
            forwardRREP(outgoingRREP, destRoute->getNextHopAsGeneric(), 100);
        }
    }
    else {    // create forward route for the destination: this path will be used by the originator to send data packets
        /** received routing message is an RREP and no routing entry was found **/
        sendRERRWhenNoRouteToForward(rrep->getOriginatorAddr());
    }
}

void LoadNg::updateRoutingTable(IRoute *route, const L3Address& nextHop, unsigned int hopCount, int64_t destSeqNum, bool isActive, simtime_t lifeTime, int metricType, unsigned int metric)
{
    EV_DETAIL << "Updating existing route: " << route << endl;

    auto it = neighbors.find(nextHop);
    if (it != neighbors.end()) {
        // include in the list
        NeigborElement elem;
        elem.lastNotification = simTime();
        if (nextHop == route->getDestinationAsGeneric()) {
            elem.seqNumber = destSeqNum;
        }
        neighbors[nextHop] = elem;
    }
    else {
        it->second.lastNotification = simTime();
        if (nextHop == route->getDestinationAsGeneric()) {
            it->second.seqNumber = destSeqNum;
         }
    }


    route->setNextHop(nextHop);
    route->setMetric(hopCount);
    LoadNgRouteData *routingData = check_and_cast<LoadNgRouteData *>(route->getProtocolData());
    ASSERT(routingData != nullptr);

    routingData->setLifeTime(lifeTime);
    routingData->setDestSeqNum(destSeqNum);
    routingData->setIsActive(isActive);

    if (metricType != -1) {
        routingData->setMetricType(metricType);
        routingData->setMetric(metric);
    }

    EV_DETAIL << "Route updated: " << route << endl;

    scheduleExpungeRoutes();
}

void LoadNg::sendAODVPacket(const Ptr<LoadNgControlPacket>& packet, const L3Address& destAddr, unsigned int timeToLive, double delay)
{
    ASSERT(timeToLive != 0);

    // TODO: Implement: support for multiple interfaces
    InterfaceEntry *ifEntry = interfaceTable->getInterfaceByName("wlan0");

    auto className = packet->getClassName();
    Packet *udpPacket = new Packet(!strncmp("inet::", className, 6) ? className + 6 : className);
    udpPacket->insertAtBack(packet);
    // TODO: was udpPacket->copyTags(*packet);
    udpPacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    udpPacket->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    udpPacket->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ifEntry->getInterfaceId());
    auto addresses = udpPacket->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(getSelfIPAddress());
    addresses->setDestAddress(destAddr);
    udpPacket->addTagIfAbsent<HopLimitReq>()->setHopLimit(timeToLive);

    if (destAddr.isBroadcast())
        lastBroadcastTime = simTime();
    if (delay == 0)
        send(udpPacket, "ipOut");
    else
        sendDelayed(udpPacket, delay, "ipOut");
}

void LoadNg::handleRREQ(const Ptr<Rreq>& rreq, const L3Address& sourceAddr, unsigned int timeToLive)
{
    EV_INFO << "AODV Route Request arrived with source addr: " << sourceAddr << " originator addr: " << rreq->getOriginatorAddr()
            << " destination addr: " << rreq->getDestAddr() << endl;

    // A node ignores all RREQs received from any node in its blacklist set.

    auto itNeig = neighbors.find(sourceAddr);
    if (itNeig == neighbors.end() || !itNeig->second.isBidirectional || itNeig->second.pendingConfirmation) {
        EV_INFO << "The sender node " << sourceAddr << " is not in the neighbors list or is not bi-directional. Ignoring the Route Request" << endl;
        return;
    }

    auto blackListIt = blacklist.find(sourceAddr);
    if (blackListIt != blacklist.end()) {
        EV_INFO << "The sender node " << sourceAddr << " is in our blacklist. Ignoring the Route Request" << endl;
        return;
    }

    // When a node receives a RREQ, it first creates or updates a route to
    // the previous hop without a valid sequence number (see section 6.2).

    int metricType = -1;
    unsigned int metric = 255;
    unsigned int metricNeg = 255;

    auto metricPos = rreq->getTlvOptionsForUpdate().findByType(METRIC);

    if (metricPos != -1) {
        auto metricTlv = check_and_cast<LoadNgMetricOption *>(rreq->getTlvOptionsForUpdate().getTlvOptionForUpdate(metricPos));
        if (metricTlv->getExtensionFlag())
            metricType = metricTlv->getExtension();
        if (metricTlv->getValueFlag())
            metric = metricTlv->getValue();
        if (metricType == HOPCOUNT) {
            if (metric < 255)
                metric++;
            metricTlv->setValue(metric);
            metricNeg = 1;
        }
        else {
            // unknow metric set to hopcount
            metricTlv->setType(HOPCOUNT);
            metricTlv->setValue(255);
        }
    }

    rreq->setHopLimits(rreq->getHopLimit()-1);

    IRoute *previousHopRoute = routingTable->findBestMatchingRoute(sourceAddr);

    if (!previousHopRoute || previousHopRoute->getSource() != this) {
        // create without valid sequence number
        previousHopRoute = createRoute(sourceAddr, sourceAddr, 1, -1, true, simTime() + activeRouteTimeout, metricType, metricNeg);
    }
    else {
        auto loadNgRouteData = check_and_cast<LoadNgRouteData *> (previousHopRoute->getProtocolData());
        updateRoutingTable(previousHopRoute, sourceAddr, 1, loadNgRouteData->getDestSeqNum(), true, simTime() + activeRouteTimeout, metricType, metricNeg);
    }

    // First, it first increments the hop count value in the RREQ by one, to
    // account for the new hop through the intermediate node.

    rreq->setHopCount(rreq->getHopCount() + 1);

    // Then the node searches for a reverse route to the Originator IP Address (see
    // section 6.2), using longest-prefix matching.

    IRoute *reverseRoute = routingTable->findBestMatchingRoute(rreq->getOriginatorAddr());

    // If need be, the route is created, or updated using the Originator Sequence Number from the
    // RREQ in its routing table.
    //
    // When the reverse route is created or updated, the following actions on
    // the route are also carried out:
    //
    //   1. the Originator Sequence Number from the RREQ is compared to the
    //      corresponding destination sequence number in the route table entry
    //      and copied if greater than the existing value there
    //
    //   2. the valid sequence number field is set to true;
    //
    //   3. the next hop in the routing table becomes the node from which the
    //      RREQ was received (it is obtained from the source IP address in
    //      the IP header and is often not equal to the Originator IP Address
    //      field in the RREQ message);
    //
    //   4. the hop count is copied from the Hop Count in the RREQ message;
    //
    //   Whenever a RREQ message is received, the Lifetime of the reverse
    //   route entry for the Originator IP address is set to be the maximum of
    //   (ExistingLifetime, MinimalLifetime), where
    //
    //   MinimalLifetime = (current time + 2*NET_TRAVERSAL_TIME - 2*HopCount*NODE_TRAVERSAL_TIME).

    unsigned int hopCount = rreq->getHopCount();
    simtime_t minimalLifeTime = simTime() + 2 * netTraversalTime - 2 * hopCount * nodeTraversalTime;
    simtime_t newLifeTime = std::max(simTime(), minimalLifeTime);
    int rreqSeqNum = rreq->getSeqNum();

    int64_t oldSeqNum = -1;

    if (!reverseRoute || reverseRoute->getSource() != this) {    // create
        // This reverse route will be needed if the node receives a RREP back to the
        // node that originated the RREQ (identified by the Originator IP Address).
        reverseRoute = createRoute(rreq->getOriginatorAddr(), sourceAddr, hopCount, rreqSeqNum, true, newLifeTime,  metricType, metric);
    }
    else {
        LoadNgRouteData *routeData = check_and_cast<LoadNgRouteData *>(reverseRoute->getProtocolData());
        int routeSeqNum = routeData->getDestSeqNum();
        oldSeqNum = routeSeqNum;

        int newSeqNum = std::max(routeSeqNum, rreqSeqNum);
        int newHopCount = rreq->getHopCount();    // Note: already incremented by 1.
        int routeHopCount = reverseRoute->getMetric();
        // The route is only updated if the new sequence number is either
        //
        //   (i)       higher than the destination sequence number in the route
        //             table, or
        //
        //   (ii)      the sequence numbers are equal, but the hop count (of the
        //             new information) plus one, is smaller than the existing hop
        //             count in the routing table, or
        //
        //   (iii)     the sequence number is unknown.

        if (rreqSeqNum > routeSeqNum ||
            (rreqSeqNum == routeSeqNum && newHopCount < routeHopCount)) {
            updateRoutingTable(reverseRoute, sourceAddr, hopCount, newSeqNum, true, newLifeTime, metricType, metric );
        }
    }

    // check 11.1. (1)
    if (rreq->getOriginatorAddr() == getSelfIPAddress())
        return;


    // check 11.1. (2)
    if (oldSeqNum >= rreq->getSeqNum())
        return;


    // A node generates a RREP if either:
    //
    // (i)       it is itself the destination, or
    //

    // check (i)
    if (rreq->getDestAddr() == getSelfIPAddress()) {
        EV_INFO << "I am the destination node for which the route was requested" << endl;

        // create RREP
        auto rrep = createRREP(rreq, nullptr, reverseRoute, sourceAddr);

        // send to the originator
        sendRREP(rrep, rreq->getOriginatorAddr(), 255);

        return;    // discard RREQ, in this case, we do not forward it.
    }


    // If a node does not generate a RREP (following the processing rules in
    // section 6.6), and if the incoming IP header has TTL larger than 1,
    // the node updates and broadcasts the RREQ to address 255.255.255.255
    // on each of its configured interfaces (see section 6.14).  To update
    // the RREQ, the TTL or hop limit field in the outgoing IP header is
    // decreased by one, and the Hop Count field in the RREQ message is
    // incremented by one, to account for the new hop through the
    // intermediate node. (!) Lastly, the Destination Sequence number for the
    // requested destination is set to the maximum of the corresponding
    // value received in the RREQ message, and the destination sequence
    // value currently maintained by the node for the requested destination.
    // However, the forwarding node MUST NOT modify its maintained value for
    // the destination sequence number, even if the value received in the
    // incoming RREQ is larger than the value currently maintained by the
    // forwarding node.

    if (rreq->getHopLimit() > 0 && (simTime() > rebootTime + deletePeriod || rebootTime == 0)) {
        auto outgoingRREQ = dynamicPtrCast<Rreq>(rreq->dupShared());
        forwardRREQ(outgoingRREQ, timeToLive);
    }
    else
        EV_WARN << "Can't forward the RREQ because of its small (<= 1) TTL: " << timeToLive << " or the AODV reboot has not completed yet" << endl;
}

IRoute *LoadNg::createRoute(const L3Address& destAddr, const L3Address& nextHop,
        unsigned int hopCount, int64_t destSeqNum,
        bool isActive, simtime_t lifeTime, int metricType, unsigned int metric)
{
    auto it = neighbors.find(nextHop);
    if (it != neighbors.end()) {
        // include in the list
        NeigborElement elem;
        elem.lastNotification = simTime();
        if (nextHop == destAddr)
            elem.seqNumber = destSeqNum;
        neighbors[nextHop] = elem;
    }
    else {
        it->second.lastNotification = simTime();
        if (nextHop == destAddr)
            it->second.seqNumber = destSeqNum;
    }

    // create a new route
    IRoute *newRoute = routingTable->createRoute();

    // adding generic fields
    newRoute->setDestination(destAddr);
    newRoute->setNextHop(nextHop);
    newRoute->setPrefixLength(addressType->getMaxPrefixLength());    // TODO:
    newRoute->setMetric(hopCount);
    InterfaceEntry *ifEntry = interfaceTable->getInterfaceByName("wlan0");    // TODO: IMPLEMENT: multiple interfaces
    if (ifEntry)
        newRoute->setInterface(ifEntry);
    newRoute->setSourceType(IRoute::MANET2);
    newRoute->setSource(this);

    // A route towards a destination that has a routing table entry
    // that is marked as valid.  Only active routes can be used to
    // forward data packets.

    // adding protocol-specific fields
    LoadNgRouteData *newProtocolData = new LoadNgRouteData();
    newProtocolData->setIsActive(isActive);
    newProtocolData->setDestSeqNum(destSeqNum);
    newProtocolData->setLifeTime(lifeTime);
    newProtocolData->setMetricType(metricType);
    newProtocolData->setMetric(metric);

    newRoute->setProtocolData(newProtocolData);

    EV_DETAIL << "Adding new route " << newRoute << endl;
    routingTable->addRoute(newRoute);

    scheduleExpungeRoutes();
    return newRoute;
}

void LoadNg::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("receiveChangeNotification");
    if (signalID == linkBrokenSignal) {
        EV_DETAIL << "Received link break signal" << endl;
        Packet *datagram = check_and_cast<Packet *>(obj);
        const auto& networkHeader = findNetworkProtocolHeader(datagram);
        if (networkHeader != nullptr) {
            L3Address unreachableAddr = networkHeader->getDestinationAddress();
            if (unreachableAddr.getAddressType() == addressType) {
                // A node initiates processing for a RERR message in three situations:
                //
                //   (i)     if it detects a link break for the next hop of an active
                //           route in its routing table while transmitting data (and
                //           route repair, if attempted, was unsuccessful), or

                // TODO: Implement: local repair

                IRoute *route = routingTable->findBestMatchingRoute(unreachableAddr);

                if (route && route->getSource() == this)
                    handleLinkBreakSendRERR(route->getNextHopAsGeneric());
            }
        }
    }
}

void LoadNg::handleLinkBreakSendRERR(const L3Address& unreachableAddr)
{
    // For case (i), the node first makes a list of unreachable destinations
    // consisting of the unreachable neighbor and any additional
    // destinations (or subnets, see section 7) in the local routing table
    // that use the unreachable neighbor as the next hop.

    // Just before transmitting the RERR, certain updates are made on the
    // routing table that may affect the destination sequence numbers for
    // the unreachable destinations.  For each one of these destinations,
    // the corresponding routing table entry is updated as follows:
    //
    // 1. The destination sequence number of this routing entry, if it
    //    exists and is valid, is incremented for cases (i) and (ii) above,
    //    and copied from the incoming RERR in case (iii) above.
    //
    // 2. The entry is invalidated by marking the route entry as invalid
    //
    // 3. The Lifetime field is updated to current time plus DELETE_PERIOD.
    //    Before this time, the entry SHOULD NOT be deleted.

    IRoute *unreachableRoute = routingTable->findBestMatchingRoute(unreachableAddr);

    if (!unreachableRoute || unreachableRoute->getSource() != this)
        return;

    std::vector<UnreachableNode> unreachableNodes;
    LoadNgRouteData *unreachableRouteData = check_and_cast<LoadNgRouteData *>(unreachableRoute->getProtocolData());

    if (unreachableRouteData->isActive()) {
        UnreachableNode node;
        node.addr = unreachableAddr;
        node.seqNum = unreachableRouteData->getDestSeqNum();
        unreachableNodes.push_back(node);
    }

    // For case (i), the node first makes a list of unreachable destinations
    // consisting of the unreachable neighbor and any additional destinations
    // (or subnets, see section 7) in the local routing table that use the
    // unreachable neighbor as the next hop.

    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);

        LoadNgRouteData *routeData = dynamic_cast<LoadNgRouteData *>(route->getProtocolData());
        if (routeData && routeData->isActive() && route->getNextHopAsGeneric() == unreachableAddr) {
            if (routeData->hasValidDestNum())
                routeData->setDestSeqNum(routeData->getDestSeqNum() + 1);

            EV_DETAIL << "Marking route to " << route->getDestinationAsGeneric() << " as inactive" << endl;

            routeData->setIsActive(false);
            routeData->setLifeTime(simTime() + deletePeriod);
            scheduleExpungeRoutes();

            UnreachableNode node;
            node.addr = route->getDestinationAsGeneric();
            node.seqNum = routeData->getDestSeqNum();
            unreachableNodes.push_back(node);
        }
    }

    // The neighboring node(s) that should receive the RERR are all those
    // that belong to a precursor list of at least one of the unreachable
    // destination(s) in the newly created RERR.  In case there is only one
    // unique neighbor that needs to receive the RERR, the RERR SHOULD be
    // unicast toward that neighbor.  Otherwise the RERR is typically sent
    // to the local broadcast address (Destination IP == 255.255.255.255,
    // TTL == 1) with the unreachable destinations, and their corresponding
    // destination sequence numbers, included in the packet.

    if (rerrCount >= rerrRatelimit) {
        EV_WARN << "A node should not generate more than RERR_RATELIMIT RERR messages per second. Canceling sending RERR" << endl;
        return;
    }

    if (unreachableNodes.empty())
        return;

    auto rerr = createRERR(unreachableNodes);
    rerrCount++;

    // broadcast
    EV_INFO << "Broadcasting Route Error message with TTL=1" << endl;
    sendAODVPacket(rerr, addressType->getBroadcastAddress(), 1, *jitterPar);
}

const Ptr<Rerr> LoadNg::createRERR(const std::vector<UnreachableNode>& unreachableNodes)
{
    auto rerr = makeShared<Rerr>(); // TODO: "AODV-RERR");
    unsigned int destCount = unreachableNodes.size();

    rerr->setPacketType(RERR);
    rerr->setUnreachableNodesArraySize(destCount);
    rerr->setHopLimits(par("maxHopLimit"));

    for (unsigned int i = 0; i < destCount; i++) {
        UnreachableNode node;
        node.addr = unreachableNodes[i].addr;
        node.seqNum = unreachableNodes[i].seqNum;
        rerr->setUnreachableNodes(i, node);
    }

    rerr->setChunkLength(B(4 + 4 * 2 * destCount));
    return rerr;
}

void LoadNg::handleRERR(const Ptr<const Rerr>& rerr, const L3Address& sourceAddr)
{
    EV_INFO << "AODV Route Error arrived with source addr: " << sourceAddr << endl;

    // A node initiates processing for a RERR message in three situations:
    // (iii)   if it receives a RERR from a neighbor for one or more
    //         active routes.
    unsigned int unreachableArraySize = rerr->getUnreachableNodesArraySize();
    std::vector<UnreachableNode> unreachableNeighbors;

    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        LoadNgRouteData *routeData = route ? dynamic_cast<LoadNgRouteData *>(route->getProtocolData()) : nullptr;

        if (!routeData)
            continue;

        // For case (iii), the list should consist of those destinations in the RERR
        // for which there exists a corresponding entry in the local routing
        // table that has the transmitter of the received RERR as the next hop.

        if (routeData->isActive() && route->getNextHopAsGeneric() == sourceAddr) {
            for (unsigned int j = 0; j < unreachableArraySize; j++) {
                if (route->getDestinationAsGeneric() == rerr->getUnreachableNodes(j).addr) {
                    // 1. The destination sequence number of this routing entry, if it
                    // exists and is valid, is incremented for cases (i) and (ii) above,
                    // ! and copied from the incoming RERR in case (iii) above.

                    routeData->setDestSeqNum(rerr->getUnreachableNodes(j).seqNum);
                    routeData->setIsActive(false);    // it means invalid, see 3. AODV Terminology p.3. in RFC 3561
                    routeData->setLifeTime(simTime() + deletePeriod);

                    // The RERR should contain those destinations that are part of
                    // the created list of unreachable destinations and have a non-empty
                    // precursor list.

                    //if (routeData->getPrecursorList().size() > 0) {
                    //    UnreachableNode node;
                    //    node.addr = route->getDestinationAsGeneric();
                    //    node.seqNum = routeData->getDestSeqNum();
                    //    unreachableNeighbors.push_back(node);
                    //}
                    scheduleExpungeRoutes();
                }
            }
        }
    }

    if (rerrCount >= rerrRatelimit) {
        EV_WARN << "A node should not generate more than RERR_RATELIMIT RERR messages per second. Canceling sending RERR" << endl;
        return;
    }

    if (unreachableNeighbors.size() > 0 && (simTime() > rebootTime + deletePeriod || rebootTime == 0)) {
        EV_INFO << "Sending RERR to inform our neighbors about link breaks." << endl;
        if (rerr->getHopLimit() > 1) {
            auto newRERR = createRERR(unreachableNeighbors);
            newRERR->setHopLimit(rerr->getHopLimit()-1);
            sendAODVPacket(newRERR, addressType->getBroadcastAddress(), 1, 0);
            rerrCount++;
        }
    }
}

void LoadNg::handleStartOperation(LifecycleOperation *operation)
{
    rebootTime = simTime();

    // RFC 5148:
    // Jitter SHOULD be applied by reducing this delay by a random amount, so that
    // the delay between consecutive transmissions of messages of the same type is
    // equal to (MESSAGE_INTERVAL - jitter), where jitter is the random value.
    if (useHelloMessages)
        scheduleAt(simTime() + helloInterval - *periodicJitter, helloMsgTimer);

    scheduleAt(simTime() + 1, counterTimer);
}

void LoadNg::handleStopOperation(LifecycleOperation *operation)
{
    clearState();
}

void LoadNg::handleCrashOperation(LifecycleOperation *operation)
{
    clearState();
}

void LoadNg::clearState()
{
    rerrCount = rreqCount = rreqId = sequenceNum = 0;
    addressToRreqRetries.clear();
    for (auto & elem : waitForRREPTimers)
        cancelAndDelete(elem.second);

    // FIXME: Drop the queued datagrams.
    //for (auto it = targetAddressToDelayedPackets.begin(); it != targetAddressToDelayedPackets.end(); it++)
    //    networkProtocol->dropQueuedDatagram(const_cast<const Packet *>(it->second));

    targetAddressToDelayedPackets.clear();

    waitForRREPTimers.clear();
    rreqsArrivalTime.clear();

    if (useHelloMessages)
        cancelEvent(helloMsgTimer);
    if (expungeTimer)
        cancelEvent(expungeTimer);
    if (counterTimer)
        cancelEvent(counterTimer);
    if (blacklistTimer)
        cancelEvent(blacklistTimer);
    if (rrepAckTimer)
        cancelEvent(rrepAckTimer);
}

void LoadNg::handleWaitForRREP(WaitForRrep *rrepTimer)
{
    EV_INFO << "We didn't get any Route Reply within RREP timeout" << endl;
    L3Address destAddr = rrepTimer->getDestAddr();

    ASSERT(addressToRreqRetries.find(destAddr) != addressToRreqRetries.end());
    if (addressToRreqRetries[destAddr] == rreqRetries) {
        cancelRouteDiscovery(destAddr);
        EV_WARN << "Re-discovery attempts for node " << destAddr << " reached RREQ_RETRIES= " << rreqRetries << " limit. Stop sending RREQ." << endl;
        return;
    }

    auto rreq = createRREQ(destAddr);

    // the node MAY try again to discover a route by broadcasting another
    // RREQ, up to a maximum of RREQ_RETRIES times at the maximum TTL value.
    if (rrepTimer->getLastTTL() == netDiameter) // netDiameter is the maximum TTL value
        addressToRreqRetries[destAddr]++;

    sendRREQ(rreq, addressType->getBroadcastAddress(), 0);
}

void LoadNg::forwardRREP(const Ptr<Rrep>& rrep, const L3Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Forwarding the Route Reply to the node " << rrep->getOriginatorAddr() << " which originated the Route Request" << endl;

    if (rrep->getHopLimit() == 0) {
        EV_WARN << "Hop limit 0. Canceling sending RREP" << endl;
        return;
    }

    IRoute *destRoute = routingTable->findBestMatchingRoute(destAddr);
    LoadNgRouteData *destRouteData = check_and_cast<LoadNgRouteData *>(destRoute->getProtocolData());

    if (!destRouteData->getIsBidirectiona() && destRoute->getNextHopAsGeneric() == destAddr) {
        // It is possible that a RREP transmission may fail, especially if the
        // RREQ transmission triggering the RREP occurs over a unidirectional
        // link.
        auto ackReq = new LoadNgAckRrepReq();
        rrep->getTlvOptionsForUpdate().insertTlvOption(ackReq);
        //rrep->setAckRequiredFlag(true);
        // when a node detects that its transmission of a RREP message has failed,
        // it remembers the next-hop of the failed RREP in a "blacklist" set.
        failedNextHop = destAddr;
        if (rrepAckTimer->isScheduled())
            cancelEvent(rrepAckTimer);
        scheduleAt(simTime() + nextHopWait, rrepAckTimer);
    }
    sendAODVPacket(rrep, destAddr, 100, *jitterPar);
}

void LoadNg::forwardRREQ(const Ptr<Rreq>& rreq, unsigned int timeToLive)
{
    EV_INFO << "Forwarding the Route Request message with TTL= " << timeToLive << endl;
    // TODO: unidirectional rreq
    if (rreq->getHopLimit() == 0) {
        EV_WARN << "Hop limit 0. Canceling sending RREQ" << endl;
        return;
    }
    sendAODVPacket(rreq, addressType->getBroadcastAddress(), timeToLive, *jitterPar);
}

void LoadNg::completeRouteDiscovery(const L3Address& target)
{
    EV_DETAIL << "Completing route discovery, originator " << getSelfIPAddress() << ", target " << target << endl;
    ASSERT(hasOngoingRouteDiscovery(target));

    auto lt = targetAddressToDelayedPackets.lower_bound(target);
    auto ut = targetAddressToDelayedPackets.upper_bound(target);

    // reinject the delayed datagrams
    for (auto it = lt; it != ut; it++) {
        Packet *datagram = it->second;
        const auto& networkHeader = getNetworkProtocolHeader(datagram);
        EV_DETAIL << "Sending queued datagram: source " << networkHeader->getSourceAddress() << ", destination " << networkHeader->getDestinationAddress() << endl;
        networkProtocol->reinjectQueuedDatagram(const_cast<const Packet *>(datagram));
    }

    // clear the multimap
    targetAddressToDelayedPackets.erase(lt, ut);

    // we have a route for the destination, thus we must cancel the WaitForRREPTimer events
    auto waitRREPIter = waitForRREPTimers.find(target);
    ASSERT(waitRREPIter != waitForRREPTimers.end());
    cancelAndDelete(waitRREPIter->second);
    waitForRREPTimers.erase(waitRREPIter);
}

void LoadNg::sendGRREP(const Ptr<Rrep>& grrep, const L3Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Sending gratuitous Route Reply to " << destAddr << endl;

    IRoute *destRoute = routingTable->findBestMatchingRoute(destAddr);
    const L3Address& nextHop = destRoute->getNextHopAsGeneric();
    if (grrep->getHopLimit() == 0) {
        EV_WARN << "Hop limit 0. Canceling sending RREQ" << endl;
        return;
    }

    sendAODVPacket(grrep, nextHop, timeToLive, 0);
}

void LoadNg::runDijkstra()
{
    dijkstra->run();
    std::map<L3Address, std::vector<L3Address>> paths;
    dijkstra->getRoutes(paths);
    for (int i = 0; routingTable->getNumRoutes(); i++) {
        IRoute *destRoute = routingTable->getRoute(i);
        //const L3Address& nextHop = destRoute->getNextHopAsGeneric();
        auto itDest = paths.find(destRoute->getDestinationAsGeneric());
        if (itDest != paths.end()) {
            // actualize
            auto neigh = itDest->second[1];
            auto itAux = neighbors.find(neigh);
            if (itAux == neighbors.end())
                throw cRuntimeError("Path error found");
            LoadNgRouteData *routingData = check_and_cast<LoadNgRouteData *>(destRoute->getProtocolData());
            simtime_t newLifeTime = routingData->getLifeTime();
            if (newLifeTime < itAux->second.lifeTime + 3 * minHelloInterval)
                newLifeTime = itAux->second.lifeTime + 3 * minHelloInterval;

            if (neigh == itDest->first) {
                updateRoutingTable(destRoute, itDest->first, 1, itAux->second.seqNumber, true, newLifeTime, HOPCOUNT, 1 );
            }
            else {
                auto itAux2 = itAux->second.listNeigbours.find(neigh);
                if (itAux2 == itAux->second.listNeigbours.end())
                    throw cRuntimeError("Path error found");
                updateRoutingTable(destRoute, neigh, 2, itAux2->second.seqNum, true, newLifeTime, HOPCOUNT, 2 );
            }
            paths.erase(itDest);
        }
        else {
            // remove
            routingTable->removeRoute(destRoute);
            // Purge all routes with the same next hop?
        }
    }
    for (const auto &elem : paths) {

        auto neigh = elem.second[1];
        auto itAux = neighbors.find(neigh);
        if (itAux == neighbors.end())
            throw cRuntimeError("Path error found");

        auto newLifeTime = itAux->second.lifeTime + 3 * minHelloInterval;
        if (neigh == elem.first) {
            createRoute(elem.first, elem.first, 1, itAux->second.seqNumber, true, newLifeTime, HOPCOUNT, 1 );
        }
        else {
            auto itAux2 = itAux->second.listNeigbours.find(neigh);
            if (itAux2 == itAux->second.listNeigbours.end())
                throw cRuntimeError("Path error found");
            createRoute(elem.first, neigh, 2, itAux2->second.seqNum, true, newLifeTime, HOPCOUNT, 2 );
        }
    }
}

void LoadNg::checkNeigList()
{
    bool recompute = false;
    for (auto it = neighbors.begin(); it != neighbors.end();) {
        if (simTime() < it->second.lifeTime + 3 * minHelloInterval) {
            // change topology,
            if (dijkstra && it->second.isBidirectional) {
                dijkstra->deleteEdge(this->getSelfIPAddress(), it->first);
                dijkstra->deleteEdge(it->first, this->getSelfIPAddress());
                for (auto elem : it->second.listNeigbours) {
                    if (elem.second.isBidirectional) {
                        dijkstra->deleteEdge(elem.first, it->first);
                        dijkstra->deleteEdge(it->first, elem.first);
                    }
                }
            }
            // delete it
            recompute = true;
            neighbors.erase(it++);
        }
        else {
            if (simTime() < it->second.lifeTime + minHelloInterval) {
                helloInterval = minHelloInterval;
                it->second.pendingConfirmation = true;
                if (!helloMsgTimer->isScheduled()) {
                    // schedule immediately
                    scheduleAt(simTime(), helloMsgTimer);
                }
                else {
                    simtime_t schduled = helloMsgTimer->getSendingTime();
                    simtime_t arrival = helloMsgTimer->getArrivalTime();
                    simtime_t interval = arrival - schduled;
                    if (interval > minHelloInterval) {
                        // Schedule immediately
                        cancelEvent(helloMsgTimer);
                        scheduleAt(simTime(), helloMsgTimer);
                    }
                }
            }
            ++it;
        }
    }
    if (recompute && dijkstra) {
        runDijkstra();
    }
}


const Ptr<Hello> LoadNg::createHelloMessage()
{
    // called a Hello message, with the RREP
    // message fields set as follows:
    //
    //    Destination IP Address         The node's IP address.
    //
    //    Destination Sequence Number    The node's latest sequence number.
    //
    //    Hop Count                      0
    //
    //    Lifetime                       ALLOWED_HELLO_LOSS *HELLO_INTERVAL

    auto helloMessage = makeShared<Hello>(); // TODO: "AODV-HelloMsg");
    helloMessage->setPacketType(HELLO);
    helloMessage->setSeqNum(sequenceNum);
    helloMessage->setOriginatorAddr(getSelfIPAddress());
    helloMessage->setHopCount(0);
    helloMessage->setHelloIdentifier(helloIdentifier++);
    helloMessage->setHopLimit(1);

    // include neighbors list


    if (!(neighbors.empty())) {
        int s = (neighbors.size()-1)*1 + std::ceil(double(neighbors.size()*2)/8.0) + 6;
        if (this->getSelfIPAddress().getType() == L3Address::IPv4)
            s += 4;
        else if (this->getSelfIPAddress().getType() == L3Address::IPv6)
            s += 16;
        else if (this->getSelfIPAddress().getType() == L3Address::MAC)
            s += 6;
        else
            throw cRuntimeError("");

        if (s < 47)
            helloMessage->setNeighAddrsArraySize(neighbors.size());
        else {
            s = 6 + (30-1)*1 + std::ceil(double(30*2)/8.0);
            helloMessage->setNeighAddrsArraySize(30);
            if (this->getSelfIPAddress().getType() == L3Address::IPv4)
                s += 4;
            else if (this->getSelfIPAddress().getType() == L3Address::IPv6)
                s += 16;
            else if (this->getSelfIPAddress().getType() == L3Address::MAC)
                s += 6;
        }
        int k = 0;
        for (auto elem : neighbors){
            NeigborData nData;
            nData.setAddr(elem.first);
            nData.setIsBidir(elem.second.isBidirectional);
            nData.setPendingConfirmation(elem.second.pendingConfirmation);
            nData.setSeqNum(elem.second.seqNumber);
            helloMessage->setNeighAddrs(k, nData);
            k++;
            if (k >= helloMessage->getNeighAddrsArraySize())
                break;
        }
        // size
        helloMessage->setChunkLength(B(34) + B(s));
    }
   // include metrict.
    auto metric = new LoadNgMetricOption();
    metric->setValue(0);
    helloMessage->getTlvOptionsForUpdate().insertTlvOption(metric);
    return helloMessage;
}

void LoadNg::sendHelloMessagesIfNeeded()
{
    ASSERT(useHelloMessages);
    // Every HELLO_INTERVAL milliseconds, the node checks whether it has
    // sent a broadcast (e.g., a RREQ or an appropriate layer 2 message)
    // within the last HELLO_INTERVAL.  If it has not, it MAY broadcast
    // a RREP with TTL = 1

    // A node SHOULD only use hello messages if it is part of an
    // active route.
    bool hasActiveRoute = false;
    return;

    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            LoadNgRouteData *routeData = check_and_cast<LoadNgRouteData *>(route->getProtocolData());
            if (routeData->isActive()) {
                hasActiveRoute = true;
                break;
            }
        }
    }

    simtime_t nextHello = simTime() + helloInterval - *periodicJitter;
    if (hasActiveRoute && (lastBroadcastTime == 0 || simTime() - lastBroadcastTime > helloInterval)) {
        EV_INFO << "It is hello time, broadcasting Hello Messages with TTL=1" << endl;
        // sequenceNum++;
        auto helloMessage = createHelloMessage();
        helloMessage->setLifetime(nextHello);
        sendAODVPacket(helloMessage, addressType->getBroadcastAddress(), 1, 0);
    }

    scheduleAt(nextHello, helloMsgTimer);
}

void LoadNg::handleHelloMessage(const Ptr<const Hello>& helloMessage)
{
    const L3Address& helloOriginatorAddr = helloMessage->getOriginatorAddr();
    auto it = neighbors.find(helloOriginatorAddr);
    bool topoChange = false;
    bool changeTimerNotification = false;
    if (it != neighbors.end()) {
        // refresh
        if (helloMessage->getSeqNum() <= it->second.seqNumber)
            return;
    }
    else {
        // add
        NeigborElement elem;
        elem.isBidirectional = false;
        neighbors[helloOriginatorAddr] = elem;
        it = neighbors.find(helloOriginatorAddr);
        topoChange = true;
        changeTimerNotification = true;
    }

    it->second.lifeTime = helloMessage->getLifetime();
    it->second.lastNotification = simTime();
    it->second.pendingConfirmation = false;
    it->second.seqNumber = helloMessage->getSeqNum();

    IRoute *routeHelloOriginator = routingTable->findBestMatchingRoute(helloOriginatorAddr);

    int metricType = -1;
    unsigned int metric = 255;
    unsigned int metricNeg = 255;
    auto metricPos = helloMessage->getTlvOptions().findByType(METRIC);

    if (metricPos != -1) {
        auto metricTlv = check_and_cast<const LoadNgMetricOption *>(helloMessage->getTlvOptions().getTlvOption(metricPos));
        if (metricTlv->getExtensionFlag())
            metricType = metricTlv->getExtension();
        if (metricTlv->getValueFlag())
            metric = metricTlv->getValue();
        if (metricType == HOPCOUNT) {
            if (metric < 255)
                metric++;
            metricNeg = 1;
        }
        else {
            // unknow metric set to hopcount
            metric = 255;
        }
    }

    std::map<L3Address, NodeStatus> nodeStatus;
    for (int k = 0 ; k < helloMessage->getNeighAddrsArraySize(); k++) {
        auto nData = helloMessage->getNeighAddrs(k);
        NodeStatus status;
        status.isBidirectional = nData.isBidir();
        status.pendingConfirmation = nData.getPendingConfirmation();
        status.seqNum = nData.getSeqNum();
        nodeStatus[nData.getAddr()] = status;
    }

    // First check nodes that are in the list, remove nodes that are not in the new list, and actualize the others
    for (auto itAux = it->second.listNeigbours.begin(); itAux != it->second.listNeigbours.end(); ) {
        auto itStatus =  nodeStatus.find(itAux->first);
        if (itStatus ==  nodeStatus.end()) {
            if (itAux->second.isBidirectional && dijkstra) {
                topoChange = true;
                dijkstra->deleteEdge(itAux->first, it->first);
                dijkstra->deleteEdge(it->first, itAux->first);
            }
            it->second.listNeigbours.erase(itAux++);
        }
        else {
            // now check change of status
            if (itAux->second.isBidirectional != itStatus->second.isBidirectional ||
                    itAux->second.pendingConfirmation != itStatus->second.pendingConfirmation) {
                // status change actualize
                topoChange = true;
                // actualize Dijkstra
                if (itStatus->second.isBidirectional && !itStatus->second.pendingConfirmation) {
                    dijkstra->addEdge(itStatus->first, itAux->first, 1.0, 0);
                    dijkstra->addEdge(itAux->first, itStatus->first, 1.0, 0);
                }
                else {
                    dijkstra->deleteEdge(itStatus->first, itAux->first);
                    dijkstra->deleteEdge(itAux->first, itStatus->first);
                }
            }
            ++itAux;

            // check if common neighbor
            auto itAux = it->second.listNeigbours.find(itStatus->first);
            if (itAux != it->second.listNeigbours.end()) {
                // check changes.
                // check if this node is also a common neighbor.
                auto addr = itAux->first;
                auto itNeigAux = neighbors.find(addr);
                if (itNeigAux != neighbors.end() && itNeigAux->second.seqNumber < itAux->second.seqNum) {
                    itNeigAux->second.pendingConfirmation = true;
                    topoChange = true;
                    changeTimerNotification = true;
                }
            }
            nodeStatus.erase(itStatus);
        }
    }

    auto itCheckBiDir = nodeStatus.find(this->getSelfIPAddress());
    if (itCheckBiDir == nodeStatus.end()) {
        if (it->second.isBidirectional && !it->second.pendingConfirmation) {
            topoChange = true;
            changeTimerNotification = true;
            dijkstra->deleteEdge( this->getSelfIPAddress(), it->first);
            dijkstra->deleteEdge(it->first,  this->getSelfIPAddress());
            it->second.isBidirectional = true;
        }
    }
    else {
        // bi-dir
        if (!it->second.isBidirectional) {
            topoChange = true;
            changeTimerNotification = true;
            dijkstra->addEdge( this->getSelfIPAddress(), it->first, 1.0, 0);
            dijkstra->addEdge(it->first,  this->getSelfIPAddress(), 1.0, 0);
            it->second.isBidirectional = true;
        }
        if (itCheckBiDir->second.pendingConfirmation) // the other node needs confirmation
            changeTimerNotification = true;
    }

    // Add new elements
    for (auto elem : nodeStatus) {
        if (elem.first == this->getSelfIPAddress())
            continue;
        // include in the list
        it->second.listNeigbours[elem.first] = elem.second;
        if (elem.second.isBidirectional && !elem.second.pendingConfirmation) {
            // configuration change
            topoChange = true;
            if (dijkstra) {
                dijkstra->addEdge(elem.first, it->first, 1.0, 0);
                dijkstra->addEdge(it->first, elem.first, 1.0, 0);
            }
        }
    }

    if (it->second.isBidirectional && !it->second.pendingConfirmation) {
        // if the link is biDirectional, delete from blacklist
        auto blackListIt = blacklist.find(it->first);
        if (blackListIt != blacklist.end())
            blacklist.erase(blackListIt);
    }

    if (topoChange && dijkstra) {
        runDijkstra();
    }

    // Whenever a node receives a Hello message from a neighbor, the node
    // SHOULD make sure that it has an active route to the neighbor, and
    // create one if necessary.  If a route already exists, then the
    // Lifetime for the route should be increased, if necessary, to be at
    // least ALLOWED_HELLO_LOSS * HELLO_INTERVAL.  The route to the
    // neighbor, if it exists, MUST subsequently contain the latest
    // Destination Sequence Number from the Hello message.  The current node
    // can now begin using this route to forward data packets.  Routes that
    // are created by hello messages and not used by any other active routes
    // will have empty precursor lists and would not trigger a RERR message
    // if the neighbor moves away and a neighbor timeout occurs.

    unsigned int latestDestSeqNum = helloMessage->getSeqNum();
    simtime_t newLifeTime = helloMessage->getLifetime() + 3 * minHelloInterval;

    if (!routeHelloOriginator || routeHelloOriginator->getSource() != this)
        createRoute(helloOriginatorAddr, helloOriginatorAddr, 1, latestDestSeqNum, true, newLifeTime, metricType, metricNeg );
    else {
        LoadNgRouteData *routeData = check_and_cast<LoadNgRouteData *>(routeHelloOriginator->getProtocolData());
        simtime_t lifeTime = routeData->getLifeTime();
        updateRoutingTable(routeHelloOriginator, helloOriginatorAddr, 1, latestDestSeqNum, true, std::max(lifeTime, newLifeTime), metricType, metricNeg);
    }

    // add the information of the hello to the routing table
    if (changeTimerNotification) {
        helloInterval = minHelloInterval;
        if (!helloMsgTimer->isScheduled()) {
            // schedule immediately
            scheduleAt(simTime(), helloMsgTimer);
        }
        else {
            simtime_t schduled = helloMsgTimer->getSendingTime();
            simtime_t arrival = helloMsgTimer->getArrivalTime();
            simtime_t interval = arrival - schduled;
            if (interval > minHelloInterval) {
                // Schedule immediately
                cancelEvent(helloMsgTimer);
                scheduleAt(simTime(), helloMsgTimer);
            }
        }
    }
    else {
        helloInterval += 2.0;
        if (helloInterval > maxHelloInterval) {
            helloInterval = maxHelloInterval;
        }
    }


    // TODO: This feature has not implemented yet.
    // A node MAY determine connectivity by listening for packets from its
    // set of neighbors.  If, within the past DELETE_PERIOD, it has received
    // a Hello message from a neighbor, and then for that neighbor does not
    // receive any packets (Hello messages or otherwise) for more than
    // ALLOWED_HELLO_LOSS * HELLO_INTERVAL milliseconds, the node SHOULD
    // assume that the link to this neighbor is currently lost.  When this
    // happens, the node SHOULD proceed as in Section 6.11.

}

void LoadNg::expungeRoutes()
{
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            LoadNgRouteData *routeData = check_and_cast<LoadNgRouteData *>(route->getProtocolData());
            ASSERT(routeData != nullptr);
            if (routeData->getLifeTime() <= simTime()) {
                if (routeData->isActive()) {
                    EV_DETAIL << "Route to " << route->getDestinationAsGeneric() << " expired and set to inactive. It will be deleted after DELETE_PERIOD time" << endl;
                    // An expired routing table entry SHOULD NOT be expunged before
                    // (current_time + DELETE_PERIOD) (see section 6.11).  Otherwise, the
                    // soft state corresponding to the route (e.g., last known hop count)
                    // will be lost.
                    routeData->setIsActive(false);
                    routeData->setLifeTime(simTime() + deletePeriod);
                }
                else {
                    // Any routing table entry waiting for a RREP SHOULD NOT be expunged
                    // before (current_time + 2 * NET_TRAVERSAL_TIME).
                    if (hasOngoingRouteDiscovery(route->getDestinationAsGeneric())) {
                        EV_DETAIL << "Route to " << route->getDestinationAsGeneric() << " expired and is inactive, but we are waiting for a RREP to this destination, so we extend its lifetime with 2 * NET_TRAVERSAL_TIME" << endl;
                        routeData->setLifeTime(simTime() + 2 * netTraversalTime);
                    }
                    else {
                        EV_DETAIL << "Route to " << route->getDestinationAsGeneric() << " expired and is inactive and we are not expecting any RREP to this destination, so we delete this route" << endl;
                        routingTable->deleteRoute(route);
                    }
                }
            }
        }
    }
    scheduleExpungeRoutes();
}

void LoadNg::scheduleExpungeRoutes()
{
    simtime_t nextExpungeTime = SimTime::getMaxTime();
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);

        if (route->getSource() == this) {
            LoadNgRouteData *routeData = check_and_cast<LoadNgRouteData *>(route->getProtocolData());
            ASSERT(routeData != nullptr);

            if (routeData->getLifeTime() < nextExpungeTime)
                nextExpungeTime = routeData->getLifeTime();
        }
    }
    if (nextExpungeTime == SimTime::getMaxTime()) {
        if (expungeTimer->isScheduled())
            cancelEvent(expungeTimer);
    }
    else {
        if (!expungeTimer->isScheduled())
            scheduleAt(nextExpungeTime, expungeTimer);
        else {
            if (expungeTimer->getArrivalTime() != nextExpungeTime) {
                cancelEvent(expungeTimer);
                scheduleAt(nextExpungeTime, expungeTimer);
            }
        }
    }
}

INetfilter::IHook::Result LoadNg::datagramForwardHook(Packet *datagram)
{
    // TODO: Implement: Actions After Reboot
    // If the node receives a data packet for some other destination, it SHOULD
    // broadcast a RERR as described in subsection 6.11 and MUST reset the waiting
    // timer to expire after current time plus DELETE_PERIOD.

    Enter_Method("datagramForwardHook");
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& destAddr = networkHeader->getDestinationAddress();
    const L3Address& sourceAddr = networkHeader->getSourceAddress();
    IRoute *ipSource = routingTable->findBestMatchingRoute(sourceAddr);

    if (destAddr.isBroadcast() || routingTable->isLocalAddress(destAddr) || destAddr.isMulticast()) {
        if (routingTable->isLocalAddress(destAddr) && ipSource && ipSource->getSource() == this)
            updateValidRouteLifeTime(ipSource->getNextHopAsGeneric(), simTime() + activeRouteTimeout);

        return ACCEPT;
    }

    // TODO: IMPLEMENT: check if the datagram is a data packet or we take control packets as data packets

    IRoute *routeDest = routingTable->findBestMatchingRoute(destAddr);
    LoadNgRouteData *routeDestData = routeDest ? dynamic_cast<LoadNgRouteData *>(routeDest->getProtocolData()) : nullptr;

    // Each time a route is used to forward a data packet, its Active Route
    // Lifetime field of the source, destination and the next hop on the
    // path to the destination is updated to be no less than the current
    // time plus ACTIVE_ROUTE_TIMEOUT

    updateValidRouteLifeTime(sourceAddr, simTime() + activeRouteTimeout);
    updateValidRouteLifeTime(destAddr, simTime() + activeRouteTimeout);

    if (routeDest && routeDest->getSource() == this)
        updateValidRouteLifeTime(routeDest->getNextHopAsGeneric(), simTime() + activeRouteTimeout);

    // Since the route between each originator and destination pair is expected
    // to be symmetric, the Active Route Lifetime for the previous hop, along the
    // reverse path back to the IP source, is also updated to be no less than the
    // current time plus ACTIVE_ROUTE_TIMEOUT.

    if (ipSource && ipSource->getSource() == this)
        updateValidRouteLifeTime(ipSource->getNextHopAsGeneric(), simTime() + activeRouteTimeout);

    EV_INFO << "We can't forward datagram because we have no active route for " << destAddr << endl;
    if (routeDest && routeDestData && !routeDestData->isActive()) {    // exists but is not active
        // A node initiates processing for a RERR message in three situations:
        // (ii)      if it gets a data packet destined to a node for which it
        //           does not have an active route and is not repairing (if
        //           using local repair)

        // TODO: check if it is not repairing (if using local repair)

        // 1. The destination sequence number of this routing entry, if it
        // exists and is valid, is incremented for cases (i) and (ii) above,
        // and copied from the incoming RERR in case (iii) above.

        if (routeDestData->hasValidDestNum())
            routeDestData->setDestSeqNum(routeDestData->getDestSeqNum() + 1);

        // 2. The entry is invalidated by marking the route entry as invalid <- it is invalid

        // 3. The Lifetime field is updated to current time plus DELETE_PERIOD.
        //    Before this time, the entry SHOULD NOT be deleted.
        routeDestData->setLifeTime(simTime() + deletePeriod);

        sendRERRWhenNoRouteToForward(destAddr);
    }
    else if (!routeDest || routeDest->getSource() != this) // doesn't exist at all
        sendRERRWhenNoRouteToForward(destAddr);

    return ACCEPT;
}

void LoadNg::sendRERRWhenNoRouteToForward(const L3Address& unreachableAddr)
{
    if (rerrCount >= rerrRatelimit) {
        EV_WARN << "A node should not generate more than RERR_RATELIMIT RERR messages per second. Canceling sending RERR" << endl;
        return;
    }

    std::vector<UnreachableNode> unreachableNodes;
    UnreachableNode node;
    node.addr = unreachableAddr;

    IRoute *unreachableRoute = routingTable->findBestMatchingRoute(unreachableAddr);
    LoadNgRouteData *unreachableRouteData = unreachableRoute ? dynamic_cast<LoadNgRouteData *>(unreachableRoute->getProtocolData()) : nullptr;

    if (unreachableRouteData && unreachableRouteData->hasValidDestNum())
        node.seqNum = unreachableRouteData->getDestSeqNum();
    else
        node.seqNum = 0;

    unreachableNodes.push_back(node);
    auto rerr = createRERR(unreachableNodes);

    rerrCount++;
    EV_INFO << "Broadcasting Route Error message with TTL=1" << endl;
    sendAODVPacket(rerr, addressType->getBroadcastAddress(), 1, *jitterPar);    // TODO: unicast if there exists a route to the source
}

void LoadNg::cancelRouteDiscovery(const L3Address& destAddr)
{
    ASSERT(hasOngoingRouteDiscovery(destAddr));
    auto lt = targetAddressToDelayedPackets.lower_bound(destAddr);
    auto ut = targetAddressToDelayedPackets.upper_bound(destAddr);
    for (auto it = lt; it != ut; it++)
        networkProtocol->dropQueuedDatagram(const_cast<const Packet *>(it->second));

    targetAddressToDelayedPackets.erase(lt, ut);

    auto waitRREPIter = waitForRREPTimers.find(destAddr);
    ASSERT(waitRREPIter != waitForRREPTimers.end());
    cancelAndDelete(waitRREPIter->second);
    waitForRREPTimers.erase(waitRREPIter);
}

bool LoadNg::updateValidRouteLifeTime(const L3Address& destAddr, simtime_t lifetime)
{
    IRoute *route = routingTable->findBestMatchingRoute(destAddr);
    if (route && route->getSource() == this) {
        LoadNgRouteData *routeData = check_and_cast<LoadNgRouteData *>(route->getProtocolData());
        if (routeData->isActive()) {
            simtime_t newLifeTime = std::max(routeData->getLifeTime(), lifetime);
            EV_DETAIL << "Updating " << route << " lifetime to " << newLifeTime << endl;
            routeData->setLifeTime(newLifeTime);
            return true;
        }
    }
    return false;
}

const Ptr<RrepAck> LoadNg::createRREPACK()
{
    auto rrepACK = makeShared<RrepAck>(); // TODO: "AODV-RREPACK");
    rrepACK->setPacketType(RREPACK);
    rrepACK->setChunkLength(B(2));
    return rrepACK;
}

void LoadNg::sendRREPACK(const Ptr<RrepAck>& rrepACK, const L3Address& destAddr)
{
    EV_INFO << "Sending Route Reply ACK to " << destAddr << endl;
    sendAODVPacket(rrepACK, destAddr, 100, 0);
}

void LoadNg::handleRREPACK(const Ptr<const RrepAck>& rrepACK, const L3Address& neighborAddr)
{
    // Note that the RREP-ACK packet does not contain any information about
    // which RREP it is acknowledging.  The time at which the RREP-ACK is
    // received will likely come just after the time when the RREP was sent
    // with the 'A' bit.
    if (rrepAckTimer->isScheduled()) {
        EV_INFO << "RREP-ACK arrived from " << neighborAddr << endl;

        IRoute *route = routingTable->findBestMatchingRoute(neighborAddr);
        if (route && route->getSource() == this) {
            EV_DETAIL << "Marking route " << route << " as active" << endl;
            LoadNgRouteData *routeData = check_and_cast<LoadNgRouteData *>(route->getProtocolData());
            routeData->setIsActive(true);
            routeData->setIsBidirectiona(true);
            cancelEvent(rrepAckTimer);
        }
    }
}

void LoadNg::handleRREPACKTimer()
{
    // when a node detects that its transmission of a RREP message has failed,
    // it remembers the next-hop of the failed RREP in a "blacklist" set.

    EV_INFO << "RREP-ACK didn't arrived within timeout. Adding " << failedNextHop << " to the blacklist" << endl;

    blacklist[failedNextHop] = simTime() + blacklistTimeout;    // lifetime

    if (!blacklistTimer->isScheduled())
        scheduleAt(simTime() + blacklistTimeout, blacklistTimer);
}

void LoadNg::handleBlackListTimer()
{
    simtime_t nextTime = SimTime::getMaxTime();

    for (auto it = blacklist.begin(); it != blacklist.end(); ) {
        auto current = it++;

        // Nodes are removed from the blacklist set after a BLACKLIST_TIMEOUT period
        if (current->second <= simTime()) {
            EV_DETAIL << "Blacklist lifetime has expired for " << current->first << " removing it from the blacklisted addresses" << endl;
            blacklist.erase(current);
        }
        else if (nextTime > current->second)
            nextTime = current->second;
    }

    if (nextTime != SimTime::getMaxTime())
        scheduleAt(nextTime, blacklistTimer);
}

LoadNg::~LoadNg()
{
    neighbors.clear();
    if (dijkstra)
        delete dijkstra;
    clearState();
    delete helloMsgTimer;
    delete expungeTimer;
    delete counterTimer;
    delete rrepAckTimer;
    delete blacklistTimer;
}

} // namespace aodv
} // namespace inet

