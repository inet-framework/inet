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
#include "inet/common/Simsignals.h"
#include "inet/queueing/sink/PassivePacketSink.h"

namespace inet {
namespace queueing {

Define_Module(PassivePacketSink);

void PassivePacketSink::initialize(int stage)
{
    PacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        consumptionIntervalParameter = &par("consumptionInterval");
        consumptionTimer = new cMessage("ConsumptionTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        if (producer != nullptr)
            producer->handleCanPushPacket(inputGate->getPathStartGate());
    }
}

void PassivePacketSink::handleMessage(cMessage *message)
{
    if (message == consumptionTimer) {
        if (producer != nullptr)
            producer->handleCanPushPacket(inputGate->getPathStartGate());
    }
    else
        PassivePacketSinkBase::handleMessage(message);
}

void PassivePacketSink::scheduleConsumptionTimer()
{
    simtime_t interval = consumptionIntervalParameter->doubleValue();
    if (interval != 0 || consumptionTimer->getArrivalModule() == nullptr)
        scheduleAt(simTime() + interval, consumptionTimer);
}

void PassivePacketSink::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    if (consumptionTimer->isScheduled() && consumptionTimer->getArrivalTime() > simTime())
        throw cRuntimeError("Another packet is already being consumed");
    else {
        emit(packetPushedSignal, packet);
        consumePacket(packet);
        scheduleConsumptionTimer();
    }
}

void PassivePacketSink::consumePacket(Packet *packet)
{
    EV_INFO << "Consuming packet " << packet->getName() << "." << endl;
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    dropPacket(packet, OTHER_PACKET_DROP);
}

} // namespace queueing
} // namespace inet

