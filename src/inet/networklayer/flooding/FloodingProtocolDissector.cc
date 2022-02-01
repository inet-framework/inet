//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/flooding/FloodingProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/flooding/FloodingHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::flooding, FloodingProtocolDissector);

void FloodingProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<FloodingHeader>();
    auto trailerPopOffset = packet->getBackOffset();
    auto payloadEndOffset = packet->getFrontOffset() + header->getPayloadLengthField();
    callback.startProtocolDataUnit(&Protocol::flooding);
    callback.visitChunk(header, &Protocol::flooding);
    packet->setBackOffset(payloadEndOffset);
    callback.dissectPacket(packet, header->getProtocol());
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(payloadEndOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::flooding);
}

} // namespace inet

