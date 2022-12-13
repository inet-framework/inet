//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/gpsr/Gpsr.h"

#include <algorithm>

#include "inet/common/INETUtils.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif

#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif

#ifdef INET_WITH_NEXTHOP
#include "inet/networklayer/nexthop/NextHopForwardingHeader_m.h"
#endif

namespace inet {

Define_Module(Gpsr);

static inline double determinant(double a1, double a2, double b1, double b2)
{
    return a1 * b2 - a2 * b1;
}

Gpsr::Gpsr()
{
}

Gpsr::~Gpsr()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(purgeNeighborsTimer);
}

//
// module interface
//

void Gpsr::initialize(int stage)
{
    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
        addressType = getSelfAddress().getAddressType();

    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // Gpsr parameters
        const char *planarizationModeString = par("planarizationMode");
        if (!strcmp(planarizationModeString, ""))
            planarizationMode = GPSR_NO_PLANARIZATION;
        else if (!strcmp(planarizationModeString, "GG"))
            planarizationMode = GPSR_GG_PLANARIZATION;
        else if (!strcmp(planarizationModeString, "RNG"))
            planarizationMode = GPSR_RNG_PLANARIZATION;
        else
            throw cRuntimeError("Unknown planarization mode");
        interfaces = par("interfaces");
        beaconInterval = par("beaconInterval");
        maxJitter = par("maxJitter");
        neighborValidityInterval = par("neighborValidityInterval");
        displayBubbles = par("displayBubbles");
        // context
        host = getContainingNode(this);
        interfaceTable.reference(this, "interfaceTableModule", true);
        outputInterface = par("outputInterface");
        mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        routingTable.reference(this, "routingTableModule", true);
        networkProtocol.reference(this, "networkProtocolModule", true);
        // internal
        beaconTimer = new cMessage("BeaconTimer");
        purgeNeighborsTimer = new cMessage("PurgeNeighborsTimer");
        // packet size
        positionByteLength = par("positionByteLength");
        // KLUDGE implement position registry protocol
        globalPositionTable.clear();
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerProtocol(Protocol::manet, gate("ipOut"), gate("ipIn"));
        host->subscribe(linkBrokenSignal, this);
        networkProtocol->registerHook(0, this);
        WATCH(neighborPositionTable);
    }
}

void Gpsr::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

//
// handling messages
//

void Gpsr::processSelfMessage(cMessage *message)
{
    if (message == beaconTimer)
        processBeaconTimer();
    else if (message == purgeNeighborsTimer)
        processPurgeNeighborsTimer();
    else
        throw cRuntimeError("Unknown self message");
}

void Gpsr::processMessage(cMessage *message)
{
    if (auto pk = dynamic_cast<Packet *>(message))
        processUdpPacket(pk);
    else
        throw cRuntimeError("Unknown message");
}

//
// beacon timers
//

void Gpsr::scheduleBeaconTimer()
{
    EV_DEBUG << "Scheduling beacon timer" << endl;
    scheduleAfter(beaconInterval + uniform(-1, 1) * maxJitter, beaconTimer);
}

void Gpsr::processBeaconTimer()
{
    EV_DEBUG << "Processing beacon timer" << endl;
    const L3Address selfAddress = getSelfAddress();
    if (!selfAddress.isUnspecified()) {
        sendBeacon(createBeacon());
        storeSelfPositionInGlobalRegistry();
    }
    scheduleBeaconTimer();
    schedulePurgeNeighborsTimer();
}

//
// handling purge neighbors timers
//

void Gpsr::schedulePurgeNeighborsTimer()
{
    EV_DEBUG << "Scheduling purge neighbors timer" << endl;
    simtime_t nextExpiration = getNextNeighborExpiration();
    if (nextExpiration == SimTime::getMaxTime()) {
        if (purgeNeighborsTimer->isScheduled())
            cancelEvent(purgeNeighborsTimer);
    }
    else {
        if (!purgeNeighborsTimer->isScheduled())
            scheduleAt(nextExpiration, purgeNeighborsTimer);
        else {
            if (purgeNeighborsTimer->getArrivalTime() != nextExpiration) {
                rescheduleAt(nextExpiration, purgeNeighborsTimer);
            }
        }
    }
}

void Gpsr::processPurgeNeighborsTimer()
{
    EV_DEBUG << "Processing purge neighbors timer" << endl;
    purgeNeighbors();
    schedulePurgeNeighborsTimer();
}

