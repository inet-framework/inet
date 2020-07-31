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

#include "inet/common/ModuleAccess.h"
#include "inet/common/PacketEventTag.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag.h"
#include "inet/protocol/transceiver/base/PacketTransmitterBase.h"

namespace inet {

PacketTransmitterBase::~PacketTransmitterBase()
{
    cancelAndDelete(txEndTimer);
    txEndTimer = nullptr;
    delete txSignal;
    txSignal = nullptr;
}

void PacketTransmitterBase::initialize(int stage)
{
    ClockUsingModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        dataratePar = &par("datarate");
        datarate = bps(*dataratePar);
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
        txEndTimer = new cMessage("TxEndTimer");
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

void PacketTransmitterBase::sendPacketStart(Signal *signal)
{
    txId = signal->getId();
    send(signal, SendOptions().duration(signal->getDuration()), outputGate);
}

void PacketTransmitterBase::sendPacketProgress(Signal *signal, b bitPosition, clocktime_t timePosition)
{
    simtime_t remainingDuration = signal->getDuration() - CLOCKTIME_AS_SIMTIME(timePosition);
    send(signal, SendOptions().duration(signal->getDuration()).updateTx(txId, remainingDuration), outputGate);
}

void PacketTransmitterBase::sendPacketEnd(Signal *signal)
{
    send(signal, SendOptions().duration(signal->getDuration()).updateTx(txId, 0), outputGate);
}

clocktime_t PacketTransmitterBase::calculateDuration(const Packet *packet) const
{
    return packet->getTotalLength().get() / datarate.get();
}

} // namespace inet

