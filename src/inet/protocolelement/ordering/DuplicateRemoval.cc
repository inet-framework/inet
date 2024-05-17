//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/ordering/DuplicateRemoval.h"

#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(DuplicateRemoval);

void DuplicateRemoval::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto header = packet->popAtFront<SequenceNumberHeader>();
    auto sequenceNumber = header->getSequenceNumber();
    if (lastSequenceNumber == sequenceNumber)
        delete packet;
    else {
        lastSequenceNumber = sequenceNumber;
        pushOrSendPacket(packet, outputGate, consumer);
    }
}

} // namespace inet

