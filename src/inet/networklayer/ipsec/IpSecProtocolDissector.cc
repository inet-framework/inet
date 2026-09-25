//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipsec/IpSecProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipsec/IPsecAuthenticationHeader_m.h"
#include "inet/networklayer/ipsec/IPsecEncapsulatingSecurityPayload_m.h"

namespace inet {
namespace ipsec {

Register_Protocol_Dissector(&Protocol::ipsecAh, IpSecAhProtocolDissector);
Register_Protocol_Dissector(&Protocol::ipsecEsp, IpSecEspProtocolDissector);

void IpSecAhProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<IPsecAuthenticationHeader>();
    callback.startProtocolDataUnit(&Protocol::ipsecAh);
    callback.visitChunk(header, &Protocol::ipsecAh);
    // the ICV field ends the header (RFC 4302 section 2), and the Payload Length is the length of
    // AH in 32-bit words, minus 2; the payload follows the header
    B icvFieldLength = B((header->getPayloadLength() + 2) * 4) - header->getChunkLength();
    if (icvFieldLength > B(0))
        callback.visitChunk(packet->popAtFront(icvFieldLength), &Protocol::ipsecAh);
    auto dataProtocol = ProtocolGroup::getIpProtocolGroup()->findProtocol(header->getNextHeader());
    callback.dissectPacket(packet, dataProtocol);
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
