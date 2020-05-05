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

#include "inet/common/Simsignals.h"
#include "inet/queueing/source/PassivePacketSource.h"

namespace inet {
namespace queueing {

Define_Module(PassivePacketSource);

void PassivePacketSource::initialize(int stage)
{
    PassivePacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        providingIntervalParameter = &par("providingInterval");
        providingTimer = new cMessage("ProvidingTimer");
        WATCH_PTR(nextPacket);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (collector != nullptr)
            collector->handleCanPullPacket(outputGate->getPathEndGate());
    }
}

void PassivePacketSource::handleMessage(cMessage *message)
{
    if (message == providingTimer) {
        if (collector != nullptr)
            collector->handleCanPullPacket(outputGate->getPathEndGate());
    }
    else
        throw cRuntimeError("Unknown message");
}

void PassivePacketSource::scheduleProvidingTimer()
{
    simtime_t interval = providingIntervalParameter->doubleValue();
    if (interval != 0 || providingTimer->getArrivalModule() == nullptr)
        scheduleAt(simTime() + interval, providingTimer);
}

Packet *PassivePacketSource::canPullPacket(cGate *gate) const
{
    if (providingTimer->isScheduled())
        return nullptr;
    else {
        if (nextPacket == nullptr)
            // TODO: KLUDGE:
            nextPacket = const_cast<PassivePacketSource *>(this)->createPacket();
        return nextPacket;
    }
}

Packet *PassivePacketSource::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    if (providingTimer->isScheduled()  && providingTimer->getArrivalTime() > simTime())
        throw cRuntimeError("Another packet is already being provided");
    else {
        auto packet = providePacket(gate);
        animateSendPacket(packet, outputGate);
        emit(packetPulledSignal, packet);
        scheduleProvidingTimer();
        updateDisplayString();
        return packet;
    }
}

Packet *PassivePacketSource::providePacket(cGate *gate)
{
    Packet *packet;
    if (nextPacket == nullptr)
        packet = createPacket();
    else {
        packet = nextPacket;
        nextPacket = nullptr;
    }
    EV_INFO << "Providing packet " << packet->getName() << "." << endl;
    updateDisplayString();
    return packet;
}

} // namespace queueing
} // namespace inet

