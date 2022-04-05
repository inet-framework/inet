//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/transceiver/base/PacketTransmitterBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/PacketEventTag.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag.h"

namespace inet {

PacketTransmitterBase::~PacketTransmitterBase()
{
    cancelAndDeleteClockEvent(txEndTimer);
    txEndTimer = nullptr;
    delete txSignal;
    txSignal = nullptr;
}

void PacketTransmitterBase::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        dataratePar = &par("datarate");
        txDatarate = bps(*dataratePar);
        inputGate = gate("in");
        outputGate = gate("out");
        producer.reference(inputGate, false);
        txEndTimer = new ClockEvent("TxEndTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
}

void PacketTransmitterBase::finish()
{
    if (auto channel = dynamic_cast<cDatarateChannel *>(outputGate->findTransmissionChannel()))
        recordScalar("propagationTime", channel->getDelay());
}

void PacketTransmitterBase::handleMessageWhenUp(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketTransmitterBase::handleStartOperation(LifecycleOperation *operation)
{
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

Signal *PacketTransmitterBase::encodePacket(Packet *packet)
{
    txDurationClockTime = calculateClockTimeDuration(packet);
    // TODO: this is just a weak approximation which ignores the past and future drift and drift rate changes of the clock
    simtime_t packetTransmissionTime = CLOCKTIME_AS_SIMTIME(txDurationClockTime);
    simtime_t bitTransmissionTime = packet->getBitLength() != 0 ? CLOCKTIME_AS_SIMTIME(txDurationClockTime / packet->getBitLength()) : 0;
    auto packetEvent = new PacketTransmittedEvent();
    packetEvent->setDatarate(packet->getTotalLength() / s(txDurationClockTime.dbl()));
    insertPacketEvent(this, packet, PEK_TRANSMITTED, bitTransmissionTime, packetEvent);
    increaseTimeTag<TransmissionTimeTag>(packet, bitTransmissionTime, packetTransmissionTime);
    if (auto channel = dynamic_cast<cDatarateChannel *>(outputGate->findTransmissionChannel())) {
        insertPacketEvent(this, packet, PEK_PROPAGATED, channel->getDelay());
        increaseTimeTag<PropagationTimeTag>(packet, channel->getDelay(), channel->getDelay());
    }
    auto signal = new Signal(packet->getName());
    signal->encapsulate(packet);
    signal->setDuration(calculateDuration(txDurationClockTime));
    return signal;
}

void PacketTransmitterBase::prepareSignal(Signal *signal)
{
    auto packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
    auto oldPacketProtocolTag = packet->removeTagIfPresent<PacketProtocolTag>();
    packet->clearTags();
    if (oldPacketProtocolTag != nullptr) {
        auto newPacketProtocolTag = packet->addTag<PacketProtocolTag>();
        *newPacketProtocolTag = *oldPacketProtocolTag;
    }
}

void PacketTransmitterBase::sendSignalStart(Signal *signal, int transmissionId)
{
    EV_INFO << "Transmitting signal start to channel" << EV_FIELD(signal) << EV_ENDL;
    prepareSignal(signal);
    send(signal, SendOptions().duration(signal->getDuration()).transmissionId(transmissionId), outputGate);
}

void PacketTransmitterBase::sendSignalProgress(Signal *signal, int transmissionId, b bitPosition, clocktime_t timePosition)
{
    simtime_t remainingDuration = signal->getDuration() - CLOCKTIME_AS_SIMTIME(timePosition);
    EV_INFO << "Transmitting signal progress to channel" << EV_FIELD(signal) << EV_FIELD(transmissionId) << EV_FIELD(remainingDuration, simsec(remainingDuration)) << EV_ENDL;
    prepareSignal(signal);
    send(signal, SendOptions().duration(signal->getDuration()).updateTx(transmissionId, remainingDuration), outputGate);
}

void PacketTransmitterBase::sendSignalEnd(Signal *signal, int transmissionId)
{
    EV_INFO << "Transmitting signal end to channel" << EV_FIELD(signal) << EV_FIELD(transmissionId) << EV_ENDL;
    prepareSignal(signal);
    send(signal, SendOptions().duration(signal->getDuration()).finishTx(transmissionId), outputGate);
}

clocktime_t PacketTransmitterBase::calculateClockTimeDuration(const Packet *packet) const
{
    s duration = packet->getTotalLength() / txDatarate;
    EV_TRACE << "Calculating signal duration" << EV_FIELD(packet) << EV_FIELD(duration, simsec(duration)) << EV_ENDL;
    return duration.get();
}

simtime_t PacketTransmitterBase::calculateDuration(clocktime_t clockTimeDuration) const
{
    return computeSimTimeFromClockTime(txStartClockTime + clockTimeDuration) - txStartTime;
}

} // namespace inet

