//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/clock/model/PiClock.h"

#include <algorithm>

#include "inet/clock/oscillator/ConstantDriftOscillator.h"
#include "inet/common/XMLUtils.h"

namespace inet {

Define_Module(PiClock);

simsignal_t PiClock::kpSignal = cComponent::registerSignal("kp");
simsignal_t PiClock::driftSignal = cComponent::registerSignal("drift");

void PiClock::initialize(int stage)
{
    SettableClock::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        offsetThreshold = &par("offsetThreshold");
        kp = par("kp");
        ki = par("ki");

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

clocktime_t PiClock::setClockTime(clocktime_t newClockTime)
{
    Enter_Method("setClockTime");
    clocktime_t oldClockTime = getClockTime();

    if (newClockTime != oldClockTime){
        emit(timeChangedSignal, oldClockTime.asSimTime());
        clocktime_t offsetNow = newClockTime - oldClockTime;

        int64_t offsetNsPrev, offsetNs, localNsPrev, localNs;
        auto offsetUs = 1e-3 * offsetNow.inUnit(SIMTIME_NS);

        auto offsetThresholdUs = offsetThreshold->doubleValueInUnit("us");
        if (phase == 2 && (offsetThresholdUs != 0 && fabs(offsetUs) >= offsetThresholdUs)) {
            EV_INFO << "Offset is too large, resetting phase\n";
            EV_INFO << "Offset: " << offsetUs << " maxOffset: " << offsetThresholdUs << "\n";
            phase = 0;
        }

        switch (phase) {
        case 0:
            // Store the offset and the local time
            offset[0] = newClockTime - oldClockTime;
            local[0] = oldClockTime;
            // Do not update frequency or anything to estimate first frequency in next step
            phase = 1;
            break;
        case 1:
            offset[1] = newClockTime - oldClockTime;
            local[1] = oldClockTime;

            offsetNsPrev = offset[0].inUnit(SIMTIME_NS);
            offsetNs = offset[1].inUnit(SIMTIME_NS);

            localNsPrev = local[0].inUnit(SIMTIME_NS);
            localNs = local[1].inUnit(SIMTIME_NS);

            drift += ppm(1e6 * (offsetNsPrev - offsetNs) / (localNsPrev - localNs));
            EV_INFO << "Drift: " << drift << "\n";

            originSimulationTime = simTime();
            originClockTime = newClockTime;

            this->oscillatorCompensation = drift;
            emit(timeChangedSignal, newClockTime.asSimTime());
            phase = 2;
            break;
        case 2:
            // offsetNanosecond_prev = offset[0].inUnit(SIMTIME_NS);

            // differenceOffsetNanosecond = offsetNanosecond - offsetNanosecond_prev;

            originSimulationTime = simTime();
            originClockTime = oldClockTime;

            // As our timestamps is in nanoseconds, to get ppm we need to multiply by 1e-3
            kpTerm = ppm(kp * offsetUs);
            kiTerm = ppm(ki * offsetUs);
            // kdTerm = ppm (kd * differenceOffsetNanosecond);

            kpTerm = std::max(kpTermMin, std::min(kpTermMax, kpTerm));
            kiTerm = std::max(kiTermMin, std::min(kiTermMax, kiTerm));
            // kdTerm = std::max(kdTermMin, std::min(kdTermMax, kdTerm));

            EV_INFO << "kpTerm: " << kpTerm << " kiTerm: " << kiTerm << " offsetUs: " << offsetUs
                    << " drift: " << drift << "\n";

            this->oscillatorCompensation = kpTerm + kiTerm + drift;
            drift += kiTerm;

            emit(kpSignal, kpTerm.get());
            break;
        }
    }
    emit(driftSignal, drift.get());
    return getClockTime();
}

} // namespace inet
