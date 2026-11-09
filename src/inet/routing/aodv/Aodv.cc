//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/aodv/Aodv.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpCommand_m.h"

#ifdef INET_WITH_IPv6
#include "inet/networklayer/icmpv6/Ipv6NdMessage_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef INET_WITH_IPv6

namespace inet {
namespace aodv {

Define_Module(Aodv);

const int KIND_DELAYEDSEND = 100;
const int KIND_SEARCH = 101;

void Aodv::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        lastBroadcastTime = SIMTIME_ZERO;
        rebootTime = SIMTIME_ZERO;
        rreqId = sequenceNum = 0;
        rreqCount = rerrCount = 0;
        host = getContainingNode(this);
        routingTable.reference(this, "routingTableModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        networkProtocol.reference(this, "networkProtocolModule", true);

        for (auto name : {"ttlStart", "ttlThreshold", "netDiameter", "ttlIncrement", "rreqRatelimit", "rerrRatelimit", "allowedHelloLoss"})
            if (par(name).intValue() <= 0 || par(name).intValue() > UINT32_MAX)
                throw cRuntimeError("AODV %s must be a positive uint32 integer", name);
        for (auto name : {"rreqRetries", "timeoutBuffer", "localAddTTL"})
            if (par(name).intValue() < 0 || par(name).intValue() > UINT32_MAX)
                throw cRuntimeError("AODV %s must be a nonnegative uint32 integer", name);
        if (par("ttlStart").intValue() > par("ttlThreshold").intValue() ||
                par("ttlThreshold").intValue() > par("netDiameter").intValue() ||
                par("netDiameter").intValue() > 255 || par("ttlIncrement").intValue() > 255)
            throw cRuntimeError("AODV TTL parameters require 1 <= ttlStart <= ttlThreshold <= netDiameter <= 255 and ttlIncrement <= 255");
        if (par("udpPort").intValue() < 1 || par("udpPort").intValue() > 65535)
            throw cRuntimeError("AODV udpPort must be in 1..65535");
        double maximumTime = SimTime::getMaxTime().dbl();
        for (auto name : {"nodeTraversalTime", "activeRouteTimeout", "helloInterval", "deletePeriod", "pathDiscoveryTime", "nextHopWait", "netTraversalTime", "myRouteTimeout", "blacklistTimeout"}) {
            double duration = par(name).doubleValue();
            if (!std::isfinite(duration) || duration <= 0 || duration > maximumTime)
                throw cRuntimeError("AODV %s must be positive and representable in simulation time", name);
        }
        double jitterBound = par("maxJitter").doubleValue();
        double periodicBound = par("maxPeriodicJitter").doubleValue();
        if (!std::isfinite(jitterBound) || jitterBound < 0 || jitterBound > maximumTime)
            throw cRuntimeError("AODV maxJitter must be nonnegative and representable in simulation time");
        if (!std::isfinite(periodicBound) || periodicBound < 0 || periodicBound > par("helloInterval").doubleValue() / 2)
            throw cRuntimeError("AODV maxPeriodicJitter must be between zero and helloInterval/2");
        double rings = 1 + std::floor((double(par("ttlThreshold").intValue()) - 1) / double(par("ttlIncrement").intValue()));
        double retries = double(par("rreqRetries").intValue());
        double traversal = par("nodeTraversalTime").doubleValue();
        double buffer = double(par("timeoutBuffer").intValue());
        double searchBound = std::max(2 * traversal * (double(par("netDiameter").intValue()) + buffer),
            rings * 2 * traversal * (double(par("ttlThreshold").intValue()) + buffer)) +
            par("netTraversalTime").doubleValue() * (std::pow(2, retries + 1) - 1) +
            (rings + retries + 1) * jitterBound;
        if (!std::isfinite(searchBound) || searchBound > maximumTime)
            throw cRuntimeError("AODV rreqRetries/search durations exceed representable simulation time");

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
        maxJitter = par("maxJitter");
        jitterPar = &par("jitter");
        periodicJitter = &par("periodicJitter");
        hasExternalGateway = !par("gatewayAddress").stdstringValue().empty();

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
        if (quantizeAdvertisementLifetime(myRouteTimeout) == SIMTIME_ZERO)
            throw cRuntimeError("AODV myRouteTimeout must fit 1..UINT32_MAX milliseconds");
        if (helloInterval.raw() > SimTime::getMaxTime().raw() / allowedHelloLoss ||
                quantizeAdvertisementLifetime(allowedHelloLoss * helloInterval) == SIMTIME_ZERO)
            throw cRuntimeError("AODV allowedHelloLoss * helloInterval must fit 1..UINT32_MAX milliseconds");
        if (pathDiscoveryTime > SimTime::getMaxTime() / 2 || myRouteTimeout < 2 * pathDiscoveryTime)
            throw cRuntimeError("AODV myRouteTimeout must be at least 2 * pathDiscoveryTime");
        if (useHelloMessages && activeRouteTimeout <= allowedHelloLoss * helloInterval)
            throw cRuntimeError("AODV activeRouteTimeout must exceed allowedHelloLoss * helloInterval when Hello is enabled");
        if (deletePeriod < activeRouteTimeout || (useHelloMessages && deletePeriod < allowedHelloLoss * helloInterval))
            throw cRuntimeError("AODV deletePeriod must cover activeRouteTimeout and the enabled Hello-loss interval");
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(usingIpv6);
        WATCH(hasExternalGateway);
        WATCH(rreqId);
        WATCH(sequenceNum);
        WATCH(rreqCount);
        WATCH(rerrCount);
        WATCH(lastBroadcastTime);
        WATCH(rebootTime);
        WATCH(failedNextHop);
        WATCH_EXPR("numRoutes", routingTable->getNumRoutes());
    }
    else if (stage == INITSTAGE_ROUTER_ID_ASSIGNMENT) {
        interface = interfaceTable->findInterfaceByName(par("interface"));
        if (!interface)
            throw cRuntimeError("Unknown network interface in parameter 'interface': '%s'", par("interface").stringValue());
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        networkProtocol->registerHook(0, this);
        host->subscribe(linkBrokenSignal, this);
    }
}

void Aodv::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg->getKind() == KIND_SEARCH)
            handleSearchTimer(msg);
        else if (msg == helloMsgTimer)
            sendHelloMessagesIfNeeded();
        else if (msg == expungeTimer)
            expungeRoutes();
        else if (msg == counterTimer) {
            refreshRateWindow();
            releaseEligiblePackets();
            drainReadySearches();
            scheduleAt(rateEpoch + SimTime(rateWindow + 1), counterTimer);
        }
        else if (msg == rrepAckTimer)
            handleRREPACKTimer();
        else if (msg == blacklistTimer)
            handleBlackListTimer();
        else if (msg->getKind() == KIND_DELAYEDSEND) {
            auto timer = check_and_cast<PacketHolderMessage *>(msg);
            socket.send(timer->removeOwnedPacket());
            delete timer;
        }
        else
            throw cRuntimeError("Unknown self message");
    }
    else
        socket.processMessage(msg);
}

void Aodv::checkIpVersionAndPacketTypeCompatibility(AodvControlPacketType packetType)
{
    switch (packetType) {
        case RREQ:
        case RREP:
        case RERR:
        case RREPACK:
            if (usingIpv6)
                throw cRuntimeError("AODV Control Packet arrived with non-IPv6 packet type %d, but AODV configured for IPv6 routing", packetType);
            break;

        case RREQ_IPv6:
        case RREP_IPv6:
        case RERR_IPv6:
        case RREPACK_IPv6:
            if (!usingIpv6)
                throw cRuntimeError("AODV Control Packet arrived with IPv6 packet type %d, but AODV configured for non-IPv6 routing", packetType);
            break;

        default:
            throw cRuntimeError("AODV Control Packet arrived with undefined packet type: %d", packetType);
    }
}

