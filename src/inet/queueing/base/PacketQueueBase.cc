//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketQueueBase.h"

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queueing {

void PacketQueueBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        displayStringTextFormat = par("displayStringTextFormat");
        numPushedPackets = 0;
        numPulledPackets = 0;
        numRemovedPackets = 0;
        numDroppedPackets = 0;
        numCreatedPackets = 0;
        WATCH(numPushedPackets);
        WATCH(numPulledPackets);
        WATCH(numRemovedPackets);
        WATCH(numDroppedPackets);
    }
}

void PacketQueueBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketQueueBase::enqueuePacket(Packet *packet)
{
    pushPacket(packet, inputGate);
}

Packet *PacketQueueBase::dequeuePacket()
{
    auto packet = pullPacket(outputGate);
    drop(packet);
    return packet;
}

void PacketQueueBase::emit(simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetPushedSignal)
        numPushedPackets++;
    else if (signal == packetPulledSignal)
        numPulledPackets++;
    else if (signal == packetRemovedSignal)
        numRemovedPackets++;
    else if (signal == packetDroppedSignal)
        numDroppedPackets++;
    cSimpleModule::emit(signal, object, details);
}

std::string PacketQueueBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(getNumPackets());
        case 'l':
            return getTotalLength().str();
        case 'u':
            return std::to_string(numPushedPackets);
        case 'o':
            return std::to_string(numPulledPackets);
        case 'r':
            return std::to_string(numRemovedPackets);
        case 'd':
            return std::to_string(numDroppedPackets);
        case 'c':
            return std::to_string(numCreatedPackets);
        case 'n':
            return !isEmpty() ? getPacket(0)->getFullName() : "";
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
}

} // namespace queueing
} // namespace inet

