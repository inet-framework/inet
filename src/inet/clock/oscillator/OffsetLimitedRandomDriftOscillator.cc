//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/clock/oscillator/OffsetLimitedRandomDriftOscillator.h"

namespace inet {

Define_Module(OffsetLimitedRandomDriftOscillator);

void OffsetLimitedRandomDriftOscillator::initialize(int stage)
{
    RandomDriftOscillator::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        maxOffset = &par("maxOffset");
        maxDriftRateAdjustment = ppm(par("maxDriftRateAdjustment"));
        // Get my clock which is my parent
        clock = dynamic_cast<OscillatorBasedClock *>(getParentModule());
    }
}

void OffsetLimitedRandomDriftOscillator::handleMessage(cMessage *message)
{
    if (message == changeTimer) {

        driftRateChangeTotal += ppm(driftRateChangeParameter->doubleValue());
        driftRateChangeTotal = std::max(driftRateChangeTotal, driftRateChangeLowerLimit);
        driftRateChangeTotal = std::min(driftRateChangeTotal, driftRateChangeUpperLimit);

        auto currSimTime = simTime();
        auto currSimTimeAsClockTime = SIMTIME_AS_CLOCKTIME(simTime());
        auto currClockTime = clock->computeClockTimeFromSimTime(currSimTime);
        auto currentOffset = currSimTimeAsClockTime - currClockTime;
        if (std::abs(currentOffset.dbl()) > maxOffset->doubleValue()) {
            // Adjust the drift rate to bring the offset back within limits
            if (currentOffset > 0) {
                driftRateChangeTotal += maxDriftRateAdjustment;
            }
            else {
                driftRateChangeTotal -= maxDriftRateAdjustment;
            }
        }

        setDriftRate(initialDriftRate + driftRateChangeTotal);

        scheduleAfter(changeIntervalParameter->doubleValue(), changeTimer);
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet
