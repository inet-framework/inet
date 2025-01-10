//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "PiServoClock.h"

#include <algorithm>

#include "inet/clock/oscillator/ConstantDriftOscillator.h"
#include "inet/common/XMLUtils.h"

namespace inet {

Define_Module(PiServoClock);

simsignal_t PiServoClock::kpSignal = cComponent::registerSignal("kp");
simsignal_t PiServoClock::driftSignal = cComponent::registerSignal("drift");

void PiServoClock::initialize(int stage)
{
    ServoClockBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        offsetThreshold = &par("offsetThreshold");
        kp = par("kp");
        ki = par("ki");
    }
}

void PiServoClock::adjustClockTo(clocktime_t newClockTime) {
    Enter_Method("adjustClockTo");
    clocktime_t oldClockTime = getClockTime();

    if (newClockTime != oldClockTime) {
        emit(timeChangedSignal, oldClockTime.asSimTime());
        clocktime_t offsetNow = newClockTime - oldClockTime;

        int64_t offsetNsPrev, offsetNs, localNsPrev, localNs;
        auto offsetUs = 1e-3 * offsetNow.inUnit(SIMTIME_NS);

        auto offsetThresholdUs = offsetThreshold->doubleValueInUnit("us");
        if (phase == 2 && (offsetThresholdUs != 0 && fabs(offsetUs) >= offsetThresholdUs)) {
            EV_INFO << "Offset is too large, resetting phase\n";
            EV_INFO << "Offset: " << offsetUs << " maxOffset: " << offsetThresholdUs << "\n";
            phase = 0;
            clockState = INIT;
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

            jumpClockTo(newClockTime);

            setOscillatorCompensation(drift);

            clockState = SYNCED;
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

            setOscillatorCompensation(kpTerm + kiTerm + drift);

            drift += kiTerm;

            emit(kpSignal, kpTerm.get());
            break;
        }
    }

    emit(driftSignal, drift.get());
}

} // namespace inet
