//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/gate/CreditBasedGate.h"

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/networklayer/common/NetworkInterface.h"
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/common/Signal.h"
#endif
#include "inet/queueing/gate/CreditGateTag_m.h"

namespace inet {
namespace queueing {

Define_Module(CreditBasedGate);

simsignal_t CreditBasedGate::creditsChangedSignal = cComponent::registerSignal("creditsChanged");

void CreditBasedGate::initialize(int stage)
{
    PacketGateBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        idleCreditGainRate = par("idleCreditGainRate");
        transmitCreditSpendRate = par("transmitCreditSpendRate");
        transmitCreditLimit = par("transmitCreditLimit");
        minCredit = par("minCredit");
        maxCredit = par("maxCredit");
        displayStringTextFormat = par("displayStringTextFormat");
        currentCredit = par("initialCredit");
        currentCreditGainRate = idleCreditGainRate;
        lastCurrentCreditEmitted = currentCredit;
        lastCurrentCreditEmittedTime = simTime();
        isOpen_ = currentCredit >= transmitCreditLimit;
        cModule *module = getContainingNicModule(this);
        module->subscribe(transmissionStartedSignal, this);
        module->subscribe(transmissionEndedSignal, this);
        module->subscribe(interpacketGapEndedSignal, this);
        changeTimer = new cMessage("ChangeTimer");
        WATCH(currentCredit);
        WATCH(currentCreditGainRate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        updateCurrentCredit();
        updateCurrentCreditGainRate();
        emitCurrentCredit();
        scheduleChangeTimer();
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void CreditBasedGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        // 1. timer is executed when currentCredit reaches transmitCreditLimit with currentCreditGainRate
        currentCredit = transmitCreditLimit;
        // 2. notify listeners and update lastCurrentCreditEmitted
        emitCurrentCredit();
        // 3. open/close gate and allow consumer to pull packet if necessary
        processChangeTimer();
        // 4. update currentCreditGainRate to know the slope when the timer is rescheduled
        updateCurrentCreditGainRate();
        // 5. reschedule change timer based on currentCredit and currentCreditGainRate
        scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void CreditBasedGate::finish()
{
    updateCurrentCredit();
    updateCurrentCreditGainRate();
    emitCurrentCredit();
}

void CreditBasedGate::refreshDisplay() const
{
    // NOTE: don't emit current credit and no need to call updateCurrentCreditGainRate
    const_cast<CreditBasedGate *>(this)->updateCurrentCredit();
    updateDisplayString();
}

void CreditBasedGate::scheduleChangeTimer()
{
    ASSERT(lastCurrentCreditEmitted == currentCredit);
    ASSERT(lastCurrentCreditEmittedTime == simTime());
    if (currentCreditGainRate == 0)
        cancelEvent(changeTimer);
    else {
        double changeTime = (transmitCreditLimit - currentCredit) / currentCreditGainRate;
        if (changeTime < 0)
            cancelEvent(changeTimer);
        else if (changeTime > 0 || (isOpen_ ? currentCreditGainRate < 0 : currentCreditGainRate > 0))
            // NOTE: schedule for future or for now if credit change direction and gate state requires
            rescheduleAfter(changeTime, changeTimer);
    }
}

void CreditBasedGate::processPacket(Packet *packet)
{
    PacketGateBase::processPacket(packet);
    packet->addTag<CreditGateTag>()->setId(getId());
}

void CreditBasedGate::processChangeTimer()
{
    if (isOpen_)
        close();
    else
        open();
}

void CreditBasedGate::updateCurrentCredit()
{
    currentCredit = lastCurrentCreditEmitted + currentCreditGainRate * (simTime() - lastCurrentCreditEmittedTime).dbl();
    currentCredit = std::max(minCredit, std::min(maxCredit, currentCredit));
}

void CreditBasedGate::emitCurrentCredit()
{
    simtime_t now = simTime();
    if (!initialized() || lastCurrentCreditEmitted != currentCredit || lastCurrentCreditEmittedTime != now) {
        emit(creditsChangedSignal, currentCredit);
        lastCurrentCreditEmitted = currentCredit;
        lastCurrentCreditEmittedTime = now;
    }
}

void CreditBasedGate::updateCurrentCreditGainRate()
{
    if (isTransmitting || isInterpacketGap)
        currentCreditGainRate = -transmitCreditSpendRate;
    else if (currentCredit < 0 || hasAvailablePacket())
        currentCreditGainRate = idleCreditGainRate;
    else
        currentCreditGainRate = 0;
}

void CreditBasedGate::receiveSignal(cComponent *source, simsignal_t simsignal, double value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simsignal));
    if (simsignal == interpacketGapEndedSignal) {
        if (isInterpacketGap) {
            isInterpacketGap = false;
            // 1. immediately set currentCredit to 0 if there are no packets to transmit
            if (!hasAvailablePacket())
                currentCredit = std::min(transmitCreditLimit, currentCredit);
            // 2. update currentCreditGainRate and notify listeners about currentCredit change
            updateCurrentCredit();
            updateCurrentCreditGainRate();
            emitCurrentCredit();
            // 3. reschedule change timer when currentCredit reaches transmitCreditLimit
            scheduleChangeTimer();
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

void CreditBasedGate::receiveSignal(cComponent *source, simsignal_t simsignal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simsignal));
    if (simsignal == transmissionStartedSignal || simsignal == transmissionEndedSignal) {
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
        auto signal = check_and_cast<physicallayer::Signal *>(object);
        auto packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
        auto creditGateTag = packet->findTag<CreditGateTag>();
        if (creditGateTag != nullptr && creditGateTag->getId() == getId()) {
            // 1. update currentCredit and currentCreditGainRate because some time may have elapsed
            updateCurrentCredit();
            updateCurrentCreditGainRate();
            emitCurrentCredit();
            // 2. update transmitting state
            if (simsignal == transmissionStartedSignal)
                isTransmitting = true;
            else if (simsignal == transmissionEndedSignal) {
                isTransmitting = false;
                isInterpacketGap = true;
            }
            else
                throw cRuntimeError("Unknown signal");
            // 3. update currentCreditGainRate and notify listeners about currentCredit change
            updateCurrentCredit();
            updateCurrentCreditGainRate();
            emitCurrentCredit();
            // 4. reschedule change timer when currentCredit reaches transmitCreditLimit
            scheduleChangeTimer();
        }
#endif
    }
    else
        throw cRuntimeError("Unknown signal");
}

void CreditBasedGate::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    // 1. update currentCredit and currentCreditGainRate
    updateCurrentCredit();
    updateCurrentCreditGainRate();
    // 2. notify listeners about currentCredit change
    emitCurrentCredit();
    // 3. reschedule change timer when currentCredit reaches transmitCreditLimit
    scheduleChangeTimer();
    PacketGateBase::handleCanPullPacketChanged(gate);
}

const char *CreditBasedGate::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'n': {
            std::stringstream stream;
            stream << currentCredit;
            result = stream.str();
            break;
        }
        default:
            return PacketGateBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace queueing
} // namespace inet

