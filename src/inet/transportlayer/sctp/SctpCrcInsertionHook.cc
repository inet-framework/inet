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
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/sctp/SctpHeader.h"
#include "inet/transportlayer/sctp/SctpCrcInsertionHook.h"
//#include "inet/common/serializer/sctp/headers/sctphdr.h"
#include "inet/common/serializer/SctpChecksum.h"



namespace inet {

namespace sctp {

INetfilter::IHook::Result SctpCrcInsertion::datagramPostRoutingHook(Packet *packet)
{
std::cout << "SctpCrcInsertion::datagramPostRoutingHook\n";
    auto networkProtocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    if (networkHeader->getProtocol() == &Protocol::sctp) {
        packet->removeFromBeginning(networkHeader->getChunkLength());
        auto sctpHeader = packet->removeHeader<SctpHeader>();
        const L3Address& srcAddress = networkHeader->getSourceAddress();
        const L3Address& destAddress = networkHeader->getDestinationAddress();
        insertCrc(networkProtocol, srcAddress, destAddress, sctpHeader, packet);
        packet->insertHeader(sctpHeader);
        packet->insertHeader(networkHeader);
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
            sctpHeader->setCrc(0x0000); // make sure that the CRC is 0 in the TCP header before computing the CRC
            MemoryOutputStream sctpHeaderStream;
            Chunk::serialize(sctpHeaderStream, sctpHeader);
            auto sctpHeaderBytes = sctpHeaderStream.getData();
            const std::vector<uint8_t> emptyData;
            auto sctpDataBytes = (packet->getDataLength() > b(0)) ? packet->peekDataBytes()->getBytes() : emptyData;
            auto headerLength = sctpHeaderBytes.size();
            auto buffer = new uint8_t[headerLength];
            std::copy(sctpHeaderBytes.begin(), sctpHeaderBytes.end(), (uint8_t *)buffer);
            auto crc = inet::serializer::SctpChecksum::checksum(buffer, headerLength);
            sctpHeader->setCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}


#if 0
uint16_t TcpCrcInsertion::computeCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const std::vector<uint8_t>& tcpHeaderBytes, const std::vector<uint8_t>& tcpDataBytes)
{
    auto pseudoHeader = makeShared<TransportPseudoHeader>();
    pseudoHeader->setSrcAddress(srcAddress);
    pseudoHeader->setDestAddress(destAddress);
    pseudoHeader->setNetworkProtocolId(networkProtocol->getId());
    pseudoHeader->setProtocolId(IP_PROT_TCP);
    pseudoHeader->setPacketLength(tcpHeaderBytes.size() + tcpDataBytes.size());
    // pseudoHeader length: ipv4: 12 bytes, ipv6: 40 bytes, generic: ???
    if (networkProtocol == &Protocol::ipv4)
        pseudoHeader->setChunkLength(B(12));
    else if (networkProtocol == &Protocol::ipv6)
        pseudoHeader->setChunkLength(B(40));
    else
        throw cRuntimeError("Unknown network protocol: %s", networkProtocol->getName());
    auto pseudoHeaderBytes = pseudoHeader->Chunk::peek<BytesChunk>(B(0), pseudoHeader->getChunkLength())->getBytes();
    // Excerpt from RFC 768:
    // Checksum is the 16-bit one's complement of the one's complement sum of a
    // pseudo header of information from the IP header, the UDP header, and the
    // data,  padded  with zero octets  at the end (if  necessary)  to  make  a
    // multiple of two octets.
    auto pseudoHeaderLength = pseudoHeaderBytes.size();
    auto tcpHeaderLength = tcpHeaderBytes.size();
    auto tcpDataLength =  tcpDataBytes.size();
    auto bufferLength = pseudoHeaderLength + tcpHeaderLength + tcpDataLength;
    auto buffer = new uint8_t[bufferLength];
    // 1. fill in the data
    std::copy(pseudoHeaderBytes.begin(), pseudoHeaderBytes.end(), (uint8_t *)buffer);
    std::copy(tcpHeaderBytes.begin(), tcpHeaderBytes.end(), (uint8_t *)buffer + pseudoHeaderLength);
    std::copy(tcpDataBytes.begin(), tcpDataBytes.end(), (uint8_t *)buffer + pseudoHeaderLength + tcpHeaderLength);
    // 2. compute the CRC
    uint16_t crc = inet::serializer::TcpIpChecksum::checksum(buffer, bufferLength);
    delete [] buffer;
    return crc;
}
#endif

} // namespace tcp

} // namespace inet


