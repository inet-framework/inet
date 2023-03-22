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
        accumulateCreditInGuardBand = par("accumulateCreditInGuardBand");
        setCurrentCredit(par("initialCredit"));
        setCurrentCreditGainRate(idleCreditGainRate);
        lastCurrentCreditEmitted = currentCredit;
        lastCurrentCreditEmittedTime = simTime();
        isOpen_ = currentCredit >= transmitCreditLimit;
        periodicGate.reference(outputGate, false);
        if (periodicGate != nullptr) {
            auto periodicGateModule = check_and_cast<cModule *>(periodicGate.get());
            periodicGateModule->subscribe(gateStateChangedSignal, this);
            if (!accumulateCreditInGuardBand)
                periodicGateModule->subscribe(PeriodicGate::guardBandStateChangedSignal, this);
        }
        cModule *module = getContainingNicModule(this);
        module->subscribe(transmissionStartedSignal, this);
        module->subscribe(transmissionEndedSignal, this);
        module->subscribe(interpacketGapEndedSignal, this);
        changeTimer = new cMessage("ChangeTimer");
        WATCH(isTransmitting);
        WATCH(isInterpacketGap);
        WATCH(currentCredit);
        WATCH(currentCreditGainRate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        updateCurrentState();
        scheduleChangeTimer();
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void CreditBasedGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        // 1. timer is executed when currentCredit reaches transmitCreditLimit with currentCreditGainRate
        setCurrentCredit(transmitCreditLimit);
        // 2. notify listeners and update lastCurrentCreditEmitted
        emitCurrentCredit();
        // 3. update currentCreditGainRate to know the slope when the timer is rescheduled
        updateCurrentCreditGainRate();
        // 4. reschedule change timer when currentCredit reaches transmitCreditLimit
        scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void CreditBasedGate::finish()
{
    updateCurrentState();
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
        simtime_t now = simTime();
        simtime_t changeTime = now + (transmitCreditLimit - currentCredit) / currentCreditGainRate;
        EV_TRACE << "Scheduling change timer to " << changeTime << std::endl;
        if (changeTime <= now)
            cancelEvent(changeTimer);
        else
            rescheduleAt(changeTime, changeTimer);
    }
}

void CreditBasedGate::processPacket(Packet *packet)
{
    PacketGateBase::processPacket(packet);
    packet->addTag<CreditGateTag>()->setId(getId());
}

void CreditBasedGate::updateCurrentState()
{
    updateCurrentCredit();
    updateCurrentCreditGainRate();
    emitCurrentCredit();
}

void CreditBasedGate::setCurrentCredit(double value)
{
    if (currentCredit != value) {
        EV_TRACE << "Setting currentCredit to " << value << std::endl;
        currentCredit = value;
        if (currentCredit >= transmitCreditLimit) {
            if (isClosed())
                open();
        }
        else {
            if (isOpen())
                close();
        }
    }
}

void CreditBasedGate::updateCurrentCredit()
{
    double value = lastCurrentCreditEmitted + currentCreditGainRate * (simTime() - lastCurrentCreditEmittedTime).dbl();
    value = std::max(minCredit, std::min(maxCredit, value));
    setCurrentCredit(value);
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

void CreditBasedGate::setCurrentCreditGainRate(double value)
{
    if (currentCreditGainRate != value) {
        EV_TRACE << "Setting currentCreditGainRate to " << currentCreditGainRate << std::endl;
        currentCreditGainRate = value;
    }
}

void CreditBasedGate::updateCurrentCreditGainRate()
{
    if (isTransmitting || isInterpacketGap)
        setCurrentCreditGainRate(-transmitCreditSpendRate);
    else if (periodicGate != nullptr && (periodicGate->isClosed() || (!accumulateCreditInGuardBand && periodicGate->isInGuardBand())))
        setCurrentCreditGainRate(0);
    else if (currentCredit < 0 || hasAvailablePacket())
        setCurrentCreditGainRate(idleCreditGainRate);
    else
        setCurrentCreditGainRate(0);
}

void CreditBasedGate::receiveSignal(cComponent *source, simsignal_t simsignal, bool value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simsignal));
    if (simsignal == gateStateChangedSignal || simsignal == PeriodicGate::guardBandStateChangedSignal) {
        // 1. update current state because some time may have elapsed since last update
        updateCurrentState();
        // 2. reschedule change timer when currentCredit reaches transmitCreditLimit
        scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown signal");
}

void CreditBasedGate::receiveSignal(cComponent *source, simsignal_t simsignal, double value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simsignal));
    if (simsignal == interpacketGapEndedSignal) {
        // NOTE: this signal also comes for other packets not in our traffic category
        if (isInterpacketGap) {
            // 1. update current state because some time may have elapsed since last update
            updateCurrentState();
            // 2. update isInterpacketGap state
            isInterpacketGap = false;
            // 3. immediately set currentCredit to 0 if there are no packets to transmit
            if (!hasAvailablePacket()) {
                setCurrentCredit(std::min(transmitCreditLimit, currentCredit));
                emitCurrentCredit();
            }
            // 4. update current state because some input has been changed
            updateCurrentState();
            // 5. reschedule change timer when currentCredit reaches transmitCreditLimit
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
            // 1. update current state because some time may have elapsed since last update
            updateCurrentState();
            // 2. update isTransmitting state
            if (simsignal == transmissionStartedSignal)
                isTransmitting = true;
            else if (simsignal == transmissionEndedSignal) {
                isTransmitting = false;
                // NOTE: mark interpacket gap only for our own transmissions
                isInterpacketGap = true;
            }
            else
                throw cRuntimeError("Unknown signal");
            // 3. update current state because some input has been changed
            updateCurrentState();
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
    // 1. update current state because some time may have elapsed since last update
    updateCurrentState();
    // 2. reschedule change timer when currentCredit reaches transmitCreditLimit
    scheduleChangeTimer();
    PacketGateBase::handleCanPullPacketChanged(gate);
}

std::string CreditBasedGate::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n': {
            std::stringstream stream;
            stream << currentCredit;
            return stream.str();
        }
        default:
            return PacketGateBase::resolveDirective(directive);
    }
}

} // namespace queueing
} // namespace inet

