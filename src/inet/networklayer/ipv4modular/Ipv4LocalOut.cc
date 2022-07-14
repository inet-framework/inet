//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4LocalOut.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/MulticastTag_m.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(Ipv4LocalOut);

void Ipv4LocalOut::initialize(int stage)
{
    PacketPusherBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        icmp.reference(this, "icmpModule", true);
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        limitedBroadcast = par("limitedBroadcast");
    }
}

void Ipv4LocalOut::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);

    datagramLocalOut(packet);
}

const NetworkInterface *Ipv4LocalOut::getSourceInterface(Packet *packet)
{
    const auto& tag = packet->findTag<InterfaceInd>();
    return tag != nullptr ? ift->getInterfaceById(tag->getInterfaceId()) : nullptr;
}

const NetworkInterface *Ipv4LocalOut::getDestInterface(Packet *packet)
{
    const auto& tag = packet->findTag<InterfaceReq>();
    return tag != nullptr ? ift->getInterfaceById(tag->getInterfaceId()) : nullptr;
}

Ipv4Address Ipv4LocalOut::getNextHop(Packet *packet)
{
    const auto& tag = packet->findTag<NextHopAddressReq>();
    return tag != nullptr ? tag->getNextHopAddress().toIpv4() : Ipv4Address::UNSPECIFIED_ADDRESS;
}

void Ipv4LocalOut::datagramLocalOut(Packet *packet)
{
    Ipv4Address destAddr = packet->peekAtFront<Ipv4Header>()->getDestAddress();

    EV_DETAIL << "Sending datagram '" << packet->getName() << "' with destination = " << destAddr << "\n";

    if (destAddr.isMulticast()) {
        routeMulticastPacket(packet);
    }
    else { // unicast and broadcast
        const NetworkInterface *destIE = getDestInterface(packet);
        // check for local delivery
        if (rt->isLocalAddress(destAddr)) {
            EV_INFO << "Delivering " << packet << " locally.\n";
            if (destIE && !destIE->isLoopback()) {
                EV_WARN << "datagram destination address is local, ignoring non-loopback destination interface '" << destIE->getInterfaceName() << "' specified in the InterfaceReq tag\n";
                destIE = nullptr;
            }
            if (!destIE) {
                destIE = ift->findFirstLoopbackInterface();
                packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE ? destIE->getInterfaceId() : -1);
            }
            ASSERT(destIE);
            packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(destAddr);
            routeUnicastPacket(packet);
        }
        else if (destAddr.isLimitedBroadcastAddress() || rt->isLocalBroadcastAddress(destAddr)) {
            routeLocalBroadcastPacket(packet);
        }
        else {
            if (packet->findTag<NextHopAddressReq>() == nullptr)
                packet->addTag<NextHopAddressReq>()->setNextHopAddress(Ipv4Address::UNSPECIFIED_ADDRESS);
            routeUnicastPacket(packet);
        }
    }
}

/* Choose the outgoing interface for the multicast datagram:
 *   1. use the interface specified by MULTICAST_IF socket option (received in the control info)
 *   2. lookup the destination address in the routing table
 *   3. if no route, choose the interface according to the source address
 *   4. or if the source address is unspecified, choose the first MULTICAST interface
 */
const NetworkInterface *Ipv4LocalOut::determineOutgoingInterfaceForMulticastDatagram(const Ptr<const Ipv4Header>& ipv4Header, const NetworkInterface *multicastIFOption)
{
    const NetworkInterface *ie = nullptr;

    if (multicastIFOption) {
        ie = multicastIFOption;
        EV_DETAIL << "multicast packet routed by socket option via output interface " << ie->getInterfaceName() << "\n";
    }

    if (!ie) {
        Ipv4Route *route = rt->findBestMatchingRoute(ipv4Header->getDestAddress());
        if (route)
            ie = route->getInterface();
        if (ie)
            EV_DETAIL << "multicast packet routed by routing table via output interface " << ie->getInterfaceName() << "\n";
    }

    if (!ie) {
        ie = rt->getInterfaceByAddress(ipv4Header->getSrcAddress());
        if (ie)
            EV_DETAIL << "multicast packet routed by source address via output interface " << ie->getInterfaceName() << "\n";
    }

    if (!ie) {
        ie = ift->findFirstMulticastInterface();
        if (ie)
            EV_DETAIL << "multicast packet routed via the first multicast interface " << ie->getInterfaceName() << "\n";
    }

    return ie;
}

