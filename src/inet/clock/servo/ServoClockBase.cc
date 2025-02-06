//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ServoClockBase.h"

namespace inet {

Register_Abstract_Class(ServoClockBase);

void ServoClockBase::initialize(int stage)
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

void ServoClockBase::setOscillatorCompensation(ppm oscillatorCompensationValue)
{
    this->oscillatorCompensation = oscillatorCompensationValue;
}

void ServoClockBase::resetOscillator() const
{
    Enter_Method("resetOscillator");
    if (auto constantDriftOscillator = dynamic_cast<ConstantDriftOscillator *>(oscillator))
        constantDriftOscillator->setTickOffset(0);
}
void ServoClockBase::resetClockState()
{
    clockState = INIT;
    EV_INFO << "Resetting clock state" << endl;
}

void ServoClockBase::rescheduleClockEvents(clocktime_t oldClockTime, clocktime_t newClockTime)
{
    clocktime_t clockDelta = newClockTime - oldClockTime;
    simtime_t currentSimTime = simTime();
    for (auto event : this->events) {
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
                auto *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                cContextSwitcher contextSwitcher(targetModule);
                targetModule->rescheduleAt(arrivalSimTime, event);
            }
        }
    }
}

simtime_t ServoClockBase::handleOverdueClockEvent(ClockEvent *event, simtime_t t)
{
    auto mode = event->getOverdueClockEventHandlingMode();
    if (mode == UNSPECIFIED)
        mode = defaultOverdueClockEventHandlingMode;

    switch (mode) {
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

void ServoClockBase::processCommand(const cXMLElement &node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "adjust-clock")) {
        clocktime_t time = ClockTime::parse(xmlutils::getMandatoryFilledAttribute(node, "time"));
        adjustClockTo(time);
    }
    if (!strcmp(node.getTagName(), "set-clock")) {
        clocktime_t time = ClockTime::parse(xmlutils::getMandatoryFilledAttribute(node, "time"));
        bool notifyListeners = xmlutils::getAttributeBoolValue(&node, "notifyListeners", true);
        jumpClockTo(time, notifyListeners);
    }
    if (!strcmp(node.getTagName(), "set-oscillator-compensation")) {
        // TODO: Refactor to directly read ppm
        const char* valueStr = xmlutils::getMandatoryFilledAttribute(node, "value");
        double valueDouble = std::atof(valueStr); // Convert string to double
        ppm oscillatorCompensationValue = ppm(valueDouble); // Create ppm object from double
        setOscillatorCompensation(oscillatorCompensationValue);
    }
    if (!strcmp(node.getTagName(), "reset-oscillator")) {
        resetOscillator();
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

void ServoClockBase::jumpClockTo(clocktime_t newClockTime, bool notifyListeners) {
    auto oldClockTime = getClockTime();

    if (newClockTime != oldClockTime) {
        emit(timeChangedSignal, oldClockTime.asSimTime());
        simtime_t currentSimTime = simTime();
        EV_DEBUG << "Setting clock time from " << oldClockTime << " to " << newClockTime << " at simtime "
                 << currentSimTime << ".\n";
        setNewOriginTime(simTime(), newClockTime);

        ASSERT(newClockTime == getClockTime());
        rescheduleClockEvents(oldClockTime, newClockTime);
        ClockJumpDetails timeJumpDetails = ClockJumpDetails();
        timeJumpDetails.oldClockTime = oldClockTime;
        timeJumpDetails.newClockTime = newClockTime;
        if (notifyListeners) {
            emit(timeJumpedSignal, this, &timeJumpDetails);
        }
        emit(timeChangedSignal, newClockTime.asSimTime());
    }

}

} // namespace inet
