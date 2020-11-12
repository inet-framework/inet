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

#include "inet/queueing/source/ActivePacketSource.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {
namespace queueing {

Define_Module(ActivePacketSource);

void ActivePacketSource::initialize(int stage)
{
    ActivePacketSourceBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        productionIntervalParameter = &par("productionInterval");
        productionTimer = new cMessage("ProductionTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (!productionTimer->isScheduled() && (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate()))) {
            scheduleProductionTimer();
            producePacket();
        }
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
    scheduleAfter(productionIntervalParameter->doubleValue(), productionTimer);
}

void ActivePacketSource::producePacket()
{
    auto packet = createPacket();
    EV_INFO << "Producing packet" << EV_FIELD(packet) << EV_ENDL;
    pushOrSendPacket(packet, outputGate, consumer);
    updateDisplayString();
}

void ActivePacketSource::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (!productionTimer->isScheduled() && (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate()))) {
        scheduleProductionTimer();
        producePacket();
    }
}

void ActivePacketSource::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
}

} // namespace queueing
} // namespace inet

