//
// Copyright (C) 2013 Opensim Ltd
// Author: Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/routing/gpsr/GPSR.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Header.h"
#endif

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#endif

#ifdef WITH_GENERIC
#include "inet/networklayer/generic/GenericDatagram.h"
#endif

namespace inet {

Define_Module(GPSR);

static inline double determinant(double a1, double a2, double b1, double b2)
{
    return a1 * b2 - a2 * b1;
}

// KLUDGE: implement position registry protocol
PositionTable GPSR::globalPositionTable;

GPSR::GPSR()
{
}

GPSR::~GPSR()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(purgeNeighborsTimer);
}

//
// module interface
//

void GPSR::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // GPSR parameters
        planarizationMode = (GPSRPlanarizationMode)(int)par("planarizationMode");
        interfaces = par("interfaces");
        beaconInterval = par("beaconInterval");
        maxJitter = par("maxJitter");
        neighborValidityInterval = par("neighborValidityInterval");
        // context
        host = getContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        outputInterface = par("outputInterface");
        mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        routingTable = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
        // internal
        beaconTimer = new cMessage("BeaconTimer");
        purgeNeighborsTimer = new cMessage("PurgeNeighborsTimer");

        // packet size
        positionByteLength = par("positionByteLength");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerProtocol(Protocol::manet, gate("ipOut"));
        globalPositionTable.clear();
        host->subscribe(NF_LINK_BREAK, this);
        addressType = getSelfAddress().getAddressType();
        networkProtocol->registerHook(0, this);
        if (isNodeUp()) {
            configureInterfaces();
            scheduleBeaconTimer();
            schedulePurgeNeighborsTimer();
        }
        WATCH(neighborPositionTable);
    }
}

void GPSR::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

//
// handling messages
//

void GPSR::processSelfMessage(cMessage *message)
{
    if (message == beaconTimer)
        processBeaconTimer();
    else if (message == purgeNeighborsTimer)
        processPurgeNeighborsTimer();
    else
        throw cRuntimeError("Unknown self message");
}

void GPSR::processMessage(cMessage *message)
{
    if (auto pk = dynamic_cast<Packet *>(message))
        processUDPPacket(pk);
    else
        throw cRuntimeError("Unknown message");
}

//
// beacon timers
//

void GPSR::scheduleBeaconTimer()
{
    EV_DEBUG << "Scheduling beacon timer" << endl;
    scheduleAt(simTime() + beaconInterval, beaconTimer);
}

void GPSR::processBeaconTimer()
{
    EV_DEBUG << "Processing beacon timer" << endl;
    L3Address selfAddress = getSelfAddress();
    if (!selfAddress.isUnspecified()) {
        sendBeacon(createBeacon(), uniform(0, maxJitter).dbl());
        // KLUDGE: implement position registry protocol
        globalPositionTable.setPosition(selfAddress, mobility->getCurrentPosition());
    }
    scheduleBeaconTimer();
    schedulePurgeNeighborsTimer();
}

//
// handling purge neighbors timers
//

void GPSR::schedulePurgeNeighborsTimer()
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
                cancelEvent(purgeNeighborsTimer);
                scheduleAt(nextExpiration, purgeNeighborsTimer);
            }
        }
    }
}

void GPSR::processPurgeNeighborsTimer()
{
    EV_DEBUG << "Processing purge neighbors timer" << endl;
    purgeNeighbors();
    schedulePurgeNeighborsTimer();
}

//
// handling UDP packets
//

void GPSR::sendUDPPacket(Packet *packet, double delay)
{
    if (delay == 0)
        send(packet, "ipOut");
    else
        sendDelayed(packet, delay, "ipOut");
}

void GPSR::processUDPPacket(Packet *packet)
{
    packet->popHeader<UdpHeader>();
    processBeacon(packet);
}

//
// handling beacons
//

Ptr<GPSRBeacon> GPSR::createBeacon()
{
    const auto& beacon = std::make_shared<GPSRBeacon>();
    beacon->setAddress(getSelfAddress());
    beacon->setPosition(mobility->getCurrentPosition());
    beacon->setChunkLength(byte(getSelfAddress().getAddressType()->getAddressByteLength() + positionByteLength));
    return beacon;
}

