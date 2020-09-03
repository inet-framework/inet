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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/protocol/transceiver/StreamingTransmitter.h"

namespace inet {

Define_Module(StreamingTransmitter);

void StreamingTransmitter::handleMessageWhenUp(cMessage *message)
{
    if (message == txEndTimer)
        endTx();
    else
        PacketTransmitterBase::handleMessageWhenUp(message);
}

void StreamingTransmitter::handleStopOperation(LifecycleOperation *operation)
{
    if (isTransmitting())
        abortTx();
}

void StreamingTransmitter::handleCrashOperation(LifecycleOperation *operation)
{
    if (isTransmitting())
        abortTx();
}

void StreamingTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    startTx(packet);
}

void StreamingTransmitter::startTx(Packet *packet)
{
    // 1. check current state
    ASSERT(!isTransmitting());
    // 2. store transmission progress
    txDatarate = bps(*dataratePar);
    txStartTime = getClockTime();
    // 3. create signal
    auto signal = encodePacket(packet);
    txSignal = signal->dup();
    txSignal->setOrigPacketId(signal->getId());
    // 5. send signal start and notify subscribers
    EV_INFO << "Starting transmission" << EV_FIELD(packet) << EV_FIELD(txDatarate) << EV_ENDL;
    emit(transmissionStartedSignal, signal);
    sendPacketStart(signal);
    // 6. schedule transmission end
    scheduleTxEndTimer(txSignal);
}

void StreamingTransmitter::endTx()
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. send signal end to receiver and notify subscribers
    auto packet = check_and_cast<Packet *>(txSignal->getEncapsulatedPacket());
    EV_INFO << "Ending transmission" << EV_FIELD(packet) << EV_FIELD(txDatarate) << EV_ENDL;
    emit(transmissionEndedSignal, txSignal);
    sendPacketEnd(txSignal);
    // 3. clear internal state
    txSignal = nullptr;
    txStartTime = -1;
    // 4. notify producer
    auto gate = inputGate->getPathStartGate();
    producer->handlePushPacketProcessed(packet, gate, true);
    producer->handleCanPushPacketChanged(gate);
}

void StreamingTransmitter::abortTx()
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. create new truncated signal
    auto packet = check_and_cast<Packet *>(txSignal->getEncapsulatedPacket());
    // TODO: we can't just simply cut the packet proportionally with time because it's not always the case (modulation, scrambling, etc.)
    clocktime_t timePosition = getClockTime() - txStartTime;
    b dataPosition = b(std::floor(txDatarate.get() * timePosition.dbl()));
    packet->eraseAtBack(packet->getTotalLength() - dataPosition);
    packet->setBitError(true);
    auto signal = encodePacket(packet);
    signal->setOrigPacketId(txSignal->getOrigPacketId());
    // 3. delete old signal
    delete txSignal;
    txSignal = nullptr;
    // 4. send signal end to receiver and notify subscribers
    EV_INFO << "Aborting transmission" << EV_FIELD(packet) << EV_FIELD(txDatarate) << EV_ENDL;
    emit(transmissionEndedSignal, txSignal);
    sendPacketEnd(signal);
    // 5. clear internal state
    txStartTime = -1;
    // 6. notify producer
    auto gate = inputGate->getPathStartGate();
    producer->handlePushPacketProcessed(packet, gate, true);
    producer->handleCanPushPacketChanged(gate);
}

void StreamingTransmitter::scheduleTxEndTimer(Signal *signal)
{
    ASSERT(txStartTime != -1);
    clocktime_t txEndTime = txStartTime + SIMTIME_AS_CLOCKTIME(signal->getDuration());
    EV_INFO << "Scheduling transmission end timer" << EV_FIELD(at, txEndTime.ustr()) << EV_ENDL;
    cancelClockEvent(txEndTimer);
    scheduleClockEventAt(txEndTime, txEndTimer);
}

} // namespace inet

