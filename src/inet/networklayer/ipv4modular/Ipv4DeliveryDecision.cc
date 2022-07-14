//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4DeliveryDecision.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(Ipv4DeliveryDecision);

void Ipv4DeliveryDecision::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        localOutputGate = gate("localOut");
        forwardOutputGate = gate("forwardOut");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        localConsumer = findConnectedModule<IPassivePacketSink>(localOutputGate);
        forwardConsumer = findConnectedModule<IPassivePacketSink>(forwardOutputGate);

        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        std::string directBroadcastInterfaces = par("directBroadcastInterfaces").stdstringValue();
        directBroadcastInterfaceMatcher.setPattern(directBroadcastInterfaces.c_str(), false, true, false);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(localOutputGate);
        checkPacketOperationSupport(forwardOutputGate);
    }
}

static Ipv4Address getNextHop(Packet *packet)
{
    const auto& tag = packet->findTag<NextHopAddressReq>();
    return tag != nullptr ? tag->getNextHopAddress().toIpv4() : Ipv4Address::UNSPECIFIED_ADDRESS;
}

void Ipv4DeliveryDecision::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);

    ASSERT(packet->getOwner() == this);

    handlePacketProcessed(packet);

    ASSERT(packet->getOwner() == this);

    const NetworkInterface *fromIE = ift->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    Ipv4Address nextHopAddr = getNextHop(packet);

    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    Ipv4Address destAddr = ipv4Header->getDestAddress();

    // route packet

    if (fromIE->isLoopback()) {
        deliverLocally(packet);
    }
    else if (destAddr.isMulticast()) {
        // check for local delivery
        // Note: multicast routers will receive IGMP datagrams even if their interface is not joined to the group
        if (fromIE->getProtocolData<Ipv4InterfaceData>()->isMemberOfMulticastGroup(destAddr) ||
            (rt->isMulticastForwardingEnabled() && ipv4Header->getProtocolId() == IP_PROT_IGMP))
            deliverLocally(packet->dup());
        else
            EV_WARN << "Skip local delivery of multicast datagram (input interface not in multicast group)\n";

        // don't forward if IP forwarding is off, or if dest address is link-scope
        if (!rt->isMulticastForwardingEnabled()) {
            EV_WARN << "Skip forwarding of multicast datagram (forwarding disabled)\n";
            delete packet;
        }
        else if (destAddr.isLinkLocalMulticast()) {
            EV_WARN << "Skip forwarding of multicast datagram (packet is link-local)\n";
            delete packet;
        }
        else if (ipv4Header->getTimeToLive() <= 1) { // TTL before decrement
            EV_WARN << "Skip forwarding of multicast datagram (TTL reached 0)\n";
            delete packet;
        }
        else
            forwardMulticastPacket(packet);
    }
    else {
        const NetworkInterface *broadcastIE = nullptr;

        // check for local delivery; we must accept also packets coming from the interfaces that
        // do not yet have an IP address assigned. This happens during DHCP requests.
        if (rt->isLocalAddress(destAddr) || fromIE->getProtocolData<Ipv4InterfaceData>()->getIPAddress().isUnspecified()) {
            deliverLocally(packet);
        }
        else if (destAddr.isLimitedBroadcastAddress() || (broadcastIE = rt->findInterfaceByLocalBroadcastAddress(destAddr))) {
            // broadcast datagram on the target subnet if we are a router
            if (broadcastIE && fromIE != broadcastIE && rt->isForwardingEnabled()) {
                if (directBroadcastInterfaceMatcher.matches(broadcastIE->getInterfaceName()) ||
                    directBroadcastInterfaceMatcher.matches(broadcastIE->getInterfaceFullPath().c_str()))
                {
                    auto packetCopy = packet->dup();
                    packetCopy->addTagIfAbsent<InterfaceReq>()->setInterfaceId(broadcastIE->getInterfaceId());
                    packetCopy->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(Ipv4Address::ALLONES_ADDRESS);
                    fragmentPostRouting(packetCopy);
                }
                else
                    EV_INFO << "Forwarding of direct broadcast packets is disabled on interface " << broadcastIE->getInterfaceName() << std::endl;
            }

            EV_INFO << "Broadcast received\n";
            deliverLocally(packet);
        }
        else if (!rt->isForwardingEnabled()) {
            EV_WARN << "forwarding off, dropping packet\n";
            dropPacket(packet, FORWARDING_DISABLED);
        }
        else {
            packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(nextHopAddr);
            routeUnicastPacket(packet);
        }
    }
    updateDisplayString();
}

void Ipv4DeliveryDecision::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

bool Ipv4DeliveryDecision::canPushSomePacket(cGate *gate) const
{
    return true;
}

bool Ipv4DeliveryDecision::canPushPacket(Packet *packet, cGate *gate) const
{
    return true;
}

void Ipv4DeliveryDecision::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    throw cRuntimeError("Invalid operation");
}

void Ipv4DeliveryDecision::pushPacketEnd(Packet *packet, cGate *gate)
{
    throw cRuntimeError("Invalid operation");
}

void Ipv4DeliveryDecision::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    throw cRuntimeError("Invalid operation");
}

void Ipv4DeliveryDecision::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void Ipv4DeliveryDecision::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

} // namespace inet
