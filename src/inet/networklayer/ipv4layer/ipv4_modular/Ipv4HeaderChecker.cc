//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4HeaderChecker.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/ipv4layer/common/Ipv4Header_m.h"

namespace inet {

Define_Module(Ipv4HeaderChecker);

void Ipv4HeaderChecker::initialize(int stage)
{
    PacketPusherBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        icmp.reference(this, "icmpModule", true);
    }
}

void Ipv4HeaderChecker::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);

    ASSERT(packet);
    emit(packetReceivedFromLowerSignal, packet);

    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    packet->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&Protocol::ipv4);
    packet->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(ipv4Header);

    EV_DETAIL << "Received datagram `" << ipv4Header->getName() << "' with dest=" << ipv4Header->getDestAddress() << "\n";

    if (!ipv4Header->isCorrect() || !ipv4Header->verifyCrc()) {
        EV_WARN << "CRC error found, drop packet\n";
        dropPacket(packet, INCORRECTLY_RECEIVED);
        return;
    }

    B headerLength = ipv4Header->getHeaderLength();
    ASSERT(headerLength.get() % 4 == 0);
    ASSERT(headerLength == ipv4Header->getChunkLength());

    // check for header biterror
    if (packet->hasBitError()) {
        // probability of bit error in header = size of header / size of total message
        // (ignore bit error if in payload)
        double relativeHeaderLength = headerLength.get() / (double)B(ipv4Header->getChunkLength()).get();
        if (dblrand() <= relativeHeaderLength) {
            EV_WARN << "bit error found, drop packet\n";
            dropPacket(packet, INCORRECTLY_RECEIVED);
            return;
        }
    }

    if (ipv4Header->getVersion() != 4) {
        EV_WARN << "bad version number in IPv4 header\n";
        sendIcmpError(packet, ICMP_PARAMETER_PROBLEM, 0);
        return;
    }

    if (headerLength < IPv4_MIN_HEADER_LENGTH
            || headerLength > IPv4_MAX_HEADER_LENGTH
            || headerLength > ipv4Header->getTotalLengthField()
            || headerLength > packet->getDataLength()
            ) {
        EV_WARN << "invalid header length field value\n";
        sendIcmpError(packet, ICMP_PARAMETER_PROBLEM, 0);
        return;
    }

    if (ipv4Header->getTotalLengthField() > packet->getDataLength()) {
        EV_WARN << "total length field larger than packet length\n";
        sendIcmpError(packet, ICMP_PARAMETER_PROBLEM, 0);
        return;
    }

    // remove lower layer paddings:
    if (ipv4Header->getTotalLengthField() < packet->getDataLength()) {
        packet->setBackOffset(packet->getFrontOffset() + ipv4Header->getTotalLengthField());
    }

    // send --> datagramPreRoutingHook --> preroutingFinish
    pushOrSendPacket(packet, outputGate, consumer);
}

void Ipv4HeaderChecker::sendIcmpError(Packet *packet, IcmpType type, IcmpCode code)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    icmp->sendErrorMessage(packet, interfaceId, ICMP_PARAMETER_PROBLEM, 0);
    delete packet;
}

} // namespace inet
