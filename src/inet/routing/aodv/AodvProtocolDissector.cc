//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/aodv/AodvProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::aodv, AodvProtocolDissector);

void AodvProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    // the AodvControlPacket serializer dispatches on the packet type to the concrete
    // Rreq/Rrep/Rerr/RrepAck subtype
    const auto& chunk = packet->popAtFront<aodv::AodvControlPacket>();
    callback.startProtocolDataUnit(&Protocol::aodv);
    callback.visitChunk(chunk, &Protocol::aodv);
    callback.endProtocolDataUnit(&Protocol::aodv);
}

} // namespace inet