void Aodv::processPacket(Packet *packet)
{
    L3Address sourceAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
    // KLUDGE I added this -1 after TTL decrement has been moved in Ipv4
    unsigned int arrivalPacketTTL = packet->getTag<HopLimitInd>()->getHopLimit() - 1;
    const auto& aodvPacket = packet->popAtFront<AodvControlPacket>();
    // TODO aodvPacket->copyTags(*udpPacket);

    auto packetType = aodvPacket->getPacketType();
    switch (packetType) {
        case RREQ:
        case RREQ_IPv6:
            checkIpVersionAndPacketTypeCompatibility(packetType);
            handleRREQ(CHK(dynamicPtrCast<Rreq>(aodvPacket->dupShared())), sourceAddr, arrivalPacketTTL);
            delete packet;
            return;

        case RREP:
        case RREP_IPv6:
            checkIpVersionAndPacketTypeCompatibility(packetType);
            handleRREP(CHK(dynamicPtrCast<Rrep>(aodvPacket->dupShared())), sourceAddr);
            delete packet;
            return;

        case RERR:
        case RERR_IPv6:
            checkIpVersionAndPacketTypeCompatibility(packetType);
            handleRERR(CHK(dynamicPtrCast<const Rerr>(aodvPacket)), sourceAddr);
            delete packet;
            return;

        case RREPACK:
        case RREPACK_IPv6:
            checkIpVersionAndPacketTypeCompatibility(packetType);
            handleRREPACK(CHK(dynamicPtrCast<const RrepAck>(aodvPacket)), sourceAddr);
            delete packet;
            return;

        default:
            throw cRuntimeError("AODV Control Packet arrived with undefined packet type: %d", packetType);
    }
}

// Dissect an inspection copy: IP owns the original, including while queued.
// Stopping at UDP supports a configured AODV port without changing global demux.
bool Aodv::isControlPacket(const Packet *packet) const
{
    class UdpFinder : public PacketDissector::ChunkFinder {
      public:
        UdpFinder() : ChunkFinder(&Protocol::udp) {}
        bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return protocol != &Protocol::udp; }
    } finder;
    std::unique_ptr<Packet> inspection(packet->dup());
    try {
#ifdef INET_WITH_IPv6
        // ND configures a directly attached link before an AODV route exists.
        // In particular, queuing a unicast NA behind discovery deadlocks ND.
        // Inspect the typed header before generic transport dissection. Like
        // reserved AODV UDP traffic, incomplete link control is never data demand.
        if (usingIpv6) {
            auto header = dynamicPtrCast<const Ipv6Header>(findNetworkProtocolHeader(inspection.get()));
            if (header && header->getProtocolId() == IP_PROT_IPv6_ICMP &&
                    inspection->peekDataAt<Ipv6NdMessage>(header->getChunkLength(), b(-1), Chunk::PF_ALLOW_NULLPTR | Chunk::PF_ALLOW_INCOMPLETE))
                return true;
        }
#endif
        PacketDissector(ProtocolDissectorRegistry::getInstance(), finder).dissectPacket(inspection.get());
        auto datagram = finder.getChunk();
        if (!datagram)
            return false;
        auto udp = datagram->peek<UdpHeader>(Chunk::ForwardIterator(B(0)), B(8));
        if (udp->getDestPort() != aodvUDPPort || udp->getSrcPort() != aodvUDPPort)
            return false;
        auto control = datagram->peek<AodvControlPacket>(Chunk::ForwardIterator(udp->getChunkLength()), b(-1), Chunk::PF_ALLOW_NULLPTR);
        if (!control)
            return true; // reserved control datagrams must never start a data search
        auto type = control->getPacketType();
        return usingIpv6 ? type >= RREQ_IPv6 && type <= RREPACK_IPv6 : type >= RREQ && type <= RREPACK;
    }
    catch (const cRuntimeError&) {
        return false;
    }
}

IRoute *Aodv::selectDataRoute(const L3Address& destination)
{
    if (defaultGatewayRoute)
        updateGatewayRoute();
    int blockedPrefix = -1;
    // Both supported IP tables enumerate routes in their native lookup order.
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        auto route = routingTable->getRoute(i);
        if (!destination.matches(route->getDestinationAsGeneric(), route->getPrefixLength()))
            continue;
        auto iface = route->getInterface();
        if (!iface || !iface->isUp())
            continue;
        if (route->getSource() != this) {
            // A connected MANET subnet describes address scope, not reachability.
            auto type = route->getSourceType();
            if (iface == interface && (type == IRoute::IFACENETMASK ||
                    type == IRoute::OWN_ADV_PREFIX || type == IRoute::ROUTER_ADVERTISEMENT))
                continue;
            return route;
        }
        expireRouteIfDue(route);
        auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
        if (!data->isActive()) {
            if (data->hasValidDestNum())
                blockedPrefix = std::max(blockedPrefix, route->getPrefixLength());
            continue;
        }
        if (route->getPrefixLength() < blockedPrefix || route->getNextHopAsGeneric().isUnspecified())
            continue;
        return route;
    }
    return nullptr;
}

bool Aodv::prepareDataPacket(Packet *packet, IRoute *route)
{
    if (!route)
        return false;
    auto destination = getNetworkProtocolHeader(packet)->getDestinationAddress();
    auto nextHop = route->getNextHopAsGeneric();
    if (nextHop.isUnspecified())
        nextHop = destination;
    auto iface = packet->findTag<InterfaceReq>();
    auto next = packet->findTag<NextHopAddressReq>();
    if ((iface && iface->getInterfaceId() != -1 && iface->getInterfaceId() != route->getInterface()->getInterfaceId()) ||
            (next && !next->getNextHopAddress().isUnspecified() && next->getNextHopAddress() != nextHop))
        return false;
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(route->getInterface()->getInterfaceId());
    packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(nextHop);
    if (route->getSource() == this) {
        auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
        data->setLifeTime(std::max(data->getLifeTime(), simTime() + activeRouteTimeout));
        updateValidRouteLifeTime(nextHop, simTime() + activeRouteTimeout);
        if (route == defaultGatewayRoute)
            updateValidRouteLifeTime(gatewayAddress, simTime() + activeRouteTimeout);
        scheduleExpungeRoutes();
    }
    return true;
}

INetfilter::IHook::Result Aodv::ensureRouteForDatagram(Packet *datagram, bool local)
{
    const auto& header = getNetworkProtocolHeader(datagram);
    auto destination = header->getDestinationAddress();
    if (destination.isBroadcast() || destination.isMulticast() || routingTable->isLocalAddress(destination) ||
            isControlPacket(datagram))
        return ACCEPT;

    L3Address target = destination;
    if (hasExternalGateway && isExternalAddress(destination)) {
        if (gatewayAddress.isUnspecified())
            gatewayAddress = L3AddressResolver().resolve(par("gatewayAddress"));
        if (getSelfIPAddress() == gatewayAddress)
            return ACCEPT;
        if (!defaultGatewayRoute)
            createDefaultGatewayRoute();
        target = gatewayAddress;
    }
    releaseEligiblePackets(); // an older FIFO must not be overtaken by this arrival
    if (auto route = selectDataRoute(destination))
        return prepareDataPacket(datagram, route) ? ACCEPT : DROP;
    if (!local) {
        sendRERRWhenNoRouteToForward(destination);
        return DROP;
    }
    delayDatagram(datagram);
    queueOwners[destination] = target;
    if (!hasOngoingRouteDiscovery(target)) {
        auto retained = findExactRoute(target);
        startRouteDiscovery(target, retained ? std::min(netDiameter, unsigned(retained->getMetric()) + ttlIncrement) : ttlStart);
    }
    return QUEUE;
}

Aodv::Aodv()
{
}

bool Aodv::hasOngoingRouteDiscovery(const L3Address& target)
{
    return searches.find(target) != searches.end();
}

