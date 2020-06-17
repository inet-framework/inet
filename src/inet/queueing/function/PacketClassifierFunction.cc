//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"

namespace inet {
namespace queueing {

static int classifyPacketByData(Packet *packet)
{
    const auto& data = packet->peekDataAt<BytesChunk>(B(0), B(1));
    return data->getBytes().at(0);
}

Register_Packet_Classifier_Function(PacketDataClassifier, classifyPacketByData);

static int classifyPacketAsCharacterOrEnter(Packet *packet)
{
    const auto& data = packet->peekDataAt<BytesChunk>(B(0), B(1));
    auto byte = data->getBytes().at(0);
    if (byte != 13)
        return 0;
    else
        return 1;
}

Register_Packet_Classifier_Function(PacketCharacterOrEnterClassifier, classifyPacketAsCharacterOrEnter);

static int classifyPacketByUserPriorityReq(Packet *packet)
{
    auto userPriorityReq = packet->getTag<UserPriorityReq>();
    return userPriorityReq->getUserPriority();
}

Register_Packet_Classifier_Function(PacketUserPriorityReqClassifier, classifyPacketByUserPriorityReq);

static int classifyPacketByUserPriorityInd(Packet *packet)
{
    const auto& userPriorityInd = packet->findTag<UserPriorityInd>();
    return userPriorityInd != nullptr ? userPriorityInd->getUserPriority() : 0;
}

Register_Packet_Classifier_Function(PacketUserPriorityIndClassifier, classifyPacketByUserPriorityInd);

} // namespace queueing
} // namespace inet

