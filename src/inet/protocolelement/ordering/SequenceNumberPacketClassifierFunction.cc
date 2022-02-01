//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/ordering/SequenceNumberPacketClassifierFunction.h"

#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Register_Class(SequenceNumberPacketClassifierFunction);

int SequenceNumberPacketClassifierFunction::classifyPacket(Packet *packet) const
{
    auto sequenceNumberHeader = packet->peekAtFront<SequenceNumberHeader>();
    return sequenceNumberHeader->getSequenceNumber();
}

} // namespace inet