//
// handling UDP packets
//

void Gpsr::sendUdpPacket(Packet *packet)
{
    send(packet, "ipOut");
}

void Gpsr::processUdpPacket(Packet *packet)
{
    packet->popAtFront<UdpHeader>();
    processBeacon(packet);
    schedulePurgeNeighborsTimer();
}

//
// handling beacons
//

const Ptr<GpsrBeacon> Gpsr::createBeacon()
{
    const auto& beacon = makeShared<GpsrBeacon>();
    beacon->setAddress(getSelfAddress());
    beacon->setPosition(mobility->getCurrentPosition());
    beacon->setChunkLength(B(getSelfAddress().getAddressType()->getAddressByteLength() + positionByteLength));
    return beacon;
}

void Gpsr::sendBeacon(const Ptr<GpsrBeacon>& beacon)
{
    EV_INFO << "Sending beacon: address = " << beacon->getAddress() << ", position = " << beacon->getPosition() << endl;
    Packet *udpPacket = new Packet("GPSRBeacon");
    udpPacket->insertAtBack(beacon);
    auto udpHeader = makeShared<UdpHeader>();
    udpHeader->setSourcePort(GPSR_UDP_PORT);
    udpHeader->setDestinationPort(GPSR_UDP_PORT);
    udpHeader->setCrcMode(CRC_DISABLED);
    udpPacket->insertAtFront(udpHeader);
    auto addresses = udpPacket->addTag<L3AddressReq>();
    addresses->setSrcAddress(getSelfAddress());
    addresses->setDestAddress(addressType->getLinkLocalManetRoutersMulticastAddress());
    udpPacket->addTag<HopLimitReq>()->setHopLimit(255);
    udpPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    udpPacket->addTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    sendUdpPacket(udpPacket);
}

void Gpsr::processBeacon(Packet *packet)
{
    const auto& beacon = packet->peekAtFront<GpsrBeacon>();
    EV_INFO << "Processing beacon: address = " << beacon->getAddress() << ", position = " << beacon->getPosition() << endl;
    neighborPositionTable.setPosition(beacon->getAddress(), beacon->getPosition());
    delete packet;
}

//
// handling packets
//

GpsrOption *Gpsr::createGpsrOption(L3Address destination)
{
    GpsrOption *gpsrOption = new GpsrOption();
    gpsrOption->setRoutingMode(GPSR_GREEDY_ROUTING);
    gpsrOption->setDestinationPosition(lookupPositionInGlobalRegistry(destination));
    gpsrOption->setLength(computeOptionLength(gpsrOption));
    return gpsrOption;
}

int Gpsr::computeOptionLength(GpsrOption *option)
{
    // routingMode
    int routingModeBytes = 1;
    // destinationPosition, perimeterRoutingStartPosition, perimeterRoutingForwardPosition
    int positionsBytes = 3 * positionByteLength;
    // currentFaceFirstSenderAddress, currentFaceFirstReceiverAddress, senderAddress
    int addressesBytes = 3 * getSelfAddress().getAddressType()->getAddressByteLength();
    // type and length
    int tlBytes = 1 + 1;

    return tlBytes + routingModeBytes + positionsBytes + addressesBytes;
}

//
// configuration
//

void Gpsr::configureInterfaces()
{
    // join multicast groups
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        NetworkInterface *networkInterface = interfaceTable->getInterface(i);
        if (networkInterface->isMulticast() && interfaceMatcher.matches(networkInterface->getInterfaceName()))
            networkInterface->joinMulticastGroup(addressType->getLinkLocalManetRoutersMulticastAddress());
    }
}

//
// position
//

// KLUDGE implement position registry protocol
Coord Gpsr::lookupPositionInGlobalRegistry(const L3Address& address) const
{
    // KLUDGE implement position registry protocol
    return globalPositionTable.getPosition(address);
}

void Gpsr::storePositionInGlobalRegistry(const L3Address& address, const Coord& position) const
{
    // KLUDGE implement position registry protocol
    globalPositionTable.setPosition(address, position);
}

void Gpsr::storeSelfPositionInGlobalRegistry() const
{
    auto selfAddress = getSelfAddress();
    if (!selfAddress.isUnspecified())
        storePositionInGlobalRegistry(selfAddress, mobility->getCurrentPosition());
}

