//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/acknowledgement/ReceiveWithAcknowledge.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/acknowledgement/AcknowledgeHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(ReceiveWithAcknowledge);

void ReceiveWithAcknowledge::pushPacket(Packet *dataPacket, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(dataPacket);
    auto dataHeader = dataPacket->popAtFront<SequenceNumberHeader>();
    send(dataPacket, "out");
    auto ackHeader = makeShared<AcknowledgeHeader>();
    ackHeader->setSequenceNumber(dataHeader->getSequenceNumber());
    auto ackPacket = new Packet("Ack", ackHeader);
    ackPacket->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::acknowledge);
    send(ackPacket, "ackOut");
}

} // namespace inet

