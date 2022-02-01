//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/dissector/ProtocolDissector.h"

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {

Register_Protocol_Dissector(nullptr, DefaultProtocolDissector);

void DefaultProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(protocol);
    callback.visitChunk(packet->peekData(), protocol);
    packet->setFrontOffset(packet->getBackOffset());
    callback.endProtocolDataUnit(protocol);
}

} // namespace

