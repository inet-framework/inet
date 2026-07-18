//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/acknowledgement/Resending.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/TransmissionAttemptTag_m.h"

namespace inet {

Define_Module(Resending);

// Duplicates the held packet and announces which attempt this transmission is (0 for the first,
// incremented for each retry) via a TransmissionAttemptReq tag, so downstream elements such as
// ExponentialBackoff can size their behaviour to the attempt number.
static Packet *dupWithAttempt(Packet *packet, int attempt)
{
    auto dup = packet->dup();
    dup->addTagIfAbsent<TransmissionAttemptReq>()->setAttempt(attempt);
    return dup;
}

void Resending::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numRetries = par("numRetries");
        WATCH(retry);
    }
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
    pushOrSendPacket(dupWithAttempt(packet, retry), outputGate, consumer);
    retry++;
}

void Resending::handlePushPacketProcessed(Packet *p, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (successful || retry == numRetries) {
        if (producer != nullptr)
            producer.handlePushPacketProcessed(packet, successful);
        delete packet;
        packet = nullptr;
        retry = 0;
        if (producer != nullptr)
            producer.handleCanPushPacketChanged();
    }
    else {
        pushOrSendPacket(dupWithAttempt(packet, retry), outputGate, consumer);
        retry++;
    }
}

} // namespace inet

