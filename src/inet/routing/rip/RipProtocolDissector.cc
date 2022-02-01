//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/rip/RipProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/routing/rip/RipPacket_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::rip, RipProtocolDissector);

void RipProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& chunk = packet->popAtFront<RipPacket>();
    callback.startProtocolDataUnit(&Protocol::rip);
    callback.visitChunk(chunk, &Protocol::rip);
    callback.endProtocolDataUnit(&Protocol::rip);
}

} // namespace inet