void Aodv::startRouteDiscovery(const L3Address& target, unsigned timeToLive)
{
    ASSERT(!hasOngoingRouteDiscovery(target));
    auto& search = searches[target];
    search.timer = new cMessage("aodv-search", KIND_SEARCH);
    search.timer->setContextPointer(const_cast<L3Address *>(&searches.find(target)->first));
    search.ttl = timeToLive ? std::min(timeToLive, netDiameter) : ttlStart;
    scheduleSearch(target);
}

void Aodv::scheduleSearch(const L3Address& target)
{
    auto& search = searches.at(target);
    search.phase = SearchPhase::SEND_DELAY;
    simtime_t jitter = *jitterPar;
    if (jitter < SIMTIME_ZERO || jitter > maxJitter)
        throw cRuntimeError("AODV jitter must be between zero and maxJitter");
    scheduleAfter(jitter, search.timer);
}

void Aodv::refreshRateWindow()
{
    int64_t window = (simTime() - rateEpoch).inUnit(SIMTIME_S);
    if (window > rateWindow) {
        rateWindow = window;
        rreqCount = rerrCount = 0;
    }
}

void Aodv::handleSearchTimer(cMessage *timer)
{
    auto target = *static_cast<L3Address *>(timer->getContextPointer());
    releaseEligiblePackets();
    auto it = searches.find(target);
    if (it == searches.end())
        return;
    auto& search = it->second;
    if (search.phase == SearchPhase::WAIT_REPLY) {
        if (search.ttl == netDiameter && search.networkAttempts >= rreqRetries + 1) {
            cancelRouteDiscovery(target);
            return;
        }
        search.ttl = search.ttl + ttlIncrement > ttlThreshold ? netDiameter : search.ttl + ttlIncrement;
        scheduleSearch(target);
    }
    else {
        search.phase = SearchPhase::READY;
        readySearches.push_back(target);
        drainReadySearches();
    }
}

void Aodv::drainReadySearches()
{
    refreshRateWindow();
    while (!readySearches.empty() && rreqCount < rreqRatelimit) {
        auto target = readySearches.front();
        readySearches.pop_front();
        auto it = searches.find(target);
        if (it == searches.end() || it->second.phase != SearchPhase::READY)
            continue;
        auto& search = it->second;
        auto request = createRREQ(target); // IDs and sequence numbers belong to actual sends
        rreqCount++;
        simtime_t wait = 2 * nodeTraversalTime * (double(search.ttl) + timeoutBuffer);
        if (search.ttl == netDiameter) {
            wait = netTraversalTime * std::pow(2.0, search.networkAttempts);
            search.networkAttempts++;
        }
        search.phase = SearchPhase::WAIT_REPLY;
        scheduleAfter(wait, search.timer);
        sendAODVPacket(request, addressType->getBroadcastAddress(), search.ttl, 0);
    }
}

void Aodv::eraseSearch(const L3Address& target)
{
    auto it = searches.find(target);
    if (it != searches.end()) {
        cancelAndDelete(it->second.timer);
        searches.erase(it);
    }
    readySearches.erase(std::remove(readySearches.begin(), readySearches.end(), target), readySearches.end());
}

void Aodv::releaseEligiblePackets()
{
    std::vector<std::pair<L3Address, std::deque<Packet *>>> released;
    for (auto it = delayedPackets.begin(); it != delayedPackets.end();) {
        if (selectDataRoute(it->first)) {
            released.emplace_back(it->first, std::move(it->second));
            queueOwners.erase(it->first);
            it = delayedPackets.erase(it);
        }
        else
            ++it;
    }
    std::vector<L3Address> completed;
    for (const auto& search : searches) {
        bool ownsQueue = false;
        for (const auto& owner : queueOwners)
            ownsQueue |= owner.second == search.first;
        if (!ownsQueue)
            completed.push_back(search.first);
    }
    for (const auto& target : completed)
        eraseSearch(target);
    // Detach every owner before calling IP: reinjection may reenter this module.
    for (const auto& fifo : released)
        for (auto packet : fifo.second) {
            if (operationalState == OPERATING && prepareDataPacket(packet, selectDataRoute(fifo.first)))
                networkProtocol->reinjectQueuedDatagram(packet);
            else
                networkProtocol->dropQueuedDatagram(packet);
        }
}

L3Address Aodv::getSelfIPAddress() const
{
    return interface->getNetworkAddress();
}

void Aodv::delayDatagram(Packet *datagram)
{
    auto destination = getNetworkProtocolHeader(datagram)->getDestinationAddress();
    delayedPackets[destination].push_back(datagram);
}

void Aodv::sendRREP(const Ptr<Rrep>& rrep, const L3Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Sending Route Reply to " << destAddr << endl;

    // When any node transmits a RREP, the precursor list for the
    // corresponding destination node is updated by adding to it
    // the next hop node to which the RREP is forwarded.

    IRoute *destRoute = routingTable->findBestMatchingRoute(destAddr);
    const L3Address& nextHop = destRoute->getNextHopAsGeneric();
    AodvRouteData *destRouteData = check_and_cast<AodvRouteData *>(destRoute->getProtocolData());
    destRouteData->addPrecursor(nextHop);

    // The node we received the Route Request for is our neighbor,
    // it is probably an unidirectional link
    if (destRoute->getMetric() == 1) {
        // It is possible that a RREP transmission may fail, especially if the
        // RREQ transmission triggering the RREP occurs over a unidirectional
        // link.

        rrep->setAckRequiredFlag(true);

        // when a node detects that its transmission of a RREP message has failed,
        // it remembers the next-hop of the failed RREP in a "blacklist" set.

        failedNextHop = nextHop;

        rescheduleAfter(nextHopWait, rrepAckTimer);
    }
    sendAODVPacket(rrep, nextHop, timeToLive, 0);
}

const Ptr<Rreq> Aodv::createRREQ(const L3Address& destAddr)
{
    auto rreqPacket = makeShared<Rreq>(); // TODO "AODV-RREQ");
    rreqPacket->setPacketType(usingIpv6 ? RREQ_IPv6 : RREQ);
    rreqPacket->setChunkLength(usingIpv6 ? B(48) : B(24));

    rreqPacket->setGratuitousRREPFlag(askGratuitousRREP);
    IRoute *lastKnownRoute = findExactRoute(destAddr);

    // The Originator Sequence Number in the RREQ message is the
    // node's own sequence number, which is incremented prior to
    // insertion in a RREQ.
    sequenceNum++;

    rreqPacket->setOriginatorSeqNum(sequenceNum);

    if (lastKnownRoute && lastKnownRoute->getSource() == this) {
        // The Destination Sequence Number field in the RREQ message is the last
        // known destination sequence number for this destination and is copied
        // from the Destination Sequence Number field in the routing table.

        AodvRouteData *routeData = check_and_cast<AodvRouteData *>(lastKnownRoute->getProtocolData());
        if (routeData && routeData->hasValidDestNum()) {
            rreqPacket->setDestSeqNum(routeData->getDestSeqNum());
            rreqPacket->setUnknownSeqNumFlag(false);
        }
        else
            rreqPacket->setUnknownSeqNumFlag(true);
    }
    else
        rreqPacket->setUnknownSeqNumFlag(true); // If no sequence number is known, the unknown sequence number flag MUST be set.

    rreqPacket->setOriginatorAddr(getSelfIPAddress());
    rreqPacket->setDestAddr(destAddr);

    // The RREQ ID field is incremented by one from the last RREQ ID used
    // by the current node. Each node maintains only one RREQ ID.
    rreqId++;
    rreqPacket->setRreqId(rreqId);

    // The Hop Count field is set to zero.
    rreqPacket->setHopCount(0);

    // Destination only flag (D) indicates that only the
    // destination may respond to this RREQ.
    rreqPacket->setDestOnlyFlag(destinationOnlyFlag);

    // Before broadcasting the RREQ, the originating node buffers the RREQ
    // ID and the Originator IP address (its own address) of the RREQ for
    // PATH_DISCOVERY_TIME.
    // In this way, when the node receives the packet again from its neighbors,
    // it will not reprocess and re-forward the packet.

    RreqIdentifier rreqIdentifier(getSelfIPAddress(), rreqId);
    rreqsArrivalTime[rreqIdentifier] = simTime();
    return rreqPacket;
}

