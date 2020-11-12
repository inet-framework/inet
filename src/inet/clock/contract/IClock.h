//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
     * Internally used method to notify the clock before a clock event is executed.
     * This method is primarily useful for clock implementations to update their
     * internal data structures related to individual clock events.
     */
    virtual void handleClockEventOccurred(ClockEvent *event) = 0;
};

} // namespace inet

#endif

