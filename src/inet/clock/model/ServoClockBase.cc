//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/clock/model/ServoClockBase.h"
#include "inet/clock/oscillator/ConstantDriftOscillator.h"

namespace inet {

Define_Module(ServoClockBase);

void ServoClockBase::setOscillatorCompensation(ppm oscillatorCompensation) {
    this->oscillatorCompensation = oscillatorCompensation;
}

void ServoClockBase::resetOscillatorOffset() const{
    if (auto constantDriftOscillator = dynamic_cast<ConstantDriftOscillator *>(oscillator))
        constantDriftOscillator->setTickOffset(0);
}

void ServoClockBase::rescheduleClockEvents(inet::clocktime_t oldClockTime, inet::clocktime_t newClockTime) {
    clocktime_t clockDelta = newClockTime - oldClockTime;
    simtime_t currentSimTime = simTime();
    for (auto event : events) {
        if (event->getRelative())
            // NOTE: the simulation time of event execution is not affected
            event->setArrivalClockTime(event->getArrivalClockTime() + clockDelta);
        else {
            clocktime_t arrivalClockTime = event->getArrivalClockTime();
            bool isOverdue = arrivalClockTime < newClockTime;
            simtime_t arrivalSimTime = isOverdue ? -1 : computeSimTimeFromClockTime(arrivalClockTime);
        }
    }
}

}

