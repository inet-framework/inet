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
#include "inet/common/serializer/TcpIpChecksum.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/common/TransportPseudoHeader_m.h"
#include "inet/transportlayer/tcp_common/TcpCrcInsertionHook.h"


namespace inet {

namespace tcp {

INetfilter::IHook::Result TcpCrcInsertion::datagramPostRoutingHook(Packet *packet)
{
    auto networkProtocol = packet->_getTag<PacketProtocolTag>()->getProtocol();
    const auto& networkHeader = getNetworkProtocolHeader(packet);
    if (networkHeader->getProtocol() == &Protocol::tcp) {
        packet->removeFromBeginning(networkHeader->getChunkLength());
        auto tcpHeader = packet->removeHeader<TcpHeader>();
        const L3Address& srcAddress = networkHeader->getSourceAddress();
        const L3Address& destAddress = networkHeader->getDestinationAddress();
        insertCrc(networkProtocol, srcAddress, destAddress, tcpHeader, packet);
        packet->insertHeader(tcpHeader);
        packet->insertHeader(networkHeader);
    }
    return ACCEPT;
}

void TcpCrcInsertion::insertCrc(const Protocol *networkProtocol, const L3Address& srcAddress, const L3Address& destAddress, const Ptr<TcpHeader>& tcpHeader, Packet *packet)
{
    tcpHeader->setCrcMode(crcMode);
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            tcpHeader->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            tcpHeader->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            // if the CRC mode is computed, then compute the CRC and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            tcpHeader->setCrc(0x0000); // make sure that the CRC is 0 in the TCP header before computing the CRC
            MemoryOutputStream tcpHeaderStream;
            Chunk::serialize(tcpHeaderStream, tcpHeader);
            auto tcpHeaderBytes = tcpHeaderStream.getData();
            const std::vector<uint8_t> emptyData;
            auto tcpDataBytes = (packet->getDataLength() > b(0)) ? packet->peekDataBytes()->getBytes() : emptyData;
            auto crc = computeCrc(networkProtocol, srcAddress, destAddress, tcpHeaderBytes, tcpDataBytes);
            tcpHeader->setCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

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


} // namespace tcp

} // namespace inet


