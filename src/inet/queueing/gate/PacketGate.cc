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

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/gate/PacketGate.h"

namespace inet {
namespace queueing {

Define_Module(PacketGate);

void PacketGate::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isOpen_ = par("initiallyOpen");
        changeTimer = new cMessage("ChangeTimer");
        double openTime = par("openTime");
        if (!std::isnan(openTime))
            changeTimes.push_back(openTime);
        double closeTime = par("closeTime");
        if (!std::isnan(closeTime))
            changeTimes.push_back(closeTime);
        const char *changeTimesAsString = par("changeTimes");
        cStringTokenizer tokenizer(changeTimesAsString);
        while (tokenizer.hasMoreTokens())
            changeTimes.push_back(SimTime::parse(tokenizer.nextToken()));

    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (changeIndex < (int)changeTimes.size())
            scheduleChangeTimer();
    }
}

void PacketGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        processChangeTimer();
        if (changeIndex < (int)changeTimes.size())
            scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

bool PacketGate::matchesPacket(const Packet *packet) const
{
    return isOpen_;
}

void PacketGate::scheduleChangeTimer()
{
    ASSERT(0 <= changeIndex && changeIndex < (int)changeTimes.size());
    scheduleAt(changeTimes[changeIndex], changeTimer);
    changeIndex++;
}

void PacketGate::processChangeTimer()
{
    if (isOpen_)
        close();
    else
        open();
}

void PacketGate::open()
{
    ASSERT(!isOpen_);
    EV_DEBUG << "Opening gate.\n";
    isOpen_ = true;
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
    if (collector != nullptr)
        collector->handleCanPullPacket(outputGate->getPathEndGate());
}

void PacketGate::close()
{
    ASSERT(isOpen_);
    EV_DEBUG << "Closing gate.\n";
    isOpen_ = false;
}

bool PacketGate::canPushSomePacket(cGate *gate) const
{
    return isOpen_ && consumer->canPushSomePacket(outputGate->getPathStartGate());
}

bool PacketGate::canPushPacket(Packet *packet, cGate *gate) const
{
    return isOpen_ && consumer->canPushPacket(packet, outputGate->getPathStartGate());
}

bool PacketGate::canPullSomePacket(cGate *gate) const
{
    return isOpen_ && provider->canPullSomePacket(inputGate->getPathStartGate());
}

Packet *PacketGate::canPullPacket(cGate *gate) const
{
    return isOpen_ ? provider->canPullPacket(inputGate->getPathStartGate()) : nullptr;
}

void PacketGate::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (isOpen_)
        PacketFilterBase::handleCanPushPacket(gate);
}

void PacketGate::handleCanPullPacket(cGate *gate)
{
    Enter_Method("handleCanPullPacket");
    if (isOpen_)
        PacketFilterBase::handleCanPullPacket(gate);
}

} // namespace queueing
} // namespace inet

