//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/function/PacketClassifierFunction.h"

#include "inet/linklayer/common/UserPriorityTag_m.h"

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

