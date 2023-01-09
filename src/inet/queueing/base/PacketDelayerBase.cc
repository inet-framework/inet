//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketDelayerBase.h"

#include "inet/common/PacketEventTag.h"
#include "inet/common/TimeTag.h"

namespace inet {
namespace queueing {

void PacketDelayerBase::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        schedulingPriority = par("schedulingPriority");
        scheduleZeroDelay = par("scheduleZeroDelay");
    }
}

void PacketDelayerBase::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        bool isPacket = message->isPacket();
        auto packet = isPacket ? check_and_cast<Packet *>(message) : static_cast<Packet *>(message->getContextPointer());
        if (!isPacket)
            delete message;
        processPacket(packet, message->getSendingTime());
    }
    else
        PacketPusherBase::handleMessage(message);
}

void PacketDelayerBase::processPacket(Packet *packet, simtime_t sendingTime)
{
    simtime_t delay = simTime() - sendingTime;
    insertPacketEvent(this, packet, PEK_DELAYED, delay / packet->getBitLength());
    increaseTimeTag<DelayingTimeTag>(packet, delay / packet->getBitLength(), delay);
    pushOrSendPacket(packet, outputGate, consumer);
}

cGate *PacketDelayerBase::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void PacketDelayerBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
#ifdef INET_WITH_CLOCK
    if (clock != nullptr) {
        clocktime_t delay = computeDelay(packet);
        EV_INFO << "Delaying packet" << EV_FIELD(delay) << EV_FIELD(packet) << EV_ENDL;
        auto clockEvent = new ClockEvent("DelayTimer");
        clockEvent->setContextPointer(packet);
        clockEvent->setSchedulingPriority(schedulingPriority);
        if (delay != 0 || scheduleZeroDelay)
            scheduleClockEventAfter(delay, clockEvent);
        else
            processPacket(packet, simTime());
    }
    else {
#else
    {
#endif
        simtime_t delay = CLOCKTIME_AS_SIMTIME(computeDelay(packet));
        EV_INFO << "Delaying packet" << EV_FIELD(delay) << EV_FIELD(packet) << EV_ENDL;
        if (delay != 0 || scheduleZeroDelay)
            scheduleAfter(delay, packet);
        else
            processPacket(packet, simTime());
    }
    handlePacketProcessed(packet);
    updateDisplayString();
}

void PacketDelayerBase::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void PacketDelayerBase::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producer != nullptr)
        producer->handlePushPacketProcessed(packet, gate, successful);
}

} // namespace queueing
} // namespace inet

