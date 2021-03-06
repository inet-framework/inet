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

#include "inet/queueing/gate/CreditBasedGate.h"

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/common/Signal.h"
#include "inet/queueing/gate/CreditGateTag_m.h"

namespace inet {
namespace queueing {

Define_Module(CreditBasedGate);

simsignal_t CreditBasedGate::currentCreditChangedSignal = cComponent::registerSignal("currentCreditChanged");

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
        currentCredit = lastCurrentCreditEmitted = par("initialCredit");
        currentCreditGainRate = idleCreditGainRate;
        isOpen_ = currentCredit >= transmitCreditLimit;
        cModule *module = getContainingNicModule(this);
        module->subscribe(transmissionStartedSignal, this);
        module->subscribe(transmissionEndedSignal, this);
        changeTimer = new cMessage("ChangeTimer");
        WATCH(currentCredit);
        WATCH(currentCreditGainRate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        updateCurrentCredit();
        emitCurrentCredit();
        scheduleChangeTimer();
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void CreditBasedGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        currentCredit = transmitCreditLimit;
        emitCurrentCredit();
        processChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void CreditBasedGate::finish()
{
    updateCurrentCredit();
}

void CreditBasedGate::refreshDisplay() const
{
    const_cast<CreditBasedGate *>(this)->updateCurrentCredit();
    updateDisplayString();
}

void CreditBasedGate::scheduleChangeTimer()
{
    ASSERT(lastCurrentCreditEmittedTime == simTime());
    double changeTime = (transmitCreditLimit - lastCurrentCreditEmitted) / currentCreditGainRate;
    if (changeTime < 0)
        cancelEvent(changeTimer);
    else if (changeTime > 0 || (isOpen_ ? currentCreditGainRate < 0 : currentCreditGainRate > 0))
        rescheduleAfter(changeTime, changeTimer);
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
    emit(currentCreditChangedSignal, currentCredit);
    lastCurrentCreditEmitted = currentCredit;
    lastCurrentCreditEmittedTime = simTime();
}

void CreditBasedGate::receiveSignal(cComponent *source, simsignal_t simsignal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simsignal));

    if (simsignal == transmissionStartedSignal || simsignal == transmissionEndedSignal) {
        auto signal = check_and_cast<physicallayer::Signal *>(object);
        auto packet = check_and_cast<Packet *>(signal->getEncapsulatedPacket());
        auto creditGateTag = packet->findTag<CreditGateTag>();
        if (creditGateTag != nullptr && creditGateTag->getId() == getId()) {
            updateCurrentCredit();
            emitCurrentCredit();
            if (simsignal == transmissionStartedSignal)
                currentCreditGainRate -= transmitCreditSpendRate;
            else if (simsignal == transmissionEndedSignal)
                currentCreditGainRate += transmitCreditSpendRate;
            else
                throw cRuntimeError("Unknown signal");
            scheduleChangeTimer();
        }
    }
    else
        throw cRuntimeError("Unknown signal");
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

