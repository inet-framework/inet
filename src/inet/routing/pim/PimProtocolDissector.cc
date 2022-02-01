//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/pim/PimProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/routing/pim/PimPacket_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::pim, PimProtocolDissector);

void PimProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<PimPacket>();
    callback.startProtocolDataUnit(&Protocol::pim);
    callback.visitChunk(header, &Protocol::pim);
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr); // TODO interpret payloads correctly
    ASSERT(packet->getDataLength() == b(0));
    callback.endProtocolDataUnit(&Protocol::pim);
}

} // namespace inet

