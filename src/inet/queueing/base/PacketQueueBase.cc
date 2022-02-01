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

const char *PacketQueueBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'p':
            result = std::to_string(getNumPackets());
            break;
        case 'l':
            result = getTotalLength().str();
            break;
        case 'u':
            result = std::to_string(numPushedPackets);
            break;
        case 'o':
            result = std::to_string(numPulledPackets);
            break;
        case 'r':
            result = std::to_string(numRemovedPackets);
            break;
        case 'd':
            result = std::to_string(numDroppedPackets);
            break;
        case 'c':
            result = std::to_string(numCreatedPackets);
            break;
        case 'n':
            result = !isEmpty() ? getPacket(0)->getFullName() : "";
            break;
        default:
            result = PacketProcessorBase::resolveDirective(directive);
            break;
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

