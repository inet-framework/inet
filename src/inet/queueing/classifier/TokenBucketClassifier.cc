//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

int TokenBucketClassifier::classifyPacket(Packet *packet)
{
    emit(tokensChangedSignal, getNumTokens());
    auto numTokens = getNumPacketTokens(packet);
    EV_DEBUG << "Checking tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
    if (tokenBucket.getNumTokens() > numTokens) {
        tokenBucket.removeTokens(numTokens);
        EV_INFO << "Removed tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
        emit(tokensChangedSignal, tokenBucket.getNumTokens());
        return 0;
    }
    else {
        EV_INFO << "Insufficient number of tokens for packet" << EV_FIELD(numTokens) << EV_FIELD(tokenBucket) << EV_FIELD(packet) << EV_ENDL;
        return 1;
    }
}

} // namespace queueing
} // namespace inet

