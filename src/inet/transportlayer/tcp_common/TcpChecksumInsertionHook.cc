//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_common/TcpChecksumInsertionHook.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/EmptyChunk.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"

namespace inet {
namespace tcp {

Define_Module(TcpChecksumInsertionHook);

INetfilter::IHook::Result TcpChecksumInsertionHook::datagramPostRoutingHook(Packet *packet)
{
    Enter_Method("datagramPostRoutingHook");

    if (packet->findTag<InterfaceInd>())
        return ACCEPT; // FORWARD
    auto networkProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    if (networkHeader->getProtocol() == &Protocol::tcp) {
        ASSERT(!networkHeader->isFragment());
        packet->eraseAtFront(networkHeader->getChunkLength());
        auto tcpHeader = packet->removeAtFront<TcpHeader>();
        ASSERT(tcpHeader->getChecksumMode() == CHECKSUM_COMPUTED);
        const L3Address& srcAddress = networkHeader->getSourceAddress();
        const L3Address& destAddress = networkHeader->getDestinationAddress();
        insertChecksum(networkProtocol, srcAddress, destAddress, tcpHeader, packet);
        packet->insertAtFront(tcpHeader);
        packet->insertAtFront(networkHeader);
    }
    return ACCEPT;
}

void TcpChecksumInsertionHook::insertChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<TcpHeader>& tcpHeader, Packet *packet)
{
    auto checksumMode = tcpHeader->getChecksumMode();
    switch (checksumMode) {
        case CHECKSUM_DECLARED_CORRECT:
            // if the checksum mode is declared to be correct, then set the checksum to an easily recognizable value
            tcpHeader->setChecksum(0xC00D);
            break;
        case CHECKSUM_DECLARED_INCORRECT:
            // if the checksum mode is declared to be incorrect, then set the checksum to an easily recognizable value
            tcpHeader->setChecksum(0xBAAD);
            break;
        case CHECKSUM_COMPUTED: {
            // if the checksum mode is computed, then compute the checksum and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            tcpHeader->setChecksum(0x0000); // make sure that the checksum is 0 in the TCP header before computing the checksum
            auto tcpData = packet->peekData(Chunk::PF_ALLOW_EMPTY);
            auto checksum = computeChecksum(networkProtocol, srcAddress, destAddress, tcpHeader, tcpData);
            tcpHeader->setChecksum(checksum);
            break;
        }
        default:
            throw cRuntimeError("Unknown checksum mode");
    }
}

uint16_t TcpChecksumInsertionHook::computeChecksum(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<const TcpHeader>& tcpHeader, const Ptr<const Chunk>& tcpData)
{
    auto pseudoHeader = makeShared<TransportPseudoHeader>();
    pseudoHeader->setSrcAddress(srcAddress);
    pseudoHeader->setDestAddress(destAddress);
    pseudoHeader->setNetworkProtocolId(networkProtocol->getId());
    pseudoHeader->setProtocolId(IP_PROT_TCP);
    pseudoHeader->setPacketLength(tcpHeader->getChunkLength() + tcpData->getChunkLength());
    // pseudoHeader length: ipv4: 12 bytes, ipv6: 40 bytes, generic: ???
    if (networkProtocol == &Protocol::ipv4)
        pseudoHeader->setChunkLength(B(12));
    else if (networkProtocol == &Protocol::ipv6)
        pseudoHeader->setChunkLength(B(40));
    else
        throw cRuntimeError("Unknown network protocol: %s", networkProtocol->getName());
    MemoryOutputStream stream;
    Chunk::serialize(stream, pseudoHeader);
    Chunk::serialize(stream, tcpHeader);
    Chunk::serialize(stream, tcpData);
    uint16_t checksum = internetChecksum(stream.getData());
    return checksum;
}

} // namespace tcp
} // namespace inet

