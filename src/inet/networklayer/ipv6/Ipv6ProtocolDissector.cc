//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv6/Ipv6ProtocolDissector.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv6/Ipv6.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ipv6, Ipv6ProtocolDissector);

void Ipv6ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto trailerPopOffset = packet->getBackOffset();
    const auto& header = packet->popAtFront<Ipv6Header>();
    auto ipv6EndOffset = packet->getFrontOffset() + B(header->getPayloadLength());
    callback.startProtocolDataUnit(&Protocol::ipv6);
    bool incorrect = (ipv6EndOffset > trailerPopOffset);
    if (incorrect) {
        callback.markIncorrect();
        ipv6EndOffset = trailerPopOffset;
    }
    callback.visitChunk(header, &Protocol::ipv6);
    packet->setBackOffset(ipv6EndOffset);

    // Walk extension header chunks
    IpProtocolId nextHdr = header->getProtocolId();
    bool isFragment = false;
    while (isIpv6ExtensionHeader(nextHdr)) {
        auto extHdr = peekIpv6ExtensionHeaderAt(packet, b(0), nextHdr);
        packet->popAtFront(extHdr->getChunkLength());
        callback.visitChunk(extHdr, &Protocol::ipv6);
        if (nextHdr == IP_PROT_IPv6EXT_FRAGMENT)
            isFragment = true;
        nextHdr = extHdr->getNextHeaderProtocol();
    }

    if (isFragment)
        callback.dissectPacket(packet, nullptr); // Fragment
    else
        callback.dissectPacket(packet, header->getProtocol());
    if (incorrect && packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(ipv6EndOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::ipv6);
}

} // namespace inet

