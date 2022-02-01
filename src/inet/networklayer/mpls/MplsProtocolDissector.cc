//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/mpls/MplsProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/mpls/MplsPacket_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::mpls, MplsProtocolDissector);

void MplsProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<MplsHeader>();
    callback.startProtocolDataUnit(&Protocol::mpls);
    callback.visitChunk(header, &Protocol::mpls);
    const Protocol *encapsulatedProtocol = header->getS() ? &Protocol::ipv4 : &Protocol::mpls;
    callback.dissectPacket(packet, encapsulatedProtocol);
    callback.endProtocolDataUnit(&Protocol::mpls);
}

} // namespace inet

