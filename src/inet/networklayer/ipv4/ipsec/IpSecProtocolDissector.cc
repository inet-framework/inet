//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/ipsec/IpSecProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv4/ipsec/IPsecAuthenticationHeader_m.h"
#include "inet/networklayer/ipv4/ipsec/IPsecEncapsulatingSecurityPayload_m.h"
#include "inet/transportlayer/udp/Udp.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {
namespace ipsec {

Register_Protocol_Dissector(&Protocol::ipsecAh, IpSecAhProtocolDissector);
Register_Protocol_Dissector(&Protocol::ipsecEsp, IpSecEspProtocolDissector);

void IpSecAhProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto originalTrailerPopOffset = packet->getBackOffset();
    auto ipSecAhHeaderOffset = packet->getFrontOffset();
    const auto& header = packet->popAtFront<IPsecAuthenticationHeader>();
    callback.startProtocolDataUnit(&Protocol::ipsecAh);
    callback.visitChunk(header, &Protocol::ipsecAh);
    auto ipSecPayloadEndOffset = ipSecAhHeaderOffset + header->getChunkLength() + B(header->getPayloadLength());
    auto dataProtocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(header->getNextHeader());
    packet->setBackOffset(ipSecPayloadEndOffset);
    auto encrypted = packet->popAtFront<EncryptedChunk>();
    Ptr<const Chunk> icv;
    if (header->getIcvBytes() > 0)
        icv = packet->popAtBack(B(header->getIcvBytes()));
    ASSERT(packet->getDataLength() == B(0));
    callback.visitChunk(encrypted, &Protocol::ipsecAh);
    auto subPacket = new Packet(packet->getName(), encrypted->getChunk());
    callback.dissectPacket(subPacket, dataProtocol);
    delete subPacket;
    if (header->getIcvBytes() > 0)
        callback.visitChunk(icv, &Protocol::ipsecAh);
    packet->setBackOffset(originalTrailerPopOffset);
    packet->setFrontOffset(ipSecPayloadEndOffset);
    callback.endProtocolDataUnit(&Protocol::ipsecAh);
}

void IpSecEspProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto originalTrailerPopOffset = packet->getBackOffset();
    const auto& header = packet->popAtFront<IPsecEspHeader>();
    callback.startProtocolDataUnit(&Protocol::ipsecEsp);
    callback.visitChunk(header, &Protocol::ipsecEsp);
    auto encrypted = packet->popAtFront<EncryptedChunk>();
    Ptr<const Chunk> icv;
    if (header->getIcvBytes() > 0)
        icv = packet->popAtBack(B(header->getIcvBytes()));
    ASSERT(packet->getDataLength() == B(0));
    auto subPacket = new Packet(packet->getName(), encrypted->getChunk());
    auto trailer = subPacket->popAtBack<IPsecEspTrailer>(B(ESP_FIXED_PAYLOAD_TRAILER_BYTES));
    auto dataProtocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(trailer->getNextHeader());
    if (trailer->getPadLength() > 0)
        subPacket->popAtBack(B(trailer->getPadLength()));
    callback.visitChunk(encrypted, &Protocol::ipsecEsp);
    callback.dissectPacket(subPacket, dataProtocol);
    callback.visitChunk(trailer, &Protocol::ipsecEsp);
    delete subPacket;
    if (header->getIcvBytes() > 0)
        callback.visitChunk(icv, &Protocol::ipsecEsp);
    packet->setBackOffset(originalTrailerPopOffset);
    packet->setFrontOffset(originalTrailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::ipsecEsp);
}

} // namespace ipsec
} // namespace inet
