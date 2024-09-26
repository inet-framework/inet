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

void PacketTransmitter::pushPacket(Packet *packet, const cGate *gate)
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
    // 2. clear internal state
    auto signal = txSignal;
    txSignal = nullptr;
    txStartTime = -1;
    txStartClockTime = -1;
    // 3. notify producer
    auto packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
    if (producer != nullptr)
        producer.handlePushPacketProcessed(packet, true);
    // 4. notify subscribers
    emit(transmissionEndedSignal, signal);
    packet = check_and_cast<Packet *>(signal->decapsulate());
    handlePacketProcessed(packet);
    // 5. notify producer
    if (producer != nullptr)
        producer.handleCanPushPacketChanged();
    delete signal;
    delete packet;
}

void PacketTransmitter::scheduleTxEndTimer(Signal *signal)
{
    scheduleClockEventAfter(SIMTIME_AS_CLOCKTIME(signal->getDuration()), txEndTimer);
}

} // namespace inet