const Ptr<Rrep> Aodv::createRREP(const Ptr<Rreq>& rreq, IRoute *destRoute, IRoute *originatorRoute, const L3Address& lastHopAddr)
{
    auto rrep = makeShared<Rrep>(); // TODO "AODV-RREP");
    rrep->setPacketType(usingIpv6 ? RREP_IPv6 : RREP);
    rrep->setChunkLength(usingIpv6 ? B(44) : B(20));

    // When generating a RREP message, a node copies the Destination IP
    // Address and the Originator Sequence Number from the RREQ message into
    // the corresponding fields in the RREP message.

    rrep->setDestAddr(rreq->getDestAddr());

    // OriginatorAddr = The IP address of the node which originated the RREQ
    // for which the route is supplied.
    rrep->setOriginatorAddr(rreq->getOriginatorAddr());

    // Processing is slightly different, depending on whether the node is
    // itself the requested destination (see section 6.6.1), or instead
    // if it is an intermediate node with an fresh enough route to the destination
    // (see section 6.6.2).

    if (rreq->getDestAddr() == getSelfIPAddress()) { // node is itself the requested destination
        // 6.6.1. Route Reply Generation by the Destination

        // Model interpretation of RFC 3561 sections 6.1 and 6.6.1: adopt a known
        // fresher request, without an additional unconditional increment.
        if (!rreq->getUnknownSeqNumFlag() && AodvSequence::isNewer(rreq->getDestSeqNum(), sequenceNum))
            sequenceNum = rreq->getDestSeqNum();

        // The destination node places its (perhaps newly incremented)
        // sequence number into the Destination Sequence Number field of
        // the RREP,
        rrep->setDestSeqNum(sequenceNum);

        // and enters the value zero in the Hop Count field
        // of the RREP.
        rrep->setHopCount(0);

        // The destination node copies the value MY_ROUTE_TIMEOUT
        // into the Lifetime field of the RREP.
        rrep->setLifeTime(quantizeAdvertisementLifetime(myRouteTimeout));
    }
    else { // intermediate node
        // 6.6.2. Route Reply Generation by an Intermediate Node

        // it copies its known sequence number for the destination into
        // the Destination Sequence Number field in the RREP message.
        AodvRouteData *destRouteData = check_and_cast<AodvRouteData *>(destRoute->getProtocolData());
        AodvRouteData *originatorRouteData = check_and_cast<AodvRouteData *>(originatorRoute->getProtocolData());
        // RFC 3561 section 6.6.2. Suppressed advertisements create no precursor obligation.
        auto remaining = quantizeAdvertisementLifetime(destRouteData->getLifeTime() - simTime());
        if (remaining == SIMTIME_ZERO)
            return nullptr;
        rrep->setLifeTime(remaining);
        rrep->setDestSeqNum(destRouteData->getDestSeqNum());

        // The intermediate node updates the forward route entry by placing the
        // last hop node (from which it received the RREQ, as indicated by the
        // source IP address field in the IP header) into the precursor list for
        // the forward route entry -- i.e., the entry for the Destination IP
        // Address.
        destRouteData->addPrecursor(lastHopAddr);

        // The intermediate node also updates its route table entry
        // for the node originating the RREQ by placing the next hop towards the
        // destination in the precursor list for the reverse route entry --
        // i.e., the entry for the Originator IP Address field of the RREQ
        // message data.

        originatorRouteData->addPrecursor(destRoute->getNextHopAsGeneric());

        // The intermediate node places its distance in hops from the
        // destination (indicated by the hop count in the routing table)
        // Hop Count field in the RREP.

        rrep->setHopCount(destRoute->getMetric());
    }

    return rrep;
}

const Ptr<Rrep> Aodv::createGratuitousRREP(const Ptr<Rreq>& rreq, IRoute *originatorRoute)
{
    ASSERT(originatorRoute != nullptr);
    auto grrep = makeShared<Rrep>(); // TODO "AODV-GRREP");
    grrep->setPacketType(usingIpv6 ? RREP_IPv6 : RREP);
    grrep->setChunkLength(usingIpv6 ? B(44) : B(20));

    AodvRouteData *routeData = check_and_cast<AodvRouteData *>(originatorRoute->getProtocolData());

    // Hop Count                        The Hop Count as indicated in the
    //                                  node's route table entry for the
    //                                  originator
    //
    // Destination IP Address           The IP address of the node that
    //                                  originated the RREQ
    //
    // Destination Sequence Number      The Originator Sequence Number from
    //                                  the RREQ
    //
    // Originator IP Address            The IP address of the Destination
    //                                  node in the RREQ
    //
    // Lifetime                         The remaining lifetime of the route
    //                                  towards the originator of the RREQ,
    //                                  as known by the intermediate node.

    grrep->setHopCount(originatorRoute->getMetric());
    grrep->setDestAddr(rreq->getOriginatorAddr());
    grrep->setDestSeqNum(rreq->getOriginatorSeqNum());
    grrep->setOriginatorAddr(rreq->getDestAddr());
    auto remaining = quantizeAdvertisementLifetime(routeData->getLifeTime() - simTime());
    if (remaining == SIMTIME_ZERO)
        return nullptr;
    grrep->setLifeTime(remaining);
    return grrep;
}

void Aodv::handleRREP(const Ptr<Rrep>& rrep, const L3Address& sourceAddr)
{
    auto process = [&]() {
        if (rrep->getOriginatorAddr().isUnspecified()) {
            if (rrep->getDestAddr() == sourceAddr)
                handleHelloMessage(rrep);
            return;
        }

        // RFC 3561 section 6.7. An aliased previous hop must not preempt reply selection.
        if (sourceAddr != rrep->getDestAddr())
            refreshNeighbor(sourceAddr);
        IRoute *destRoute = findExactRoute(rrep->getDestAddr());
        expireRouteIfDue(destRoute);
        auto data = destRoute ? check_and_cast<AodvRouteData *>(destRoute->getProtocolData()) : nullptr;
        uint32_t sequence = rrep->getDestSeqNum();
        unsigned int hops = unsigned(rrep->getHopCount()) + 1;
        bool accepted = rrep->getLifeTime() > SIMTIME_ZERO && hops <= 255 &&
            (!data || !data->hasValidDestNum() || AodvSequence::isNewer(sequence, data->getDestSeqNum()) ||
             (sequence == data->getDestSeqNum() && (!data->isActive() || hops < unsigned(destRoute->getMetric()))));
        if (!accepted)
            return;

        rrep->setHopCount(hops);
        if (destRoute)
            updateRoutingTable(destRoute, sourceAddr, hops, true, sequence, true, simTime() + rrep->getLifeTime());
        else
            destRoute = createRoute(rrep->getDestAddr(), sourceAddr, hops, true, sequence, true,
                    simTime() + rrep->getLifeTime());
        data = check_and_cast<AodvRouteData *>(destRoute->getProtocolData());

        if (getSelfIPAddress() == rrep->getOriginatorAddr()) {
            if (!gatewayAddress.isUnspecified() && rrep->getDestAddr() == gatewayAddress) {
                if (!defaultGatewayRoute)
                    createDefaultGatewayRoute();
                updateGatewayRoute();
            }
            return;
        }

        auto reverse = findExactRoute(rrep->getOriginatorAddr());
        expireRouteIfDue(reverse);
        auto reverseData = reverse ? check_and_cast<AodvRouteData *>(reverse->getProtocolData()) : nullptr;
        if (!reverseData || !reverseData->isActive())
            return;
        updateValidRouteLifeTime(rrep->getOriginatorAddr(), simTime() + activeRouteTimeout);
        if (simTime() > rebootTime + deletePeriod || rebootTime == 0) {
            if (rrep->getAckRequiredFlag()) {
                sendRREPACK(createRREPACK(), sourceAddr);
                rrep->setAckRequiredFlag(false);
            }
            auto nextHop = reverse->getNextHopAsGeneric();
            data->addPrecursor(nextHop);
            if (auto neighbor = findExactRoute(destRoute->getNextHopAsGeneric()))
                check_and_cast<AodvRouteData *>(neighbor->getProtocolData())->addPrecursor(nextHop);
            forwardRREP(rrep, nextHop, 100);
        }
    };
    process();
    releaseEligiblePackets();
}