void GPSR::sendBeacon(const Ptr<GPSRBeacon>& beacon, double delay)
{
    EV_INFO << "Sending beacon: address = " << beacon->getAddress() << ", position = " << beacon->getPosition() << endl;
    Packet *udpPacket = new Packet("GPRSBeacon");
    beacon->markImmutable();
    udpPacket->append(beacon);
    auto udpHeader = std::make_shared<UdpHeader>();
    udpHeader->setSourcePort(GPSR_UDP_PORT);
    udpHeader->setDestinationPort(GPSR_UDP_PORT);
    udpHeader->markImmutable();
    udpPacket->pushHeader(udpHeader);
    auto addresses = udpPacket->ensureTag<L3AddressReq>();
    addresses->setSrcAddress(getSelfAddress());
    addresses->setDestAddress(addressType->getLinkLocalManetRoutersMulticastAddress());
    udpPacket->ensureTag<HopLimitReq>()->setHopLimit(255);
    udpPacket->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    udpPacket->ensureTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    sendUDPPacket(udpPacket, delay);
}

void GPSR::processBeacon(Packet *packet)
{
    const auto& beacon = packet->peekHeader<GPSRBeacon>();
    EV_INFO << "Processing beacon: address = " << beacon->getAddress() << ", position = " << beacon->getPosition() << endl;
    neighborPositionTable.setPosition(beacon->getAddress(), beacon->getPosition());
    delete packet;
}
//
// handling packets
//

GPSROption *GPSR::createGpsrOption(L3Address destination)
{
    GPSROption *gpsrOption = new GPSROption();
    gpsrOption->setRoutingMode(GPSR_GREEDY_ROUTING);
    // KLUDGE: implement position registry protocol
    gpsrOption->setDestinationPosition(getDestinationPosition(destination));
    gpsrOption->setLength(computeOptionLength(gpsrOption));
    return gpsrOption;
}

int GPSR::computeOptionLength(GPSROption *option)
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

bool GPSR::isNodeUp() const
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void GPSR::configureInterfaces()
{
    // join multicast groups
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
        if (interfaceEntry->isMulticast() && interfaceMatcher.matches(interfaceEntry->getName()))
            interfaceEntry->joinMulticastGroup(addressType->getLinkLocalManetRoutersMulticastAddress());
    }
}

//
// position
//

Coord GPSR::intersectSections(Coord& begin1, Coord& end1, Coord& begin2, Coord& end2)
{
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
    if (x1 < x && x < x2 && x3 < x && x < x4 && y1 < y && y < y2 && y3 < y && y < y4)
        return Coord(x, y, 0);
    else
        return Coord(NaN, NaN, NaN);
}

Coord GPSR::getDestinationPosition(const L3Address& address) const
{
    // KLUDGE: implement position registry protocol
    return globalPositionTable.getPosition(address);
}

Coord GPSR::getNeighborPosition(const L3Address& address) const
{
    return neighborPositionTable.getPosition(address);
}

//
// angle
//

double GPSR::getVectorAngle(Coord vector)
{
    double angle = atan2(-vector.y, vector.x);
    if (angle < 0)
        angle += 2 * M_PI;
    return angle;
}

double GPSR::getDestinationAngle(const L3Address& address)
{
    return getVectorAngle(getDestinationPosition(address) - mobility->getCurrentPosition());
}

double GPSR::getNeighborAngle(const L3Address& address)
{
    return getVectorAngle(getNeighborPosition(address) - mobility->getCurrentPosition());
}

//
// address
//

std::string GPSR::getHostName() const
{
    return host->getFullName();
}

L3Address GPSR::getSelfAddress() const
{
    //TODO choose self address based on a new 'interfaces' parameter
    L3Address ret = routingTable->getRouterIdAsGeneric();
#ifdef WITH_IPv6
    if (ret.getType() == L3Address::IPv6) {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry *ie = interfaceTable->getInterface(i);
            if ((!ie->isLoopback()) && ie->ipv6Data() != nullptr) {
                ret = interfaceTable->getInterface(i)->ipv6Data()->getPreferredAddress();
                break;
            }
        }
    }
#endif
    return ret;
}

L3Address GPSR::getSenderNeighborAddress(const Ptr<const NetworkHeaderBase>& datagram) const
{
    const GPSROption *gpsrOption = getGpsrOptionFromNetworkDatagram(datagram);
    return gpsrOption->getSenderAddress();
}

