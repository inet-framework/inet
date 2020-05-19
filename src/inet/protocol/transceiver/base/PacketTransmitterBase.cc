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

void PacketTransmitterBase::initialize(int stage)
{
    ClockUsingModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        producer = findConnectedModule<IActivePacketSource>(inputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(inputGate);
        if (producer != nullptr)
            producer->handleCanPushPacket(inputGate->getPathStartGate());
    }
}

void PacketTransmitterBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

Signal *PacketTransmitterBase::encodePacket(const Packet *txPacket) const
{
    auto packet = txPacket->dup();
    auto duration = calculateDuration(packet);
    auto bitTransmissionTime = CLOCKTIME_AS_SIMTIME(duration / packet->getBitLength());
    auto packetEvent = new PacketTransmittedEvent();
    packetEvent->setDatarate(packet->getTotalLength() / s(duration.dbl()));
    insertPacketEvent(this, packet, PEK_TRANSMITTED, bitTransmissionTime, packetEvent);
    increaseTimeTag<TransmissionTimeTag>(packet, bitTransmissionTime);
    if (auto channel = dynamic_cast<cTransmissionChannel *>(outputGate->findTransmissionChannel())) {
        insertPacketEvent(this, packet, PEK_PROPAGATED, channel->getDelay());
        increaseTimeTag<PropagationTimeTag>(packet, channel->getDelay());
    }
    auto oldPacketProtocolTag = packet->removeTagIfPresent<PacketProtocolTag>();
    packet->clearTags();
    if (oldPacketProtocolTag != nullptr) {
        auto newPacketProtocolTag = packet->addTag<PacketProtocolTag>();
        *newPacketProtocolTag = *oldPacketProtocolTag;
        delete oldPacketProtocolTag;
    }
    auto signal = new Signal(packet->getName());
    signal->encapsulate(packet);
    signal->setDuration(CLOCKTIME_AS_SIMTIME(duration));
    return signal;
}

} // namespace inet

