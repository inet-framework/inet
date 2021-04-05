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

#include "inet/clock/model/SettableClock.h"

#include "inet/clock/oscillator/ConstantDriftOscillator.h"
#include "inet/common/XMLUtils.h"

namespace inet {

Define_Module(SettableClock);

void SettableClock::initialize(int stage)
{
    OscillatorBasedClock::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *text = par("defaultOverdueClockEventHandlingMode");
        if (!strcmp(text, "execute"))
            defaultOverdueClockEventHandlingMode = EXECUTE;
        else if (!strcmp(text, "skip"))
            defaultOverdueClockEventHandlingMode = SKIP;
        else if (!strcmp(text, "error"))
            defaultOverdueClockEventHandlingMode = ERROR;
        else
            throw cRuntimeError("Unknown defaultOverdueClockEventHandlingMode parameter value");
    }
}

OverdueClockEventHandlingMode SettableClock::getOverdueClockEventHandlingMode(ClockEvent *event) const
{
    auto mode = event->getOverdueClockEventHandlingMode();
    if (mode == UNSPECIFIED)
        return defaultOverdueClockEventHandlingMode;
    else
        return mode;
}

simtime_t SettableClock::handleOverdueClockEvent(ClockEvent *event, simtime_t t)
{
    switch (getOverdueClockEventHandlingMode(event)) {
        case EXECUTE:
            EV_WARN << "Scheduling overdue clock event " << event->getName() << " to current simulation time.\n";
            return t;
        case SKIP:
            EV_WARN << "Skipping overdue clock event " << event->getName() << ".\n";
            cancelClockEvent(event);
            return -1;
        case ERROR:
            throw cRuntimeError("Clock event is overdue");
        default:
            throw cRuntimeError("Unknown overdue clock event handling mode");
    }
}

void SettableClock::setClockTime(clocktime_t newClockTime, bool resetOscillator)
{
    Enter_Method("setClockTime");
    clocktime_t oldClockTime = getClockTime();
    if (newClockTime != oldClockTime) {
        emit(timeChangedSignal, oldClockTime.asSimTime());
        if (resetOscillator) {
            auto constantDriftOscillator = check_and_cast<ConstantDriftOscillator *>(oscillator);
            constantDriftOscillator->setTickOffset(0);
        }
        simtime_t currentSimTime = simTime();
        EV_DEBUG << "Setting clock time from " << oldClockTime << " to " << newClockTime << " at simtime " << currentSimTime << ".\n";
        int64_t numTicks = oscillator->computeTicksForInterval(currentSimTime - oscillator->getComputationOrigin());
        originClockTick = newClockTime.raw() / oscillator->getNominalTickLength().raw() - numTicks;
        ASSERT(newClockTime == getClockTime());
        clocktime_t clockDelta = newClockTime - oldClockTime;
        for (auto event : events) {
            if (event->getRelative())
                // NOTE: the simulation time of event execution is not affected
                event->setArrivalClockTime(event->getArrivalClockTime() + clockDelta);
            else {
                clocktime_t arrivalClockTime = event->getArrivalClockTime();
                bool isOverdue = arrivalClockTime < newClockTime;
                simtime_t arrivalSimTime = isOverdue ? -1 : computeSimTimeFromClockTime(arrivalClockTime);
                if (isOverdue || arrivalSimTime < currentSimTime)
                    arrivalSimTime = handleOverdueClockEvent(event, currentSimTime);
                if (event->isScheduled()) {
                    cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                    cContextSwitcher contextSwitcher(targetModule);
                    targetModule->rescheduleAt(arrivalSimTime, event);
                }
            }
        }
        emit(timeChangedSignal, newClockTime.asSimTime());
    }
}

void SettableClock::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-clock")) {
        clocktime_t time = ClockTime::parse(xmlutils::getMandatoryFilledAttribute(node, "time"));
        bool resetOscillator = xmlutils::getAttributeBoolValue(&node, "reset-oscillator", true);
        setClockTime(time, resetOscillator);
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

} // namespace inet

