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
#include "inet/queueing/PacketProvider.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(PacketProvider);

void PacketProvider::initialize(int stage)
{
    PacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        collector = dynamic_cast<IPacketCollector *>(getConnectedModule(outputGate));
        providingIntervalParameter = &par("providingInterval");
        providingTimer = new cMessage("ProvidingTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (collector != nullptr) {
            checkPopPacketSupport(outputGate);
            collector->handleCanPopPacket(outputGate);
        }
    }
}

void PacketProvider::handleMessage(cMessage *message)
{
    if (message == providingTimer) {
        if (collector != nullptr)
            collector->handleCanPopPacket(outputGate);
    }
    else
        throw cRuntimeError("Unknown message");
}

void PacketProvider::scheduleProvidingTimer()
{
    simtime_t interval = providingIntervalParameter->doubleValue();
    if (interval != 0 || providingTimer->getArrivalModule() == nullptr)
        scheduleAt(simTime() + interval, providingTimer);
}

Packet *PacketProvider::canPopPacket(cGate *gate)
{
    if (providingTimer->isScheduled())
        return nullptr;
    else {
        if (nextPacket == nullptr)
            nextPacket = createPacket();
        return nextPacket;
    }
}

Packet *PacketProvider::popPacket(cGate *gate)
{
    Enter_Method_Silent();
    if (providingTimer->isScheduled()  && providingTimer->getArrivalTime() > simTime())
        throw cRuntimeError("Another packet is already being provided");
    else {
        auto packet = providePacket(gate);
        animateSend(packet, outputGate);
        emit(packetPoppedSignal, packet);
        scheduleProvidingTimer();
        return packet;
    }
}

Packet *PacketProvider::providePacket(cGate *gate)
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

