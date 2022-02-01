//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RANDOMDRIFTOSCILLATOR_H
#define __INET_RANDOMDRIFTOSCILLATOR_H

#include "inet/clock/base/DriftingOscillatorBase.h"

namespace inet {

class INET_API RandomDriftOscillator : public DriftingOscillatorBase
{
  protected:
    cMessage *changeTimer = nullptr;
    cPar *driftRateParameter = nullptr;
    cPar *driftRateChangeParameter = nullptr;
    cPar *changeIntervalParameter = nullptr;
    double driftRateChangeTotal = 0;
    double driftRateChangeLowerLimit = NaN;
    double driftRateChangeUpperLimit = NaN;

  protected:
    virtual ~RandomDriftOscillator() { cancelAndDelete(changeTimer); }

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
};

} // namespace inet

#endif

