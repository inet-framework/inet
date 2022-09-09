//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4PrepareForward.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(Ipv4PrepareForward);

void Ipv4PrepareForward::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        icmp.reference(this, "icmpModule", true);
    }
}

void Ipv4PrepareForward::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);

    // TODO FIXME missing implementation

    // Ipv4::prepareForForwarding(Packet *packet) const
    // decrement TTL:
    const auto& ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
    ipv4Header->setTimeToLive(ipv4Header->getTimeToLive() - 1);
    insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);

    if (ipv4Header->getDestAddress().isMulticast()) {
        forwardMulticastPacket(packet);
    }
    else {
        routeUnicastPacket(packet);
    }
}

//void Ipv4::routeUnicastPacket(Packet *packet)
void Ipv4PrepareForward::routeUnicastPacket(Packet *packet)
{
    int fromIeId = packet->getTag<InterfaceInd>()->getInterfaceId();
    const auto& interfaceReq = packet->findTag<InterfaceReq>();
    const NetworkInterface *destIE = interfaceReq != nullptr ? ift->getInterfaceById(interfaceReq->getInterfaceId()) : nullptr;
    const auto& nextHopAddressReq = packet->findTag<NextHopAddressReq>();
    Ipv4Address nextHopAddress = nextHopAddressReq != nullptr ? nextHopAddressReq->getNextHopAddress().toIpv4() : Ipv4Address::UNSPECIFIED_ADDRESS;

    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    Ipv4Address destAddr = ipv4Header->getDestAddress();
    EV_INFO << "Routing " << packet << " with destination = " << destAddr << ", ";

    // if output port was explicitly requested, use that, otherwise use Ipv4 routing
    if (destIE) {
        EV_DETAIL << "using manually specified output interface " << destIE->getInterfaceName() << "\n";
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
        EV_WARN << "unroutable, sending ICMP_DESTINATION_UNREACHABLE, dropping packet\n";
        icmp->sendErrorMessage(packet, fromIeId, ICMP_DESTINATION_UNREACHABLE, 0);
        dropPacket(packet, NO_ROUTE_FOUND);
    }
    else {
        pushOrSendPacket(packet, outputGate, consumer);
    }
}

void Ipv4PrepareForward::forwardMulticastPacket(Packet *packet)
{
    const NetworkInterface *fromIE = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    const Ipv4Address& srcAddr = ipv4Header->getSrcAddress();
    const Ipv4Address& destAddr = ipv4Header->getDestAddress();
    ASSERT(destAddr.isMulticast());
    ASSERT(!destAddr.isLinkLocalMulticast());

    EV_INFO << "Forwarding multicast datagram `" << packet->getName() << "' with dest=" << destAddr << "\n";

    const Ipv4MulticastRoute *route = rt->findBestMatchingMulticastRoute(srcAddr, destAddr);
    if (!route) {
        EV_WARN << "Multicast route does not exist, try to add.\n";
        // TODO no need to emit fromIE when tags will be used in place of control infos
        emit(ipv4NewMulticastSignal, ipv4Header.get(), const_cast<NetworkInterface *>(fromIE));

        // read new record
        route = rt->findBestMatchingMulticastRoute(srcAddr, destAddr);

        if (!route) {
            EV_ERROR << "No route, packet dropped.\n";
            dropPacket(packet, NO_ROUTE_FOUND);
            return;
        }
    }

    if (route->getInInterface() && fromIE != route->getInInterface()->getInterface()) {
        EV_ERROR << "Did not arrive on input interface, packet dropped.\n";
        // TODO no need to emit fromIE when tags will be used in place of control infos
        emit(ipv4DataOnNonrpfSignal, ipv4Header.get(), const_cast<NetworkInterface *>(fromIE));
        dropPacket(packet, OTHER_PACKET_DROP);
    }
    // backward compatible: no parent means shortest path interface to source (RPB routing)
    else if (!route->getInInterface() && fromIE != rt->getInterfaceForDestAddr(ipv4Header->getSrcAddress())) {
        EV_ERROR << "Did not arrive on shortest path, packet dropped.\n";
        dropPacket(packet, OTHER_PACKET_DROP);
    }
    else {
        // TODO no need to emit fromIE when tags will be used in place of control infos
        emit(ipv4DataOnRpfSignal, ipv4Header.get(), const_cast<NetworkInterface *>(fromIE)); // forwarding hook

        // copy original datagram for multiple destinations
        for (unsigned int i = 0; i < route->getNumOutInterfaces(); i++) {
            Ipv4MulticastRoute::OutInterface *outInterface = route->getOutInterface(i);
            const NetworkInterface *destIE = outInterface->getInterface();
            if (destIE != fromIE && outInterface->isEnabled()) {
                int ttlThreshold = destIE->getProtocolData<Ipv4InterfaceData>()->getMulticastTtlThreshold();
                if (ipv4Header->getTimeToLive() <= ttlThreshold)
                    EV_WARN << "Not forwarding to " << destIE->getInterfaceName() << " (ttl threshold reached)\n";
                else if (outInterface->isLeaf() && !destIE->getProtocolData<Ipv4InterfaceData>()->hasMulticastListener(destAddr))
                    EV_WARN << "Not forwarding to " << destIE->getInterfaceName() << " (no listeners)\n";
                else {
                    EV_DETAIL << "Forwarding to " << destIE->getInterfaceName() << "\n";
                    auto packetCopy = packet->dup();
                    packetCopy->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destIE->getInterfaceId());
                    packetCopy->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(destAddr);
                    pushOrSendPacket(packetCopy, outputGate, consumer);
                }
            }
        }

        // TODO no need to emit fromIE when tags will be use, d in place of control infos
        emit(ipv4MdataRegisterSignal, packet, const_cast<NetworkInterface *>(fromIE)); // postRouting hook

        // only copies sent, delete original packet
        delete packet;
    }
}

} // namespace inet
