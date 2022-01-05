//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/linklayer/base/MacProtocolBaseExtQ.h"

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

MacProtocolBaseExtQ::MacProtocolBaseExtQ()
{
}

MacProtocolBaseExtQ::~MacProtocolBaseExtQ()
{
    delete currentTxFrame;
}

void MacProtocolBaseExtQ::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        currentTxFrame = nullptr;
    }
}

void MacProtocolBaseExtQ::deleteCurrentTxFrame()
{
    delete currentTxFrame;
    currentTxFrame = nullptr;
}

void MacProtocolBaseExtQ::dropCurrentTxFrame(PacketDropDetails& details)
{
    emit(packetDroppedSignal, currentTxFrame, &details);
    delete currentTxFrame;
    currentTxFrame = nullptr;
}

void MacProtocolBaseExtQ::flushQueue(PacketDropDetails& details)
{
    // code would look slightly nicer with a pop() function that returns nullptr if empty
    if (txQueue)
        while (canDequeuePacket()) {
            auto packet = dequeuePacket();
            emit(packetDroppedSignal, packet, &details); // FIXME this signal lumps together packets from the network and packets from higher layers! separate them
            delete packet;
        }
}

void MacProtocolBaseExtQ::clearQueue()
{
    if (txQueue)
        while (canDequeuePacket())
            delete dequeuePacket();
}

void MacProtocolBaseExtQ::handleStopOperation(LifecycleOperation *operation)
{
    PacketDropDetails details;
    details.setReason(INTERFACE_DOWN);
    if (currentTxFrame)
        dropCurrentTxFrame(details);
    flushQueue(details);

    MacProtocolBase::handleStopOperation(operation);
}

void MacProtocolBaseExtQ::handleCrashOperation(LifecycleOperation *operation)
{
    deleteCurrentTxFrame();
    clearQueue();
    MacProtocolBase::handleCrashOperation(operation);
}

queueing::IPacketQueue *MacProtocolBaseExtQ::getQueue(cGate *gate) const
{
    for (auto g = gate->getPreviousGate(); g != nullptr; g = g->getPreviousGate()) {
        if (g->getType() == cGate::OUTPUT) {
            auto m = dynamic_cast<queueing::IPacketQueue *>(g->getOwnerModule());
            if (m)
                return m;
        }
    }
    throw cRuntimeError("Gate %s is not connected to a module of type queueing::IPacketQueue", gate->getFullPath().c_str());
}

bool MacProtocolBaseExtQ::canDequeuePacket() const
{
    return txQueue && txQueue->canPullSomePacket(gate(upperLayerInGateId)->getPathStartGate());
}

Packet *MacProtocolBaseExtQ::dequeuePacket()
{
    Packet *packet = txQueue->dequeuePacket();
    take(packet);
    packet->setArrival(getId(), upperLayerInGateId, simTime());
    return packet;
}

} // namespace inet

