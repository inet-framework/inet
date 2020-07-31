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

#include "inet/protocol/transceiver/PacketTransmitter.h"

namespace inet {

Define_Module(PacketTransmitter);

void PacketTransmitter::handleMessageWhenUp(cMessage *message)
{
    if (message == txEndTimer)
        endTx();
    else
        PacketTransmitterBase::handleMessageWhenUp(message);
}

void PacketTransmitter::handleStopOperation(LifecycleOperation *operation)
{
}

void PacketTransmitter::handleCrashOperation(LifecycleOperation *operation)
{
}

void PacketTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    startTx(packet);
}

void PacketTransmitter::startTx(Packet *packet)
{
    ASSERT(txSignal == nullptr);
    txSignal = encodePacket(packet);
    emit(transmissionStartedSignal, txSignal);
    send(txSignal->dup(), SendOptions().duration(txSignal->getDuration()), outputGate);
    scheduleTxEndTimer(txSignal);
}

void PacketTransmitter::endTx()
{
    emit(transmissionStartedSignal, txSignal);
    auto packet = check_and_cast<Packet *>(txSignal->getEncapsulatedPacket());
    producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
    delete txSignal;
    txSignal = nullptr;
    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleClockEvent(getClockTime() + SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

} // namespace inet