void Ipv4LocalOut::routeUnicastPacket(Packet *packet)
{
    const NetworkInterface *destIE = getDestInterface(packet);
    Ipv4Address destAddr = packet->peekAtFront<Ipv4Header>()->getDestAddress();

    EV_INFO << "Routing " << packet << " with destination = " << destAddr << ", ";

    // if output port was explicitly requested, use that, otherwise use Ipv4 routing
    if (destIE) {
        EV_DETAIL << "using manually specified output interface " << destIE->getInterfaceName() << "\n";

        Ipv4Address nextHopAddress = getNextHop(packet);

        // and nextHopAddr remains unspecified
        if (!nextHopAddress.isUnspecified()) {
            // do nothing, next hop address already specified
        }
        // special case ICMP reply
        else if (destIE->isBroadcast()) {
            // if the interface is broadcast we must search the next hop
            const Ipv4Route *re = rt->findBestMatchingRoute(destAddr);
            if (re && re->getInterface() == destIE) {
                packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(re->getGateway());
            }
        }
    }
    else {
        // use Ipv4 routing (lookup in routing table)
        const Ipv4Route *re = rt->findBestMatchingRoute(destAddr);
        if (re) {
            destIE = re->getInterface();
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());
            packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(re->getGateway());
        }
    }

    if (!destIE) { // no route found
        auto interfaceIndTag = packet->findTag<InterfaceInd>();
        int fromIeId = interfaceIndTag ? interfaceIndTag->getInterfaceId() : -1;
        EV_WARN << "unroutable, sending ICMP_DESTINATION_UNREACHABLE, dropping packet\n";
        icmp->sendErrorMessage(packet, fromIeId, ICMP_DESTINATION_UNREACHABLE, 0);
        dropPacket(packet, NO_ROUTE_FOUND);
    }
    else { // fragment and send
        setHeaderSrcAddressByDestInterface(packet);
        pushOrSendPacket(packet, outputGate, consumer);
    }
}

void Ipv4LocalOut::routeMulticastPacket(Packet *packet)
{
    const NetworkInterface *destIE = getDestInterface(packet);
    Ipv4Address requestedNextHopAddress = getNextHop(packet);

    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    Ipv4Address destAddr = ipv4Header->getDestAddress();

    bool multicastLoop = false;
    const auto& mcr = packet->findTag<MulticastReq>();
    if (mcr != nullptr) {
        multicastLoop = mcr->getMulticastLoop();
    }

    destIE = determineOutgoingInterfaceForMulticastDatagram(ipv4Header, destIE);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE ? destIE->getInterfaceId() : -1);

    // loop back a copy
    if (multicastLoop && (!destIE || !destIE->isLoopback())) {
        const NetworkInterface *loopbackIF = ift->findFirstLoopbackInterface();
        if (loopbackIF) {
            auto packetCopy = packet->dup();
            packetCopy->addTagIfAbsent<InterfaceReq>()->setInterfaceId(loopbackIF->getInterfaceId());
            packetCopy->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(destAddr);
            setHeaderSrcAddressByDestInterface(packetCopy);
            pushOrSendPacket(packetCopy, outputGate, consumer);
        }
    }

    if (destIE) {
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId()); // KLUDGE is it needed?
        packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(destAddr);
        setHeaderSrcAddressByDestInterface(packet);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else {
        EV_ERROR << "No multicast interface, packet dropped\n";
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Ipv4LocalOut::routeLocalBroadcastPacket(Packet *packet)
{
    const NetworkInterface *destIE = getDestInterface(packet);
    // The destination address is 255.255.255.255 or local subnet broadcast address.
    // We always use 255.255.255.255 as nextHopAddress, because it is recognized by ARP,
    // and mapped to the broadcast MAC address.
    if (destIE != nullptr) {
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId()); // KLUDGE is it needed?
        packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(Ipv4Address::ALLONES_ADDRESS);
        setHeaderSrcAddressByDestInterface(packet);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else if (limitedBroadcast) {
        auto destAddr = packet->peekAtFront<Ipv4Header>()->getDestAddress();
        // forward to each matching interfaces including loopback
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            const NetworkInterface *ie = ift->getInterface(i);
            if (!destAddr.isLimitedBroadcastAddress()) {
                Ipv4Address interfaceAddr = ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
                Ipv4Address broadcastAddr = interfaceAddr.makeBroadcastAddress(ie->getProtocolData<Ipv4InterfaceData>()->getNetmask());
                if (destAddr != broadcastAddr)
                    continue;
            }
            auto packetCopy = packet->dup();
            packetCopy->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
            packetCopy->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(Ipv4Address::ALLONES_ADDRESS);
            setHeaderSrcAddressByDestInterface(packetCopy);
            pushOrSendPacket(packetCopy, outputGate, consumer);
        }
        delete packet;
    }
    else {
        PacketDropDetails details;
        details.setReason(NO_INTERFACE_FOUND);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Ipv4LocalOut::setHeaderSrcAddressByDestInterface(Packet *packet)
{
    const NetworkInterface *destIE = ift->getInterfaceById(packet->getTag<InterfaceReq>()->getInterfaceId());
    // fill in source address
    if (packet->peekAtFront<Ipv4Header>()->getSrcAddress().isUnspecified()) {
        auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
        ipv4Header->setSrcAddress(destIE->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
    }
}

} // namespace inet
