//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEALOSCILLATOR_H
#define __INET_IDEALOSCILLATOR_H

#include "inet/clock/base/OscillatorBase.h"
#include "inet/common/INETMath.h"

namespace inet {

class INET_API IdealOscillator : public OscillatorBase
{
  protected:
    simtime_t origin;
    simtime_t tickLength;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    virtual simtime_t getComputationOrigin() const override { return origin; }
    virtual simtime_t getNominalTickLength() const override { return tickLength; }

    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;
};

} // namespace inet

#endif

