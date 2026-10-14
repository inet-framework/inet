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
        size_t i;
        for (i = 0; i < consumers.size() && tokenizer.hasMoreTokens(); ++i)
            buckets[i] = weights[i] = (int)utils::atoul(tokenizer.nextToken());

        if (i < consumers.size())
            throw cRuntimeError("Too few values given in the weights parameter.");
        if (tokenizer.hasMoreTokens())
            throw cRuntimeError("Too many values given in the weights parameter.");
    }
}

int WrrClassifier::classifyPacket(Packet *packet) const
{
    bool isEmpty = true;
    for (size_t i = 0; i < consumers.size(); ++i) {
        if (consumers[i].canPushSomePacket()) {
            isEmpty = false;
            if (buckets[i] > 0)
                return i;
        }
    }

    if (isEmpty)
        return -1;

    // a new round starts: the first consumer with a weight that can take the packet
    for (size_t i = 0; i < consumers.size(); ++i)
        if (weights[i] > 0 && consumers[i].canPushSomePacket())
            return i;
    return -1;
}

void WrrClassifier::handlePacketClassified(Packet *packet, int index)
{
    // an empty bucket of the selected consumer means that a new round starts
    if (buckets[index] == 0)
        for (size_t i = 0; i < consumers.size(); ++i)
            buckets[i] = weights[i];
    buckets[index]--;
}

} // namespace queueing
} // namespace inet

