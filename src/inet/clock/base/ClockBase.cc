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

#include "inet/clock/base/ClockBase.h"

namespace inet {

simsignal_t ClockBase::timeChangedSignal = cComponent::registerSignal("timeChanged");

void ClockBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        displayStringTextFormat = par("displayStringTextFormat");
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

void ClockBase::refreshDisplay() const
{
    updateDisplayString();
}

void ClockBase::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(displayStringTextFormat, this);
        getDisplayString().setTagArg("t", 0, text);
    }
}

clocktime_t ClockBase::getClockTime() const
{
    return computeClockTimeFromSimTime(simTime());
}

void ClockBase::scheduleClockEventAt(clocktime_t t, ClockEvent *msg)
{
    ASSERT(msg->getClock() == nullptr);
    if (t < getClockTime())
        throw cRuntimeError("Cannot schedule clock event in the past");
    cSimpleModule *targetModule = getTargetModule();
    msg->setClock(this);
    msg->setRelative(false);
    msg->setArrivalClockTime(t);
    targetModule->scheduleAt(computeSimTimeFromClockTime(t), msg);
}

void ClockBase::scheduleClockEventAfter(clocktime_t clockTimeDelay, ClockEvent *msg)
{
    ASSERT(msg->getClock() == nullptr);
    if (clockTimeDelay < 0)
        throw cRuntimeError("Cannot schedule clock event with negative delay");
    cSimpleModule *targetModule = getTargetModule();
    msg->setClock(this);
    msg->setRelative(true);
    clocktime_t nowClock = getClockTime();
    clocktime_t arrivalClockTime = nowClock + clockTimeDelay;
    msg->setArrivalClockTime(arrivalClockTime);
    simtime_t simTimeDelay = clockTimeDelay.isZero() ? SIMTIME_ZERO : computeSimTimeFromClockTime(arrivalClockTime) - simTime();
    targetModule->scheduleAfter(simTimeDelay, msg);
}

ClockEvent *ClockBase::cancelClockEvent(ClockEvent *msg)
{
    msg->setClock(nullptr);
    return static_cast<ClockEvent *>(getTargetModule()->cancelEvent(msg));
}

void ClockBase::handleClockEventOccurred(ClockEvent *msg)
{
    msg->setClock(nullptr);
}

const char *ClockBase::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 't':
            result = getClockTime().str() + " s";
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

} // namespace inet

