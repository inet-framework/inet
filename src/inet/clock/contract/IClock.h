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
 * The typical way to use a clock is to derive your class or module from either
 * ClockUserModuleBase or ClockUserModuleMixin. Then you can use the inherited
 * clock related methods or the methods of this interface on the inherited clock
 * field.
 *
 * The clock interface requires the following properties.
 *
 * 1. When a clock event is executed the clock time is guaranteed to be equal to
 *    the arrival clock time of the event.
 *
 * 2. The scheduling and the execution of a new independent clock event doesn't
 *    change the arrival simulation times (and of course arrival clock times) of
 *    all other events.
 */
class INET_API IClock
{
  public:
    virtual ~IClock() {}

    /**
     * Returns the current clock time. Note that the clock time is not necessarily
     * monotonous in execution order. For example, the clock time may decrease
     * even at the same simulation time.
     */
    virtual clocktime_t getClockTime() const = 0;

    /**
     * Returns the clock time for the specified future simulation time according
     * to the current state of the clock. This method implements a monotonic
     * function with respect to the simulation time argument. It's allowed to
     * return a different value for the same argument value if the clock is set
     * between calls. The time argument must be greater than or equal to the current
     * simulation time, otherwise an error is raised. See SIMTIME_AS_CLOCKTIME
     * macro for simple type conversion.
     */
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time) const = 0;

    /**
     * Returns the simulation time (first moment) for the specified future clock
     * time according to the current state of the clock. This method implements
     * a monotonic function with respect to the clock time argument. It's allowed
     * to return a different value for the same argument value if the clock is
     * set between calls. The time argument must be greater or than equal to the
     * current clock time, otherwise an error is raised. See CLOCKTIME_AS_SIMTIME
     * macro for simple type conversion.
     */
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time) const = 0;

    /**
     * Schedules an event to be delivered to the caller module (i.e. the context
     * module) at the specified clock time. The event is anchored to a specific
     * clock time value, so the actual simulation time when this event is executed
     * will be affected if the clock time is set later.
     */
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) = 0;

    /**
     * Schedules an event to be delivered to the caller module (i.e. the context
     * module) after the given clock time delay has elapsed. The event is anchored
     * to a specific clock time duration, so the actual simulation time when this
     * event is executed is not affected if the clock time is set later. On the
     * other hand, setting the clock drift still affects the simulation time of
     * the event execution.
     */
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) = 0;

    /**
     * Cancels a previously scheduled clock event. The clock event ownership is
     * transferred to the caller.
     */
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) = 0;

    /**
     * Called by the clock event to be executed in the context of this clock.
     * This method is primarily useful for clock implementations to update their
     * internal data structures related to individual clock events.
     */
    virtual void handleClockEvent(ClockEvent *event) = 0;
};

} // namespace inet

#endif

