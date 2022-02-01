//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    // 2. store transmission progress
    txDatarate = bps(*dataratePar);
    txStartTime = simTime();
    txStartClockTime = getClockTime();
    // 3. create signal
    auto signal = encodePacket(packet);
    txSignal = signal->dup();
    // 4. send signal start and notify subscribers
    emit(transmissionStartedSignal, signal);
    prepareSignal(signal);
    EV_INFO << "Transmitting signal to channel" << EV_FIELD(signal) << EV_ENDL;
    send(signal, SendOptions().duration(signal->getDuration()), outputGate);
    // 5. schedule transmission end timer
    scheduleTxEndTimer(txSignal);
}

void PacketTransmitter::endTx()
{
    // 1. check current state
    ASSERT(isTransmitting());
    // 2. notify subscribers
    emit(transmissionEndedSignal, txSignal);
    auto packet = check_and_cast<Packet *>(txSignal->decapsulate());
    handlePacketProcessed(packet);
    // 3. clear internal state
    delete txSignal;
    txSignal = nullptr;
    txStartTime = -1;
    txStartClockTime = -1;
    // 4. notify producer
    if (producer != nullptr) {
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), true);
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
    delete packet;
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleClockEventAfter(SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

} // namespace inet

