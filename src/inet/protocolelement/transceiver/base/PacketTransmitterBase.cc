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

Signal *PacketTransmitterBase::encodePacket(Packet *packet) const
{
    auto duration = calculateDuration(packet);
    auto bitTransmissionTime = CLOCKTIME_AS_SIMTIME(duration / packet->getBitLength());
    auto packetEvent = new PacketTransmittedEvent();
    packetEvent->setDatarate(packet->getTotalLength() / s(duration.dbl()));
    insertPacketEvent(this, packet, PEK_TRANSMITTED, bitTransmissionTime, packetEvent);
    increaseTimeTag<TransmissionTimeTag>(packet, bitTransmissionTime);
    if (auto channel = dynamic_cast<cDatarateChannel *>(outputGate->findTransmissionChannel())) {
        insertPacketEvent(this, packet, PEK_PROPAGATED, channel->getDelay());
        increaseTimeTag<PropagationTimeTag>(packet, channel->getDelay());
    }
    auto oldPacketProtocolTag = packet->removeTagIfPresent<PacketProtocolTag>();
    packet->clearTags();
    if (oldPacketProtocolTag != nullptr) {
        auto newPacketProtocolTag = packet->addTag<PacketProtocolTag>();
        *newPacketProtocolTag = *oldPacketProtocolTag;
    }
    auto signal = new Signal(packet->getName());
    signal->encapsulate(packet);
    signal->setDuration(CLOCKTIME_AS_SIMTIME(duration));
    return signal;
}

void PacketTransmitterBase::sendSignalStart(Signal *signal, int transmissionId)
{
    EV_INFO << "Sending signal start to channel" << EV_FIELD(signal) << EV_ENDL;
    send(signal, SendOptions().duration(signal->getDuration()).transmissionId(transmissionId), outputGate);
}

void PacketTransmitterBase::sendSignalProgress(Signal *signal, int transmissionId, b bitPosition, clocktime_t timePosition)
{
    simtime_t remainingDuration = signal->getDuration() - CLOCKTIME_AS_SIMTIME(timePosition);
    EV_INFO << "Sending signal progress to channel" << EV_FIELD(signal) << EV_FIELD(transmissionId) << EV_FIELD(remainingDuration, simsec(remainingDuration)) << EV_ENDL;
    send(signal, SendOptions().duration(signal->getDuration()).updateTx(transmissionId, remainingDuration), outputGate);
}

void PacketTransmitterBase::sendSignalEnd(Signal *signal, int transmissionId)
{
    EV_INFO << "Sending signal end to channel" << EV_FIELD(signal) << EV_FIELD(transmissionId) << EV_ENDL;
    send(signal, SendOptions().duration(signal->getDuration()).finishTx(transmissionId), outputGate);
}

clocktime_t PacketTransmitterBase::calculateDuration(const Packet *packet) const
{
    s duration = packet->getTotalLength() / txDatarate;
    EV_TRACE << "Calculating signal duration" << EV_FIELD(packet) << EV_FIELD(duration, simsec(duration)) << EV_ENDL;
    return duration.get();
}

} // namespace inet

