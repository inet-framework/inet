//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/acknowledgement/SendWithAcknowledge.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/acknowledgement/AcknowledgeHeader_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(SendWithAcknowledge);

void SendWithAcknowledge::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        acknowledgeTimeout = par("acknowledgeTimeout");
        sequenceNumber = 0;
        registerService(AccessoryProtocol::withAcknowledge, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::withAcknowledge, outputGate, nullptr);
        registerService(AccessoryProtocol::acknowledge, gate("ackIn"), gate("ackIn"));
    }
}

void SendWithAcknowledge::handleMessage(cMessage *message)
{
    auto sequenceNumber = message->getKind();
    timers.erase(timers.find(sequenceNumber));
    auto packet = static_cast<Packet *>(message->getContextPointer());
    producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), false);
    delete message;
}

void SendWithAcknowledge::processPacket(Packet *packet)
{
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == &AccessoryProtocol::acknowledge) {
        auto header = packet->popAtFront<AcknowledgeHeader>();
        auto sequenceNumber = header->getSequenceNumber();
        auto it = timers.find(sequenceNumber);
        auto timer = it->second;
        timers.erase(it);
        delete packet;
        packet = static_cast<Packet *>(timer->getContextPointer());
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
        cancelAndDelete(timer);
    }
    else {
        auto header = makeShared<SequenceNumberHeader>();
        header->setSequenceNumber(sequenceNumber);
        packet->insertAtFront(header);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::withAcknowledge);
        send(packet, "out");
        auto timer = new cMessage("AcknowledgeTimer");
        timer->setKind(sequenceNumber);
        timer->setContextPointer(packet);
        timers[sequenceNumber] = timer;
        scheduleAt(simTime() + acknowledgeTimeout, timer);
        sequenceNumber++;
    }
}

} // namespace inet

