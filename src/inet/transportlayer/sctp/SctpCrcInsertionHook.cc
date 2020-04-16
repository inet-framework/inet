//
// Copyright 2017 OpenSim Ltd.
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 3 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include "inet/common/INETDefs.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/sctp/SctpChecksum.h"
#include "inet/transportlayer/sctp/SctpCrcInsertionHook.h"
#include "inet/transportlayer/sctp/SctpHeader.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace sctp {

INetfilter::IHook::Result SctpCrcInsertion::datagramPostRoutingHook(Packet *packet)
{
    if (packet->findTag<InterfaceInd>())
        return ACCEPT;  // FORWARD
    auto networkProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    if (networkHeader->getProtocol() == &Protocol::sctp) {
        ASSERT(!networkHeader->isFragment());
        packet->eraseAtFront(networkHeader->getChunkLength());
        auto sctpHeader = packet->removeAtFront<SctpHeader>();
        ASSERT(sctpHeader->getCrcMode() == CRC_COMPUTED);
        const L3Address& srcAddress = networkHeader->getSourceAddress();
        const L3Address& destAddress = networkHeader->getDestinationAddress();
        insertCrc(networkProtocol, srcAddress, destAddress, sctpHeader, packet);
        packet->insertAtFront(sctpHeader);
        packet->insertAtFront(networkHeader);
    }
    return ACCEPT;
}

void SctpCrcInsertion::insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<SctpHeader>& sctpHeader, Packet *packet)
{
    sctpHeader->setCrcMode(crcMode);
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            sctpHeader->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            sctpHeader->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            // if the CRC mode is computed, then compute the CRC and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            sctpHeader->setCrc(0); // make sure that the CRC is 0 in the SCTP header before computing the CRC
            MemoryOutputStream sctpPacketStream;
            Chunk::serialize(sctpPacketStream, sctpHeader);
            Chunk::serialize(sctpPacketStream, packet->peekData(Chunk::PF_ALLOW_EMPTY));
            const auto& sctpPacketBytes = sctpPacketStream.getData();
            auto length = sctpPacketBytes.size();
            auto buffer = new uint8_t[length];
            std::copy(sctpPacketBytes.begin(), sctpPacketBytes.end(), (uint8_t *)buffer);
            auto crc = SctpChecksum::checksum(buffer, length);
            sctpHeader->setCrc(crc);
            delete [] buffer;
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

} // namespace sctp
} // namespace inet

