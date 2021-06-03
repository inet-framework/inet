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

#include "inet/queueing/classifier/MultiTokenBucketClassifier.h"

namespace inet {
namespace queueing {

Define_Module(MultiTokenBucketClassifier);

template class MultiTokenBucketMixin<PacketClassifierBase>;

void MultiTokenBucketClassifier::initialize(int stage)
{
    MultiTokenBucketMixin<PacketClassifierBase>::initialize(stage);
    if (stage == INITSTAGE_QUEUEING) {
        if (tokenBuckets.size() != outputGates.size() - 1)
            throw cRuntimeError("Number of buckets must be 1 less than the number of output gates");
    }
}

int MultiTokenBucketClassifier::classifyPacket(Packet *packet)
{
    emit(tokensChangedSignal, getNumTokens());
    auto numTokens = b(packet->getDataLength()).get();
    for (int i = 0; i < tokenBuckets.size(); i++) {
        auto& tokenBucket = tokenBuckets[i];
        EV_DEBUG << "Checking tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
        if (tokenBucket.putPacket(packet)) {
            EV_INFO << "Removed tokens from ith bucket for packet" << EV_FIELD(numTokens) << EV_FIELD(i) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
            emit(tokensChangedSignal, getNumTokens());
            return i;
        }
    }
    EV_INFO << "Insufficient number of tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(packet) << EV_ENDL;
    return outputGates.size() - 1;
}

} // namespace queueing
} // namespace inet