IRoute *Aodv::findExactRoute(const L3Address& destination) const
{
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        auto route = routingTable->getRoute(i);
        if (route->getSource() == this && route->getDestinationAsGeneric() == destination &&
                route->getPrefixLength() == addressType->getMaxPrefixLength())
            return route;
    }
    return nullptr;
}

void Aodv::expireRouteIfDue(IRoute *route)
{
    if (!route)
        return;
    auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
    if (data->isActive() && data->getLifeTime() <= simTime()) {
        // RFC 3561 section 6.11. Idle expiry preserves sequence knowledge.
        data->setIsActive(false);
        data->setLifeTime(simTime() + deletePeriod);
        scheduleExpungeRoutes();
    }
}

void Aodv::refreshNeighbor(const L3Address& sender)
{
    auto route = findExactRoute(sender);
    expireRouteIfDue(route);
    if (!route)
        createRoute(sender, sender, 1, false, 0, true, simTime() + activeRouteTimeout);
    else {
        auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
        auto lifetime = simTime() + activeRouteTimeout;
        if (data->isActive())
            lifetime = std::max(lifetime, data->getLifeTime());
        // The previous hop's sequence is unrelated to the advertised destination.
        updateRoutingTable(route, sender, 1, data->hasValidDestNum(), data->getDestSeqNum(), true, lifetime);
    }
}

simtime_t Aodv::quantizeAdvertisementLifetime(simtime_t duration)
{
    if (duration <= SIMTIME_ZERO)
        return SIMTIME_ZERO;
    // RFC 3561 section 5.2 encodes a relative duration in uint32 milliseconds.
    if (duration.inUnit(SIMTIME_MS) > UINT32_MAX)
        return SIMTIME_ZERO;
    return duration.trunc(SIMTIME_MS);
}

void Aodv::updateRoutingTable(IRoute *route, const L3Address& nextHop, unsigned int hopCount, bool hasValidDestNum, uint32_t destSeqNum, bool isActive, simtime_t lifeTime)
{
    EV_DETAIL << "Updating existing route: " << route << endl;

    route->setNextHop(nextHop);
    route->setMetric(hopCount);

    AodvRouteData *routingData = check_and_cast<AodvRouteData *>(route->getProtocolData());
    ASSERT(routingData != nullptr);

    routingData->setLifeTime(lifeTime);
    routingData->setDestSeqNum(destSeqNum);
    routingData->setIsActive(isActive);
    routingData->setHasValidDestNum(hasValidDestNum);

    EV_DETAIL << "Route updated: " << route << endl;

    scheduleExpungeRoutes();
}

void Aodv::sendAODVPacket(const Ptr<AodvControlPacket>& aodvPacket, const L3Address& destAddr, unsigned int timeToLive, double delay)
{
    ASSERT(timeToLive != 0);

    const char *className = aodvPacket->getClassName();
    Packet *packet = new Packet(!strncmp("inet::", className, 6) ? className + 6 : className, aodvPacket);

    int interfaceId = interface->getInterfaceId(); // TODO Implement: support for multiple interfaces
    packet->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
    packet->addTag<HopLimitReq>()->setHopLimit(timeToLive);
    auto addresses = packet->addTag<L3AddressReq>();
    addresses->setDestAddress(destAddr);
    addresses->setSrcAddress(getSelfIPAddress());
    if (!destAddr.isBroadcast() && !destAddr.isMulticast())
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(destAddr);
    packet->addTag<L4PortReq>()->setDestPort(aodvUDPPort);

    if (destAddr.isBroadcast())
        lastBroadcastTime = simTime();

    numSent++;
    if (delay == 0)
        socket.send(packet);
    else {
        auto *timer = new PacketHolderMessage("aodv-send-jitter", KIND_DELAYEDSEND);
        timer->setOwnedPacket(packet);
        scheduleAfter(delay, timer);
    }
}

void Aodv::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    numReceived++;
    processPacket(packet);
}

void Aodv::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void Aodv::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void Aodv::handleRREQ(const Ptr<Rreq>& rreq, const L3Address& sourceAddr, unsigned int timeToLive)
{
    auto process = [&]() {
        if (containsKey(blacklist, sourceAddr))
            return;
        // RFC 3561 sections 6.2 and 6.5. Only the originator transition owns an aliased record.
        if (sourceAddr != rreq->getOriginatorAddr())
            refreshNeighbor(sourceAddr);
        if (rreq->getOriginatorAddr() == getSelfIPAddress())
            return;
        RreqIdentifier identifier(rreq->getOriginatorAddr(), rreq->getRreqId());
        auto previous = rreqsArrivalTime.find(identifier);
        if (previous != rreqsArrivalTime.end() && simTime() - previous->second <= pathDiscoveryTime)
            return;
        rreqsArrivalTime[identifier] = simTime();
        unsigned int hops = unsigned(rreq->getHopCount()) + 1;
        if (hops > 255)
            return;
        rreq->setHopCount(hops);
        IRoute *reverse = findExactRoute(rreq->getOriginatorAddr());
        expireRouteIfDue(reverse);
        auto reverseData = reverse ? check_and_cast<AodvRouteData *>(reverse->getProtocolData()) : nullptr;
        uint32_t sequence = rreq->getOriginatorSeqNum();
        simtime_t lifetime = std::max(simTime(), simTime() + 2 * netTraversalTime - 2 * hops * nodeTraversalTime);
        if (reverseData)
            lifetime = std::max(lifetime, reverseData->getLifeTime());
        if (!reverse)
            reverse = createRoute(rreq->getOriginatorAddr(), sourceAddr, hops, true, sequence, true, lifetime);
        else if (!reverseData->hasValidDestNum() || AodvSequence::isNewer(sequence, reverseData->getDestSeqNum()) ||
                (sequence == reverseData->getDestSeqNum() && hops < unsigned(reverse->getMetric())))
            updateRoutingTable(reverse, sourceAddr, hops, true, sequence, true, lifetime);

        // RFC 3561 section 6.5. Lifetime refresh is independent of path selection.
        reverseData = check_and_cast<AodvRouteData *>(reverse->getProtocolData());
        reverseData->setLifeTime(std::max(reverseData->getLifeTime(), lifetime));
        scheduleExpungeRoutes();

        auto destination = findExactRoute(rreq->getDestAddr());
        expireRouteIfDue(destination);
        auto destinationData = destination ? check_and_cast<AodvRouteData *>(destination->getProtocolData()) : nullptr;
        if (rreq->getDestAddr() == getSelfIPAddress()) {
            sendRREP(createRREP(rreq, destination, reverse, sourceAddr), rreq->getOriginatorAddr(), 255);
            return;
        }
        bool known = destinationData && destinationData->hasValidDestNum();
        bool fresh = known && (rreq->getUnknownSeqNumFlag() ||
            AodvSequence::isAtLeast(destinationData->getDestSeqNum(), rreq->getDestSeqNum()));
        if (fresh && destinationData->isActive() && !rreq->getDestOnlyFlag() &&
                !destination->getNextHopAsGeneric().isUnspecified() && destination->getNextHopAsGeneric() != sourceAddr)
        {
            auto reply = createRREP(rreq, destination, reverse, sourceAddr);
            if (reply) {
                sendRREP(reply, rreq->getOriginatorAddr(), 255);
                if (rreq->getGratuitousRREPFlag()) {
                    auto gratuitousReply = createGratuitousRREP(rreq, reverse);
                    if (gratuitousReply)
                        sendGRREP(gratuitousReply, rreq->getDestAddr(), 100);
                }
                return;
            }
        }
        if (timeToLive > 0 && (simTime() > rebootTime + deletePeriod || rebootTime == 0)) {
            // RFC 3561 section 6.5. Unknown placeholders are not serial operands.
            if (known) {
                if (rreq->getUnknownSeqNumFlag() ||
                        AodvSequence::isNewer(destinationData->getDestSeqNum(), rreq->getDestSeqNum()))
                    rreq->setDestSeqNum(destinationData->getDestSeqNum());
                rreq->setUnknownSeqNumFlag(false);
            }
            forwardRREQ(rreq, timeToLive);
        }
    };
    process();
    releaseEligiblePackets();
}

