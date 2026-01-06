//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/model/SettableClock.h"

#include "inet/clock/base/DriftingOscillatorBase.h"
#include "inet/common/XMLUtils.h"

namespace inet {

Define_Module(SettableClock);

simsignal_t SettableClock::oscillatorCompensationChangedSignal = cComponent::registerSignal("oscillatorCompensationChanged");

void SettableClock::initialize(int stage)
{
    OscillatorBasedClock::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        oscillatorCompensation = ppm(par("initialOscillatorCompensation"));
        emit(oscillatorCompensationChangedSignal, oscillatorCompensation.get<ppm>());
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

void SettableClock::scheduleClockEventAt(clocktime_t time, ClockEvent *event)
{
    OscillatorBasedClock::scheduleClockEventAt(time, event);
    std::make_heap(events.begin(), events.end(), compareClockEvents);
}

void SettableClock::scheduleClockEventAfter(clocktime_t delay, ClockEvent *event)
{
    OscillatorBasedClock::scheduleClockEventAfter(delay, event);
    std::make_heap(events.begin(), events.end(), compareClockEvents);
}

ClockEvent *SettableClock::cancelClockEvent(ClockEvent *event)
{
    OscillatorBasedClock::cancelClockEvent(event);
    std::make_heap(events.begin(), events.end(), compareClockEvents);
    return event;
}

void SettableClock::handleClockEvent(ClockEvent *event)
{
    std::make_heap(events.begin(), events.end(), compareClockEvents);
    OscillatorBasedClock::handleClockEvent(event);
}

OverdueClockEventHandlingMode SettableClock::getOverdueClockEventHandlingMode(ClockEvent *event) const
{
    auto mode = event->getOverdueClockEventHandlingMode();
    if (mode == UNSPECIFIED)
        return defaultOverdueClockEventHandlingMode;
    else
        return mode;
}

void SettableClock::setClockTime(clocktime_t newClockTime, ppm oscillatorCompensation, bool resetOscillator)
{
    Enter_Method("setClockTime");
    if (oscillatorCompensation < -ppm(500000) || oscillatorCompensation > ppm(1000000))
        throw cRuntimeError("Invalid argument: oscillatorCompensation = %s (must be in the range [-500000, 1000000] ppm)", oscillatorCompensation.str().c_str());
    clocktime_t oldClockTime = getClockTime();
    newClockTime.setRaw(roundingFunction(newClockTime.raw(), oscillator->getNominalTickLength().raw()));
    if (newClockTime != oldClockTime) {
        clocktime_t clockDelta = newClockTime - oldClockTime;
        emit(timeChangedSignal, oldClockTime.asSimTime());
        if (resetOscillator) {
            if (auto driftingOscillator = dynamic_cast<DriftingOscillatorBase *>(oscillator))
                driftingOscillator->setTickOffset(0);
        }
        simtime_t currentSimTime = simTime();
        EV_INFO << "Setting clock time from " << oldClockTime << " to " << newClockTime << " at simtime " << currentSimTime << ".\n";
        for (auto event : events)
            if (event->getRelative())
                event->setArrivalClockTime(event->getArrivalClockTime() + clockDelta);
        std::make_heap(events.begin(), events.end(), compareClockEvents);
        while (!events.empty() && events.front()->getArrivalClockTime() < newClockTime)
        {
            std::pop_heap(events.begin(), events.end(), compareClockEvents);
            auto event = events.back();
            ASSERT(!event->getRelative());
            events.erase(std::remove(events.begin(), events.end(), event), events.end());
            switch (getOverdueClockEventHandlingMode(event)) {
                case EXECUTE: {
                    EV_WARN << "Executing overdue clock event " << event->getName() << ".\n";
                    cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                    cContextSwitcher contextSwitcher(targetModule);
                    this->originSimulationTime = currentSimTime;
                    this->originClockTime = event->getArrivalClockTime();
                    targetModule->cancelEvent(event);
                    event->setClock(nullptr);
                    event->execute();
                    break;
                }
                case SKIP: {
                    EV_WARN << "Skipping overdue clock event " << event->getName() << ".\n";
                    cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                    cContextSwitcher contextSwitcher(targetModule);
                    targetModule->cancelEvent(event);
                    event->setClock(nullptr);
                    break;
                }
                case ERROR:
                    throw cRuntimeError("Clock event is overdue");
                default:
                    throw cRuntimeError("Unknown overdue clock event handling mode");
            }
        }
        this->oscillatorCompensation = oscillatorCompensation;
        emit(oscillatorCompensationChangedSignal, oscillatorCompensation.get<ppm>());
        this->originSimulationTime = currentSimTime;
        this->originClockTime = newClockTime;
        for (auto event : events) {
            cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
            cContextSwitcher contextSwitcher(targetModule);
            targetModule->rescheduleAt(computeScheduleTime(event->getArrivalClockTime()), event);
        }
        emit(timeChangedSignal, newClockTime.asSimTime());
    }
}

void SettableClock::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-clock")) {
        clocktime_t time = ClockTime::parse(xmlutils::getMandatoryFilledAttribute(node, "time"));
        ppm oscillatorCompensation = ppm(xmlutils::getAttributeDoubleValue(&node, "oscillator-compensation", 0));
        bool resetOscillator = xmlutils::getAttributeBoolValue(&node, "reset-oscillator", true);
        setClockTime(time, oscillatorCompensation, resetOscillator);
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

} // namespace inet

