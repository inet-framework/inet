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

#include "inet/protocolelement/transceiver/PacketTransmitter.h"

namespace inet {

Define_Module(PacketTransmitter);

void PacketTransmitter::handleMessageWhenUp(cMessage *message)
{
    if (message == txEndTimer)
        endTx();
    else
        PacketTransmitterBase::handleMessageWhenUp(message);
    updateDisplayString();
}

void PacketTransmitter::handleStopOperation(LifecycleOperation *operation)
{
    ASSERT(!isTransmitting());
}

void PacketTransmitter::handleCrashOperation(LifecycleOperation *operation)
{
    ASSERT(!isTransmitting());
}

void PacketTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    startTx(packet);
    updateDisplayString();
}

void PacketTransmitter::startTx(Packet *packet)
{
    // 1. check current state
    ASSERT(!isTransmitting());
    // 2. create signal
    auto signal = encodePacket(packet);
    txSignal = signal->dup();
    // 3. send signal start and notify subscribers
    emit(transmissionStartedSignal, signal);
    send(signal, SendOptions().duration(signal->getDuration()), outputGate);
    // 4. schedule transmission end timer
    scheduleTxEndTimer(txSignal);
}

void PacketTransmitter::endTx()
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. notify subscribers
    auto packet = check_and_cast<Packet *>(txSignal->getEncapsulatedPacket());
    handlePacketProcessed(packet);
    emit(transmissionEndedSignal, txSignal);
    // 3. clear internal state
    delete txSignal;
    txSignal = nullptr;
    // 4. notify producer
    if (producer != nullptr) {
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleClockEventAfter(SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

} // namespace inet

