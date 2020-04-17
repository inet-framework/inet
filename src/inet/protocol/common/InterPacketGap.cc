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
        duration = par("duration");
        lastPacketEndTime = par("initialChannelBusy") ? 0 : -duration;
    }
}

void InterPacketGap::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        Packet *packet;
        if (message->isPacket()) {
            packet = check_and_cast<Packet *>(message);
            pushOrSendPacket(packet, outputGate, consumer);
            lastPacket = nullptr;
        }
        else if (auto progress = dynamic_cast<cProgress *>(message)) {
            packet = check_and_cast<Packet *>(progress->removePacket());
            receiveProgress(packet, progress->getArrivalGate(), progress->getKind(), progress->getBitPosition(), progress->getTimePosition(), progress->getExtraProcessableBitLength(), progress->getExtraProcessableDuration());
            delete progress;
        }
        else
            throw cRuntimeError("Unknown message");
        numProcessedPackets++;
        processedTotalLength += packet->getTotalLength();
        updateDisplayString();
    }
    else {
        if (message->isPacket()) {
            auto packet = check_and_cast<Packet *>(message);
            pushPacket(packet, packet->getArrivalGate());
        }
        else if (auto progress = dynamic_cast<cProgress *>(message)) {
            throw cRuntimeError("module error: obsolete code");
            auto now = simTime();
            auto packet = check_and_cast<Packet *>(progress->getPacket());
            if (packet == lastPacket) {     //FIXME
                lastPacketEndTime = now + lastDelay + packet->getDuration() - progress->getTimePosition();
                if (lastDelay == 0)
                    receiveProgress(progress->removePacket(), progress->getArrivalGate(), progress->getKind(), progress->getBitPosition(), progress->getTimePosition(), progress->getExtraProcessableBitLength(), progress->getExtraProcessableDuration());
                else
                    scheduleAt(simTime() + lastDelay, message);
            }
            else {
                lastPacket = packet;
                lastDelay = lastPacketEndTime + duration - simTime() + progress->getTimePosition();
                if (lastDelay < 0)
                    lastDelay = 0;
                lastPacketEndTime = now + lastDelay + packet->getDuration() - progress->getTimePosition();
                if (lastDelay == 0)
                    receiveProgress(progress->removePacket(), progress->getArrivalGate(), progress->getKind(), progress->getBitPosition(), progress->getTimePosition(), progress->getExtraProcessableBitLength(), progress->getExtraProcessableDuration());
                else {
                    EV_INFO << "Inserting packet gap before " << packet->getName() << "." << endl;
                    scheduleAt(now + lastDelay, message);
                }
            }
            numProcessedPackets++;
            processedTotalLength += packet->getTotalLength();
            updateDisplayString();
        }
        else
            throw cRuntimeError("Unknown message");
    }
}

void InterPacketGap::receivePacketStart(cPacket *cpacket)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    animateSend(packet, outputGate);
    pushOrSendPacketStart(packet, outputGate->getPathEndGate(), consumer);
}

void InterPacketGap::receivePacketProgress(cPacket *cpacket, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    animateSend(packet, outputGate);
    pushOrSendPacketProgress(packet, outputGate->getPathEndGate(), consumer, b(bitPosition), b(extraProcessableBitLength));
    if (bitPosition == packet->getBitLength())
        lastPacket = nullptr;
}

void InterPacketGap::receivePacketEnd(cPacket *cpacket)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    animateSend(packet, outputGate);
    pushOrSendPacketEnd(packet, outputGate->getPathEndGate(), consumer);
    lastPacket = nullptr;
}

bool InterPacketGap::canPushSomePacket(cGate *gate) const
{
    return simTime() >= lastPacketEndTime &&
           (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate()));
}

bool InterPacketGap::canPushPacket(Packet *packet, cGate *gate) const
{
    return simTime() >= lastPacketEndTime &&
           (consumer == nullptr || consumer->canPushPacket(packet, outputGate->getPathEndGate()));
}

void InterPacketGap::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    packet->setArrival(getId(), inputGate->getId(), simTime());
    auto now = simTime();
    lastPacket = packet;
    lastDelay = lastPacketEndTime + duration - now;
    if (lastDelay < 0)
        lastDelay = 0;
    lastPacketEndTime = now + lastDelay + packet->getDuration();
    if (lastDelay == 0) {
        pushOrSendPacket(packet, outputGate, consumer);
        lastPacket = nullptr;
    }
    else {
        EV_INFO << "Inserting packet gap before " << packet->getName() << "." << endl;
        scheduleAt(now + lastDelay, packet);
    }
}

void InterPacketGap::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producer != nullptr)
        producer->handleCanPushPacket(inputGate->getPathStartGate());
}

void InterPacketGap::pushPacketStart(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketStart");
    auto now = simTime();
    lastPacket = packet;
    lastDelay = lastPacketEndTime + duration - now;
    if (lastDelay < 0)
        lastDelay = 0;
    lastPacketEndTime = now + lastDelay + packet->getDuration();
    if (lastDelay == 0) {
        animateSend(packet, outputGate);
        pushOrSendPacketStart(packet, outputGate->getPathEndGate(), consumer);
    }
    else
        sendPacketStart(packet, nullptr, 0, lastDelay);
}

void InterPacketGap::pushPacketProgress(Packet *packet, cGate *gate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    animateSend(packet, outputGate);
    pushOrSendPacketProgress(packet, outputGate->getPathEndGate(), consumer, position, extraProcessableLength);
}

void InterPacketGap::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    animateSend(packet, outputGate);
    pushOrSendPacketEnd(packet, outputGate->getPathEndGate(), consumer);
}

b InterPacketGap::getPushPacketProcessedLength(Packet *packet, cGate *gate)
{
    ASSERT(consumer != nullptr);
    return consumer->getPushPacketProcessedLength(packet, outputGate->getPathEndGate());
}

void InterPacketGap::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    lastPacketEndTime = simTime();
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

} // namespace inet