Coord Gpsr::computeIntersectionInsideLineSegments(Coord& begin1, Coord& end1, Coord& begin2, Coord& end2) const
{
    // NOTE: we must explicitly avoid computing the intersection points inside due to double instability
    if (begin1 == begin2 || begin1 == end2 || end1 == begin2 || end1 == end2)
        return Coord::NIL;
    else {
        double x1 = begin1.x;
        double y1 = begin1.y;
        double x2 = end1.x;
        double y2 = end1.y;
        double x3 = begin2.x;
        double y3 = begin2.y;
        double x4 = end2.x;
        double y4 = end2.y;
        double a = determinant(x1, y1, x2, y2);
        double b = determinant(x3, y3, x4, y4);
        double c = determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
        double x = determinant(a, x1 - x2, b, x3 - x4) / c;
        double y = determinant(a, y1 - y2, b, y3 - y4) / c;
        if ((x <= x1 && x <= x2) || (x >= x1 && x >= x2) || (x <= x3 && x <= x4) || (x >= x3 && x >= x4) ||
            (y <= y1 && y <= y2) || (y >= y1 && y >= y2) || (y <= y3 && y <= y4) || (y >= y3 && y >= y4))
            return Coord::NIL;
        else
            return Coord(x, y, 0);
    }
}

Coord Gpsr::getNeighborPosition(const L3Address& address) const
{
    return neighborPositionTable.getPosition(address);
}

//
// angle
//

double Gpsr::getVectorAngle(Coord vector) const
{
    ASSERT(vector != Coord::ZERO);
    double angle = atan2(-vector.y, vector.x);
    if (angle < 0)
        angle += 2 * M_PI;
    return angle;
}

double Gpsr::getNeighborAngle(const L3Address& address) const
{
    return getVectorAngle(getNeighborPosition(address) - mobility->getCurrentPosition());
}

//
// address
//

std::string Gpsr::getHostName() const
{
    return host->getFullName();
}

L3Address Gpsr::getSelfAddress() const
{
    // TODO choose self address based on a new 'interfaces' parameter
    L3Address ret = routingTable->getRouterIdAsGeneric();
#ifdef INET_WITH_IPv6
    if (ret.getType() == L3Address::IPv6) {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            NetworkInterface *ie = interfaceTable->getInterface(i);
            if ((!ie->isLoopback())) {
                if (auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>()) {
                    ret = ipv6Data->getPreferredAddress();
                    break;
                }
            }
        }
    }
#endif
    return ret;
}

L3Address Gpsr::getSenderNeighborAddress(const Ptr<const NetworkHeaderBase>& networkHeader) const
{
    const GpsrOption *gpsrOption = getGpsrOptionFromNetworkDatagram(networkHeader);
    return gpsrOption->getSenderAddress();
}

//
// neighbor
//

simtime_t Gpsr::getNextNeighborExpiration()
{
    simtime_t oldestPosition = neighborPositionTable.getOldestPosition();
    if (oldestPosition == SimTime::getMaxTime())
        return oldestPosition;
    else
        return oldestPosition + neighborValidityInterval;
}

void Gpsr::purgeNeighbors()
{
    neighborPositionTable.removeOldPositions(simTime() - neighborValidityInterval);
}

std::vector<L3Address> Gpsr::getPlanarNeighbors() const
{
    std::vector<L3Address> planarNeighbors;
    std::vector<L3Address> neighborAddresses = neighborPositionTable.getAddresses();
    Coord selfPosition = mobility->getCurrentPosition();
    for (auto it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        auto neighborAddress = *it;
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        if (planarizationMode == GPSR_NO_PLANARIZATION)
            return neighborAddresses;
        else if (planarizationMode == GPSR_RNG_PLANARIZATION) {
            double neighborDistance = (neighborPosition - selfPosition).length();
            for (auto& witnessAddress : neighborAddresses) {
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - selfPosition).length();
                double neighborWitnessDistance = (witnessPosition - neighborPosition).length();
                if (neighborAddress == witnessAddress)
                    continue;
                else if (neighborDistance > std::max(witnessDistance, neighborWitnessDistance))
                    goto eliminate;
            }
        }
        else if (planarizationMode == GPSR_GG_PLANARIZATION) {
            Coord middlePosition = (selfPosition + neighborPosition) / 2;
            double neighborDistance = (neighborPosition - middlePosition).length();
            for (auto& witnessAddress : neighborAddresses) {
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - middlePosition).length();
                if (neighborAddress == witnessAddress)
                    continue;
                else if (witnessDistance < neighborDistance)
                    goto eliminate;
            }
        }
        else
            throw cRuntimeError("Unknown planarization mode");
        planarNeighbors.push_back(*it);
      eliminate:;
    }
    return planarNeighbors;
}

