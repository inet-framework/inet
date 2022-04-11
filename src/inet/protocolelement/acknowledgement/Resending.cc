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

void Resending::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handleMessage(packet);
}

void Resending::handleMessage(cMessage *message)
{
    ASSERT(retry == 0);
    packet = check_and_cast<Packet *>(message);
    pushOrSendPacket(packet->dup(), outputGate, consumer);
    retry++;
}

void Resending::handlePushPacketProcessed(Packet *p, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (successful || retry == numRetries) {
        if (producer != nullptr)
            producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
        delete packet;
        packet = nullptr;
        retry = 0;
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
    else {
        pushOrSendPacket(packet->dup(), outputGate, consumer);
        retry++;
    }
}

} // namespace inet

