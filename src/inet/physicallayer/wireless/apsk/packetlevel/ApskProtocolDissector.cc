//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::apskPhy, ApskProtocolDissector);

void ApskProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<ApskPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::apskPhy);
    callback.visitChunk(header, &Protocol::apskPhy);
    auto headerPaddingLength = header->getHeaderLengthField() - header->getChunkLength();
    if (headerPaddingLength > b(0)) {
        const auto& headerPadding = packet->popAtFront(headerPaddingLength);
        callback.visitChunk(headerPadding, &Protocol::apskPhy);
    }
    auto trailerPaddingLength = packet->getDataLength() - header->getPayloadLengthField();
    auto trailerPadding = trailerPaddingLength > b(0) ? packet->popAtBack(trailerPaddingLength) : nullptr;
    auto payloadProtocol = header->getPayloadProtocol();
    callback.dissectPacket(packet, payloadProtocol);
    if (trailerPaddingLength > b(0))
        callback.visitChunk(trailerPadding, &Protocol::apskPhy);
    callback.endProtocolDataUnit(&Protocol::apskPhy);
}

} // namespace inet

