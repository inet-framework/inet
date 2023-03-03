//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/transceiver/base/StreamingTransmitterBase.h"

namespace inet {

void StreamingTransmitterBase::initialize(int stage)
{
    PacketTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        transmissionChannel = outputGate->findTransmissionChannel();
        if (transmissionChannel != nullptr) {
            transmissionChannel->subscribe(PRE_MODEL_CHANGE, this);
            transmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        }
        subscribe(PRE_MODEL_CHANGE, this);
        subscribe(POST_MODEL_CHANGE, this);
    }
}

void StreamingTransmitterBase::scheduleAt(simtime_t t, cMessage *message)
{
    if (message == txEndTimer) {
        if (txStartTime + txSignal->getDuration() != t) {
            clocktime_t timePosition = getClockTime() - txStartClockTime;
            b bitPosition = b(std::floor(txDatarate.get() * timePosition.dbl()));
            txSignal->setDuration(t - txStartTime);
            sendSignalProgress(txSignal->dup(), txSignal->getId(), bitPosition, timePosition);
        }
    }
    ClockUserModuleMixin::scheduleAt(t, message);
}

bool StreamingTransmitterBase::canPushSomePacket(cGate *gate) const
{
    return transmissionChannel != nullptr && !transmissionChannel->isDisabled() && PacketTransmitterBase::canPushSomePacket(gate);
}

void StreamingTransmitterBase::scheduleTxEndTimer(Signal *signal)
{
    ASSERT(txStartClockTime != -1);
    clocktime_t txEndClockTime = txStartClockTime + txDurationClockTime;
    EV_INFO << "Scheduling transmission end timer" << EV_FIELD(at, txEndClockTime.ustr()) << EV_ENDL;
    rescheduleClockEventAt(txEndClockTime, txEndTimer);
}

void StreamingTransmitterBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
#if OMNETPP_BUILDNUM < 2001
    if (getSimulation()->getSimulationStage() == STAGE(CLEANUP))
#else
    if (getSimulation()->getStage() == STAGE(INITIALIZE))
#endif
        return;
    if (signal == PRE_MODEL_CHANGE) {
        if (auto notification = dynamic_cast<cPrePathCutNotification *>(object)) {
            if (outputGate == notification->pathStartGate && isTransmitting())
                abortTx();
        }
        else if (auto notification = dynamic_cast<cPreParameterChangeNotification *>(object)) {
            if (notification->par->getOwner() == transmissionChannel &&
                notification->par->getType() == cPar::BOOL && strcmp(notification->par->getName(), "disabled") == 0 &&
                !transmissionChannel->isDisabled() &&
                isTransmitting()) // TODO the new value of parameter currently unavailable: notification->newValue == true
            {
                abortTx();
            }
        }
    }
    else if (signal == POST_MODEL_CHANGE) {
        if (auto notification = dynamic_cast<cPostPathCreateNotification *>(object)) {
            if (outputGate == notification->pathStartGate) {
                transmissionChannel = outputGate->findTransmissionChannel();
                if (transmissionChannel != nullptr && !transmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
                    transmissionChannel->subscribe(POST_MODEL_CHANGE, this);
                if (producer != nullptr)
                    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
            }
        }
        else if (auto notification = dynamic_cast<cPostPathCutNotification *>(object)) {
            if (outputGate == notification->pathStartGate) {
                transmissionChannel = nullptr;
                if (producer != nullptr)
                    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
            }
        }
        else if (auto notification = dynamic_cast<cPostParameterChangeNotification *>(object)) {
            if (producer != nullptr && notification->par->getOwner() == transmissionChannel && notification->par->getType() == cPar::BOOL && strcmp(notification->par->getName(), "disabled") == 0)
                producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
        }
    }
}

} // namespace inet

