//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/nexthop/NextHopProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/nexthop/NextHopForwardingHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::nextHopForwarding, NextHopProtocolDissector);

void NextHopProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<NextHopForwardingHeader>();
    auto trailerPopOffset = packet->getBackOffset();
    auto endOffset = packet->getFrontOffset() + header->getPayloadLengthField();
    callback.startProtocolDataUnit(&Protocol::nextHopForwarding);
    callback.visitChunk(header, &Protocol::nextHopForwarding);
    packet->setBackOffset(endOffset);
    callback.dissectPacket(packet, header->getProtocol());
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(endOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::nextHopForwarding);
}

} // namespace inet

