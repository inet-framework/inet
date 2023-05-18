//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/meter/MultiTokenBucketMeter.h"

#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(MultiTokenBucketMeter);

template class TokenBucketMeterMixin<MultiTokenBucketMixin<PacketMeterBase>>;

void MultiTokenBucketMeter::initialize(int stage)
{
    TokenBucketMeterMixin<MultiTokenBucketMixin<PacketMeterBase>>::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cValueArray *bucketConfigurations = check_and_cast<cValueArray *>(par("buckets").objectValue());
        for (int i = 0; i < bucketConfigurations->size(); i++) {
            cValueMap *bucketConfiguration = check_and_cast<cValueMap *>(bucketConfigurations->get(i).objectValue());
            labels.push_back(bucketConfiguration->get("label").stringValue());
        }
        WATCH_VECTOR(tokenBuckets);
    }
}

void MultiTokenBucketMeter::meterPacket(Packet *packet)
{
    emitTokensChangedSignals();
    auto numTokens = getNumPacketTokens(packet);
    for (int i = 0; i < tokenBuckets.size(); i++) {
        auto& tokenBucket = tokenBuckets[i];
        EV_DEBUG << "Checking tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
        if (tokenBucket.getNumTokens() >= numTokens) {
            tokenBucket.removeTokens(numTokens);
            auto label = labels[i].c_str();
            EV_INFO << "Removed tokens, labeling packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(label) << EV_FIELD(packet) << EV_ENDL;
            packet->addTagIfAbsent<LabelsTag>()->appendLabels(label);
            emitTokensChangedSignals();
            return;
        }
    }
    EV_INFO << "Insufficient number of tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(packet) << EV_ENDL;
    if (defaultLabel != nullptr)
        packet->addTagIfAbsent<LabelsTag>()->appendLabels(defaultLabel);
}

} // namespace queueing
} // namespace inet

