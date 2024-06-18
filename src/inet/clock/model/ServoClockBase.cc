//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/ServoClockBase.h"

namespace inet {

    Define_Module(ServoClockBase);
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

    void ServoClockBase::rescheduleClockEvents(clocktime_t oldClockTime, clocktime_t newClockTime) const
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
                    cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
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


} // namespace inet