IRoute *Aodv::createRoute(const L3Address& destAddr, const L3Address& nextHop,
        unsigned int hopCount, bool hasValidDestNum, uint32_t destSeqNum,
        bool isActive, simtime_t lifeTime)
{
    // create a new route
    IRoute *newRoute = routingTable->createRoute();

    // adding generic fields
    newRoute->setDestination(destAddr);
    newRoute->setNextHop(nextHop);
    newRoute->setPrefixLength(addressType->getMaxPrefixLength()); // TODO
    newRoute->setMetric(hopCount);
    newRoute->setInterface(interface); // TODO IMPLEMENT: multiple interfaces
    newRoute->setSourceType(IRoute::AODV);
    newRoute->setSource(this);

    // A route towards a destination that has a routing table entry
    // that is marked as valid.  Only active routes can be used to
    // forward data packets.

    // adding protocol-specific fields
    AodvRouteData *newProtocolData = new AodvRouteData();
    newProtocolData->setIsActive(isActive);
    newProtocolData->setHasValidDestNum(hasValidDestNum);
    newProtocolData->setDestSeqNum(destSeqNum);
    newProtocolData->setLifeTime(lifeTime);
    newRoute->setProtocolData(newProtocolData);

    EV_DETAIL << "Adding new route " << newRoute << endl;
    routingTable->addRoute(newRoute);

    scheduleExpungeRoutes();
    return newRoute;
}

void Aodv::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

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

                // TODO Implement: local repair

                IRoute *route = routingTable->findBestMatchingRoute(unreachableAddr);

                if (route && route->getSource() == this)
                    handleLinkBreakSendRERR(route->getNextHopAsGeneric());
            }
        }
    }
}

void Aodv::handleLinkBreakSendRERR(const L3Address& unreachableAddr)
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
    AodvRouteData *unreachableRouteData = check_and_cast<AodvRouteData *>(unreachableRoute->getProtocolData());

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

        AodvRouteData *routeData = dynamic_cast<AodvRouteData *>(route->getProtocolData());
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

    refreshRateWindow();
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

const Ptr<Rerr> Aodv::createRERR(const std::vector<UnreachableNode>& unreachableNodes)
{
    auto rerr = makeShared<Rerr>(); // TODO "AODV-RERR");
    rerr->setPacketType(usingIpv6 ? RERR_IPv6 : RERR);

    unsigned int destCount = unreachableNodes.size();
    rerr->setUnreachableNodesArraySize(destCount);

    for (unsigned int i = 0; i < destCount; i++) {
        UnreachableNode node;
        node.addr = unreachableNodes[i].addr;
        node.seqNum = unreachableNodes[i].seqNum;
        rerr->setUnreachableNodes(i, node);
    }

    rerr->setChunkLength(B(4 + destCount * (usingIpv6 ? (4 + 16) : (4 + 4))));

    return rerr;
}

void Aodv::handleRERR(const Ptr<const Rerr>& rerr, const L3Address& sourceAddr)
{
    EV_INFO << "AODV Route Error arrived with source addr: " << sourceAddr << endl;

    // A node initiates processing for a RERR message in three situations:
    // (iii)   if it receives a RERR from a neighbor for one or more
    //         active routes.
    unsigned int unreachableArraySize = rerr->getUnreachableNodesArraySize();
    std::vector<UnreachableNode> unreachableNeighbors;

    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        AodvRouteData *routeData = route ? dynamic_cast<AodvRouteData *>(route->getProtocolData()) : nullptr;

        if (!routeData)
            continue;

        // For case (iii), the list should consist of those destinations in the RERR
        // for which there exists a corresponding entry in the local routing
        // table that has the transmitter of the received RERR as the next hop.

        if (routeData->isActive() && route->getNextHopAsGeneric() == sourceAddr) {
            for (unsigned int j = 0; j < unreachableArraySize; j++) {
                if (route->getSource() == this && route->getDestinationAsGeneric() == rerr->getUnreachableNodes(j).addr &&
                        (!routeData->hasValidDestNum() || AodvSequence::isAtLeast(
                            rerr->getUnreachableNodes(j).seqNum, routeData->getDestSeqNum()))) {
                    // 1. The destination sequence number of this routing entry, if it
                    // exists and is valid, is incremented for cases (i) and (ii) above,
                    // ! and copied from the incoming RERR in case (iii) above.

                    routeData->setDestSeqNum(rerr->getUnreachableNodes(j).seqNum);
                    routeData->setIsActive(false); // it means invalid, see 3. AODV Terminology p.3. in RFC 3561
                    routeData->setLifeTime(simTime() + deletePeriod);

                    // The RERR should contain those destinations that are part of
                    // the created list of unreachable destinations and have a non-empty
                    // precursor list.

                    if (routeData->getPrecursorList().size() > 0) {
                        UnreachableNode node;
                        node.addr = route->getDestinationAsGeneric();
                        node.seqNum = routeData->getDestSeqNum();
                        unreachableNeighbors.push_back(node);
                    }
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
        auto newRERR = createRERR(unreachableNeighbors);
        sendAODVPacket(newRERR, addressType->getBroadcastAddress(), 1, 0);
        rerrCount++;
    }
}

void Aodv::handleStartOperation(LifecycleOperation *operation)
{
    // initialize variables that require INITSTAGE_ROUTING stage here to support starting the node in DOWN sate
    addressType = getSelfIPAddress().getAddressType();
    usingIpv6 = (getSelfIPAddress().getType() == L3Address::IPv6);

    rebootTime = simTime();
    rateEpoch = simTime();
    rateWindow = 0;

    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);
    socket.bind(L3Address(), aodvUDPPort);
    socket.setBroadcast(true);

    // RFC 5148:
    // Jitter SHOULD be applied by reducing this delay by a random amount, so that
    // the delay between consecutive transmissions of messages of the same type is
    // equal to (MESSAGE_INTERVAL - jitter), where jitter is the random value.
    if (useHelloMessages)
        scheduleAfter(helloInterval - samplePeriodicJitter(), helloMsgTimer);

    scheduleAfter(1, counterTimer);
}

void Aodv::handleStopOperation(LifecycleOperation *operation)
{
    socket.close();
    clearState();
}

void Aodv::handleCrashOperation(LifecycleOperation *operation)
{
    socket.destroy();
    clearState();
}

void Aodv::clearState(bool disposeQueued)
{
    rerrCount = rreqCount = rreqId = sequenceNum = 0;
    for (auto& search : searches)
        cancelAndDelete(search.second.timer);
    searches.clear();
    readySearches.clear();
    auto queues = std::move(delayedPackets);
    delayedPackets.clear();
    queueOwners.clear();
    if (disposeQueued)
        for (const auto& fifo : queues)
            for (auto packet : fifo.second)
                networkProtocol->dropQueuedDatagram(packet);
    rreqsArrivalTime.clear();
    if (helloMsgTimer)
        cancelEvent(helloMsgTimer);
    if (expungeTimer)
        cancelEvent(expungeTimer);
    if (counterTimer)
        cancelEvent(counterTimer);
    if (blacklistTimer)
        cancelEvent(blacklistTimer);
    if (rrepAckTimer)
        cancelEvent(rrepAckTimer);
    defaultGatewayRoute = nullptr;
}

void Aodv::forwardRREP(const Ptr<Rrep>& rrep, const L3Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Forwarding the Route Reply to the node " << rrep->getOriginatorAddr() << " which originated the Route Request" << endl;

    // RFC 5148:
    // When a node forwards a message, it SHOULD be jittered by delaying it
    // by a random duration.  This delay SHOULD be generated uniformly in an
    // interval between zero and MAXJITTER.
    sendAODVPacket(rrep, destAddr, 100, *jitterPar);
}

void Aodv::forwardRREQ(const Ptr<Rreq>& rreq, unsigned int timeToLive)
{
    EV_INFO << "Forwarding the Route Request message with TTL= " << timeToLive << endl;
    sendAODVPacket(rreq, addressType->getBroadcastAddress(), timeToLive, *jitterPar);
}

void Aodv::sendGRREP(const Ptr<Rrep>& grrep, const L3Address& destAddr, unsigned int timeToLive)
{
    EV_INFO << "Sending gratuitous Route Reply to " << destAddr << endl;

    IRoute *destRoute = routingTable->findBestMatchingRoute(destAddr);
    const L3Address& nextHop = destRoute->getNextHopAsGeneric();

    sendAODVPacket(grrep, nextHop, timeToLive, 0);
}

const Ptr<Rrep> Aodv::createHelloMessage()
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

    auto helloMessage = makeShared<Rrep>(); // TODO "AODV-HelloMsg");
    helloMessage->setPacketType(usingIpv6 ? RREP_IPv6 : RREP);
    helloMessage->setChunkLength(usingIpv6 ? B(44) : B(20));

    helloMessage->setDestAddr(getSelfIPAddress());
    helloMessage->setDestSeqNum(sequenceNum);
    helloMessage->setHopCount(0);
    helloMessage->setLifeTime(quantizeAdvertisementLifetime(allowedHelloLoss * helloInterval));

    return helloMessage;
}

