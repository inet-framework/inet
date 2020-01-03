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
#include "inet/queueing/source/ActivePacketSource.h"

namespace inet {
namespace queueing {

Define_Module(ActivePacketSource);

void ActivePacketSource::initialize(int stage)
{
    PacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        productionIntervalParameter = &par("productionInterval");
        productionTimer = new cMessage("ProductionTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPushPacketSupport(outputGate);
        if (consumer == nullptr && !productionTimer->isScheduled())
            scheduleProductionTimer();
    }
}

void ActivePacketSource::handleMessage(cMessage *message)
{
    if (message == productionTimer) {
        if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
            scheduleProductionTimer();
            producePacket();
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

void ActivePacketSource::scheduleProductionTimer()
{
    scheduleAt(simTime() + productionIntervalParameter->doubleValue(), productionTimer);
}

void ActivePacketSource::producePacket()
{
    auto packet = createPacket();
    EV_INFO << "Producing packet " << packet->getName() << "." << endl;
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void ActivePacketSource::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (gate->getPathStartGate() == outputGate && !productionTimer->isScheduled()) {
        scheduleProductionTimer();
        producePacket();
    }
}

} // namespace queueing
} // namespace inet

