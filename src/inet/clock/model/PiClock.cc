//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <algorithm>
#include "inet/clock/model/PiClock.h"

#include "inet/clock/oscillator/ConstantDriftOscillator.h"
#include "inet/common/XMLUtils.h"

namespace inet {

Define_Module(PiClock);

void PiClock::initialize(int stage)
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

        double pi_proportional_const = 0.7;
        double pi_integral_const = 0.3;
    }
}

OverdueClockEventHandlingMode PiClock::getOverdueClockEventHandlingMode(ClockEvent *event) const
{
    auto mode = event->getOverdueClockEventHandlingMode();
    if (mode == UNSPECIFIED)
        return defaultOverdueClockEventHandlingMode;
    else
        return mode;
}

simtime_t PiClock::handleOverdueClockEvent(ClockEvent *event, simtime_t t)
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

void PiClock::setClockTime(clocktime_t newClockTime)
{
    Enter_Method("setClockTime");
    clocktime_t oldClockTime = getClockTime();

    if (newClockTime != oldClockTime) {
        emit(timeChangedSignal, oldClockTime.asSimTime());

        originSimulationTime = simTime();
//        originClockTime = oldClockTime;
        originClockTime = oldClockTime;

        offset_prev = offset;
        offset = newClockTime - oldClockTime;

        offsetNanosecond_prev = offset_prev.inUnit(SIMTIME_NS);
        offsetNanosecond = offset.inUnit(SIMTIME_NS);
        accumulatedOffsetNanosecond += offsetNanosecond;
        differenceOffsetNanosecond = offsetNanosecond - offsetNanosecond_prev;

        kpTerm = ppm (kp * offsetNanosecond);
        kiTerm = ppm (ki * accumulatedOffsetNanosecond);
        kdTerm = ppm (kd * differenceOffsetNanosecond);

        kpTerm = std::max(kpTermMin, std::min(kpTermMax, kpTerm));
        kiTerm = std::max(kiTermMin, std::min(kiTermMax, kiTerm));
        kdTerm = std::max(kdTermMin, std::min(kdTermMax, kdTerm));

        this->oscillatorCompensation = kpTerm + kiTerm + kdTerm;

    }
}

void PiClock::processCommand(const cXMLElement &node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-clock")) {
        clocktime_t time = ClockTime::parse(xmlutils::getMandatoryFilledAttribute(node, "time"));
        setClockTime(time);
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

} // namespace inet