void Aodv::sendHelloMessagesIfNeeded()
{
    ASSERT(useHelloMessages);
    // Every HELLO_INTERVAL milliseconds, the node checks whether it has
    // sent a broadcast (e.g., a RREQ or an appropriate layer 2 message)
    // within the last HELLO_INTERVAL.  If it has not, it MAY broadcast
    // a RREP with TTL = 1

    // A node SHOULD only use hello messages if it is part of an
    // active route.
    bool hasActiveRoute = false;

    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            AodvRouteData *routeData = check_and_cast<AodvRouteData *>(route->getProtocolData());
            if (routeData->isActive()) {
                hasActiveRoute = true;
                break;
            }
        }
    }

    if (hasActiveRoute && (lastBroadcastTime == 0 || simTime() - lastBroadcastTime > helloInterval)) {
        EV_INFO << "It is hello time, broadcasting Hello Messages with TTL=1" << endl;
        auto helloMessage = createHelloMessage();
        sendAODVPacket(helloMessage, addressType->getBroadcastAddress(), 1, 0);
    }

    scheduleAfter(helloInterval - samplePeriodicJitter(), helloMsgTimer);
}

simtime_t Aodv::samplePeriodicJitter()
{
    simtime_t sample = *periodicJitter;
    if (sample < SIMTIME_ZERO || sample > SimTime(par("maxPeriodicJitter")))
        throw cRuntimeError("AODV periodicJitter must be between zero and maxPeriodicJitter");
    return sample;
}

void Aodv::handleHelloMessage(const Ptr<Rrep>& helloMessage)
{
    if (helloMessage->getLifeTime() <= SIMTIME_ZERO)
        return;
    auto sender = helloMessage->getDestAddr();
    auto route = findExactRoute(sender);
    expireRouteIfDue(route);
    auto data = route ? check_and_cast<AodvRouteData *>(route->getProtocolData()) : nullptr;
    uint32_t sequence = helloMessage->getDestSeqNum();
    // RFC 3561 sections 6.1 and 6.9. A Hello cannot replace fresher retained state.
    if (data && data->hasValidDestNum() && !AodvSequence::isAtLeast(sequence, data->getDestSeqNum()))
        return;
    auto lifetime = simTime() + std::max(helloMessage->getLifeTime(), allowedHelloLoss * helloInterval);
    if (data && data->isActive())
        lifetime = std::max(lifetime, data->getLifeTime());
    if (route)
        updateRoutingTable(route, sender, 1, true, sequence, true, lifetime);
    else
        createRoute(sender, sender, 1, true, sequence, true, lifetime);
    releaseEligiblePackets();
}

void Aodv::expungeRoutes()
{
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);
        if (route->getSource() == this) {
            AodvRouteData *routeData = check_and_cast<AodvRouteData *>(route->getProtocolData());
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
                    if (hasOngoingRouteDiscovery(route->getDestinationAsGeneric()) || queueOwners.count(route->getDestinationAsGeneric())) {
                        EV_DETAIL << "Route to " << route->getDestinationAsGeneric() << " expired and is inactive, but we are waiting for a RREP to this destination, so we extend its lifetime with 2 * NET_TRAVERSAL_TIME" << endl;
                        routeData->setLifeTime(simTime() + 2 * netTraversalTime);
                    }
                    else {
                        EV_DETAIL << "Route to " << route->getDestinationAsGeneric() << " expired and is inactive and we are not expecting any RREP to this destination, so we delete this route" << endl;
                        if (route == defaultGatewayRoute)
                            defaultGatewayRoute = nullptr;
                        routingTable->deleteRoute(route);
                    }
                }
            }
        }
    }
    scheduleExpungeRoutes();
}

void Aodv::scheduleExpungeRoutes()
{
    simtime_t nextExpungeTime = SimTime::getMaxTime();
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        IRoute *route = routingTable->getRoute(i);

        if (route->getSource() == this) {
            AodvRouteData *routeData = check_and_cast<AodvRouteData *>(route->getProtocolData());
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
                rescheduleAt(nextExpungeTime, expungeTimer);
            }
        }
    }
}