std::vector<L3Address> Gpsr::getPlanarNeighborsCounterClockwise(double startAngle) const
{
    std::vector<L3Address> neighborAddresses = getPlanarNeighbors();
    std::sort(neighborAddresses.begin(), neighborAddresses.end(), [&] (const L3Address& address1, const L3Address& address2) {
        // NOTE: make sure the neighbor at startAngle goes to the end
        auto angle1 = getNeighborAngle(address1) - startAngle;
        auto angle2 = getNeighborAngle(address2) - startAngle;
        if (angle1 <= 0)
            angle1 += 2 * M_PI;
        if (angle2 <= 0)
            angle2 += 2 * M_PI;
        return angle1 < angle2;
    });
    return neighborAddresses;
}

//
// next hop
//

L3Address Gpsr::findNextHop(const L3Address& destination, GpsrOption *gpsrOption)
{
    switch (gpsrOption->getRoutingMode()) {
        case GPSR_GREEDY_ROUTING: return findGreedyRoutingNextHop(destination, gpsrOption);
        case GPSR_PERIMETER_ROUTING: return findPerimeterRoutingNextHop(destination, gpsrOption);
        default: throw cRuntimeError("Unknown routing mode");
    }
}

L3Address Gpsr::findGreedyRoutingNextHop(const L3Address& destination, GpsrOption *gpsrOption)
{
    EV_DEBUG << "Finding next hop using greedy routing: destination = " << destination << endl;
    L3Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord destinationPosition = gpsrOption->getDestinationPosition();
    double bestDistance = (destinationPosition - selfPosition).length();
    L3Address bestNeighbor;
    std::vector<L3Address> neighborAddresses = neighborPositionTable.getAddresses();
    for (auto& neighborAddress : neighborAddresses) {
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        double neighborDistance = (destinationPosition - neighborPosition).length();
        if (neighborDistance < bestDistance) {
            bestDistance = neighborDistance;
            bestNeighbor = neighborAddress;
        }
    }
    if (bestNeighbor.isUnspecified()) {
        EV_DEBUG << "Switching to perimeter routing: destination = " << destination << endl;
        if (displayBubbles && hasGUI())
            getContainingNode(host)->bubble("Switching to perimeter routing");
        gpsrOption->setRoutingMode(GPSR_PERIMETER_ROUTING);
        gpsrOption->setPerimeterRoutingStartPosition(selfPosition);
        gpsrOption->setPerimeterRoutingForwardPosition(selfPosition);
        gpsrOption->setCurrentFaceFirstSenderAddress(selfAddress);
        gpsrOption->setCurrentFaceFirstReceiverAddress(L3Address());
        return findPerimeterRoutingNextHop(destination, gpsrOption);
    }
    else
        return bestNeighbor;
}

