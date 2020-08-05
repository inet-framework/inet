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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/protocol/transceiver/StreamThroughTransmitter.h"

namespace inet {

Define_Module(StreamThroughTransmitter);

void StreamThroughTransmitter::handleMessageWhenUp(cMessage *message)
{
    if (message == txEndTimer)
        endTx();
    else
        PacketTransmitterBase::handleMessageWhenUp(message);
}

void StreamThroughTransmitter::handleStopOperation(LifecycleOperation *operation)
{
}

void StreamThroughTransmitter::handleCrashOperation(LifecycleOperation *operation)
{
}

void StreamThroughTransmitter::startTx(Packet *packet)
{
    datarate = bps(*dataratePar);
    txStartTime = getClockTime();
    ASSERT(txSignal == nullptr);
    txSignal = encodePacket(packet);
    EV_INFO << "Starting transmission: packetName = " << packet->getName() << ", length = " << packet->getTotalLength() << ", duration = " << txSignal->getDuration() << std::endl;
    scheduleTxEndTimer(txSignal);
    emit(transmissionStartedSignal, txSignal);
    sendPacketStart(txSignal->dup());
}

void StreamThroughTransmitter::endTx()
{
    auto packet = check_and_cast<Packet *>(txSignal->getEncapsulatedPacket());
    EV_INFO << "Ending transmission: packetName = " << packet->getName() << std::endl;
    emit(transmissionEndedSignal, txSignal);
    sendPacketEnd(txSignal);
    txSignal = nullptr;
    txId = -1;
    txStartTime = -1;
    producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void StreamThroughTransmitter::scheduleTxEndTimer(Signal *signal)
{
    ASSERT(txStartTime != -1);
    scheduleClockEventAt(txStartTime + SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

void StreamThroughTransmitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    startTx(packet);
}

void StreamThroughTransmitter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    startTx(packet);
}

void StreamThroughTransmitter::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    delete txSignal;
    txSignal = encodePacket(packet);
    cancelClockEvent(txEndTimer);
    scheduleTxEndTimer(txSignal);
}

void StreamThroughTransmitter::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    delete txSignal;
    txSignal = encodePacket(packet);
    clocktime_t timePosition = getClockTime() - txStartTime;
    b bitPosition = b(std::floor(datarate.get() * timePosition.dbl()));
    sendPacketProgress(txSignal->dup(), bitPosition, timePosition);
    cancelClockEvent(txEndTimer);
    scheduleTxEndTimer(txSignal);
}

b StreamThroughTransmitter::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    if (txSignal == nullptr)
        return b(0);
    clocktime_t transmissionDuration = getClockTime() - txStartTime;
    return b(std::floor(datarate.get() * transmissionDuration.dbl()));
}

} // namespace inet

