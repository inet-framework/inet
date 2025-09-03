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
#include "inet/common/SimTimeScale.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * Mathematical definitions:
 *  - o: computation origin [s], o >= 0
 *  - l: nominal tick length [s], l > 0
 *  - d: physical drift factor; near 1 [-], d > 0
 *  - f: frequency compensation factor; near 1 [-], f > 0
 *  - g = d * f: effective tick length factor; near 1 (drift * compensation) [-], g > 0
 *  - n: number of ticks from computation origin [-]
 *  - t: time interval from computation origin [s]
 *  - x: time of the next tick from computation origin [s]
 *
 * Mathematical semantics (g is piecewise constant between origin updates):
 *  - Ticks occur at times: o + x, o + x + l / g, o + x + 2 * l / g, …
 *  - When d or f changes at simulation time s, set a new computation origin:
 *      o = s, choose x so that the first tick >= s is at time o + x.
 */
class INET_API DriftingOscillatorBase : public OscillatorBase, public IScriptable
{
  public:
    static simsignal_t driftRateChangedSignal;
    static simsignal_t frequencyCompensationRateChangedSignal;

  protected:
    simtime_t nominalTickLength; // nominal tick length in simulation time

    ppm driftRate = ppm(NaN); // 0 means nominal, higher value means faster oscillator, e.g. 100 ppm means the oscillator gains 100 microseconds for every second in simulation time
                              // 100 ppm value means the current tick length is smaller by a factor of (1 / (1 + 100 / 1E+6)) than the nominal tick length measured in simulation time
    SimTimeScale driftFactor; // a number near 1 representing the ratio of the nominal tick length to the current tick length, positive driftRate means driftFactor > 1
    ppm frequencyCompensationRate = ppm(0); // 0 means no frequency compensation
    SimTimeScale frequencyCompensationFactor; // a number near 1 representing the fine tuning of the oscillator frequency to compensate for the drift
    SimTimeScale effectiveTickLengthFactor;

    simtime_t origin; // simulation time from which the computeClockTicksForInterval and computeIntervalForClockTicks is measured, it is always in the past but not always at a tick
    simtime_t nextTickFromOrigin; // simulation time interval from the computation origin to the next tick
    int64_t numTicksAtOrigin; // total number of ticks at the computation origin

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleTickTimer() override;
    virtual void scheduleTickTimer() override;

    virtual void setOrigin(simtime_t origin);
    virtual void setDriftFactor(SimTimeScale driftFactor);
    virtual void setFrequencyCompensationFactor(SimTimeScale frequencyCompensationFactor);
    virtual void setEffectiveTickLengthFactor(SimTimeScale effectiveTickLengthFactor);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

    int64_t doComputeTicksForInterval(simtime_t timeInterval) const;
    simtime_t doComputeIntervalForTicks(int64_t numTicks) const;

  public:
    virtual simtime_t getComputationOrigin() const override { return origin; }
    virtual simtime_t getNominalTickLength() const override { return nominalTickLength; }
    virtual simtime_t getCurrentTickLength() const;

    virtual int64_t getNumTicksAtOrigin() const override { return numTicksAtOrigin; }

    virtual ppm getDriftRate() const { return driftRate; }
    virtual void setDriftRate(ppm driftRate);

    virtual ppm getFrequencyCompensationRate() const { return frequencyCompensationRate; }
    virtual void setFrequencyCompensationRate(ppm frequencyCompensationRate);

    virtual void setTickOffset(simtime_t tickOffset);

    /**
     * See comment for IOscillator interface method.
     *
     * Mathematical definition: n = computeTicksForInterval(t)
     *   n = 0                             if t == 0
     *   n = floor(g * (t − x) / l) + 1    otherwise
     * where g = d * f.
     */
    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const override;

    /**
     * See comment for IOscillator interface method.
     *
     * Mathematical definition: t = computeIntervalForTicks(n)
     *   t = 0                             if n == 0
     *   t = x + (n − 1) * l / g           otherwise
     * where g = d * f.
     */
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif

