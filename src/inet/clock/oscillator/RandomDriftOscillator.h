//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RANDOMDRIFTOSCILLATOR_H
#define __INET_RANDOMDRIFTOSCILLATOR_H

#include "inet/clock/base/DriftingOscillatorBase.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

class INET_API RandomDriftOscillator : public DriftingOscillatorBase
{
  protected:
    cMessage *changeTimer = nullptr;
    ppm initialDriftRate = ppm(NaN);
    cPar *driftRateChangeParameter = nullptr;
    cPar *changeIntervalParameter = nullptr;
    ppm driftRateChangeTotal = ppm(0);
    ppm driftRateChangeLowerLimit = ppm(NaN);
    ppm driftRateChangeUpperLimit = ppm(NaN);

  protected:
    virtual ~RandomDriftOscillator() { cancelAndDelete(changeTimer); }

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
};

} // namespace inet

#endif

