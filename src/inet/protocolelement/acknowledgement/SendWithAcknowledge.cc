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
        ackInGate = gate("ackIn");
        WATCH(sequenceNumber);
        registerService(AccessoryProtocol::withAcknowledge, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::withAcknowledge, outputGate, nullptr);
        registerService(AccessoryProtocol::acknowledge, ackInGate, ackInGate);
    }
}

// A data packet arrives here (a direct push on the input gate). Number it, transmit it, and
// start the acknowledgement timeout; completion is reported later, from handleMessage(), when
// the acknowledgement arrives (success) or the timeout fires (failure). pushPacket() is
// overridden -- rather than processPacket() -- so PacketFlowBase does not also push/emit the
// packet after we have already sent it (which would double-send it).
void SendWithAcknowledge::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto header = makeShared<SequenceNumberHeader>();
    header->setSequenceNumber(sequenceNumber);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::withAcknowledge);
    auto timer = new cMessage("AcknowledgeTimer");
    timer->setKind(sequenceNumber);
    timer->setContextPointer(packet); // opaque id for the pending send; not dereferenced once sent
    timers[sequenceNumber] = timer;
    scheduleAt(simTime() + acknowledgeTimeout, timer);
    sequenceNumber++;
    send(packet, "out");
}

void SendWithAcknowledge::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        // acknowledgement timeout: report failure so an upstream resender retransmits
        int seq = message->getKind();
        timers.erase(seq);
        auto dataPacket = static_cast<Packet *>(message->getContextPointer());
        if (producer != nullptr)
            producer.handlePushPacketProcessed(dataPacket, false);
        delete message;
    }
    else if (message->getArrivalGate() == ackInGate) {
        // an acknowledgement arrived: cancel its timer and report success upstream
        auto ackPacket = check_and_cast<Packet *>(message);
        auto header = ackPacket->popAtFront<AcknowledgeHeader>();
        int seq = header->getSequenceNumber();
        delete ackPacket;
        auto it = timers.find(seq);
        if (it != timers.end()) {
            auto timer = it->second;
            timers.erase(it);
            auto dataPacket = static_cast<Packet *>(timer->getContextPointer());
            cancelAndDelete(timer);
            if (producer != nullptr)
                producer.handlePushPacketProcessed(dataPacket, true);
        }
    }
    else
        PacketFlowBase::handleMessage(message);
}

} // namespace inet

