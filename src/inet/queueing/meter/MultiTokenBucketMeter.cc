//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/queueing/meter/MultiTokenBucketMeter.h"

#include "inet/queueing/common/LabelsTag_m.h"

namespace inet {
namespace queueing {

Define_Module(MultiTokenBucketMeter);

template class MultiTokenBucketMixin<PacketMeterBase>;

void MultiTokenBucketMeter::initialize(int stage)
{
    MultiTokenBucketMixin<PacketMeterBase>::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        cValueArray *bucketConfigurations = check_and_cast<cValueArray *>(par("buckets").objectValue());
        for (int i = 0; i < bucketConfigurations->size(); i++) {
            cValueMap *bucketConfiguration = check_and_cast<cValueMap *>(bucketConfigurations->get(i).objectValue());
            labels.push_back(bucketConfiguration->get("label").stringValue());
        }
        defaultLabel = par("defaultLabel");
        WATCH_VECTOR(tokenBuckets);
    }
}

void MultiTokenBucketMeter::meterPacket(Packet *packet)
{
    emit(tokensChangedSignal, getNumTokens());
    auto numTokens = b(packet->getDataLength()).get();
    for (int i = 0; i < tokenBuckets.size(); i++) {
        auto& tokenBucket = tokenBuckets[i];
        EV_DEBUG << "Checking tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
        if (tokenBucket.putPacket(packet)) {
            auto label = labels[i].c_str();
            EV_INFO << "Removed tokens, labeling packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(label) << EV_FIELD(packet) << EV_ENDL;
            packet->addTagIfAbsent<LabelsTag>()->insertLabels(label);
            emit(tokensChangedSignal, getNumTokens());
            return;
        }
    }
    EV_INFO << "Insufficient number of tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(packet) << EV_ENDL;
    if (defaultLabel != nullptr)
        packet->addTagIfAbsent<LabelsTag>()->insertLabels(defaultLabel);
}

} // namespace queueing
} // namespace inet

