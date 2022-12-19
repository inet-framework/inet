//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ppp/PppProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ppp/PppFrame_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ppp, PppProtocolDissector);

void PppProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::ppp);
    const auto& header = packet->popAtFront<PppHeader>();
    const auto& trailer = packet->popAtBack<PppTrailer>(PPP_TRAILER_LENGTH);
    callback.visitChunk(header, &Protocol::ppp);
    auto payloadProtocol = ProtocolGroup::getPppProtocolGroup()->findProtocol(header->getProtocol());
    callback.dissectPacket(packet, payloadProtocol);
    callback.visitChunk(trailer, &Protocol::ppp);
    callback.endProtocolDataUnit(&Protocol::ppp);
}

} // namespace inet