L3Address Gpsr::findPerimeterRoutingNextHop(const L3Address& destination, GpsrOption *gpsrOption)
{
    EV_DEBUG << "Finding next hop using perimeter routing: destination = " << destination << endl;
    L3Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord perimeterRoutingStartPosition = gpsrOption->getPerimeterRoutingStartPosition();
    Coord destinationPosition = gpsrOption->getDestinationPosition();
    double selfDistance = (destinationPosition - selfPosition).length();
    double perimeterRoutingStartDistance = (destinationPosition - perimeterRoutingStartPosition).length();
    if (selfDistance < perimeterRoutingStartDistance) {
        EV_DEBUG << "Switching to greedy routing: destination = " << destination << endl;
        if (displayBubbles && hasGUI())
            getContainingNode(host)->bubble("Switching to greedy routing");
        gpsrOption->setRoutingMode(GPSR_GREEDY_ROUTING);
        gpsrOption->setPerimeterRoutingStartPosition(Coord());
        gpsrOption->setPerimeterRoutingForwardPosition(Coord());
        gpsrOption->setCurrentFaceFirstSenderAddress(L3Address());
        gpsrOption->setCurrentFaceFirstReceiverAddress(L3Address());
        return findGreedyRoutingNextHop(destination, gpsrOption);
    }
    else {
        const L3Address& firstSenderAddress = gpsrOption->getCurrentFaceFirstSenderAddress();
        const L3Address& firstReceiverAddress = gpsrOption->getCurrentFaceFirstReceiverAddress();
        auto senderNeighborAddress = gpsrOption->getSenderAddress();
        auto neighborAngle = senderNeighborAddress.isUnspecified() ? getVectorAngle(destinationPosition - mobility->getCurrentPosition()) : getNeighborAngle(senderNeighborAddress);
        L3Address selectedNeighborAddress;
        std::vector<L3Address> neighborAddresses = getPlanarNeighborsCounterClockwise(neighborAngle);
        for (auto& neighborAddress : neighborAddresses) {
            Coord neighborPosition = getNeighborPosition(neighborAddress);
            Coord intersection = computeIntersectionInsideLineSegments(perimeterRoutingStartPosition, destinationPosition, selfPosition, neighborPosition);
            if (std::isnan(intersection.x)) {
                selectedNeighborAddress = neighborAddress;
                break;
            }
            else {
                EV_DEBUG << "Edge to next hop intersects: intersection = " << intersection << ", nextNeighbor = " << selectedNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
                gpsrOption->setCurrentFaceFirstSenderAddress(selfAddress);
                gpsrOption->setCurrentFaceFirstReceiverAddress(L3Address());
                gpsrOption->setPerimeterRoutingForwardPosition(intersection);
            }
        }
        if (selectedNeighborAddress.isUnspecified()) {
            EV_DEBUG << "No suitable planar graph neighbor found in perimeter routing: firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            return L3Address();
        }
        else if (firstSenderAddress == selfAddress && firstReceiverAddress == selectedNeighborAddress) {
            EV_DEBUG << "End of perimeter reached: firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            if (displayBubbles && hasGUI())
                getContainingNode(host)->bubble("End of perimeter reached");
            return L3Address();
        }
        else {
            if (gpsrOption->getCurrentFaceFirstReceiverAddress().isUnspecified())
                gpsrOption->setCurrentFaceFirstReceiverAddress(selectedNeighborAddress);
            return selectedNeighborAddress;
        }
    }
}

//
// routing
//

INetfilter::IHook::Result Gpsr::routeDatagram(Packet *datagram, GpsrOption *gpsrOption)
{
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& source = networkHeader->getSourceAddress();
    const L3Address& destination = networkHeader->getDestinationAddress();
    EV_INFO << "Finding next hop: source = " << source << ", destination = " << destination << endl;
    auto nextHop = findNextHop(destination, gpsrOption);
    datagram->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(nextHop);
    if (nextHop.isUnspecified()) {
        EV_WARN << "No next hop found, dropping packet: source = " << source << ", destination = " << destination << endl;
        if (displayBubbles && hasGUI())
            getContainingNode(host)->bubble("No next hop found, dropping packet");
        return DROP;
    }
    else {
        EV_INFO << "Next hop found: source = " << source << ", destination = " << destination << ", nextHop: " << nextHop << endl;
        gpsrOption->setSenderAddress(getSelfAddress());
        auto networkInterface = CHK(interfaceTable->findInterfaceByName(outputInterface));
        datagram->addTagIfAbsent<InterfaceReq>()->setInterfaceId(networkInterface->getInterfaceId());
        return ACCEPT;
    }
}

