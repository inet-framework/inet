//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapProtocolDissector.h"

#include "inet/applications/coap/CoapHeader_m.h"
#include "inet/applications/coap/CoapProtocol.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {
namespace coap {

Register_Protocol_Dissector(&getCoapProtocol(), CoapProtocolDissector);

void CoapProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const Protocol& coap = getCoapProtocol();
    callback.startProtocolDataUnit(&coap);

    if (packet->getDataLength() < B(4)) {
        callback.markIncorrect();
        if (packet->getDataLength() > b(0))
            callback.visitChunk(packet->popAtFront(packet->getDataLength(), Chunk::PF_ALLOW_INCORRECT), &coap);
        callback.endProtocolDataUnit(&coap);
        return;
    }

    const auto& header = packet->popAtFront<CoapHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT);
    if (header->isIncorrect() || !header->isComplete())
        callback.markIncorrect();
    callback.visitChunk(header, &coap);

    if (packet->getDataLength() > b(0)) {
        const auto& payload = packet->popAtFront(packet->getDataLength(), Chunk::PF_ALLOW_INCORRECT);
        if (!header->getPayloadMarkerPresent())
            callback.markIncorrect();
        callback.visitChunk(payload, &coap);
    }
    else if (header->getPayloadMarkerPresent()) {
        callback.markIncorrect();
    }

    callback.endProtocolDataUnit(&coap);
}

} // namespace coap
} // namespace inet
