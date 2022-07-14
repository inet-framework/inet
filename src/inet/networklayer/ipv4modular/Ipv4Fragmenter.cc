//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4modular/Ipv4Fragmenter.h"

#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(Ipv4Fragmenter);

void Ipv4Fragmenter::initialize(int stage)
{
    queueing::PacketPusherBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        arp.reference(this, "arpModule", true);
        icmp.reference(this, "icmpModule", true);
        ift.reference(this, "interfaceTableModule", true);

        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);

        defaultTimeToLive = par("timeToLive");
        defaultMCTimeToLive = par("multicastTimeToLive");

        curFragmentId = 0;
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        cModule *arpModule = check_and_cast<cModule *>(arp.get());
        arpModule->subscribe(IArp::arpResolutionCompletedSignal, this);
        arpModule->subscribe(IArp::arpResolutionFailedSignal, this);
    }
}

// void Ipv4::fragmentAndSend(Packet *packet)
void Ipv4Fragmenter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);

    const NetworkInterface *destIE = ift->getInterfaceById(packet->getTag<InterfaceReq>()->getInterfaceId());
    const auto& tag = packet->findTag<NextHopAddressReq>();
    Ipv4Address nextHopAddr = tag != nullptr ? tag->getNextHopAddress().toIpv4() : Ipv4Address::UNSPECIFIED_ADDRESS;
    if (nextHopAddr.isUnspecified()) {
        nextHopAddr = packet->peekAtFront<Ipv4Header>()->getDestAddress();
        packet->addTagIfAbsent<NextHopAddressReq>()->setNextHopAddress(nextHopAddr);
    }

    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();

    // hop counter check
    if (ipv4Header->getTimeToLive() <= 0) {
        // drop datagram, destruction responsibility in ICMP
        PacketDropDetails details;
        details.setReason(HOP_LIMIT_REACHED);
        emit(packetDroppedSignal, packet, &details);
        EV_WARN << "datagram TTL reached zero, sending ICMP_TIME_EXCEEDED\n";
        icmp->sendErrorMessage(packet, -1 /*TODO*/, ICMP_TIME_EXCEEDED, 0);
        delete packet;
        return;
    }

    int mtu = destIE->getMtu();

    // send datagram straight out if it doesn't require fragmentation (note: mtu==0 means infinite mtu)
    if (mtu == 0 || packet->getByteLength() <= mtu) {
        if (crcMode == CRC_COMPUTED) {
            auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
            Ipv4Header::setComputedCrc(ipv4Header);
            insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
        }
        sendDatagramToOutput(packet);
        return;
    }

    // if "don't fragment" bit is set, throw datagram away and send ICMP error message
    if (ipv4Header->getDontFragment()) {
        PacketDropDetails details;
        emit(packetDroppedSignal, packet, &details);
        EV_WARN << "datagram larger than MTU and don't fragment bit set, sending ICMP_DESTINATION_UNREACHABLE\n";
        icmp->sendPtbMessage(packet, mtu);
        delete packet;
        return;
    }

    // FIXME some IP options should not be copied into each fragment, check their COPY bit
    int headerLength = B(ipv4Header->getHeaderLength()).get();
    int payloadLength = B(packet->getDataLength()).get() - headerLength;
    int fragmentLength = ((mtu - headerLength) / 8) * 8; // payload only (without header)
    int offsetBase = ipv4Header->getFragmentOffset();
    if (fragmentLength <= 0)
        throw cRuntimeError("Cannot fragment datagram: MTU=%d too small for header size (%d bytes)", mtu, headerLength); // exception and not ICMP because this is likely a simulation configuration error, not something one wants to simulate

    int noOfFragments = (payloadLength + fragmentLength - 1) / fragmentLength;
    EV_DETAIL << "Breaking datagram into " << noOfFragments << " fragments\n";

    // create and send fragments
    std::string fragMsgName = packet->getName();
    fragMsgName += "-frag-";

    int offset = 0;
    while (offset < payloadLength) {
        bool lastFragment = (offset + fragmentLength >= payloadLength);
        // length equal to fragmentLength, except for last fragment;
        int thisFragmentLength = lastFragment ? payloadLength - offset : fragmentLength;

        std::string curFragName = fragMsgName + std::to_string(offset);
        if (lastFragment)
            curFragName += "-last";
        Packet *fragment = new Packet(curFragName.c_str()); // TODO add offset or index to fragment name

        // copy Tags from packet to fragment
        fragment->copyTags(*packet);

        ASSERT(fragment->getByteLength() == 0);
        auto fraghdr = staticPtrCast<Ipv4Header>(ipv4Header->dupShared());
        const auto& fragData = packet->peekDataAt(B(headerLength + offset), B(thisFragmentLength));
        ASSERT(fragData->getChunkLength() == B(thisFragmentLength));
        fragment->insertAtBack(fragData);

        // "more fragments" bit is unchanged in the last fragment, otherwise true
        if (!lastFragment)
            fraghdr->setMoreFragments(true);

        fraghdr->setFragmentOffset(offsetBase + offset);
        fraghdr->setTotalLengthField(B(headerLength + thisFragmentLength));
        if (crcMode == CRC_COMPUTED)
            Ipv4Header::setComputedCrc(fraghdr);

        fragment->insertAtFront(fraghdr);
        ASSERT(fragment->getByteLength() == headerLength + thisFragmentLength);
        sendDatagramToOutput(fragment);
        offset += thisFragmentLength;
    }

    delete packet;
}

