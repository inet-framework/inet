//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/ordering/SequenceNumbering.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(SequenceNumbering);

void SequenceNumbering::processPacket(Packet *packet)
{
    auto header = makeShared<SequenceNumberHeader>();
    header->setSequenceNumber(sequenceNumber++);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::sequenceNumber);
}

} // namespace inet

