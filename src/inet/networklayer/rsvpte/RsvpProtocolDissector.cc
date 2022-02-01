//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/rsvpte/RsvpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/rsvpte/RsvpPacket_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::rsvpTe, RsvpProtocolDissector);

void RsvpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<RsvpMessage>();
    callback.startProtocolDataUnit(&Protocol::rsvpTe);
    callback.visitChunk(header, &Protocol::rsvpTe);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::rsvpTe);
}

} // namespace inet

