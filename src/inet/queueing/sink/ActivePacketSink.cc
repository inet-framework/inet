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
#include "inet/queueing/sink/ActivePacketSink.h"

namespace inet {
namespace queueing {

Define_Module(ActivePacketSink);

void ActivePacketSink::initialize(int stage)
{
    PacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        collectionIntervalParameter = &par("collectionInterval");
        collectionTimer = new cMessage("CollectionTimer");
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPopPacketSupport(inputGate);
}

void ActivePacketSink::handleMessage(cMessage *message)
{
    if (message == collectionTimer) {
        if (provider->canPopSomePacket(inputGate->getPathStartGate())) {
            scheduleCollectionTimer();
            collectPacket();
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

void ActivePacketSink::scheduleCollectionTimer()
{
    scheduleAfter(collectionIntervalParameter->doubleValue(), collectionTimer);
}

void ActivePacketSink::collectPacket()
{
    auto packet = provider->popPacket(inputGate->getPathStartGate());
    EV_INFO << "Collecting packet " << packet->getName() << "." << endl;
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    dropPacket(packet, OTHER_PACKET_DROP);
}

void ActivePacketSink::handleCanPopPacket(cGate *gate)
{
    Enter_Method("handleCanPopPacket");
    if (gate->getPathEndGate() == inputGate && !collectionTimer->isScheduled()) {
        scheduleCollectionTimer();
        collectPacket();
    }
}

} // namespace queueing
} // namespace inet

