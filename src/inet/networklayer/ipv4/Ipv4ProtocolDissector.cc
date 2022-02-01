//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/Ipv4ProtocolDissector.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv4/Ipv4.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ipv4, Ipv4ProtocolDissector);

void Ipv4ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto trailerPopOffset = packet->getBackOffset();
    auto ipv4EndOffset = packet->getFrontOffset();
    const auto& header = packet->popAtFront<Ipv4Header>();
    ipv4EndOffset += B(header->getTotalLengthField());
    callback.startProtocolDataUnit(&Protocol::ipv4);
    bool incorrect = (ipv4EndOffset > trailerPopOffset || header->getTotalLengthField() <= header->getHeaderLength());
    if (incorrect) {
        callback.markIncorrect();
        ipv4EndOffset = trailerPopOffset;
    }
    callback.visitChunk(header, &Protocol::ipv4);
    packet->setBackOffset(ipv4EndOffset);
    if (header->getFragmentOffset() == 0 && !header->getMoreFragments()) {
        callback.dissectPacket(packet, header->getProtocol());
    }
    else {
        // TODO Fragmentation
        callback.dissectPacket(packet, nullptr);
    }
    if (incorrect && packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(incorrect || packet->getDataLength() == b(0));
    packet->setFrontOffset(ipv4EndOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::ipv4);
}

} // namespace inet

