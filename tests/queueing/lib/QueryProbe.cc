//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "QueryProbe.h"

namespace inet {

Define_Module(QueryProbe);

void QueryProbe::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        numRepeats = par("numRepeats");
}

void QueryProbe::processPacket(Packet *packet)
{
    // the push operations call this function after they take the packet and before they push it on
    if (consumer != nullptr) {
        for (int i = 0; i < numRepeats; i++) {
            consumer.canPushSomePacket();
            consumer.canPushPacket(packet);
        }
    }
}

bool QueryProbe::canPushSomePacket(const cGate *gate) const
{
    bool result = PacketFlowBase::canPushSomePacket(gate);
    for (int i = 0; i < numRepeats; i++)
        if (PacketFlowBase::canPushSomePacket(gate) != result)
            throw cRuntimeError("A repeated canPushSomePacket() gives another answer");
    return result;
}

bool QueryProbe::canPushPacket(Packet *packet, const cGate *gate) const
{
    bool result = PacketFlowBase::canPushPacket(packet, gate);
    for (int i = 0; i < numRepeats; i++)
        if (PacketFlowBase::canPushPacket(packet, gate) != result)
            throw cRuntimeError("A repeated canPushPacket() gives another answer for packet %s", packet->getName());
    return result;
}

bool QueryProbe::canPullSomePacket(const cGate *gate) const
{
    bool result = PacketFlowBase::canPullSomePacket(gate);
    for (int i = 0; i < numRepeats; i++)
        if (PacketFlowBase::canPullSomePacket(gate) != result)
            throw cRuntimeError("A repeated canPullSomePacket() gives another answer");
    return result;
}

Packet *QueryProbe::canPullPacket(const cGate *gate) const
{
    auto packet = PacketFlowBase::canPullPacket(gate);
    for (int i = 0; i < numRepeats; i++)
        if (PacketFlowBase::canPullPacket(gate) != packet)
            throw cRuntimeError("A repeated canPullPacket() offers another packet");
    return packet;
}

Packet *QueryProbe::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    if (numRepeats == 0)
        return PacketFlowBase::pullPacket(gate);
    // the offered packet can be deleted before the pull returns, so keep its identity only
    auto offeredPacket = canPullPacket(gate);
    long offeredPacketId = offeredPacket != nullptr ? offeredPacket->getId() : -1;
    std::string offeredPacketName = offeredPacket != nullptr ? offeredPacket->getName() : "";
    auto packet = PacketFlowBase::pullPacket(gate);
    if (offeredPacketId != -1 && packet->getId() != offeredPacketId)
        throw cRuntimeError("The pulled packet %s is not the offered packet %s", packet->getName(), offeredPacketName.c_str());
    return packet;
}

} // namespace inet