// void Ipv4::sendDatagramToOutput(Packet *packet)
void Ipv4Fragmenter::sendDatagramToOutput(Packet *packet)
{
    const NetworkInterface *ie = ift->getInterfaceById(packet->getTag<InterfaceReq>()->getInterfaceId());
    auto nextHopAddressReq = packet->removeTag<NextHopAddressReq>();
    Ipv4Address nextHopAddr = nextHopAddressReq->getNextHopAddress().toIpv4();
    if (!ie->isBroadcast() || ie->getMacAddress().isUnspecified()) // we can't do ARP
        sendPacketToNIC(packet);
    else {
        MacAddress nextHopMacAddr = resolveNextHopMacAddress(packet, nextHopAddr, ie);
        if (nextHopMacAddr.isUnspecified()) {
            EV_INFO << "Pending " << packet << " to ARP resolution.\n";
            pendingPackets[nextHopAddr].insert(packet);
        }
        else {
            ASSERT2(!containsKey(pendingPackets, nextHopAddr), "Ipv4-ARP error: nextHopAddr found in ARP table, but Ipv4 queue for nextHopAddr not empty");
            packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(nextHopMacAddr);
            sendPacketToNIC(packet);
        }
    }
}

// void Ipv4::arpResolutionCompleted(IArp::Notification *entry)
void Ipv4Fragmenter::arpResolutionCompleted(IArp::Notification *entry)
{
    if (entry->l3Address.getType() != L3Address::IPv4)
        return;
    auto it = pendingPackets.find(entry->l3Address.toIpv4());
    if (it != pendingPackets.end()) {
        cPacketQueue& packetQueue = it->second;
        EV << "ARP resolution completed for " << entry->l3Address << ". Sending " << packetQueue.getLength()
           << " waiting packets from the queue\n";

        while (!packetQueue.isEmpty()) {
            Packet *packet = check_and_cast<Packet *>(packetQueue.pop());
            EV << "Sending out queued packet " << packet << "\n";
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(entry->ie->getInterfaceId());
            packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(entry->macAddress);
            sendPacketToNIC(packet);
        }
        pendingPackets.erase(it);
    }
}

// void Ipv4::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
void Ipv4Fragmenter::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == IArp::arpResolutionCompletedSignal) {
        arpResolutionCompleted(check_and_cast<IArp::Notification *>(obj));
    }
    if (signalID == IArp::arpResolutionFailedSignal) {
        arpResolutionTimedOut(check_and_cast<IArp::Notification *>(obj));
    }
}

// MacAddress Ipv4::resolveNextHopMacAddress(cPacket *packet, Ipv4Address nextHopAddr, const NetworkInterface *destIE)
MacAddress Ipv4Fragmenter::resolveNextHopMacAddress(cPacket *packet, Ipv4Address nextHopAddr, const NetworkInterface *destIE)
{
    if (nextHopAddr.isLimitedBroadcastAddress() || nextHopAddr == destIE->getProtocolData<Ipv4InterfaceData>()->getNetworkBroadcastAddress()) {
        EV_DETAIL << "destination address is broadcast, sending packet to broadcast MAC address\n";
        return MacAddress::BROADCAST_ADDRESS;
    }

    if (nextHopAddr.isMulticast()) {
        MacAddress macAddr = nextHopAddr.mapToMulticastMacAddress();
        EV_DETAIL << "destination address is multicast, sending packet to MAC address " << macAddr << "\n";
        return macAddr;
    }

    return arp->resolveL3Address(nextHopAddr, destIE);
}

// void Ipv4::arpResolutionTimedOut(IArp::Notification *entry)
void Ipv4Fragmenter::arpResolutionTimedOut(IArp::Notification *entry)
{
    if (entry->l3Address.getType() != L3Address::IPv4)
        return;
    auto it = pendingPackets.find(entry->l3Address.toIpv4());
    if (it != pendingPackets.end()) {
        cPacketQueue& packetQueue = it->second;
        EV << "ARP resolution failed for " << entry->l3Address << ",  dropping " << packetQueue.getLength() << " packets\n";
        for (int i = 0; i < packetQueue.getLength(); i++) {
            auto packet = packetQueue.get(i);
            PacketDropDetails details;
            details.setReason(ADDRESS_RESOLUTION_FAILED);
            emit(packetDroppedSignal, packet, &details);
        }
        packetQueue.clear();
        pendingPackets.erase(it);
    }
}

//void Ipv4::sendPacketToNIC(Packet *packet)
void Ipv4Fragmenter::sendPacketToNIC(Packet *packet)
{
    auto networkInterface = ift->getInterfaceById(packet->getTag<InterfaceReq>()->getInterfaceId());
    EV_INFO << "Sending " << packet << " to output interface = " << networkInterface->getInterfaceName() << ".\n";
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
    packet->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::ipv4);
    auto networkInterfaceProtocol = networkInterface->getProtocol();
    auto dispatchProtocol = networkInterfaceProtocol;
    if (auto encapsulationProtocolReq = packet->findTagForUpdate<EncapsulationProtocolReq>()) {
        dispatchProtocol = encapsulationProtocolReq->getProtocol(0);
        encapsulationProtocolReq->eraseProtocol(0);
        if (networkInterfaceProtocol != nullptr)
            encapsulationProtocolReq->appendProtocol(networkInterfaceProtocol);
        else if (encapsulationProtocolReq->getProtocolArraySize() == 0)
            packet->removeTag<EncapsulationProtocolReq>();
    }
    if (dispatchProtocol == nullptr)
        packet->removeTagIfPresent<DispatchProtocolReq>();
    else
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(dispatchProtocol);
    ASSERT(packet->findTag<InterfaceReq>() != nullptr);
    send(packet, "out");
}

} // namespace inet

