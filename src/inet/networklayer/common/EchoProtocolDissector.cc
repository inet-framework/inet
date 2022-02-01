//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/common/EchoProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/common/EchoPacket_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::echo, EchoProtocolDissector);

void EchoProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<EchoPacket>();
    callback.startProtocolDataUnit(&Protocol::echo);
    callback.visitChunk(header, &Protocol::echo);
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::echo);
}

} // namespace inet