//
// neighbor
//

simtime_t GPSR::getNextNeighborExpiration()
{
    simtime_t oldestPosition = neighborPositionTable.getOldestPosition();
    if (oldestPosition == SimTime::getMaxTime())
        return oldestPosition;
    else
        return oldestPosition + neighborValidityInterval;
}

void GPSR::purgeNeighbors()
{
    neighborPositionTable.removeOldPositions(simTime() - neighborValidityInterval);
}

std::vector<L3Address> GPSR::getPlanarNeighbors()
{
    std::vector<L3Address> planarNeighbors;
    std::vector<L3Address> neighborAddresses = neighborPositionTable.getAddresses();
    Coord selfPosition = mobility->getCurrentPosition();
    for (auto it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        const L3Address& neighborAddress = *it;
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        if (planarizationMode == GPSR_RNG_PLANARIZATION) {
            double neighborDistance = (neighborPosition - selfPosition).length();
            for (auto & neighborAddresse : neighborAddresses) {
                const L3Address& witnessAddress = neighborAddresse;
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - selfPosition).length();
                ;
                double neighborWitnessDistance = (witnessPosition - neighborPosition).length();
                if (*it == neighborAddresse)
                    continue;
                else if (neighborDistance > std::max(witnessDistance, neighborWitnessDistance))
                    goto eliminate;
            }
        }
        else if (planarizationMode == GPSR_GG_PLANARIZATION) {
            Coord middlePosition = (selfPosition + neighborPosition) / 2;
            double neighborDistance = (neighborPosition - middlePosition).length();
            for (auto & neighborAddresse : neighborAddresses) {
                const L3Address& witnessAddress = neighborAddresse;
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - middlePosition).length();
                ;
                if (*it == neighborAddresse)
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

L3Address GPSR::getNextPlanarNeighborCounterClockwise(const L3Address& startNeighborAddress, double startNeighborAngle)
{
    EV_DEBUG << "Finding next planar neighbor (counter clockwise): startAddress = " << startNeighborAddress << ", startAngle = " << startNeighborAngle << endl;
    L3Address bestNeighborAddress = startNeighborAddress;
    double bestNeighborAngleDifference = 2 * M_PI;
    std::vector<L3Address> neighborAddresses = getPlanarNeighbors();
    for (auto & neighborAddress : neighborAddresses) {
        double neighborAngle = getNeighborAngle(neighborAddress);
        double neighborAngleDifference = neighborAngle - startNeighborAngle;
        if (neighborAngleDifference < 0)
            neighborAngleDifference += 2 * M_PI;
        EV_DEBUG << "Trying next planar neighbor (counter clockwise): address = " << neighborAddress << ", angle = " << neighborAngle << endl;
        if (neighborAngleDifference != 0 && neighborAngleDifference < bestNeighborAngleDifference) {
            bestNeighborAngleDifference = neighborAngleDifference;
            bestNeighborAddress = neighborAddress;
        }
    }
    return bestNeighborAddress;
}

//
// next hop
//

L3Address GPSR::findNextHop(const Ptr<const NetworkHeaderBase>& datagram, const L3Address& destination)
{
    const GPSROption *gpsrOption = getGpsrOptionFromNetworkDatagram(datagram);
    switch (gpsrOption->getRoutingMode()) {
        case GPSR_GREEDY_ROUTING: return findGreedyRoutingNextHop(datagram, destination);
        case GPSR_PERIMETER_ROUTING: return findPerimeterRoutingNextHop(datagram, destination);
        default: throw cRuntimeError("Unknown routing mode");
    }
}

L3Address GPSR::findGreedyRoutingNextHop(const Ptr<const NetworkHeaderBase>& datagram, const L3Address& destination)
{
    EV_DEBUG << "Finding next hop using greedy routing: destination = " << destination << endl;
    const GPSROption *gpsrOption = getGpsrOptionFromNetworkDatagram(datagram);
    L3Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord destinationPosition = gpsrOption->getDestinationPosition();
    double bestDistance = (destinationPosition - selfPosition).length();
    L3Address bestNeighbor;
    std::vector<L3Address> neighborAddresses = neighborPositionTable.getAddresses();
    for (auto& neighborAddress: neighborAddresses) {
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        double neighborDistance = (destinationPosition - neighborPosition).length();
        if (neighborDistance < bestDistance) {
            bestDistance = neighborDistance;
            bestNeighbor = neighborAddress;
        }
    }
    if (bestNeighbor.isUnspecified()) {
        EV_DEBUG << "Switching to perimeter routing: destination = " << destination << endl;
        // KLUDGE: TODO: const_cast<GPSROption *>(gpsrOption)
        const_cast<GPSROption *>(gpsrOption)->setRoutingMode(GPSR_PERIMETER_ROUTING);
        const_cast<GPSROption *>(gpsrOption)->setPerimeterRoutingStartPosition(selfPosition);
        const_cast<GPSROption *>(gpsrOption)->setCurrentFaceFirstSenderAddress(selfAddress);
        const_cast<GPSROption *>(gpsrOption)->setCurrentFaceFirstReceiverAddress(L3Address());
        return findPerimeterRoutingNextHop(datagram, destination);
    }
    else
        return bestNeighbor;
}

L3Address GPSR::findPerimeterRoutingNextHop(const Ptr<const NetworkHeaderBase>& datagram, const L3Address& destination)
{
    EV_DEBUG << "Finding next hop using perimeter routing: destination = " << destination << endl;
    const GPSROption *gpsrOption = getGpsrOptionFromNetworkDatagram(datagram);
    L3Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord perimeterRoutingStartPosition = gpsrOption->getPerimeterRoutingStartPosition();
    Coord destinationPosition = gpsrOption->getDestinationPosition();
    double selfDistance = (destinationPosition - selfPosition).length();
    double perimeterRoutingStartDistance = (destinationPosition - perimeterRoutingStartPosition).length();
    if (selfDistance < perimeterRoutingStartDistance) {
        EV_DEBUG << "Switching to greedy routing: destination = " << destination << endl;
        // KLUDGE: TODO: const_cast<GPSROption *>(gpsrOption)
        const_cast<GPSROption *>(gpsrOption)->setRoutingMode(GPSR_GREEDY_ROUTING);
        const_cast<GPSROption *>(gpsrOption)->setPerimeterRoutingStartPosition(Coord());
        const_cast<GPSROption *>(gpsrOption)->setPerimeterRoutingForwardPosition(Coord());
        return findGreedyRoutingNextHop(datagram, destination);
    }
    else {
        const L3Address& firstSenderAddress = gpsrOption->getCurrentFaceFirstSenderAddress();
        const L3Address& firstReceiverAddress = gpsrOption->getCurrentFaceFirstReceiverAddress();
        L3Address nextNeighborAddress = getSenderNeighborAddress(datagram);
        bool hasIntersection;
        do {
            if (nextNeighborAddress.isUnspecified())
                nextNeighborAddress = getNextPlanarNeighborCounterClockwise(nextNeighborAddress, getDestinationAngle(destination));
            else
                nextNeighborAddress = getNextPlanarNeighborCounterClockwise(nextNeighborAddress, getNeighborAngle(nextNeighborAddress));
            if (nextNeighborAddress.isUnspecified())
                break;
            EV_DEBUG << "Intersecting towards next hop: nextNeighbor = " << nextNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            Coord nextNeighborPosition = getNeighborPosition(nextNeighborAddress);
            Coord intersection = intersectSections(perimeterRoutingStartPosition, destinationPosition, selfPosition, nextNeighborPosition);
            hasIntersection = !std::isnan(intersection.x);
            if (hasIntersection) {
                EV_DEBUG << "Edge to next hop intersects: intersection = " << intersection << ", nextNeighbor = " << nextNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
                // KLUDGE: TODO: const_cast<GPSROption *>(gpsrOption)
                const_cast<GPSROption *>(gpsrOption)->setCurrentFaceFirstSenderAddress(selfAddress);
                const_cast<GPSROption *>(gpsrOption)->setCurrentFaceFirstReceiverAddress(L3Address());
            }
        } while (hasIntersection);
        if (firstSenderAddress == selfAddress && firstReceiverAddress == nextNeighborAddress) {
            EV_DEBUG << "End of perimeter reached: firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            return L3Address();
        }
        else {
            if (gpsrOption->getCurrentFaceFirstReceiverAddress().isUnspecified())
                // KLUDGE: TODO: const_cast<GPSROption *>(gpsrOption)
                const_cast<GPSROption *>(gpsrOption)->setCurrentFaceFirstReceiverAddress(nextNeighborAddress);
            return nextNeighborAddress;
        }
    }
}

//
// routing
//

INetfilter::IHook::Result GPSR::routeDatagram(Packet *datagram, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop)
{
    const auto& networkHeader = peekNetworkHeader(datagram);
    const L3Address& source = networkHeader->getSourceAddress();
    const L3Address& destination = networkHeader->getDestinationAddress();
    EV_INFO << "Finding next hop: source = " << source << ", destination = " << destination << endl;
    nextHop = findNextHop(networkHeader, destination);
    if (nextHop.isUnspecified()) {
        EV_WARN << "No next hop found, dropping packet: source = " << source << ", destination = " << destination << endl;
        return DROP;
    }
    else {
        EV_INFO << "Next hop found: source = " << source << ", destination = " << destination << ", nextHop: " << nextHop << endl;
        const GPSROption *gpsrOption = getGpsrOptionFromNetworkDatagram(networkHeader);
        // KLUDGE: TODO: const_cast<GPSROption *>(gpsrOption)
        const_cast<GPSROption *>(gpsrOption)->setSenderAddress(getSelfAddress());
        outputInterfaceEntry = interfaceTable->getInterfaceByName(outputInterface);
        ASSERT(outputInterfaceEntry);
        return ACCEPT;
    }
}

void GPSR::setGpsrOptionOnNetworkDatagram(Packet *packet, const Ptr<const NetworkHeaderBase>& nwHeader)
{
    packet->removePoppedHeaders();
    GPSROption *gpsrOption = createGpsrOption(nwHeader->getDestinationAddress());
#ifdef WITH_IPv4
    if (std::dynamic_pointer_cast<const IPv4Header>(nwHeader)) {
        auto ipv4Header = packet->removeHeader<IPv4Header>();
        gpsrOption->setType(IPOPTION_TLV_GPSR);
        int oldHlen = ipv4Header->calculateHeaderByteLength();
        ASSERT(ipv4Header->getHeaderLength() == oldHlen);
        ipv4Header->addOption(gpsrOption);
        int newHlen = ipv4Header->calculateHeaderByteLength();
        ipv4Header->setHeaderLength(newHlen);
        ipv4Header->setChunkLength(ipv4Header->getChunkLength() + byte(newHlen - oldHlen));  // it was ipv4Header->addByteLength(newHlen - oldHlen);
        ipv4Header->setTotalLengthField(ipv4Header->getTotalLengthField() + newHlen - oldHlen);
        packet->insertHeader(ipv4Header);
    }
    else
#endif
#ifdef WITH_IPv6
    if (std::dynamic_pointer_cast<const IPv6Header>(nwHeader)) {
        auto dgram = packet->removeHeader<IPv6Header>();
        gpsrOption->setType(IPv6TLVOPTION_TLV_GPSR);
        int oldHlen = dgram->calculateHeaderByteLength();
        IPv6HopByHopOptionsHeader *hdr = check_and_cast_nullable<IPv6HopByHopOptionsHeader *>(dgram->findExtensionHeaderByType(IP_PROT_IPv6EXT_HOP));
        if (hdr == nullptr) {
            hdr = new IPv6HopByHopOptionsHeader();
            hdr->setByteLength(8);
            dgram->addExtensionHeader(hdr);
        }
        hdr->getTlvOptions().add(gpsrOption);
        hdr->setByteLength(utils::roundUp(2 + hdr->getTlvOptions().getLength(), 8));
        int newHlen = dgram->calculateHeaderByteLength();
        dgram->setChunkLength(dgram->getChunkLength() + byte(newHlen - oldHlen));  // it was dgram->addByteLength(newHlen - oldHlen);
        packet->insertHeader(dgram);
    }
    else
#endif
#ifdef WITH_GENERIC
    if (std::dynamic_pointer_cast<const GenericDatagramHeader>(nwHeader)) {
        auto dgram = packet->removeHeader<GenericDatagramHeader>();
        gpsrOption->setType(GENERIC_TLVOPTION_TLV_GPSR);
        int oldHlen = dgram->getTlvOptions().getLength();
        dgram->getTlvOptions().add(gpsrOption);
        int newHlen = dgram->getTlvOptions().getLength();
        dgram->setChunkLength(dgram->getChunkLength() + byte(newHlen - oldHlen));  // it was dgram->addByteLength(newHlen - oldHlen);
        packet->insertHeader(dgram);
    }
    else
#endif
    {
    }
}

const GPSROption *GPSR::findGpsrOptionInNetworkDatagram(const Ptr<const NetworkHeaderBase>& datagram)
{
    const GPSROption *gpsrOption = nullptr;

#ifdef WITH_IPv4
    if (auto dgram = std::dynamic_pointer_cast<const IPv4Header>(datagram)) {
        gpsrOption = check_and_cast_nullable<const GPSROption *>(dgram->findOptionByType(IPOPTION_TLV_GPSR));
    }
    else
#endif
#ifdef WITH_IPv6
    if (auto dgram = std::dynamic_pointer_cast<const IPv6Header>(datagram)) {
        IPv6HopByHopOptionsHeader *hdr = check_and_cast_nullable<IPv6HopByHopOptionsHeader *>(dgram->findExtensionHeaderByType(IP_PROT_IPv6EXT_HOP));
        if (hdr != nullptr) {
            int i = (hdr->getTlvOptions().findByType(IPv6TLVOPTION_TLV_GPSR));
            if (i >= 0)
                gpsrOption = check_and_cast_nullable<GPSROption *>(&(hdr->getTlvOptions().at(i)));
        }
    }
    else
#endif
#ifdef WITH_GENERIC
    if (auto dgram = std::dynamic_pointer_cast<const GenericDatagramHeader>(datagram)) {
        int i = (dgram->getTlvOptions().findByType(GENERIC_TLVOPTION_TLV_GPSR));
        if (i >= 0)
            gpsrOption = check_and_cast_nullable<const GPSROption *>(&(dgram->getTlvOptions().at(i)));
    }
    else
#endif
    {
    }
    return gpsrOption;
}

const GPSROption *GPSR::getGpsrOptionFromNetworkDatagram(const Ptr<const NetworkHeaderBase>& datagram)
{
    const GPSROption *gpsrOption = findGpsrOptionInNetworkDatagram(datagram);
    if (gpsrOption == nullptr)
        throw cRuntimeError("GPSR option not found in datagram!");
    return gpsrOption;
}

//
// netfilter
//

INetfilter::IHook::Result GPSR::datagramPreRoutingHook(Packet *datagram, const InterfaceEntry *inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop)
{
    Enter_Method("datagramPreRoutingHook");
    const auto& networkHeader = peekNetworkHeader(datagram);
    const L3Address& destination = networkHeader->getDestinationAddress();
    if (destination.isMulticast() || destination.isBroadcast() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else
        return routeDatagram(datagram, outputInterfaceEntry, nextHop);
}

INetfilter::IHook::Result GPSR::datagramLocalOutHook(Packet *packet, const InterfaceEntry *& outputInterfaceEntry, L3Address& nextHop)
{
    Enter_Method("datagramLocalOutHook");
    const auto& networkHeader = peekNetworkHeader(packet);
    const L3Address& destination = networkHeader->getDestinationAddress();
    if (destination.isMulticast() || destination.isBroadcast() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else {
        setGpsrOptionOnNetworkDatagram(packet, networkHeader);
        return routeDatagram(packet, outputInterfaceEntry, nextHop);
    }
}

//
// lifecycle
//

bool GPSR::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            configureInterfaces();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER) {
            // TODO: send a beacon to remove ourself from peers neighbor position table
            neighborPositionTable.clear();
            cancelEvent(beaconTimer);
            cancelEvent(purgeNeighborsTimer);
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            neighborPositionTable.clear();
            cancelEvent(beaconTimer);
            cancelEvent(purgeNeighborsTimer);
        }
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

//
// notification
//

void GPSR::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("receiveChangeNotification");
    if (signalID == NF_LINK_BREAK) {
        EV_WARN << "Received link break" << endl;
        // TODO: shall we remove the neighbor?
    }
}

} // namespace inet

