//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {
namespace sctp {

Register_Protocol_Dissector(&Protocol::sctp, SctpProtocolDissector);

void SctpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    // TODO
    callback.startProtocolDataUnit(&Protocol::sctp);
    auto header = packet->popAtFront<SctpHeader>();
    callback.visitChunk(header, &Protocol::sctp);
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == b(0));
    callback.endProtocolDataUnit(&Protocol::udp);
}

} // namespace sctp
} // namespace inet

