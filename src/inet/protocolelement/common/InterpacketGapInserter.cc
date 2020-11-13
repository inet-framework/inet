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

#include "inet/protocolelement/common/InterpacketGapInserter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/common/ProgressTag_m.h"

namespace inet {

Define_Module(InterpacketGapInserter);

// TODO: review streaming operation with respect to holding back packet push until the packet gap elapses

InterpacketGapInserter::~InterpacketGapInserter()
{
    cancelAndDelete(timer);
    cancelAndDeleteClockEvent(progress);
}

void InterpacketGapInserter::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        durationPar = &par("duration");
        packetEndTime = par("initialChannelBusy") ? getClockTime() : getClockTime().setRaw(INT64_MIN / 2); // INT64_MIN / 2 to prevent overflow
        timer = new ClockEvent("IfgTimer");
        progress = new ClockEvent("ProgressTimer");
        WATCH(packetStartTime);
        WATCH(packetEndTime);
    }
    else if (stage == INITSTAGE_LAST) {
        if (packetEndTime + durationPar->doubleValue() > getClockTime())
            scheduleClockEventAt(packetEndTime + durationPar->doubleValue(), timer);
    }
}

void InterpacketGapInserter::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        if (message == timer) {
            if (canPushSomePacket(inputGate))
                if (producer != nullptr)
                    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
        }
        else if (message == progress) {
            auto packet = static_cast<Packet *>(message->getContextPointer());
            if (packet->isUpdate()) {
                auto progressTag = packet->getTag<ProgressTag>();
                pushOrSendPacketProgress(packet, outputGate, consumer, progressTag->getDatarate(), progressTag->getPosition(), progressTag->getExtraProcessableLength(), packet->getTransmissionId());
            }
            else
                pushOrSendPacket(packet, outputGate, consumer);
            handlePacketProcessed(packet);
        }
        else
            throw cRuntimeError("Unknown message");
    }
    else {
        // if an asynchronous message is received from the input gate
        // then it's processed as if it were pushed as synchronous message
        if (message->isPacket()) {
            auto packet = check_and_cast<Packet *>(message);
            if (packet->isUpdate()) {
                auto progressTag = packet->getTag<ProgressTag>();
                pushOrSendPacketProgress(packet, outputGate, consumer, progressTag->getDatarate(), progressTag->getPosition(), progressTag->getExtraProcessableLength(), packet->getTransmissionId());
            }
            else
                pushPacket(packet, packet->getArrivalGate());
            handlePacketProcessed(packet);
        }
        else
            throw cRuntimeError("Unknown message");
    }
    updateDisplayString();
}

void InterpacketGapInserter::receivePacketStart(cPacket *cpacket, cGate *gate, double datarate)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    pushOrSendPacketStart(packet, outputGate, consumer, bps(datarate), packet->getTransmissionId());
}

void InterpacketGapInserter::receivePacketProgress(cPacket *cpacket, cGate *gate, double datarate, int bitPosition, simtime_t timePosition, int extraProcessableBitLength, simtime_t extraProcessableDuration)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    pushOrSendPacketProgress(packet, outputGate, consumer, bps(datarate), b(bitPosition), b(extraProcessableBitLength), packet->getTransmissionId());
}

void InterpacketGapInserter::receivePacketEnd(cPacket *cpacket, cGate *gate, double datarate)
{
    auto packet = check_and_cast<Packet *>(cpacket);
    pushOrSendPacketEnd(packet, outputGate, consumer, packet->getTransmissionId());
}

bool InterpacketGapInserter::canPushSomePacket(cGate *gate) const
{
    // TODO: getting a value from the durationPar here is wrong, because it's volatile and this method can be called any number of times
    return (getClockTime() >= packetEndTime + durationPar->doubleValue()) &&
           (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate()));
}

bool InterpacketGapInserter::canPushPacket(Packet *packet, cGate *gate) const
{
    // TODO: getting a value from the durationPar here is wrong, because it's volatile and this method can be called any number of times
    return (getClockTime() >= packetEndTime + durationPar->doubleValue()) &&
           (consumer == nullptr || consumer->canPushPacket(packet, outputGate->getPathEndGate()));
}

void InterpacketGapInserter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto now = getClockTime();
    packetDelay = packetEndTime + durationPar->doubleValue() - now;
    if (packetDelay < 0)
        packetDelay = 0;
    packetStartTime = now + packetDelay;
    packetEndTime = packetStartTime + SIMTIME_AS_CLOCKTIME(packet->getDuration());
    if (packetDelay == 0) {
        handlePacketProcessed(packet);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else {
        EV_INFO << "Inserting packet gap before" << EV_FIELD(packet) << EV_ENDL;
        progress->setContextPointer(packet);
        scheduleClockEventAt(now + packetDelay, progress);
    }
    updateDisplayString();
}

void InterpacketGapInserter::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (packetEndTime + durationPar->doubleValue() <= getClockTime()) {
        if (producer != nullptr)
            producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    }
    else {
        if (timer->isScheduled())
            cancelClockEvent(timer);
        scheduleClockEventAt(packetEndTime + durationPar->doubleValue(), timer);
    }
}

void InterpacketGapInserter::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    pushOrSendOrSchedulePacketProgress(packet, gate, datarate, b(0), b(0));
    updateDisplayString();
}

void InterpacketGapInserter::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    pushOrSendOrSchedulePacketProgress(packet, gate, bps(NaN), packet->getDataLength(), b(0));
    updateDisplayString();
}

void InterpacketGapInserter::pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    Enter_Method("pushPacketProgress");
    take(packet);
    pushOrSendOrSchedulePacketProgress(packet, gate, datarate, position, extraProcessableLength);
    updateDisplayString();
}

void InterpacketGapInserter::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    packetEndTime = getClockTime();
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, inputGate->getPathStartGate(), successful);
}

void InterpacketGapInserter::pushOrSendOrSchedulePacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength)
{
    auto now = getClockTime();
    if (now >= packetEndTime) {
        packetDelay = packetEndTime + durationPar->doubleValue() - now;
        if (packetDelay < 0)
            packetDelay = 0;
        packetStartTime = now + packetDelay;
    }
    packetEndTime = packetStartTime + SIMTIME_AS_CLOCKTIME(packet->getDuration());
    if (progress == nullptr || !progress->isScheduled()) {
        if (packet->getTotalLength() == position + extraProcessableLength)
            handlePacketProcessed(packet);
        pushOrSendPacketProgress(packet, outputGate, consumer, datarate, position, extraProcessableLength, packet->getTransmissionId());
    }
    else {
        EV_INFO << "Inserting packet gap before" << EV_FIELD(packet) << EV_ENDL;
        cancelClockEvent(progress);
        auto progressTag = packet->addTagIfAbsent<ProgressTag>();
        progressTag->setDatarate(datarate);
        progressTag->setPosition(position);
        progressTag->setExtraProcessableLength(extraProcessableLength);
        progress->setContextPointer(packet);
        scheduleClockEventAt(packetStartTime, progress);
    }
}

const char *InterpacketGapInserter::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'g':
            result = simtime_t(durationPar->doubleValue()).ustr().c_str();
            break;
        case 'd':
            result = CLOCKTIME_AS_SIMTIME(packetDelay).ustr().c_str();
            break;
        default:
            return PacketPusherBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace inet