void Gpsr::setGpsrOptionOnNetworkDatagram(Packet *packet, const Ptr<const NetworkHeaderBase>& networkHeader, GpsrOption *gpsrOption)
{
    packet->trimFront();
#ifdef INET_WITH_IPv4
    if (dynamicPtrCast<const Ipv4Header>(networkHeader)) {
        auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
        gpsrOption->setType(IPOPTION_TLV_GPSR);
        B oldHlen = ipv4Header->calculateHeaderByteLength();
        ASSERT(ipv4Header->getHeaderLength() == oldHlen);
        ipv4Header->addOption(gpsrOption);
        B newHlen = ipv4Header->calculateHeaderByteLength();
        ipv4Header->setHeaderLength(newHlen);
        ipv4Header->addChunkLength(newHlen - oldHlen);
        ipv4Header->setTotalLengthField(ipv4Header->getTotalLengthField() + newHlen - oldHlen);
        insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
    }
    else
#endif
#ifdef INET_WITH_IPv6
    if (dynamicPtrCast<const Ipv6Header>(networkHeader)) {
        auto ipv6Header = removeNetworkProtocolHeader<Ipv6Header>(packet);
        gpsrOption->setType(IPv6TLVOPTION_TLV_GPSR);
        B oldHlen = ipv6Header->calculateHeaderByteLength();
        Ipv6HopByHopOptionsHeader *hdr = check_and_cast_nullable<Ipv6HopByHopOptionsHeader *>(ipv6Header->findExtensionHeaderByTypeForUpdate(IP_PROT_IPv6EXT_HOP));
        if (hdr == nullptr) {
            hdr = new Ipv6HopByHopOptionsHeader();
            hdr->setByteLength(B(8));
            ipv6Header->addExtensionHeader(hdr);
        }
        hdr->getTlvOptionsForUpdate().appendTlvOption(gpsrOption);
        hdr->setByteLength(B(utils::roundUp(2 + B(hdr->getTlvOptions().getLength()).get(), 8)));
        B newHlen = ipv6Header->calculateHeaderByteLength();
        ipv6Header->addChunkLength(newHlen - oldHlen);
        insertNetworkProtocolHeader(packet, Protocol::ipv6, ipv6Header);
    }
    else
#endif
#ifdef INET_WITH_NEXTHOP
    if (dynamicPtrCast<const NextHopForwardingHeader>(networkHeader)) {
        auto nextHopHeader = removeNetworkProtocolHeader<NextHopForwardingHeader>(packet);
        gpsrOption->setType(NEXTHOP_TLVOPTION_TLV_GPSR);
        int oldHlen = nextHopHeader->getTlvOptions().getLength();
        nextHopHeader->getTlvOptionsForUpdate().appendTlvOption(gpsrOption);
        int newHlen = nextHopHeader->getTlvOptions().getLength();
        nextHopHeader->addChunkLength(B(newHlen - oldHlen));
        insertNetworkProtocolHeader(packet, Protocol::nextHopForwarding, nextHopHeader);
    }
    else
#endif
    {
    }
}

const GpsrOption *Gpsr::findGpsrOptionInNetworkDatagram(const Ptr<const NetworkHeaderBase>& networkHeader) const
{
    const GpsrOption *gpsrOption = nullptr;

#ifdef INET_WITH_IPv4
    if (auto ipv4Header = dynamicPtrCast<const Ipv4Header>(networkHeader)) {
        gpsrOption = check_and_cast_nullable<const GpsrOption *>(ipv4Header->findOptionByType(IPOPTION_TLV_GPSR));
    }
    else
#endif
#ifdef INET_WITH_IPv6
    if (auto ipv6Header = dynamicPtrCast<const Ipv6Header>(networkHeader)) {
        const Ipv6HopByHopOptionsHeader *hdr = check_and_cast_nullable<const Ipv6HopByHopOptionsHeader *>(ipv6Header->findExtensionHeaderByType(IP_PROT_IPv6EXT_HOP));
        if (hdr != nullptr) {
            int i = (hdr->getTlvOptions().findByType(IPv6TLVOPTION_TLV_GPSR));
            if (i >= 0)
                gpsrOption = check_and_cast<const GpsrOption *>(hdr->getTlvOptions().getTlvOption(i));
        }
    }
    else
#endif
#ifdef INET_WITH_NEXTHOP
    if (auto nextHopHeader = dynamicPtrCast<const NextHopForwardingHeader>(networkHeader)) {
        int i = (nextHopHeader->getTlvOptions().findByType(NEXTHOP_TLVOPTION_TLV_GPSR));
        if (i >= 0)
            gpsrOption = check_and_cast<const GpsrOption *>(nextHopHeader->getTlvOptions().getTlvOption(i));
    }
    else
#endif
    {
    }
    return gpsrOption;
}

