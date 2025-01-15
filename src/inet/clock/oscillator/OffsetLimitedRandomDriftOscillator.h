//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OFFSETLIMITEDRANDOMDRIFTOSCILLATOR_H
#define __INET_OFFSETLIMITEDRANDOMDRIFTOSCILLATOR_H

#include "inet/clock/oscillator/RandomDriftOscillator.h"
#include "inet/clock/model/OscillatorBasedClock.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

class INET_API OffsetLimitedRandomDriftOscillator : public RandomDriftOscillator
{
  protected:
    OscillatorBasedClock *clock = nullptr;
    cPar *maxOffset = nullptr;
    ppm maxDriftRateAdjustment = ppm(NaN);

  protected:
    void handleMessage(cMessage *message) override;
    void initialize(int stage) override;
};

} // namespace inet

#endif

