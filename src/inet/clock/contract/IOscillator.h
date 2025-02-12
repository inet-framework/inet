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
 * TODO: oscillator computation origin may not have a tick
 *
 * @brief Interface for oscillators.
 *
 * An oscillator produces periodic "ticks". It defines a nominal tick interval that
 * serves as a reference duration between ticks, although the actual intervals may vary.
 * Ticks are measured relative to a defined computation origin, which is the simulation
 * time at which the oscillator starts counting ticks. Although a tick can be scheduled
 * at the computation origin, the mapping functions below count only ticks that occur
 * after the computation origin.
 *
 * To improve simulation performance, the oscillator does not generate individual events
 * for every tick. Instead, it provides two mapping functions:
 * - One that computes the number of ticks that occur within a given time interval measured from the origin.
 * - One that computes the minimal time interval required to include a specified number of ticks measured from the origin.
 *
 * When the underlying state affecting tick timing changes, the oscillator emits signals
 * to notify interested listeners:
 * - preOscillatorStateChangedSignal is emitted immediately before the change.
 * - postOscillatorStateChangedSignal is emitted immediately after the change.
 *
 * @note For detailed configuration, see the corresponding NED file.
 */
class INET_API IOscillator
{
  public:
    /// Signal emitted before a change in tick computation occurs.
    static simsignal_t preOscillatorStateChangedSignal;
    /// Signal emitted after a change in tick computation occurs.
    static simsignal_t postOscillatorStateChangedSignal;

  public:
    virtual ~IOscillator() {}

    /**
     * @brief Returns the oscillator's nominal tick interval.
     *
     * This is the fixed, reference duration between ticks. Even if the actual tick intervals
     * vary, this value remains constant over time.
     *
     * @return Nominal tick interval.
     */
    virtual simtime_t getNominalTickLength() const = 0;

    /**
     * @brief Returns the computation origin.
     *
     * The computation origin is the simulation time (less than or equal to the current simulation time)
     * from which tick counts are measured. Although a tick can be scheduled at the computation origin,
     * it is not counted by the mapping functions.
     *
     * @return Simulation time corresponding to the computation origin.
     */
    virtual simtime_t getComputationOrigin() const = 0;

    /**
     * @brief Computes the number of ticks within a specified time interval.
     *
     * This method returns the number of ticks that occur in the time interval (0, timeInterval],
     * measured relative to the computation origin. That is, the tick at the computation origin
     * is not counted, while a tick occurring exactly at time = computationOrigin + timeInterval is included.
     *
     * For example, if the first tick occurs T seconds after the computation origin:
     * - A time interval in the range [0, T) contains 0 ticks.
     * - A time interval equal to T contains 1 tick.
     *
     * @param timeInterval Duration (relative to the computation origin) over which ticks are counted.
     *                     The value must be non-negative.
     * @return Number of ticks occurring within the specified interval.
     */
    virtual int64_t computeTicksForInterval(simtime_t timeInterval) const = 0;

    /**
     * @brief Computes the minimal time interval that contains a specified number of ticks.
     *
     * This method determines the smallest simulation time interval (starting at the computation origin)
     * that includes the given number of ticks. As before, the tick at the computation origin is not counted,
     * but a tick occurring exactly at the end of the interval is considered part of the count.
     *
     * For example, if the first tick occurs T seconds after the computation origin:
     * - 0 ticks correspond to a time interval of 0 seconds.
     * - 1 tick corresponds to an interval of duration T.
     *
     * @param numTicks The desired number of ticks (must be non-negative).
     * @return The minimal time interval required to cover the given number of ticks.
     */
    virtual simtime_t computeIntervalForTicks(int64_t numTicks) const = 0;
};

} // namespace inet

#endif

