//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpChecksumInsertionHook.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/sctp/SctpChecksum.h"
#include "inet/transportlayer/sctp/SctpHeader.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace sctp {

Define_Module(SctpChecksumInsertion);

INetfilter::IHook::Result SctpChecksumInsertion::datagramPostRoutingHook(Packet *packet)
{
    Enter_Method("datagramPostRoutingHook");

    if (packet->findTag<InterfaceInd>())
        return ACCEPT; // FORWARD
    auto networkProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    if (networkHeader->getProtocol() == &Protocol::sctp) {
        ASSERT(!networkHeader->isFragment());
        packet->eraseAtFront(networkHeader->getChunkLength());
        auto sctpHeader = packet->removeAtFront<SctpHeader>();
        ASSERT(sctpHeader->getChecksumMode() == CHECKSUM_COMPUTED);
        const L3Address& srcAddress = networkHeader->getSourceAddress();
        const L3Address& destAddress = networkHeader->getDestinationAddress();
        insertChecksum(networkProtocol, srcAddress, destAddress, sctpHeader, packet);
        packet->insertAtFront(sctpHeader);
        packet->insertAtFront(networkHeader);
    }
    return ACCEPT;
}

void SctpChecksumInsertion::insertChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<SctpHeader>& sctpHeader, Packet *packet)
{
    sctpHeader->setChecksumMode(checksumMode);
    switch (checksumMode) {
        case CHECKSUM_DECLARED_CORRECT:
            // if the checksum mode is declared to be correct, then set the checksum to an easily recognizable value
            sctpHeader->setChecksum(0xC00D);
            break;
        case CHECKSUM_DECLARED_INCORRECT:
            // if the checksum mode is declared to be incorrect, then set the checksum to an easily recognizable value
            sctpHeader->setChecksum(0xBAAD);
            break;
        case CHECKSUM_COMPUTED: {
            // if the checksum mode is computed, then compute the checksum and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            sctpHeader->setChecksum(0); // make sure that the checksum is 0 in the SCTP header before computing the checksum
            MemoryOutputStream sctpPacketStream;
            Chunk::serialize(sctpPacketStream, sctpHeader);
            Chunk::serialize(sctpPacketStream, packet->peekData(Chunk::PF_ALLOW_EMPTY));
            const auto& sctpPacketBytes = sctpPacketStream.getData();
            auto length = sctpPacketBytes.size();
            auto buffer = new uint8_t[length];
            std::copy(sctpPacketBytes.begin(), sctpPacketBytes.end(), (uint8_t *)buffer);
            auto checksum = SctpChecksum::checksum(buffer, length);
            sctpHeader->setChecksum(checksum);
            delete[] buffer;
            break;
        }
        default:
            throw cRuntimeError("Unknown checksum mode");
    }
}

} // namespace sctp
} // namespace inet

