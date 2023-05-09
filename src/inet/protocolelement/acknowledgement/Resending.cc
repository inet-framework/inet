//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/acknowledgement/Resending.h"

#include "inet/common/ProtocolTag_m.h"

namespace inet {

Define_Module(Resending);

void Resending::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        numRetries = par("numRetries");
}

void Resending::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handleMessage(packet);
}

void Resending::handleMessage(cMessage *message)
{
    ASSERT(retry == 0);
    packet = check_and_cast<Packet *>(message);
    pushOrSendPacket(packet->dup(), outputGate, consumer.getReferencedGate(), consumer);
    retry++;
}

void Resending::handlePushPacketProcessed(Packet *p, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (successful || retry == numRetries) {
        if (producer != nullptr)
            producer->handlePushPacketProcessed(packet, producer.getReferencedGate(), successful);
        delete packet;
        packet = nullptr;
        retry = 0;
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(producer.getReferencedGate());
    }
    else {
        pushOrSendPacket(packet->dup(), outputGate, consumer.getReferencedGate(), consumer);
        retry++;
    }
}

} // namespace inet

