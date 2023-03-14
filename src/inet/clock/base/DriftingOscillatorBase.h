//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DRIFTINGOSCILLATORBASE_H
#define __INET_DRIFTINGOSCILLATORBASE_H

#include "inet/clock/base/OscillatorBase.h"
#include "inet/common/INETMath.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

class INET_API DriftingOscillatorBase : public OscillatorBase, public IScriptable
{
  protected:
    simtime_t nominalTickLength;
    ppm driftRate = ppm(NaN); // 0 means nominal, higher value means faster oscillator, e.g. 100 ppm means the oscillator gains 100 microseconds for every second in simulation time
                              // 100 ppm value means the current tick length is smaller by a factor of (1 / (1 + 100 / 1E+6)) than the nominal tick length measured in simulation time
    ppm inverseDriftRate = ppm(NaN); // stored for faster computation, calculated as (1 / (1 + driftRate / 1E+6) - 1) * 1E-6

    simtime_t origin; // simulation time from which the computeClockTicksForInterval and computeIntervalForClockTicks is measured
    simtime_t nextTickFromOrigin; // simulation time interval from the computation origin to the next tick

  protected:
    virtual void initialize(int stage) override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

    ppm invertDriftRate(ppm driftRate) const { return unit(1 / (1 + unit(driftRate).get()) - 1); }

    int64_t increaseWithDriftRate(int64_t value) const { return increaseWithDriftRate(value, driftRate); }
    int64_t increaseWithDriftRate(int64_t value, ppm driftRate) const { return value + (int64_t)(value * unit(driftRate).get()); }

    int64_t decreaseWithDriftRate(int64_t value) const { return decreaseWithDriftRate(value, inverseDriftRate); }
    int64_t decreaseWithDriftRate(int64_t value, ppm inverseDriftRate) const { return value + (int64_t)(value * unit(inverseDriftRate).get()); }

  public:
    virtual simtime_t getComputationOrigin() const override { return origin; }
    virtual simtime_t getNominalTickLength() const override { return nominalTickLength; }
    virtual double getCurrentTickLength() const { return nominalTickLength.dbl() + nominalTickLength.dbl() * unit(inverseDriftRate).get(); }

    virtual ppm getDriftRate() const { return driftRate; }
    virtual void setDriftRate(ppm driftRate);
    virtual void setTickOffset(simtime_t tickOffset);

    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif

