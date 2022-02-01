//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/acknowledgement/ReceiveWithAcknowledge.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/acknowledgement/AcknowledgeHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(ReceiveWithAcknowledge);

void ReceiveWithAcknowledge::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(AccessoryProtocol::withAcknowledge, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::withAcknowledge, nullptr, outputGate);
    }
}

void ReceiveWithAcknowledge::pushPacket(Packet *dataPacket, cGate *gate)
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

