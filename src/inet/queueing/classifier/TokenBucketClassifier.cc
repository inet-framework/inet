//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/TokenBucketClassifier.h"

namespace inet {
namespace queueing {

Define_Module(TokenBucketClassifier);

template class TokenBucketClassifierMixin<TokenBucketMixin<PacketClassifierBase>>;

void TokenBucketClassifier::initialize(int stage)
{
    TokenBucketClassifierMixin<TokenBucketMixin<PacketClassifierBase>>::initialize(stage);
    if (stage == INITSTAGE_QUEUEING) {
        if (outputGates.size() != 2)
            throw cRuntimeError("Number of output gates must be 2");
    }
}

int TokenBucketClassifier::classifyPacket(Packet *packet) const
{
    return tokenBucket.getNumTokens() > getNumPacketTokens(packet) ? 0 : 1;
}

void TokenBucketClassifier::handlePacketClassified(Packet *packet, int index)
{
    emit(tokensChangedSignal, getNumTokens());
    auto numTokens = getNumPacketTokens(packet);
    EV_DEBUG << "Checking tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
    if (index == 0) {
        tokenBucket.removeTokens(numTokens);
        EV_INFO << "Removed tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
        emit(tokensChangedSignal, tokenBucket.getNumTokens());
    }
    else
        EV_INFO << "Insufficient number of tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
}

} // namespace queueing
} // namespace inet

