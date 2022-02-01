//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IOSCILLATOR_H
#define __INET_IOSCILLATOR_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class defines the interface for oscillators. See the corresponding NED file for details.
 */
class INET_API IOscillator
{
  public:
    static simsignal_t preOscillatorStateChangedSignal;
    static simsignal_t postOscillatorStateChangedSignal;

  public:
    virtual ~IOscillator() {}

    /**
     * Returns the nominal time interval between subsequent ticks. This value
     * is usually different from the actual amount of elapsed time between ticks.
     */
    virtual simtime_t getNominalTickLength() const = 0;

    /**
     * Returns the oscillator computation origin from which the ticks are measured.
     * There may or may not be an actual tick at the returned simulation time.
     */
    virtual simtime_t getComputationOrigin() const = 0;

    /**
     * Returns the number of ticks in the specified time interval measured from
     * the oscillator computation origin. Note that each tick interval taken into
     * consideration may be different from the nominal tick length. Returns the
     * #ticks in (computation origin, computation origin + time interval].
     */
    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const = 0;

    /**
     * Returns the smallest simulation time interval for the specified number
     * of ticks measured from the oscillator computation origin. For example,
     * for 0 ticks returns 0, for 1 tick returns the remaining time till the
     * next tick, and so on.
     */
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const = 0;
};

} // namespace inet

#endif

