//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/PacketClassifier.h"

namespace inet {
namespace queueing {

Define_Module(PacketClassifier);

void PacketClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        packetClassifierFunction = createClassifierFunction(par("classifierClass"));
}

IPacketClassifierFunction *PacketClassifier::createClassifierFunction(const char *classifierClass) const
{
    return check_and_cast<IPacketClassifierFunction *>(createOne(classifierClass));
}

int PacketClassifier::classifyPacket(Packet *packet)
{
    int index = packetClassifierFunction->classifyPacket(packet);
    return index == -1 ? index : getOutputGateIndex(index);
}

} // namespace queueing
} // namespace inet

