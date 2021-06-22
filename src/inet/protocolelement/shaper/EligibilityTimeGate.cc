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

#include "inet/protocolelement/shaper/EligibilityTimeGate.h"

#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"

namespace inet {

Define_Module(EligibilityTimeGate);

void EligibilityTimeGate::initialize(int stage)
{
    PacketGateBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        eligibilityTimer = new cMessage("EligibilityTimer");
    else if (stage == INITSTAGE_QUEUEING)
        updateOpen();
}

void EligibilityTimeGate::handleMessage(cMessage *message)
{
    if (message == eligibilityTimer)
        updateOpen();
    else
        throw cRuntimeError("Unknown message");
}

void EligibilityTimeGate::updateOpen()
{
    auto packet = provider->canPullPacket(inputGate->getPathStartGate());
    if (packet == nullptr || packet->getTag<EligibilityTimeTag>()->getEligibilityTime() <= simTime()) {
        if (isClosed())
            open();
    }
    else {
        if (isOpen())
            close();
    }
    packet = provider->canPullPacket(inputGate->getPathStartGate());
    if (packet != nullptr) {
        simtime_t eligibilityTime = packet->getTag<EligibilityTimeTag>()->getEligibilityTime();
        if (eligibilityTime > simTime())
            rescheduleAt(eligibilityTime, eligibilityTimer);
    }
}

Packet *EligibilityTimeGate::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = PacketGateBase::pullPacket(gate);
    updateOpen();
    return packet;
}

void EligibilityTimeGate::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    updateOpen();
    PacketGateBase::handleCanPullPacketChanged(gate);
}

} // namespace inet

