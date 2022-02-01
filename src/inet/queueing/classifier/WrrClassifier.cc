//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/WrrClassifier.h"

#include "inet/common/INETUtils.h"

namespace inet {
namespace queueing {

Define_Module(WrrClassifier);

WrrClassifier::~WrrClassifier()
{
    delete[] weights;
    delete[] buckets;
}

void WrrClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        weights = new int[consumers.size()];
        buckets = new int[consumers.size()];

        cStringTokenizer tokenizer(par("weights"));
        int i;
        for (i = 0; i < (int)consumers.size() && tokenizer.hasMoreTokens(); ++i)
            buckets[i] = weights[i] = (int)utils::atoul(tokenizer.nextToken());

        if (i < (int)consumers.size())
            throw cRuntimeError("Too few values given in the weights parameter.");
        if (tokenizer.hasMoreTokens())
            throw cRuntimeError("Too many values given in the weights parameter.");
    }
}

int WrrClassifier::classifyPacket(Packet *packet)
{
    bool isEmpty = true;
    for (int i = 0; i < (int)consumers.size(); ++i) {
        if (consumers[i]->canPushSomePacket(outputGates[i]->getPathEndGate())) {
            isEmpty = false;
            if (buckets[i] > 0) {
                buckets[i]--;
                return i;
            }
        }
    }

    if (isEmpty)
        return -1;

    int result = -1;
    for (int i = 0; i < (int)consumers.size(); ++i) {
        buckets[i] = weights[i];
        if (result == -1 && buckets[i] > 0 && consumers[i]->canPushSomePacket(outputGates[i]->getPathEndGate())) {
            buckets[i]--;
            result = i;
        }
    }
    return result;
}

} // namespace queueing
} // namespace inet

