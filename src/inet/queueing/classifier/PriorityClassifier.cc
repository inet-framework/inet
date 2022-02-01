//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/PriorityClassifier.h"

namespace inet {
namespace queueing {

Define_Module(PriorityClassifier);

int PriorityClassifier::classifyPacket(Packet *packet)
{
    for (size_t i = 0; i < consumers.size(); i++) {
        size_t outputGateIndex = getOutputGateIndex(i);
        if (consumers[outputGateIndex]->canPushPacket(packet, outputGates[outputGateIndex]))
            return outputGateIndex;
    }
    return -1;
}

bool PriorityClassifier::canPushSomePacket(cGate *gate) const
{
    for (size_t i = 0; i < consumers.size(); i++) {
        auto outputConsumer = consumers[i];
        if (outputConsumer->canPushSomePacket(outputGates[i]))
            return true;
    }
    return false;
}

bool PriorityClassifier::canPushPacket(Packet *packet, cGate *gate) const
{
    for (size_t i = 0; i < consumers.size(); i++) {
        auto consumer = consumers[i];
        if (consumer->canPushPacket(packet, outputGates[i]))
            return true;
    }
    return false;
}

} // namespace queueing
} // namespace inet

