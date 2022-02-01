//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/probabilistic/ProbabilisticProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/probabilistic/ProbabilisticBroadcastHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::probabilistic, ProbabilisticProtocolDissector);

void ProbabilisticProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<ProbabilisticBroadcastHeader>();
    auto trailerPopOffset = packet->getBackOffset();
    auto payloadEndOffset = packet->getFrontOffset() + header->getPayloadLengthField();
    callback.startProtocolDataUnit(&Protocol::probabilistic);
    bool incorrect = (payloadEndOffset > trailerPopOffset);
    if (incorrect) {
        callback.markIncorrect();
        payloadEndOffset = trailerPopOffset;
    }
    callback.visitChunk(header, &Protocol::probabilistic);
    packet->setBackOffset(payloadEndOffset);
    callback.dissectPacket(packet, header->getProtocol());
    if (incorrect && packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(payloadEndOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::probabilistic);
}

} // namespace inet

