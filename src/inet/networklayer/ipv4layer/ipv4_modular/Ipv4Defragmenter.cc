//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ipv4layer/ipv4_modular/Ipv4Defragmenter.h"

#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/ipv4layer/common/Ipv4Header_m.h"

namespace inet {

Define_Module(Ipv4Defragmenter);

void Ipv4Defragmenter::initialize(int stage)
{
    queueing::PacketPusherBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        icmp.reference(this, "icmpModule", true);
        fragmentTimeoutTime = par("fragmentTimeout");
        lastCheckTime = SIMTIME_ZERO;
    }
}

void Ipv4Defragmenter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);

    packet = reassembleAndDeliver(packet);

    if (packet) {
        pushOrSendPacket(packet, outputGate, consumer);
    }
}

Packet *Ipv4Defragmenter::reassembleAndDeliver(Packet *packet)
{
    EV_INFO << "Delivering " << packet << " locally.\n";

    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    if (ipv4Header->getSrcAddress().isUnspecified())
        EV_WARN << "Received datagram '" << packet->getName() << "' without source address filled in\n";

    // reassemble the packet (if fragmented)
    if (ipv4Header->getFragmentOffset() != 0 || ipv4Header->getMoreFragments()) {
        EV_DETAIL << "Datagram fragment: offset=" << ipv4Header->getFragmentOffset()
                  << ", MORE=" << (ipv4Header->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if (simTime() >= lastCheckTime + 10) {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(icmp, simTime() - fragmentTimeoutTime);
        }

        packet = fragbuf.addFragment(packet, simTime());
        if (!packet) {
            EV_DETAIL << "No complete datagram yet.\n";
            return nullptr;
        }
        if (packet->peekAtFront<Ipv4Header>()->getCrcMode() == CRC_COMPUTED) {
            auto ipv4Header = removeNetworkProtocolHeader<Ipv4Header>(packet);
            ipv4Header->updateCrc();
            insertNetworkProtocolHeader(packet, Protocol::ipv4, ipv4Header);
        }
        EV_DETAIL << "This fragment completes the datagram.\n";
    }
    return packet;
}

} // namespace inet
