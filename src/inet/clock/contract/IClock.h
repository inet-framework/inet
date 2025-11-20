//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICLOCK_H
#define __INET_ICLOCK_H

#include "inet/clock/contract/ClockEvent.h"
#include "inet/clock/contract/ClockTime.h"

namespace inet {

/**
 * This class defines the interface for clocks. See the corresponding NED file for details.
 *
 * Terminology:
 * - Simulation time (t): OMNeT++'s global time axis.
 * - Clock time (c): time on this clock's own time axis.
 * - C(t): the (piecewise-monotone, right-continuous) mapping from simulation time to clock time,
 *         as defined by the current clock configuration and any past adjustments.
 *
 * General guarantees (unless the clock is explicitly modified between two simulation instants):
 *
 * 1) Monotonicity vs. simulation time:
 *    C(t) is non-decreasing in t. It may have jumps (e.g., when the clock is set),
 *    but it must not decrease unless an explicit modification is applied.
 *
 * 2) Clock events execute “on their clock”:
 *    When a ClockEvent is delivered, getClockTime() == the event’s arrival clock time.
 *
 * 3) Causality for scheduled events:
 *    The arrival clock time of any scheduled event is >= the current clock time
 *    at scheduling.
 *
 * 4) Consistency of absolute scheduling:
 *    For an event scheduled “at” a clock time c_a, its arrival simulation time t_a
 *    equals the (expected) simulation time at which C(t) reaches c_a according to
 *    the clock’s state at scheduling (subject to later clock adjustments; see below).
 *
 * 5) Dual consistency:
 *    At the arrival simulation time t_a of a scheduled event, the clock time equals
 *    the event’s arrival clock time (C(t_a) == c_a).
 *
 * Scheduling models:
 * - Absolute (“at”): anchoring to a specific clock time value. Later absolute changes
 *   to the clock (e.g., setClockTime, setOrigin) will shift the arrival simulation time.
 * - Relative (“after”): anchoring to a clock-duration from the scheduling instant.
 *   Later absolute changes to the clock do NOT change the arrival simulation time,
 *   but changes that affect the clock’s *rate* (drift) still do.
 *
 * Typical use:
 * - Derive from ClockUserModuleBase or ClockUserModuleMixin and call the inherited
 *   helpers or the methods below on the injected clock field.
 */
class INET_API IClock
{
  public:
    virtual ~IClock() {}

    /**
     * Returns the current clock time C(now).
     *
     * Notes:
     * - In execution order, getClockTime() values observed by different modules
     *   at the same simulation time may differ if the clock is explicitly modified
     *   within that time instant. Thus, the value is not guaranteed to be monotonically
     *   non-decreasing *in event order*, only wrt. simulation time between adjustments.
     */
    virtual clocktime_t getClockTime() const = 0;

    /**
     * Converts a (current or future) simulation time to clock time according to C(t).
     *
     * Precondition:
     * - time must be >= the current simulation time; otherwise an error is raised.
     *
     * Discontinuity handling:
     * - If C(t) has a jump at time, lowerBound selects the side:
     *     lowerBound==true  → value just before/at the jump (the lower value)
     *     lowerBound==false → value just after the jump (the higher value)
     *
     * Stability:
     * - This conversion is monotone in time. The result may differ between calls
     *   with the same argument if the clock was explicitly modified in-between.
     *
     * See also: SIMTIME_AS_CLOCKTIME for trivial type conversion.
     */
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time, bool lowerBound = false) const = 0;

    /**
     * Converts a (current or future) clock time to simulation time using the generalized
     * inverse of C(t) starting from now.
     *
     * Precondition:
     * - time must be >= the current clock time; otherwise an error is raised.
     *
     * Discontinuity handling:
     * - Because C(t) can be flat or have jumps, a given clock time may correspond to a
     *   non-zero-length simulation interval [t_low, t_high).
     * - lowerBound selects which bound to return:
     *     lowerBound==true  → inf { t >= now | C(t) >= time }   (lower/left bound)
     *     lowerBound==false → inf { t >= now | C(t) >  time }   (upper/right bound)
     *
     * Stability:
     * - This conversion is monotone in time. The result may differ between calls
     *   with the same argument if the clock was explicitly modified in-between.
     *
     * See also: CLOCKTIME_AS_SIMTIME for trivial type conversion.
     */
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time, bool lowerBound = true) const = 0;

    /**
     * Returns true if and only if the given ClockEvent is currently scheduled.
     */
    virtual bool isScheduledClockEvent(ClockEvent *event) const = 0;

    /**
     * Schedules an event to the caller (context) module at an absolute clock time.
     *
     * Semantics (“absolute”):
     * - Anchored to a specific clock time value c_a.
     * - Arrival simulation time is the lower bound of C^{-1}(c_a) at scheduling time.
     * - Later *absolute* changes to the clock (e.g., setting/stepping the clock,
     *   changing origin) shift the arrival simulation time accordingly.
     * - Changes to the clock’s *rate* (drift) also affect the arrival time.
     */
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) = 0;

    /**
     * Schedules an event after a given clock-duration has elapsed.
     *
     * Semantics (“relative”):
     * - Anchored to a duration Δc measured on the clock from the scheduling instant.
     * - Equivalent target condition: C(t_arrival) - C(now) == Δc.
     * - Later *absolute* changes to the clock do NOT change the arrival simulation time.
     * - Changes to the clock’s *rate* (drift) still affect the arrival time.
     */
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) = 0;

    /**
     * Cancels a previously scheduled clock event and returns it to the caller.
     * No-op if the event is not scheduled.
     */
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) = 0;

    /**
     * Called by ClockEvent to execute within this clock’s context.
     * Implementations may use this hook to update per-event internal structures
     * (e.g., bookkeeping around absolute/relative anchoring or drift snapshots).
     */
    virtual void handleClockEvent(ClockEvent *event) = 0;
};

} // namespace inet

#endif

