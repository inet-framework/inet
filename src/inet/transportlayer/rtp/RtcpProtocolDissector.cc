//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/rtp/RtcpProtocolDissector.h"

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/rtp/RtcpPacket_m.h"

namespace inet {
namespace rtp {

Register_Protocol_Dissector(&Protocol::rtcp, RtcpProtocolDissector);

void RtcpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::rtcp);
    // An RTCP datagram is a compound packet: several RTCP packets back to back, each
    // announcing its own length in the third and fourth octet of its header (in 32-bit
    // words, minus one). Bound every packet by that length, so a type whose deserializer
    // consumes less than the sender wrote -- an unmodelled report block, an SDES item
    // INET does not know -- does not shift the ones behind it.
    auto endOffset = packet->getBackOffset();
    while (packet->getDataLength() > b(0)) {
        const auto& lengthField = packet->peekDataAt<BytesChunk>(B(2), B(2));
        auto rtcpPacketLength = B(4) * ((lengthField->getByte(0) << 8 | lengthField->getByte(1)) + 1);
        packet->setBackOffset(packet->getFrontOffset() + rtcpPacketLength);
        callback.visitChunk(packet->popAtFront<RtcpPacket>(), &Protocol::rtcp);
        if (packet->getDataLength() > b(0))
            callback.dissectPacket(packet, nullptr);
        packet->setBackOffset(endOffset);
    }
    callback.endProtocolDataUnit(&Protocol::rtcp);
}

} // namespace rtp
} // namespace inet
