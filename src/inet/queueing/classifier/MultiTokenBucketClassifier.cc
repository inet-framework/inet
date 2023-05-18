//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/MultiTokenBucketClassifier.h"

namespace inet {
namespace queueing {

Define_Module(MultiTokenBucketClassifier);

template class TokenBucketClassifierMixin<MultiTokenBucketMixin<PacketClassifierBase>>;

void MultiTokenBucketClassifier::initialize(int stage)
{
    TokenBucketClassifierMixin<MultiTokenBucketMixin<PacketClassifierBase>>::initialize(stage);
    if (stage == INITSTAGE_QUEUEING) {
        if (tokenBuckets.size() != outputGates.size() - 1)
            throw cRuntimeError("Number of buckets must be 1 less than the number of output gates");
    }
}

int MultiTokenBucketClassifier::classifyPacket(Packet *packet)
{
    emitTokensChangedSignals();
    auto numTokens = getNumPacketTokens(packet);
    for (int i = 0; i < tokenBuckets.size(); i++) {
        auto& tokenBucket = tokenBuckets[i];
        EV_DEBUG << "Checking tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
        if (tokenBucket.getNumTokens() > numTokens) {
            tokenBucket.removeTokens(numTokens);
            EV_INFO << "Removed tokens from ith bucket for packet" << EV_FIELD(numTokens) << EV_FIELD(i) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
            emitTokensChangedSignals();
            return i;
        }
    }
    EV_INFO << "Insufficient number of tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(packet) << EV_ENDL;
    return outputGates.size() - 1;
}

} // namespace queueing
} // namespace inet

