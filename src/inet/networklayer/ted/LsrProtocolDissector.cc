//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ted/LsrProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ted/LinkStatePacket_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::linkStateRouting, LsrProtocolDissector);

void LsrProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<LinkStateMsg>();
    callback.startProtocolDataUnit(&Protocol::linkStateRouting);
    callback.visitChunk(header, &Protocol::linkStateRouting);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::linkStateRouting);
}

} // namespace inet

