//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/transceiver/StreamThroughTransmitter.h"

namespace inet {

Define_Module(StreamThroughTransmitter);

void StreamThroughTransmitter::initialize(int stage)
{
    StreamingTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        bufferUnderrunTimer = new cMessage("BufferUnderrunTimer");
}

void StreamThroughTransmitter::handleMessageWhenUp(cMessage *message)
{
    if (message == txEndTimer)
        endTx(check_and_cast<Packet *>(txSignal->decapsulate()));
    else if (message == bufferUnderrunTimer)
        throw cRuntimeError("Buffer underrun during transmission");
    else
        StreamingTransmitterBase::handleMessageWhenUp(message);
    updateDisplayString();
}

void StreamThroughTransmitter::handleStopOperation(LifecycleOperation *operation)
{
    if (isTransmitting())
        abortTx();
}

void StreamThroughTransmitter::handleCrashOperation(LifecycleOperation *operation)
{
    if (isTransmitting())
        abortTx();
}

void StreamThroughTransmitter::startTx(Packet *packet, bps datarate, b position)
{
    // 1. check current state
    ASSERT(!isTransmitting());
    // 2. store input progress
    lastInputDatarate = datarate;
    lastInputProgressTime = simTime();
    lastInputProgressPosition = position;
    // 3. store transmission progress
    txDatarate = bps(*dataratePar);
    txStartTime = simTime();
    txStartClockTime = getClockTime();
    lastTxProgressTime = simTime();
    lastTxProgressPosition = b(0);
    // 4. create signal
    auto signal = encodePacket(packet);
    txSignal = signal->dup();
    // 5. send signal start and notify subscribers
    EV_INFO << "Starting transmission" << EV_FIELD(packet) << EV_FIELD(datarate, txDatarate) << EV_ENDL;
    emit(transmissionStartedSignal, signal);
    sendSignalStart(signal, packet->getTransmissionId());
    // 6. schedule transmission end timer and buffer underrun timer
    scheduleTxEndTimer(txSignal);
    scheduleBufferUnderrunTimer();
}

void StreamThroughTransmitter::progressTx(Packet *packet, bps datarate, b position)
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. store input progress
    b inputProgressPosition = lastInputProgressPosition + b(std::floor((simTime() - lastInputProgressTime).dbl() * lastInputDatarate.get()));
    auto txPacket = check_and_cast<Packet *>(txSignal->getEncapsulatedPacket());
    bool isInputProgressAtEnd = inputProgressPosition == packet->getTotalLength() && packet->getTotalLength() == txPacket->getTotalLength();
    bool isPacketUnchangedSinceLastProgress = isInputProgressAtEnd || packet->peekAll()->containsSameData(*txPacket->peekAll().get());
    lastInputDatarate = datarate;
    lastInputProgressTime = simTime();
    lastInputProgressPosition = position;
    // 3. store transmission progress
    clocktime_t timePosition = getClockTime() - txStartClockTime;
    lastTxProgressTime = simTime();
    lastTxProgressPosition = b(std::floor(txDatarate.get() * timePosition.dbl()));
    if (isPacketUnchangedSinceLastProgress)
        delete packet;
    else {
        // 4. create progress signal
        auto signal = encodePacket(packet);
        delete txSignal;
        txSignal = signal->dup();
        // 5. send signal progress
        EV_INFO << "Progressing transmission" << EV_FIELD(packet) << EV_FIELD(datarate, txDatarate) << EV_ENDL;
        sendSignalProgress(signal, packet->getTransmissionId(), lastTxProgressPosition, timePosition);
    }
    // 6. reschedule timers
    scheduleTxEndTimer(txSignal);
    scheduleBufferUnderrunTimer();
}