GpsrOption *Gpsr::findGpsrOptionInNetworkDatagramForUpdate(const Ptr<NetworkHeaderBase>& networkHeader)
{
    GpsrOption *gpsrOption = nullptr;

#ifdef INET_WITH_IPv4
    if (auto ipv4Header = dynamicPtrCast<Ipv4Header>(networkHeader)) {
        gpsrOption = check_and_cast_nullable<GpsrOption *>(ipv4Header->findMutableOptionByType(IPOPTION_TLV_GPSR));
    }
    else
#endif
#ifdef INET_WITH_IPv6
    if (auto ipv6Header = dynamicPtrCast<Ipv6Header>(networkHeader)) {
        Ipv6HopByHopOptionsHeader *hdr = check_and_cast_nullable<Ipv6HopByHopOptionsHeader *>(ipv6Header->findExtensionHeaderByTypeForUpdate(IP_PROT_IPv6EXT_HOP));
        if (hdr != nullptr) {
            int i = (hdr->getTlvOptions().findByType(IPv6TLVOPTION_TLV_GPSR));
            if (i >= 0)
                gpsrOption = check_and_cast<GpsrOption *>(hdr->getTlvOptionsForUpdate().getTlvOptionForUpdate(i));
        }
    }
    else
#endif
#ifdef INET_WITH_NEXTHOP
    if (auto nextHopHeader = dynamicPtrCast<NextHopForwardingHeader>(networkHeader)) {
        int i = (nextHopHeader->getTlvOptions().findByType(NEXTHOP_TLVOPTION_TLV_GPSR));
        if (i >= 0)
            gpsrOption = check_and_cast<GpsrOption *>(nextHopHeader->getTlvOptionsForUpdate().getTlvOptionForUpdate(i));
    }
    else
#endif
    {
    }
    return gpsrOption;
}

const GpsrOption *Gpsr::getGpsrOptionFromNetworkDatagram(const Ptr<const NetworkHeaderBase>& networkHeader) const
{
    const GpsrOption *gpsrOption = findGpsrOptionInNetworkDatagram(networkHeader);
    if (gpsrOption == nullptr)
        throw cRuntimeError("Gpsr option not found in datagram!");
    return gpsrOption;
}

GpsrOption *Gpsr::getGpsrOptionFromNetworkDatagramForUpdate(const Ptr<NetworkHeaderBase>& networkHeader)
{
    GpsrOption *gpsrOption = findGpsrOptionInNetworkDatagramForUpdate(networkHeader);
    if (gpsrOption == nullptr)
        throw cRuntimeError("Gpsr option not found in datagram!");
    return gpsrOption;
}

//
// netfilter
//

INetfilter::IHook::Result Gpsr::datagramPreRoutingHook(Packet *datagram)
{
    Enter_Method("datagramPreRoutingHook");
    const auto& networkHeader = getNetworkProtocolHeader(datagram);
    const L3Address& destination = networkHeader->getDestinationAddress();
    if (destination.isMulticast() || destination.isBroadcast() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else {
        // KLUDGE this allows overwriting the GPSR option inside
        auto gpsrOption = const_cast<GpsrOption *>(getGpsrOptionFromNetworkDatagram(networkHeader));
        return routeDatagram(datagram, gpsrOption);
    }
}

INetfilter::IHook::Result Gpsr::datagramLocalOutHook(Packet *packet)
{
    Enter_Method("datagramLocalOutHook");
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    const L3Address& destination = networkHeader->getDestinationAddress();
    if (destination.isMulticast() || destination.isBroadcast() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else {
        GpsrOption *gpsrOption = createGpsrOption(networkHeader->getDestinationAddress());
        setGpsrOptionOnNetworkDatagram(packet, networkHeader, gpsrOption);
        return routeDatagram(packet, gpsrOption);
    }
}

//
// lifecycle
//

void Gpsr::handleStartOperation(LifecycleOperation *operation)
{
    configureInterfaces();
    storeSelfPositionInGlobalRegistry();
    scheduleBeaconTimer();
}

void Gpsr::handleStopOperation(LifecycleOperation *operation)
{
    // TODO send a beacon to remove ourself from peers neighbor position table
    neighborPositionTable.clear();
    cancelEvent(beaconTimer);
    cancelEvent(purgeNeighborsTimer);
}

void Gpsr::handleCrashOperation(LifecycleOperation *operation)
{
    neighborPositionTable.clear();
    cancelEvent(beaconTimer);
    cancelEvent(purgeNeighborsTimer);
}

//
// notification
//

void Gpsr::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == linkBrokenSignal) {
        EV_WARN << "Received link break" << endl;
        // TODO remove the neighbor
    }
}

} // namespace inet