INetfilter::IHook::Result Aodv::datagramForwardHook(Packet *datagram)
{
    // TODO Implement: Actions After Reboot
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

    // TODO IMPLEMENT: check if the datagram is a data packet or we take control packets as data packets

    IRoute *routeDest = routingTable->findBestMatchingRoute(destAddr);
    AodvRouteData *routeDestData = routeDest ? dynamic_cast<AodvRouteData *>(routeDest->getProtocolData()) : nullptr;

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
    if (routeDest && routeDestData && !routeDestData->isActive()) { // exists but is not active
        // A node initiates processing for a RERR message in three situations:
        // (ii)      if it gets a data packet destined to a node for which it
        //           does not have an active route and is not repairing (if
        //           using local repair)

        // TODO check if it is not repairing (if using local repair)

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

void Aodv::sendRERRWhenNoRouteToForward(const L3Address& unreachableAddr)
{
    if (rerrCount >= rerrRatelimit) {
        EV_WARN << "A node should not generate more than RERR_RATELIMIT RERR messages per second. Canceling sending RERR" << endl;
        return;
    }
    std::vector<UnreachableNode> unreachableNodes;
    UnreachableNode node;
    node.addr = unreachableAddr;

    IRoute *unreachableRoute = routingTable->findBestMatchingRoute(unreachableAddr);
    AodvRouteData *unreachableRouteData = unreachableRoute ? dynamic_cast<AodvRouteData *>(unreachableRoute->getProtocolData()) : nullptr;

    if (unreachableRouteData && unreachableRouteData->hasValidDestNum())
        node.seqNum = unreachableRouteData->getDestSeqNum();
    else
        node.seqNum = 0;

    unreachableNodes.push_back(node);
    auto rerr = createRERR(unreachableNodes);

    rerrCount++;
    EV_INFO << "Broadcasting Route Error message with TTL=1" << endl;
    sendAODVPacket(rerr, addressType->getBroadcastAddress(), 1, *jitterPar); // TODO unicast if there exists a route to the source
}

void Aodv::cancelRouteDiscovery(const L3Address& target)
{
    std::vector<Packet *> dropped;
    for (auto it = queueOwners.begin(); it != queueOwners.end();) {
        if (it->second == target) {
            auto queue = delayedPackets.find(it->first);
            if (queue != delayedPackets.end()) {
                dropped.insert(dropped.end(), queue->second.begin(), queue->second.end());
                delayedPackets.erase(queue);
            }
            it = queueOwners.erase(it);
        }
        else
            ++it;
    }
    eraseSearch(target);
    for (auto packet : dropped)
        networkProtocol->dropQueuedDatagram(packet);
}

bool Aodv::updateValidRouteLifeTime(const L3Address& destAddr, simtime_t lifetime)
{
    IRoute *route = routingTable->findBestMatchingRoute(destAddr);
    if (route && route->getSource() == this) {
        AodvRouteData *routeData = check_and_cast<AodvRouteData *>(route->getProtocolData());
        if (routeData->isActive()) {
            simtime_t newLifeTime = std::max(routeData->getLifeTime(), lifetime);
            EV_DETAIL << "Updating " << route << " lifetime to " << newLifeTime << endl;
            routeData->setLifeTime(newLifeTime);
            scheduleExpungeRoutes();
            return true;
        }
    }
    return false;
}

const Ptr<RrepAck> Aodv::createRREPACK()
{
    auto rrepAck = makeShared<RrepAck>(); // TODO "AODV-RREPACK");
    rrepAck->setPacketType(usingIpv6 ? RREPACK_IPv6 : RREPACK);
    return rrepAck;
}

void Aodv::sendRREPACK(const Ptr<RrepAck>& rrepACK, const L3Address& destAddr)
{
    EV_INFO << "Sending Route Reply ACK to " << destAddr << endl;
    sendAODVPacket(rrepACK, destAddr, 100, 0);
}

void Aodv::handleRREPACK(const Ptr<const RrepAck>& rrepACK, const L3Address& neighborAddr)
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
            AodvRouteData *routeData = check_and_cast<AodvRouteData *>(route->getProtocolData());
            routeData->setIsActive(true);
            cancelEvent(rrepAckTimer);
        }
    }
}

void Aodv::handleRREPACKTimer()
{
    // when a node detects that its transmission of a RREP message has failed,
    // it remembers the next-hop of the failed RREP in a "blacklist" set.

    EV_INFO << "RREP-ACK didn't arrived within timeout. Adding " << failedNextHop << " to the blacklist" << endl;

    blacklist[failedNextHop] = simTime() + blacklistTimeout; // lifetime

    if (!blacklistTimer->isScheduled())
        scheduleAfter(blacklistTimeout, blacklistTimer);
}

void Aodv::handleBlackListTimer()
{
    simtime_t nextTime = SimTime::getMaxTime();

    for (auto it = blacklist.begin(); it != blacklist.end();) {
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

bool Aodv::isExternalAddress(const L3Address& address) const
{
    ASSERT(!address.isUnspecified());

    // if no external gateway is configured, we consider every address as non-external
    if (!hasExternalGateway)
        return false;

    if (address.isBroadcast() || address.isMulticast())
        return false;

    switch (address.getType()) {
#ifdef INET_WITH_IPv4
        case L3Address::IPv4:
            if (auto ipv4Data = interface->findProtocolData<Ipv4InterfaceData>()) {
                Ipv4Address ipv4Addr = ipv4Data->getIPAddress();
                Ipv4Address netmask = ipv4Data->getNetmask();
                return !Ipv4Address::maskedAddrAreEqual(address.toIpv4(), ipv4Addr, netmask);
            }
            break;
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
        case L3Address::IPv6:
            if (auto ipv6Data = interface->findProtocolData<Ipv6InterfaceData>()) {
                for (int j = 0; j < ipv6Data->getNumAdvPrefixes(); j++) {
                    const Ipv6InterfaceData::AdvPrefix& advPrefix = ipv6Data->getAdvPrefix(j);
                    if (address.toIpv6().matches(advPrefix.prefix, advPrefix.prefixLength))
                        return false;  // matches local prefix -> not external
                }
                return true;  // no prefix matched -> external
            }
            break;

#endif // ifdef INET_WITH_IPv6
        default:
            throw cRuntimeError("Unsupported address type");
    }
    return false;
}

void Aodv::createDefaultGatewayRoute()
{
    ASSERT(defaultGatewayRoute == nullptr);

    // Create inactive default route (0.0.0.0/0)
    Ipv4Route *route = new Ipv4Route();
    route->setDestination(Ipv4Address::UNSPECIFIED_ADDRESS);
    route->setNetmask(Ipv4Address::UNSPECIFIED_ADDRESS);  // /0 netmask
    route->setGateway(Ipv4Address::UNSPECIFIED_ADDRESS);  // No next hop initially
    route->setInterface(interfaceTable->findInterfaceByName(par("interface")));
    route->setSourceType(IRoute::AODV);
    route->setSource(this);
    route->setMetric(0);

    // Create AODV route data - inactive initially
    AodvRouteData *routeData = new AodvRouteData();
    routeData->setIsActive(false);
    routeData->setHasValidDestNum(false);
    routeData->setDestSeqNum(0);
    routeData->setLifeTime(SimTime::getMaxTime());  // Permanent until gateway found
    route->setProtocolData(routeData);

    routingTable->addRoute(route);
    defaultGatewayRoute = route;

    EV_INFO << "Created inactive default gateway route (0.0.0.0/0)" << endl;
}

void Aodv::updateGatewayRoute()
{
    ASSERT(defaultGatewayRoute != nullptr);
    ASSERT(!gatewayAddress.isUnspecified());

    AodvRouteData *defaultRouteData = check_and_cast<AodvRouteData *>(defaultGatewayRoute->getProtocolData());

    // Find route to gateway
    IRoute *gatewayRoute = findExactRoute(gatewayAddress);
    expireRouteIfDue(gatewayRoute);
    if (gatewayRoute && check_and_cast<AodvRouteData *>(gatewayRoute->getProtocolData())->isActive()) {
        AodvRouteData *gatewayRouteData = check_and_cast<AodvRouteData *>(gatewayRoute->getProtocolData());

        if (gatewayRouteData->isActive()) {
            // Activate default route using gateway route info
            defaultGatewayRoute->setNextHop(gatewayRoute->getNextHopAsGeneric());
            defaultGatewayRoute->setMetric(gatewayRoute->getMetric() + 1);
            defaultRouteData->setIsActive(true);
            defaultRouteData->setLifeTime(gatewayRouteData->getLifeTime());

            EV_INFO << "Default gateway route activated via " << gatewayRoute->getNextHopAsGeneric() << endl;
        }
    } else {
        // No route to gateway, deactivate default route
        if (defaultRouteData->isActive()) {
            defaultRouteData->setIsActive(false);
            defaultGatewayRoute->setNextHop(L3Address());
            EV_INFO << "Default gateway route deactivated - no route to gateway" << endl;
        }
    }
}

Aodv::~Aodv()
{
    clearState(false);
    delete helloMsgTimer;
    delete expungeTimer;
    delete counterTimer;
    delete rrepAckTimer;
    delete blacklistTimer;
}

} // namespace aodv
} // namespace inet