void StreamThroughTransmitter::endTx(Packet *packet)
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. send signal end to receiver and notify subscribers
    EV_INFO << "Ending transmission" << EV_FIELD(packet) << EV_FIELD(datarate, txDatarate) << EV_ENDL;
    handlePacketProcessed(packet);
    auto signal = encodePacket(packet);
    delete txSignal;
    txSignal = signal->dup();
    emit(transmissionEndedSignal, txSignal);
    sendSignalEnd(txSignal, packet->getTransmissionId());
    // 3. clear internal state
    txSignal = nullptr;
    txDatarate = bps(NaN);
    txStartTime = -1;
    txStartClockTime = -1;
    lastTxProgressTime = -1;
    lastTxProgressPosition = b(-1);
    lastInputDatarate = bps(NaN);
    lastInputProgressTime = -1;
    lastInputProgressPosition = b(-1);
    // 4. notify producer
    auto gate = inputGate->getPathStartGate();
    if (producer != nullptr) {
        producer->handlePushPacketProcessed(packet, gate, true);
        producer->handleCanPushPacketChanged(gate);
    }
    delete signal;
}

void StreamThroughTransmitter::abortTx()
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. create new truncated signal
    auto packet = check_and_cast<Packet *>(txSignal->decapsulate());
    // TODO we can't just simply cut the packet proportionally with time because it's not always the case (modulation, scrambling, etc.)
    simtime_t timePosition = simTime() - txStartTime;
    b dataPosition = b(std::floor(txDatarate.get() * timePosition.dbl()));
    packet->eraseAtBack(packet->getTotalLength() - dataPosition);
    packet->setBitError(true);
    auto signal = encodePacket(packet);
    signal->setDuration(timePosition);
    // 3. delete old signal
    delete txSignal;
    txSignal = nullptr;
    // 4. send signal end to receiver and notify subscribers
    EV_INFO << "Aborting transmission" << EV_FIELD(packet) << EV_FIELD(datarate, txDatarate) << EV_ENDL;
    handlePacketProcessed(packet);
    emit(transmissionEndedSignal, signal);
    sendSignalEnd(signal, packet->getTransmissionId());
    // 5. clear internal state
    txDatarate = bps(NaN);
    txStartTime = -1;
    txStartClockTime = -1;
    lastTxProgressTime = -1;
    lastTxProgressPosition = b(-1);
    lastInputDatarate = bps(NaN);
    lastInputProgressTime = -1;
    lastInputProgressPosition = b(-1);
    // 6. notify producer
    auto gate = inputGate->getPathStartGate();
    if (producer != nullptr) {
        producer->handlePushPacketProcessed(packet, gate, true);
        producer->handleCanPushPacketChanged(gate);
    }
}

void StreamThroughTransmitter::scheduleBufferUnderrunTimer()
{
    cancelEvent(bufferUnderrunTimer);
    if (lastInputDatarate < txDatarate) {
        // Underrun occurs when the following two values become equal:
        // inputProgressPosition = lastInputProgressPosition + inputDatarate * (simTime() - lastInputProgressTime)
        // txProgressPosition = lastTxProgressPosition + txDatarate * (simTime() - lastTxProgressTime)
        simtime_t bufferUnderrunTime = s((-lastInputProgressPosition + lastInputDatarate * s(lastInputProgressTime.dbl()) + lastTxProgressPosition - txDatarate * s(lastTxProgressTime.dbl())) / (lastInputDatarate - txDatarate)).get();
        EV_INFO << "Scheduling buffer underrun timer" << EV_FIELD(at, bufferUnderrunTime.ustr()) << EV_ENDL;
        scheduleAt(bufferUnderrunTime, bufferUnderrunTimer);
    }
}

void StreamThroughTransmitter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    startTx(packet, datarate, b(0));
    updateDisplayString();
}

void StreamThroughTransmitter::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    if (txEndTimer->getArrivalTime() == simTime()) {
        cancelClockEvent(txEndTimer);
        endTx(packet);
    }
    else
        progressTx(packet, txDatarate, packet->getDataLength());
    updateDisplayString();
}

void StreamThroughTransmitter::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    if (isTransmitting())
        progressTx(packet, datarate, position);
    else
        startTx(packet, datarate, position);
    updateDisplayString();
}

} // namespace inet

