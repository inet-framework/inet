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
#include "inet/protocol/common/InterPacketGap.h"

namespace inet {

Define_Module(InterPacketGap);

void InterPacketGap::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        durationPar = &par("duration");
        packetEndTime = par("initialChannelBusy") ? simTime() : SimTime().setRaw(INT64_MIN);
        progress = new cProgress(nullptr, cProgress::PACKET_START);
        WATCH(packetStartTime);
        WATCH(packetEndTime);
    }
}

void InterPacketGap::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        if (message->isPacket()) {
            auto packet = check_and_cast<Packet *>(message);
            pushOrSendPacket(packet, outputGate, consumer);
            handlePacketProcessed(packet);
        }
        else if (auto progress = dynamic_cast<cProgress *>(message)) {
            auto packet = check_and_cast<Packet *>(progress->removePacket());
            receiveProgress(packet, progress->getArrivalGate(), progress->getKind(), progress->getDatarate(), progress->getBitPosition(), progress->getTimePosition(), progress->getExtraProcessableBitLength(), progress->getExtraProcessableDuration());
            handlePacketProcessed(packet);
        }
        else
            throw cRuntimeError("Unknown message");
        updateDisplayString();
    }
    else {
        // if an asynchronous message is received from the input gate
        // then it's processed as if it were pushed as synchronous message
        if (message->isPacket()) {
            auto packet = check_and_cast<Packet *>(message);
            pushPacket(packet, packet->getArrivalGate());
        }
        else if (auto progress = dynamic_cast<cProgress *>(message)) {
            auto packet = check_and_cast<Packet *>(progress->removePacket());
            pushProgress(packet, progress->getArrivalGate(), this, message->getKind(), bps(progress->getDatarate()), b(progress->getBitPosition()), b(progress->getExtraProcessableBitLength()));
            delete progress;
        }
        else
            throw cRuntimeError("Unknown message");
    }
}

void InterPacketGap::receivePacketStart(cPacket *cpacket, cGate *gate, double datarate)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    pushOrSendPacketStart(packet, outputGate, consumer, bps(datarate));
}

void InterPacketGap::receivePacketProgress(cPacket *cpacket, cGate *gate, double datarate, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    pushOrSendPacketProgress(packet, outputGate, consumer, bps(datarate), b(bitPosition), b(extraProcessableBitLength));
}

void InterPacketGap::receivePacketEnd(cPacket *cpacket, cGate *gate, double datarate)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    pushOrSendPacketEnd(packet, outputGate, consumer, bps(datarate));
}

bool InterPacketGap::canPushSomePacket(cGate *gate) const
{
    return simTime() >= packetEndTime &&
           (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate()));
}

bool InterPacketGap::canPushPacket(Packet *packet, cGate *gate) const
{
    return simTime() >= packetEndTime &&
           (consumer == nullptr || consumer->canPushPacket(packet, outputGate->getPathEndGate()));
}

void InterPacketGap::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto now = simTime();
    packetDelay = packetEndTime + *durationPar - now;
    if (packetDelay < 0)
        packetDelay = 0;
    packetStartTime = now + packetDelay;
    packetEndTime = packetStartTime + packet->getDuration();
    if (packetDelay == 0)
        pushOrSendPacket(packet, outputGate, consumer);
    else {
        EV_INFO << "Inserting packet gap before " << packet->getName() << "." << endl;
        scheduleAt(now + packetDelay, packet);
    }
}

void InterPacketGap::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void InterPacketGap::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    pushOrSendOrScheduleProgress(packet, gate, cProgress::PACKET_START, datarate, b(0), b(0));
}

void InterPacketGap::pushPacketEnd(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    pushOrSendOrScheduleProgress(packet, gate, cProgress::PACKET_END, datarate, packet->getDataLength(), b(0));
}

void InterPacketGap::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    pushOrSendOrScheduleProgress(packet, gate, cProgress::PACKET_PROGRESS, datarate, position, extraProcessableLength);
}

void InterPacketGap::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    packetEndTime = simTime();
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

b InterPacketGap::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    ASSERT(consumer != nullptr);
    return consumer->getPushPacketProcessedLength(packet, outputGate->getPathEndGate());
}

void InterPacketGap::pushOrSendOrScheduleProgress(Packet *packet, cGate *gate, int progressKind, bps datarate, b position, b extraProcessableLength)
{
    auto now = simTime();
    if (now >= packetEndTime) {
        packetDelay = packetEndTime + *durationPar - now;
        if (packetDelay < 0)
            packetDelay = 0;
        packetStartTime = now + packetDelay;
    }
    packetEndTime = packetStartTime + packet->getDuration();
    if (!progress->isScheduled())
        pushOrSendProgress(packet, outputGate, consumer, progressKind, datarate, position, extraProcessableLength);
    else {
        EV_INFO << "Inserting packet gap before " << packet->getName() << "." << endl;
        cancelEvent(progress);
        progress->setKind(progressKind);
        progress->setPacket(packet);
        progress->setDatarate(bps(datarate).get());
        progress->setBitPosition(b(position).get());
        progress->setExtraProcessableBitLength(b(extraProcessableLength).get());
        scheduleAt(packetStartTime, progress);
    }
}

} // namespace inet

