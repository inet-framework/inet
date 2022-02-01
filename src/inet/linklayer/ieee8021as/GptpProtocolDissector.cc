//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021as/GptpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"

namespace inet {

namespace physicallayer {

Register_Protocol_Dissector(&Protocol::gptp, GptpProtocolDissector);

void GptpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<GptpBase>();
    callback.startProtocolDataUnit(protocol);
    callback.visitChunk(header, protocol);
    callback.endProtocolDataUnit(protocol);
}

} // namespace physicallayer

} // namespace inet

