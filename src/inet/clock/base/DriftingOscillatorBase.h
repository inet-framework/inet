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
    bool computeAsSeparateTicks;
    simtime_t nominalTickLength;
    ppm driftRate = ppm(NaN); // 0 means nominal, higher value means faster oscillator, e.g. 100 ppm means the oscillator gains 100 microseconds for every second in simulation time
                              // 100 ppm value means the current tick length is smaller by a factor of (1 / (1 + 100 / 1E+6)) than the nominal tick length measured in simulation time
    long double driftFactor;

    simtime_t origin; // simulation time from which the computeClockTicksForInterval and computeIntervalForClockTicks is measured, it is always in the past
    simtime_t nextTickFromOrigin; // simulation time interval from the computation origin to the next tick

  protected:
    virtual void initialize(int stage) override;
    virtual void handleTickTimer() override;
    virtual void scheduleTickTimer() override;

    virtual void setOrigin(simtime_t origin);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

    int64_t doComputeTicksForInterval(simtime_t timeInterval) const;
    simtime_t doComputeIntervalForTicks(int64_t numTicks) const;

  public:
    virtual simtime_t getComputationOrigin() const override { return origin; }
    virtual simtime_t getNominalTickLength() const override { return nominalTickLength; }
    virtual simtime_t getCurrentTickLength() const { return SimTime::fromRaw(roundl(nominalTickLength.raw() / driftFactor)); }

    virtual ppm getDriftRate() const { return driftRate; }
    virtual void setDriftRate(ppm driftRate);
    virtual void setTickOffset(simtime_t tickOffset);

    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif

